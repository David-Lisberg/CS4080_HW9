#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.75

void initTable(Table* table) {
  table->count = 0;
  table->capacity = 0;
  table->entries = NULL;
}

void freeTable(Table* table) {
  FREE_ARRAY(Entry, table->entries, table->capacity);
  initTable(table);
}

static bool isEmptyKey(ValueKey key) {
  return IS_NONE(key.value);
}

static bool keysEqual(ValueKey a, ValueKey b) {
  return a.hash == b.hash && valuesEqual(a.value, b.value);
}

static Entry* findEntry(Entry* entries, int capacity, ValueKey key) {
  uint32_t index = key.hash % capacity;
  Entry* tombstone = NULL;

  for (;;) {
    Entry* entry = &entries[index];

    if (isEmptyKey(entry->key)) {
      if (IS_NIL(entry->value)) {
        // Empty entry: return the first tombstone if one exists.
        return tombstone != NULL ? tombstone : entry;
      }

      // Tombstone: remember it, but continue searching.
      if (tombstone == NULL) {
        tombstone = entry;
      }
    } else if (keysEqual(entry->key, key)) {
      // Existing key.
      return entry;
    }

    index = (index + 1) % capacity;
  }
}

static void adjustCapacity(Table* table, int capacity) {
  Entry* entries = ALLOCATE(Entry, capacity);

  for (int i = 0; i < capacity; i++) {
    entries[i].key.value = NONE_VAL;
    entries[i].key.hash = 0;
    entries[i].value = NIL_VAL;
  }

  int oldCount = table->count;
  table->count = 0;

  for (int i = 0; i < table->capacity; i++) {
    Entry* entry = &table->entries[i];

    // Skip empty entries and tombstones.
    if (isEmptyKey(entry->key)) {
      continue;
    }

    Entry* dest = findEntry(entries, capacity, entry->key);
    dest->key = entry->key;
    dest->value = entry->value;
    table->count++;
  }

  FREE_ARRAY(Entry, table->entries, table->capacity);
  table->entries = entries;
  table->capacity = capacity;

  (void)oldCount;
}

bool tableGet(Table* table, ValueKey key, Value* value) {
  if (table->count == 0) {
    return false;
  }

  Entry* entry = findEntry(table->entries, table->capacity, key);

  if (isEmptyKey(entry->key)) {
    return false;
  }

  *value = entry->value;
  return true;
}

bool tableSet(Table* table, ValueKey key, Value value) {
  if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
    int capacity = GROW_CAPACITY(table->capacity);
    adjustCapacity(table, capacity);
  }

  Entry* entry = findEntry(table->entries, table->capacity, key);

  bool isNewKey = isEmptyKey(entry->key);

  // Only increase count if this is a genuinely unused entry,
  // rather than a tombstone.
  if (isNewKey && IS_NIL(entry->value)) {
    table->count++;
  }

  entry->key = key;
  entry->value = value;

  return isNewKey;
}

bool tableDelete(Table* table, ValueKey key) {
  if (table->count == 0) {
    return false;
  }

  Entry* entry = findEntry(table->entries, table->capacity, key);

  if (isEmptyKey(entry->key)) {
    return false;
  }

  // Preserve the probe chain with a tombstone.
  entry->key.value = NONE_VAL;
  entry->key.hash = 0;
  entry->value = BOOL_VAL(true);

  return true;
}

void tableAddAll(Table* from, Table* to) {
  for (int i = 0; i < from->capacity; i++) {
    Entry* entry = &from->entries[i];

    if (!isEmptyKey(entry->key)) {
      tableSet(to, entry->key, entry->value);
    }
  }
}

Value* tableFindValue(
    Table* table,
    const char* chars,
    int length,
    uint32_t hash
) {
  if (table->count == 0) {
    return NULL;
  }

  uint32_t index = hash % table->capacity;

  for (;;) {
    Entry* entry = &table->entries[index];

    if (isEmptyKey(entry->key)) {
      // Stop at a truly empty entry, but skip tombstones.
      if (IS_NIL(entry->value)) {
        return NULL;
      }
    } else if (IS_STRING(entry->key.value)) {
      ObjString* string = AS_STRING(entry->key.value);

      if (string->length == length &&
          entry->key.hash == hash &&
          memcmp(string->chars, chars, length) == 0) {
        // Return the address of the stored key value.
        return &entry->key.value;
      }
    }

    index = (index + 1) % table->capacity;
  }
}