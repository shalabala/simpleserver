#include <sys/socket.h>

#include "request.h"
#include "../types/sb.h"

sb* reqrecieve(int socket, size_t max_size){

    sb *sbuff = sbinit(64);
    if (!sbuff) {
        return NULL; // Memory allocation failed
    }

    char buffer[1024];

    int bytes_received = 0;
    while((bytes_received = recv(socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0'; // Null-terminate the received data
        if (sbappend(sbuff, buffer, bytes_received) != 0) {
            sbfree(sbuff); // Free memory on error
            return NULL; // Memory allocation failed
        }
        if (sbuff->size >= max_size) {
            sbfree(sbuff); // Free memory if max size is reached
            sbuff = NULL; // Set to NULL to indicate no further processing
            return NULL;
        }
    }
    return sbuff; // Return the received request as a string buffer
}
