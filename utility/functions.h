#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#define MIN(a, b) ((a) > (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (b) : (a))

#define STRLEN(s) (sizeof(s) - 1)

/**
 * Returns true if both keys are equal
 * @param key1 First key to compare
 * @param key1len Length of the first key
 * @param key2 Second key to compare
 * @param key2len Length of the second key
 * @return true if keys are equal, false otherwise
 */
bool strneq(const char *key1, size_t key1len, const char *key2, size_t key2len);

bool isalphabetical(char c);

bool isnumeric(char c);

bool isalphanumeric(char c);

size_t strncount(char *str, char c, size_t n);
#endif // FUNCTIONS_H