#ifndef REQUEST_H
#define REQUEST_H
#include <netinet/in.h>

#include "../types/sb.h"
#include "../types/smap.h"
typedef struct _request {
  char *resource;
  smap header;
  smap query;
  smap body;
} request;

/**
 * Receives a request from a socket and returns it as a string buffer.
 * @param str The string buffer to store the received request.
 * @param socket The socket from which to receive the request.
 * @param max_size The maximum size of the request to receive.
 */
int reqrecieve(sb *str, int socket, size_t max_size);

int acceptreq(int incoming_socket, struct sockaddr_in *client_address);

int parse( request *req, sb *str);

#endif // REQUEST_H