#include "cmap.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../com/request.h"
#include "../com/response.h"
#include "../utility/error.h"
#include "../utility/functions.h"

#define MAX_MATCHSCORE 255

typedef struct _matchscore {
  uint8_t as;   // matched *s
  uint8_t das;  // matched **s
  uint8_t vars; // matched {vars}
} matchscore;

static const matchscore no_match =
    (matchscore){MAX_MATCHSCORE, MAX_MATCHSCORE, MAX_MATCHSCORE};

static bool ismatch(matchscore m) {
  return m.as != no_match.as || m.das != no_match.das ||
         m.vars != no_match.vars;
}

static matchscore matchcalc(centry *entry, char *resbase, size_t resbaselen) {
  char *path = entry->path;
  size_t pathlen = entry->pathlen;

  matchscore m = {0};
  size_t path_i = 0, res_i = 0;
  while (path_i < pathlen && res_i < resbaselen) {
    if (path[path_i] == resbase[res_i]) {
      ++path_i;
      ++res_i;
      continue;
    }
    if (path[path_i] == '{') {
      // flush path until next }
      // flush res until next /
      while (path_i < pathlen && path[path_i] != '}') {
        ++path_i;
      }
      ++path_i; // this is either length of path or the next '/'

      while (res_i < resbaselen && resbase[res_i] != '/') {
        ++res_i;
      }

      ++m.vars;
      if (m.vars >= MAX_MATCHSCORE) {
        return no_match;
      }
      continue;
    }
    if (path[path_i] == '*') {
      ++path_i;
      if (path_i < pathlen && path[path_i] == '*') {
        // ** matches rest of string no matter what
        // we need to adjust score and return
        ++m.das;
        if (m.das >= MAX_MATCHSCORE) {
          return no_match;
        }
        return m;
      }
      // we flush rest of the resource until next /
      while (res_i < resbaselen && resbase[res_i] != '/') {
        ++res_i;
      }

      ++m.as;
      if (m.as >= MAX_MATCHSCORE) {
        return no_match;
      }
      continue;
    } else {
      // we have diverging chars at path_i and res_i respectively
      // means paths dont match
      // return no match
      return no_match;
    }
  }
  return m;
}

static int matchcomp(matchscore a, matchscore b) {
  // the different matchers weigh in differently
  // ** matchers are more important than * matchers, which
  // are more important than {vars}
  if (a.das == b.das) {
    if (a.as == b.as) {
      if (a.vars == b.vars) {
        return 0;
      }
      return a.vars > b.vars ? 1 : -1;
    }
    return a.as > b.as ? 1 : -1;
  }
  return a.das > b.das ? 1 : -1;
}

int cmap_init(cmap *map, size_t capacity) {
  if (capacity == 0) {
    return RAISE_ARGERR("capacity must be greater than 0");
  }
  map->entries = calloc(capacity, sizeof(centry));
  if (!map->entries) {
    return RAISE_MALLOC("failed to allocate memory for controller map");
  }
  memset(map->index, 0, INDEX_MAX * sizeof(size_t));
  map->capacity = capacity;
  map->size = 0;
  return OK;
}

// a valid path:
// - has exactly as many {s as }s, no / or * in between them
// - before a ** always comes nothing or /, after it does not have anything
// - a * or a { is preceeded by nothing or /
// - a * or a } is superceeded by nothing or /
// - other than {}*/ only has alphabetical characters (support numbers ?)
static bool valid_path(char *path, size_t pathlen) {
  size_t i = 0;
  while (i < pathlen) {
    if (path[i] == '{') {                 // if char is {
      if (i != 0 && path[i - 1] != '/') { // prev char must be nothing or /
        return false;
      }
      ++i;
      while (i < pathlen && path[i] != '}') { // between { and }
        if (!isalphanumeric(path[i])) {       // only alphabetical chars
          return false;
        }
        ++i;
      }
      ++i;
      if (i != pathlen &&
          path[i] != '/') { // after } either the string ends or there is a /
        return false;
      }
      continue;
    }
    if (path[i] == '*') {                 // if char is *
      if (i != 0 && path[i - 1] != '/') { // the prev char has to be nothing or
                                          // /
        return false;
      }
      ++i;
      if (i >= pathlen) { // if there is nothing after * it is correct syntax
        return true;
      }
      if (path[i] == '*') {  // if char is * after *
        if (++i < pathlen) { // if it is not at end its false syntax
          return false;
        }
        return true;
      }
      if (path[i] !=
          '/') {      // if after * the subsequent char is not nothing * or /
        return false; // its incorrect
      }
      continue;
    }
    if (path[i] != '/' &&
        !isalphanumeric(path[i])) { // only accept [a..zA..Z/*{}]
      return false;
    }
    ++i;
  }
  return true; // no mistakes found
}

static int
centry_init(centry *entry, char *path, size_t pathlen, controller c) {
  if (!entry) {
    return RAISE_ARGERR("cannot initialize null centry");
  }
  entry->path = malloc(pathlen + 1);
  if (!entry->path) {
    return RAISE_MALLOC("could not allocate memory for centry.path");
  }

  strncpy(entry->path, path, pathlen);
  entry->path[pathlen] = 0;

  entry->c = c;
  entry->pathlen = pathlen;
  return OK;
}

static int get_index(cmap *map, char c, int *index) {
  if (c >= 'A' && c <= 'Z') {
    c += 'a' - 'A';
  }
  if (c >= 'a' && c <= 'z') {
    *index = c - 'a' + 1;
    return OK;
  }
  if (c >= '0' && c <= '9') {
    *index = c - '0' + ('z' - 'a' + 1);
    return OK;
  }
  if (c == '*' || c == '{' || c == 0) {
    *index = 0;
    return OK;
  }
  return RAISE_ARGERR("Cannot find mapping for non-alphabetical mapping %c", c);
}

