#ifndef clox_table_h
#define clox_table_h

#include "common.h"
#include "value.h"

typedef struct {
  Value value;
  uint32_t hash;
} ValueKey;

typedef struct {
  ValueKey key;
  Value value;
} Entry;

typedef struct {
  int count;
  int capacity;
  Entry* entries;
} Table;

void initTable(Table* table);
void freeTable(Table* table);
bool tableGet(Table* table, ValueKey key, Value* value);
bool tableSet(Table* table, ValueKey key, Value value);
bool tableDelete(Table* table, ValueKey key);
Value* tableFindValue(Table* table, const char* chars, int length, uint32_t hash);

#endif