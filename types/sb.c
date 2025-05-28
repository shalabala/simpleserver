
#include "sb.h"
#include <stdlib.h>

sb* sbinit(size_t initial_capacity)
{
    sb* str = malloc(sizeof(sb));
    if (!str) {
        return NULL; // Memory allocation failed
    }
    str->data = malloc(initial_capacity);
    if (!str->data) {
        free(str);
        return NULL; // Memory allocation failed
    }
    str->size = 0;
    str->capacity = initial_capacity;
    return str;
}

int sbappend(sb* str, const char* data, size_t len)
{
    if (str->size + len >= str->capacity) {
        size_t new_capacity = str->capacity * 2;
        while (str->size + len >= new_capacity) {
            new_capacity *= 2; // Double the capacity until it fits
        }
        char* new_data = realloc(str->data, new_capacity);
        if (!new_data) {
            return -1; // Memory allocation failed
        }
        str->data = new_data;
        str->capacity = new_capacity;
    }
    memcpy(str->data + str->size, data, len);
    str->size += len;
    return 0; // Success
}

int sbinsert(sb* str, size_t pos, const char* data, size_t len)
{
    if (pos > str->size) {
        return -1; // Invalid position
    }
    if (str->size + len >= str->capacity) {
        size_t new_capacity = str->capacity * 2;
        while (str->size + len >= new_capacity) {
            new_capacity *= 2; // Double the capacity until it fits
        }
        char* new_data = realloc(str->data, new_capacity);
        if (!new_data) {
            return -1; // Memory allocation failed
        }
        str->data = new_data;
        str->capacity = new_capacity;
    }
    memmove(str->data + pos + len, str->data + pos, str->size - pos);
    memcpy(str->data + pos, data, len);
    str->size += len;
    return 0; // Success
}

void sbfree(sb* str)
{
    if (str) {
        if(str->data) {
            free(str->data);
        }
        free(str);
        str->data = NULL; // Avoid dangling pointer
        str->size = 0;
        str->capacity = 0; // Reset capacity
    }
}