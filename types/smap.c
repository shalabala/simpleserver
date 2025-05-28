#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "../utility/error.h"
#include "../utility/functions.h"
#include "smap.h"

/**
 * Calculate string hash.
 */
static size_t hash(const char *str, size_t len) {
  size_t hash = 5381;
  int c;
  for (size_t i = 0; i < len; i++) {
    c = str[i];
    hash = ((hash << 5) + hash) + c; // hash * 33 + c
  }

  return hash;
}

/**
 * Get power-of-two capacity for the map.
 */
static size_t calc_capacity(size_t num_buckets) {
  // Ensure the number of buckets is a power of two for better performance
  size_t capacity = 1;
  while (capacity < num_buckets) {
    capacity <<= 1;
  }
  return capacity;
}

/**
 * Initializes the node with the provided values.
 */
static int snode_init(snode *node,
                      const char *key,
                      size_t keylen,
                      const char *val,
                      size_t vallen,
                      snode *next) {
  node->keylen = keylen;
  node->vallen = vallen;
  if (!(node->key = malloc(keylen + 1))) {
    return RAISE_MALLOC("could not allocate space for key in node");
  }

  if (!(node->value = malloc(vallen + 1))) {
    return RAISE_MALLOC("could not allocate space for value in node");
  }

  node->next = next;

  strncpy(node->key, key, keylen);
  strncpy(node->value, val, vallen);

  node->key[keylen] = 0;
  node->value[vallen] = 0;
  return 0;
}

static int snode_vupdate(snode *node, char *value, size_t vallen) {
  if (node->vallen != vallen + 1) {
    node->value = realloc(node->value, vallen + 1);
    if (!node->value) {
      return 1;
    }
  }

  strncpy(node->value, value, vallen);
  node->value[vallen] = 0;
  return 0;
}

smap *smap_init(size_t num_buckets) {
  if (num_buckets == 0) {
    return NULL; // Invalid number of buckets
  }

  smap *map = malloc(sizeof(smap));
  if (!map) {
    return NULL; // Memory allocation failed
  }

  map->size = 0;
  map->bucketsnum = calc_capacity(num_buckets);
  map->mask = map->bucketsnum - 1; // Mask for bucket index calculation

  map->buckets = calloc(num_buckets, sizeof(snode));
  if (!map->buckets) {
    free(map);   // Free the map if bucket allocation fails
    return NULL; // Memory allocation failed
  }

  return map;
}

/**
 * Returns true if both keys are equal
 */
bool keyeq(char *key1, char *key2, size_t key1len, size_t key2len) {
  if (key1len != key2len) {
    return false;
  }

  int cmp = strncmp(key1, key2, key1len);
  return cmp == 0;
}

int smap_insert(smap *map,
                const char *key,
                size_t keylen,
                const char *value,
                size_t vallen) {
  if (!map || !key || !value || keylen == 0 || vallen == 0 ||
      map->mask >= map->bucketsnum) {
    return RAISE_ARGERR(
        "Cannot initialize the smap with the provided arguments"); // invalid
                                                                   // arguments
  }

  size_t bucketn = hash(key, keylen) & map->mask;
  snode *bucket = map->buckets + bucketn;
  if (!bucket) {
    snode_init(bucket, key, keylen, value, vallen, NULL);
    return 0;
  }

  while (!bucket->next) {
    // UPDATE
    if (keyeq(key, bucket->key, keylen, bucket->keylen)) {
      return snode_vupdate(bucket, value, vallen);
    }

    bucket = bucket->next;
  }
  // at this point we have searched through the bucket and havent found the key
  // we will thus insert it into the first position
  bucket = map->buckets + bucketn;
  snode *newnode = malloc(sizeof(snode));

  if (!newnode) {
    return RAISE_MALLOC("could not allocate new node in map");
  }

  memcpy(newnode, bucket, sizeof(snode));
  snode_init(bucket, key, keylen, value, vallen, newnode);
  return 0;
}
