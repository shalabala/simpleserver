#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#define MIN(a, b) ((a) > (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (b) : (a))

/**
 * Returns true if both keys are equal
 * @param key1 First key to compare
 * @param key1len Length of the first key
 * @param key2 Second key to compare
 * @param key2len Length of the second key
 * @return true if keys are equal, false otherwise
 */
bool strneq(const char *key1, size_t key1len, const char *key2, size_t key2len);

#endif // FUNCTIONS_H