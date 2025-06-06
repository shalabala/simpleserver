#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "../configuration/const.h"
#include "../types/cmap.h"
#include "../types/sb.h"
#include "../utility/error.h"
#include "../utility/functions.h"
#include "coordination.h"
#include "request.h"

int reqrecieve(sb *str, int socket, size_t max_size) {
  int error;
  check_for_errors();
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

int acceptreq(int incoming_socket,
              cmap *ctrls,
              struct sockaddr_in *client_address) {
  check_for_errors();
  int error;
  if (incoming_socket < 0) {
    return RAISE_RECVFAIL(
        "Could not accept request"); // Continue to the next iteration
  }

  sb reqbuff;
  request req;
  centry *matched_controller;
  response response;
  smap urlargs;
  smap context;

  if ((error = sbinit(&reqbuff, 64) || smap_init(&urlargs, 32) ||
               smap_init(&context, 64) || respinit(&response))) {
    sbfree(&reqbuff);
    smap_free(&context);
    smap_free(&context);
    respfree(&response);
    return error;
  }

  printf("Connection accepted from %s:%d\n",
         inet_ntoa(client_address->sin_addr),
         ntohs(client_address->sin_port));

  // send(incoming_socket,
  //      welcomemsg,
  //      strlen(welcomemsg),
  //      0); // Send a welcome message

  reqrecieve(&reqbuff, incoming_socket, MAX_REQUEST_SIZE);
  parse(&req, &reqbuff);
  reqprint(&req);

  matched_controller = cmap_match(ctrls, req.resource.data, req.resource.size);

  parseurl(req.resource.data,
           req.resource.size,
           matched_controller->path,
           matched_controller->pathlen,
           &urlargs);

  matched_controller->c(&req, &urlargs, &response, &context);
  respprint(&response);
  if (geterrcode()) {
    fprintf(
        stderr,
        "Something went wrong while handling request. Error description: %s\n",
        globerr->description);
    respclear(&response);
    create_html(&response, "error.html", &context);
    set_resp_code(&response, "Internal Server Error", 500);
    cleargloberr();
    respprint(&response);
  }

  respsend(&response, incoming_socket);

  reqfree(&req);
  respfree(&response);
  sbfree(&reqbuff);
  smap_free(&context);
  smap_free(&urlargs);

  printf("Client disconnected\n");
  return geterrcode();
}

int parseurl(
    char *reqres, size_t reqlen, char *path, size_t pathlen, smap *urlparams) {
  int error;
  size_t req_i = 1, path_i = 0; // centry path does not include beginning /
  size_t keystart, valstart, keylen, vallen;
  while (req_i < reqlen && path_i < pathlen) {
    if (path[path_i] == '{') {
      keystart = ++path_i;
      while (path_i < pathlen && path[path_i] != '}') {
        ++path_i;
      }
      if (path_i == pathlen) {
        return RAISE_INVALIDCPATH("controller path invalid %s", path);
      }
      keylen = path_i - keystart;

      valstart = req_i;
      while (req_i < reqlen && reqres[req_i] != '/') {
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
