#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

#define KEY_COUNT 100000
#define LOOKUP_COUNT 1000000

static double now(void) {
  return (double)clock() / CLOCKS_PER_SEC;
}

static ObjString* makeKey(const char* text) {
  return copyString(text, (int)strlen(text));
}

static void insertKey(Table* table, ObjString* key) {
  tableSet(table, key, BOOL_VAL(true));
}

static void benchmarkInsertion(void) {
  Table table;
  initTable(&table);

  double start = now();

  for (int i = 0; i < KEY_COUNT; i++) {
    char buffer[32];
    insertKey(&table, makeKey(buffer));
  }

  double elapsed = now() - start;

  printf("Insertion: %.4f s, %.0f ops/s\n", elapsed, KEY_COUNT / elapsed);

  freeTable(&table);
}

static void benchmarkLookup(void) {
  Table table;
  initTable(&table);

  ObjString** keys = malloc(sizeof(ObjString*) * KEY_COUNT);

  for (int i = 0; i < KEY_COUNT; i++) {
    char buffer[32];
    keys[i] = makeKey(buffer);
    insertKey(&table, keys[i]);
  }

  volatile int hits = 0;
  double start = now();

  for (int i = 0; i < LOOKUP_COUNT; i++) {
    Value value;
    if (tableGet(&table, keys[i % KEY_COUNT], &value)) {
      hits++;
    }
  }

  double elapsed = now() - start;

  printf("Lookup: %.4f s, %.0f ops/s, hits=%d\n", elapsed, LOOKUP_COUNT / elapsed, hits);

  free(keys);
  freeTable(&table);
}

static void benchmarkDeleteInsert(void) {
  Table table;
  initTable(&table);

  ObjString** keys = malloc(sizeof(ObjString*) * KEY_COUNT);

  for (int i = 0; i < KEY_COUNT; i++) {
    char buffer[32];
    keys[i] = makeKey(buffer);
    insertKey(&table, keys[i]);
  }

  double start = now();

  for (int i = 0; i < LOOKUP_COUNT; i++) {
    int index = i % KEY_COUNT;

    tableDelete(&table, keys[index]);
    insertKey(&table, keys[index]);
  }

  double elapsed = now() - start;

  printf("Delete/insert: %.4f s, %.0f operations/s\n", elapsed, (2.0 * LOOKUP_COUNT) / elapsed);

  free(keys);
  freeTable(&table);
}

int main(void) {
  initVM();

  printf("Keys: %d, Lookups: %d\n\n", KEY_COUNT, LOOKUP_COUNT);

  benchmarkInsertion();
  benchmarkLookup();
  benchmarkDeleteInsert();

  freeVM();
  return 0;
}