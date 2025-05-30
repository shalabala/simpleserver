#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "../configuration/const.h"
#include "../types/cmap.h"
#include "../types/sb.h"
#include "../utility/error.h"
#include "../utility/functions.h"
#include "request.h"

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
  request req;
  printf("Connection accepted from %s:%d\n",
         inet_ntoa(client_address->sin_addr),
         ntohs(client_address->sin_port));

  send(incoming_socket,
       welcomemsg,
       strlen(welcomemsg),
       0); // Send a welcome message

  sbinit(&reqbuff, 64);

  reqrecieve(&reqbuff, incoming_socket, MAX_REQUEST_SIZE);
  // printf("REQ_RECIEVE buffer of size %lu:\n%s\n", reqbuff.size,
  // reqbuff.data);
  parse(&req, &reqbuff);
  reqprint(&req);
  reqfree(&req);
  sbfree(&reqbuff);

  printf("Client disconnected\n");
  return OK;
}

int parseurl(
    char *reqres, size_t reqlen, char *path, size_t pathlen, smap *urlparams) {
  int error;
  size_t req_i = 1, path_i = 0; // centry path does not include beginning /
  size_t keystart, valstart, keylen, vallen;
  while (req_i < reqlen && path_i < pathlen) {
    if (path[path_i] == '{') {
      keystart = ++path_i;
      while (path_i < pathlen && path_i != '}') {
        ++path_i;
      }
      if (path_i == pathlen) {
        return RAISE_INVALIDCPATH("controller path invalid %s", path);
      }
      keylen = path_i - keystart;

      valstart = req_i;
      while (req_i > reqlen && reqres[req_i] != '/') {
        ++req_i;
      }
      vallen = req_i - valstart;
      if ((error = smap_upsert(urlparams,
                               path + keystart,
                               keylen,
                               reqres + valstart,
                               vallen))) {
        return error;
      }
      continue;
    } else {
      ++req_i;
      ++path_i;
    }
  }
  return OK;
}
