#ifndef CMAP_H
#define CMAP_H
#include "../com/request.h"
#include "../com/response.h"

#define INDEX_MAX 37

/**
 * Signature for a controller function.
 * @param req Pointer to the request structure.
 * @param url_params Pointer to the smap containing url parameters.
 * @param res Pointer to the response structure to fill with data.
 * @param context Pointer to the initialized context
 * @return An integer indicating the result of the controller execution.
 *        0 on success, or an error code on failure.
 */
typedef int (*controller)(request *, smap *, response *, smap*);

/**
 * Controller entry structure. Contains the path and the controller function.
 */
typedef struct _centry {
  char *path; // path of the controller, e.g. "/users/{id}"
  size_t pathlen; // length of the path
  controller c; // controller function to call when the path matches
} centry;

/**
 * Controller map structure. Used to register and match controllers.
 */
typedef struct _cmap {
  centry *entries; // controller entries
  size_t size;     // count of currently inserted entries
  size_t capacity; // max number of entries
  // index for the starting char of paths. Empty string
  // (matching "/?..") starts at _index['\0']
  size_t index[INDEX_MAX]; // index for start of the paths. index[0] is for empty paths
} cmap;

/**
 * Initializes a controller map with a given capacity.
 * @param map Pointer to the controller map to initialize.
 * @param capacity The maximum number of entries the map can hold.
 * @return 0 on success, or an error code on failure.
 */
int cmap_init(cmap *map, size_t capacity);

/**
 * Registers a controller for a specific path in the controller map.
 * @param map Pointer to the controller map.
 * @param path The path to register the controller for.
 * @param c The controller function to register.
 * @return 0 on success, or an error code on failure.
 */
int cmap_reg(cmap *map, char *path, controller c);

/**
 * Matches a request resource to a registered controller in the map.
 * @param map Pointer to the controller map.
 * @param res The resource string to match against registered controllers.
 * @param reslen The length of the resource string.
 * @return The controller entry if a match is found, or NULL if no match is
 * found.
 */
centry* cmap_match(cmap *map, char *res, size_t reslen);

/**
 * Frees the memory allocated for the controller map.
 * @param map Pointer to the controller map to free.
 */
void cmap_free(cmap *map);

/**
 * Clears contents of the map. mainly used for testing.
 */
int cmap_clear(cmap *map);

#endif // CMAP_H