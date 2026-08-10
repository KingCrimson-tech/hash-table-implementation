#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash_table.h"
#include "prime.h"

/* Tombstones keep probe chains intact after an item is deleted. */
static ht_item HT_DELETED_ITEM = {NULL, NULL};

static void ht_resize_up(ht_hash_table *ht);
static void ht_resize_down(ht_hash_table *ht);

static void *ht_alloc(size_t size) {
  void *pointer = malloc(size);
  if (pointer == NULL) {
    fputs("hash table: out of memory\n", stderr);
    exit(EXIT_FAILURE);
  }
  return pointer;
}

static char *ht_strdup(const char *text) {
  size_t length = strlen(text) + 1;
  char *copy = ht_alloc(length);
  memcpy(copy, text, length);
  return copy;
}

static ht_item *ht_new_item(const char *key, const char *value) {
  ht_item *item = ht_alloc(sizeof(*item));
  item->key = ht_strdup(key);
  item->value = ht_strdup(value);
  return item;
}

static void ht_del_item(ht_item *item) {
  free(item->key);
  free(item->value);
  free(item);
}

static ht_hash_table *ht_new_sized(int base_size) {
  ht_hash_table *ht = ht_alloc(sizeof(*ht));
  ht->base_size = base_size;
  ht->size = next_prime(base_size);
  ht->count = 0;
  ht->items = calloc((size_t)ht->size, sizeof(*ht->items));
  if (ht->items == NULL) {
    free(ht);
    fputs("hash table: out of memory\n", stderr);
    exit(EXIT_FAILURE);
  }
  return ht;
}

ht_hash_table *ht_new(void) { return ht_new_sized(HT_INITIAL_BASE_SIZE); }

void ht_del_hash_table(ht_hash_table *ht) {
  if (ht == NULL) {
    return;
  }
  for (int i = 0; i < ht->size; i++) {
    ht_item *item = ht->items[i];
    if (item != NULL && item != &HT_DELETED_ITEM) {
      ht_del_item(item);
    }
  }
  free(ht->items);
  free(ht);
}

/* Evaluate a polynomial hash modulo m without floating-point arithmetic. */
static int ht_hash(const char *text, int base, int m) {
  unsigned long hash = 0;
  for (const unsigned char *c = (const unsigned char *)text; *c != '\0'; c++) {
    hash = (hash * (unsigned long)base + *c) % (unsigned long)m;
  }
  return (int)hash;
}

static int ht_get_hash(const char *key, int buckets, int attempt) {
  int hash_a = ht_hash(key, HT_PRIME_1, buckets);
  int hash_b = ht_hash(key, HT_PRIME_2, buckets);
  return (hash_a + attempt * (hash_b + 1)) % buckets;
}

/* Insert a pre-built item; used only while rebuilding the table. */
static void ht_place_item(ht_hash_table *ht, ht_item *item) {
  for (int attempt = 0; attempt < ht->size; attempt++) {
    int index = ht_get_hash(item->key, ht->size, attempt);
    if (ht->items[index] == NULL) {
      ht->items[index] = item;
      ht->count++;
      return;
    }
  }
  fputs("hash table: no free bucket\n", stderr);
  exit(EXIT_FAILURE);
}

void ht_insert(ht_hash_table *ht, const char *key, const char *value) {
  if (ht == NULL || key == NULL || value == NULL) {
    return;
  }
  if (ht->count * 100 / ht->size >= 70) {
    ht_resize_up(ht);
  }

  int first_deleted = -1;
  for (int attempt = 0; attempt < ht->size; attempt++) {
    int index = ht_get_hash(key, ht->size, attempt);
    ht_item *item = ht->items[index];
    if (item == NULL) {
      if (first_deleted >= 0) {
        index = first_deleted;
      }
      ht->items[index] = ht_new_item(key, value);
      ht->count++;
      return;
    }
    if (item == &HT_DELETED_ITEM) {
      if (first_deleted < 0) {
        first_deleted = index;
      }
    } else if (strcmp(item->key, key) == 0) {
      char *new_value = ht_strdup(value);
      free(item->value);
      item->value = new_value;
      return;
    }
  }
  /* A table of tombstones can be reused even when no NULL bucket remains. */
  if (first_deleted >= 0) {
    ht->items[first_deleted] = ht_new_item(key, value);
    ht->count++;
  }
}

char *ht_search(const ht_hash_table *ht, const char *key) {
  if (ht == NULL || key == NULL) {
    return NULL;
  }
  for (int attempt = 0; attempt < ht->size; attempt++) {
    int index = ht_get_hash(key, ht->size, attempt);
    ht_item *item = ht->items[index];
    if (item == NULL) {
      return NULL;
    }
    if (item != &HT_DELETED_ITEM && strcmp(item->key, key) == 0) {
      return item->value;
    }
  }
  return NULL;
}

void ht_delete(ht_hash_table *ht, const char *key) {
  if (ht == NULL || key == NULL) {
    return;
  }
  for (int attempt = 0; attempt < ht->size; attempt++) {
    int index = ht_get_hash(key, ht->size, attempt);
    ht_item *item = ht->items[index];
    if (item == NULL) {
      return;
    }
    if (item != &HT_DELETED_ITEM && strcmp(item->key, key) == 0) {
      ht_del_item(item);
      ht->items[index] = &HT_DELETED_ITEM;
      ht->count--;
      if (ht->count * 100 / ht->size < 10) {
        ht_resize_down(ht);
      }
      return;
    }
  }
}

static void ht_resize(ht_hash_table *ht, int base_size) {
  if (base_size < HT_INITIAL_BASE_SIZE) {
    return;
  }
  ht_hash_table *new_ht = ht_new_sized(base_size);
  for (int i = 0; i < ht->size; i++) {
    ht_item *item = ht->items[i];
    if (item != NULL && item != &HT_DELETED_ITEM) {
      ht_place_item(new_ht, item);
    }
  }
  free(ht->items); /* Items now belong to new_ht; only the bucket array changes.
                    */
  ht->base_size = new_ht->base_size;
  ht->size = new_ht->size;
  ht->count = new_ht->count;
  ht->items = new_ht->items;
  free(new_ht);
}

static void ht_resize_up(ht_hash_table *ht) {
  ht_resize(ht, ht->base_size * 2);
}

static void ht_resize_down(ht_hash_table *ht) {
  ht_resize(ht, ht->base_size / 2);
}

int ht_count(const ht_hash_table *ht) { return ht == NULL ? 0 : ht->count; }

int ht_capacity(const ht_hash_table *ht) { return ht == NULL ? 0 : ht->size; }