int cmap_reg(cmap *map, char *path, controller c) {
  int error;
  if (map->size == map->capacity) {
    return RAISE_CMAPFULL(
        "The cmap has no capacity left, it could not be registered.");
  }
  if (path[0] ==
      '/') { // should be safe since all strings are at least 1 char long
    // idea: dont store beginning / because it is always there
    path++;
  }
  size_t pathlen = strlen(path);
  size_t insert_at;
  if (!valid_path(path, pathlen)) {
    return RAISE_INVALIDCPATH("Invalid controller path: %s", path);
  }
  int index;
  // global matcher goes to index one
  if (pathlen == 2 && path[0] == '*' && path[1] == '*') {
    index = -1;
    insert_at = 0;
  } else {
    if ((error = get_index(map, pathlen == 0 ? 0 : path[0], &index))) {
      return error;
    }
    insert_at = map->index[index];
  }

  // copy the rest of the array one place to the right, making place for the new
  // entry
  memmove(map->entries + insert_at + 1,
          map->entries + insert_at,
          (map->size - insert_at) * sizeof(centry));

  // shifting the larger indices by one
  for (size_t i = index + 1; i < INDEX_MAX; ++i) {
    ++map->index[i];
  }
  // initializing the entry with the data
  if ((error = centry_init(map->entries + insert_at, path, pathlen, c))) {
    return error;
  }
  ++map->size;
  return OK;
}

static int get_start_and_end(cmap *map, char c, size_t *start, size_t *end) {
  int index, error;
  if ((error = get_index(map, c, &index))) {
    return error;
  }
  *start = map->index[index];
  *end = index + 1 == INDEX_MAX ? map->size : map->index[index + 1];
  return OK;
}

static centry *getmatch(centry *entries,
                        char *resbase,
                        size_t resbaselen,
                        size_t startindex,
                        size_t endindex) {
  centry *best = entries + startindex, *current;
  matchscore bestmatch = matchcalc(best, resbase, resbaselen), currentmatch;
  for (size_t i = startindex + 1; i < endindex; ++i) {
    current = entries + i;
    currentmatch = matchcalc(current, resbase, resbaselen);
    // we take the smaller match score
    if (matchcomp(currentmatch, bestmatch) < 0) {
      best = current;
      bestmatch = currentmatch;
    }
  }
  if (ismatch(bestmatch)) {
    return best;
  }
  return NULL;
}

static size_t get_resbaselen(char* resbase, size_t maxlen ){
  for(size_t i = 0; i < maxlen; ++i){
    if(resbase[i] == 0 || resbase[i] == '?'){
      return i;
    }
    ++i;
  }
  return maxlen;
}

centry *cmap_match(cmap *map, char *res, size_t reslen) {
  int error;
  centry *result;

  // We want to go through the entries that could match the
  // requested resource. We calculate for each a match score
  // that is based on how well the path of the entry matches
  // our request. We will return the best matching controller
  if (!map || !res) {
    return NULL;
  }
  char *resbase = res + 1;
  //we are only interested in the part after the first / and before the ?
  size_t resbaselen =get_resbaselen(resbase, reslen - 1) ;
  // get the index where the entries that could match are
  size_t startindex, endindex;
  if (!resbaselen ) {
    // this are the paths that match "/?.."
    if ((error = get_start_and_end(map, 0, &startindex, &endindex))) {
      return NULL;
    }
  } else {
    // these are the paths that match /[a..zA..Z]+
    if ((error = get_start_and_end(map, resbase[0], &startindex, &endindex))) {
      return NULL;
    }
  }

  if (startindex < endindex) {
    if ((result = getmatch(
             map->entries, resbase, resbaselen, startindex, endindex))) {
      return result;
    }
  }
  // if the requested resource only contains a single token like /foo than maybe
  // one of the paths at index[0] could match it - this is where a matcher like
  // "/*" or "/{var}" would be inserted at
  size_t sepcount = strncount(resbase, '/', resbaselen);
  // only check if there is at most one separator, and if its there its the last char
  if (sepcount == 0 || (sepcount == 1 && resbase[resbaselen - 1] == '/')) {
    if ((error = get_start_and_end(map, 0, &startindex, &endindex))) {
      return NULL;
    }
    if ((result = getmatch(
             map->entries, resbase, resbaselen, startindex, endindex))) {
      return result;
    }
  }

  // if we made it thus far that means that there were no matches
  // we see if there is a global matcher on position 0
  // if there is none, we return NULL
  if (map->entries[0].path &&
      strneq(map->entries[0].path, map->entries[0].pathlen, "**", 2)) {
    return map->entries;
  }

  return NULL;
}

void cmap_free(cmap *map) {
  if (map) {
    for (size_t i = 0; i < map->size; ++i) { // entries stored non-sequentially
      free(map->entries[i].path);
    }
    free(map->entries);
    map->capacity = 0;
    map->size = 0;
  }
}

int cmap_clear(cmap *map) {
  if (map && map->entries) {
    for (size_t i = 0; i < map->size; ++i) { // entries stored non-sequentially
      free(map->entries[i].path);
    }
    memset(map->entries, 0, map->capacity * sizeof(centry));
    memset(map->index, 0, INDEX_MAX * sizeof(size_t));
    map->size = 0;
    return OK;
  } else {
    return RAISE_ARGERR("cannot clear null or uninitialized cmap");
  }
}