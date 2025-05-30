#ifndef COORDINATION_H
#define COORDINATION_H
#include <netinet/in.h>

#include "request.h"
#include "../types/sb.h"
#include "../types/smap.h"
#include "../types/cmap.h"

/**
 * Receives a request from a socket and returns it as a string buffer.
 * @param str The string buffer to store the received request.
 * @param socket The socket from which to receive the request.
 * @param max_size The maximum size of the request to receive.
 */
int reqrecieve(sb *str, int socket, size_t max_size);

/**
 * Accepts the incoming request and processes it.
 * @param incoming_socket The socket to accept the request from.
 * @param client_address The address of the client making the request.
 * @return 0 on success, or an error code on failure.
 */
int acceptreq(int incoming_socket, struct sockaddr_in *client_address);

/**
 * Parses the url parameters based on the matched controller entry and the request resource.
 */
int parseurl(char *reqres, size_t reqlen, char *path, size_t pathlen, smap *urlparams);

#endif