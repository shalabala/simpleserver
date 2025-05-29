#ifndef SMAP_H
#define SMAP_H
#include <string.h>

typedef struct _snode {
  size_t keylen;       // Length of the key
  size_t vallen;       // Length of the value
  struct _snode *next; // Pointer to the next node in the linked list
  char *key;           // Key for the map entry
  char *value;         // Value associated with the key
} snode;

typedef struct _smap {
  size_t size; // Number of elements in the map
  size_t bucketsnum;
  size_t mask;
  snode *buckets;
} smap;

/**
 * Creates a new smap with the specified number of buckets.
 *
 * @param num_buckets The number of buckets for the map.
 * @return A pointer to the initialized smap, or NULL on failure.
 */
smap *smap_create(size_t num_buckets);

/**
 * Initializes the smap with the specified number of buckets.
 *
 * @param map Pointer to the smap to initialize.
 * @param num_buckets The number of buckets for the map.
 * @return 0 on success, or -1 on failure (e.g., memory allocation error).
 */
int smap_init(smap *map, size_t num_buckets);

/**
 * Gets the node associated with the specified key in the smap.
 * @param map Pointer to the smap.
 * @param key Pointer to the key to search for.
 * @param keylen Length of the key.
 * @return Pointer to the node if found, or NULL if not found.
 */
snode *smap_get(smap *map, const char *key, size_t keylen);

/**
 * Inserts a key-value pair into the smap.
 *
 * @param map Pointer to the smap.
 * @param key Pointer to the key to insert.
 * @param keylen Length of the key.
 * @param value Pointer to the value to insert.
 * @param vallen Length of the value.
 * @return 0 on success, or -1 on failure (e.g., memory allocation error).
 */
int smap_upsert(smap *map,
                const char *key,
                size_t keylen,
                const char *value,
                size_t vallen);
/**
 * Frees the memory that was allocated for the fields of this smap. Does not free the smap itself.
 * @param map Pointer to the smap to free.
 * @return void
 */
void smap_free(smap* map);

#endif // SMAP_H