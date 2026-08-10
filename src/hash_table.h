#ifndef HASH_TABLE_H
#define HASH_TABLE_H

/* Initial bucket count and the two bases used for double hashing. */
#define HT_INITIAL_BASE_SIZE 53
#define HT_PRIME_1 151
#define HT_PRIME_2 163

typedef struct {
    char *key;
    char *value;
} ht_item;

typedef struct {
    int base_size;
    int size;
    int count;
    ht_item **items;
} ht_hash_table;

ht_hash_table *ht_new(void);
void ht_del_hash_table(ht_hash_table *ht);
void ht_insert(ht_hash_table *ht, const char *key, const char *value);
char *ht_search(const ht_hash_table *ht, const char *key);
void ht_delete(ht_hash_table *ht, const char *key);
int ht_count(const ht_hash_table *ht);
int ht_capacity(const ht_hash_table *ht);

#endif
