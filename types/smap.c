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

static int snode_vupdate(snode *node, const char *value, size_t vallen) {
  if (node->vallen != vallen + 1) {
    node->value = realloc(node->value, vallen + 1);
    if (!node->value) {
      return 1;
    }
  }

  strncpy(node->value, value, vallen);
  node->vallen = vallen;
  node->value[vallen] = 0;
  return 0;
}

int smap_init(smap *map, size_t num_buckets) {
  if (!map || num_buckets == 0) {
    return RAISE_ARGERR(
        "Cannot initialize the smap with the provided arguments");
  }
  map->size = 0;
  map->bucketsnum = calc_capacity(num_buckets);
  map->mask = map->bucketsnum - 1; // Mask for bucket index calculation

  map->buckets = calloc(map->bucketsnum, sizeof(snode));
  if (!map->buckets) {
    free(map); // Free the map if bucket allocation fails
    return RAISE_MALLOC(
        "could not allocate memory for smap nodes"); // Memory allocation failed
  }

  return 0;
}

smap *smap_create(size_t num_buckets) {
  if (num_buckets == 0) {
    return NULL; // Invalid number of buckets
  }

  smap *map = malloc(sizeof(smap));
  if (!map) {
    return NULL; // Memory allocation failed
  }
  if (smap_init(map, num_buckets) != 0) {
    free(map);   // Free the map if initialization fails
    return NULL; // Memory allocation failed
  }

  return map; // Return the initialized map
}

int smap_upsert(smap *map,
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
  if (!bucket->key) {
    int success = snode_init(bucket, key, keylen, value, vallen, NULL);
    if (success == 0) {
      ++(map->size);
    }
    return success; // If the bucket is empty, initialize it with the new node
  }
  while (bucket) {
    // UPDATE
    if (strneq(key, keylen, bucket->key, bucket->keylen)) {
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
  int success = snode_init(bucket, key, keylen, value, vallen, newnode);
  if (success == 0) {
    ++(map->size);
  }
  return success;
}

snode *smap_get(smap *map, const char *key, size_t keylen) {
  size_t bucketn = hash(key, keylen) & map->mask;
  snode *bucket = map->buckets + bucketn;
  if(!bucket || !bucket->key) {
    return NULL; // No bucket or no key in the bucket
  }

  while (bucket  && !strneq(key, keylen, bucket->key, bucket->keylen)) {
    bucket = bucket->next;
  }
  return bucket;
}

void smap_free(smap *map) {
  if (!map || !map->buckets) {
    return; // Nothing to free
  }

  for (size_t i = 0; i < map->bucketsnum; ++i) {
    snode *bucket = map->buckets + i;
    free(bucket->key);
    free(bucket->value);
    // first bucket is part of map->buckets, ergo it does not have to be freed
    // individually
    bucket = bucket->next;
    while (bucket) {
      snode *next = bucket->next;
      free(bucket->key);
      free(bucket->value);
      free(bucket);
      bucket = next;
    }
  }
  free(map->buckets);
}