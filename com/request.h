#ifndef REQUEST_H
#define REQUEST_H
#include "../types/sb.h"
#include "../types/smap.h"
typedef struct _request
{
    char *method;
    smap data;
} request;

/**
 * Receives a request from a socket and returns it as a string buffer.
 * @param socket The socket from which to receive the request.
 * @param max_size The maximum size of the request to receive.
 */
sb* reqrecieve(int socket, size_t max_size);
#endif // REQUEST_H