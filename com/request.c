#include <sys/socket.h>
#include <netinet/in.h>

#include "request.h"
#include "../configuration/const.h"
#include "../types/sb.h"
#include "../utility/error.h"

int reqrecieve(sb *str, int socket, size_t max_size) {
  int error;
  if (!str) {
    return RAISE_ARGERR("cannot receive into NULL buffer");
  }

  char buffer[1024];
  int flags = 0;
  ssize_t bytes_received = 0;
  while ((bytes_received = recv(socket, buffer, sizeof(buffer) - 1, flags)) >
         0) {
    if (bytes_received < 0) {
      return RAISE_RECVFAIL("Failed to recieve");
    }
    buffer[bytes_received] = '\0'; // Null-terminate the received data
    if (str->size + bytes_received >= max_size) {
      if ((error = sbappend(
               str, buffer, str->size + bytes_received - max_size)) != 0) {
        return error;
      }
      return RAISE_BUFFLIMIT("buffer limit has been reached");
    }
    if ((error = sbappend(str, buffer, bytes_received)) != 0) {
      return error; // Memory allocation failed
    }
    flags = MSG_DONTWAIT; // after receiving one message only receive if its
                          // already sent
  }
  return OK; // Return the received request as a string buffer
}

int acceptreq(int incoming_socket, struct sockaddr_in *client_address) {
  if (incoming_socket < 0) {
    perror("Accept failed");
    return RAISE_RECVFAIL(
        "Could not accept request"); // Continue to the next iteration
  }
  sb reqbuff;

  printf("Connection accepted from %s:%d\n",
         inet_ntoa(client_address->sin_addr),
         ntohs(client_address->sin_port));

  // send(incoming_socket,
  //      welcomemsg,
  //      strlen(welcomemsg),
  //      0); // Send a welcome message
  sb reqbuff;
  sbinit(&reqbuff, 64);
  reqrecieve(&reqbuff, incoming_socket, MAX_REQUEST_SIZE);
  printf("REQ_RECIEVE buffer of size %lu:\n%s\n", reqbuff.size, reqbuff.data);
  
  

  sbfree(&reqbuff);

  printf("Client disconnected\n");
}
