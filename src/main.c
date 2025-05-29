#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../com/request.h"
#include "../configuration/const.h"

int main(int argc, char *argv[]) {
  static char *welcomemsg = "HTTP/1.1 200 OK\r\n"
                            "Content-Type: text/html; charset=UTF-8\r\n"
                            "Content-Length: 64\r\n\r\n"
                            //"{\"hello\":\"world\"}\r\n";
                            "<html><head></head><body><h1>Hello, client!</h1><pre>"
                            "Hello\r\nWorld</pre></body></html>\r\n";

  int outgoing_socket, incoming_socket;
  struct sockaddr_in server_address, client_address;
  socklen_t client_address_length = sizeof(client_address);
  int yes = 1;

  if ((outgoing_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  if (setsockopt(outgoing_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) <
      0) {
    perror("setsockopt failed");
    close(outgoing_socket);
    exit(EXIT_FAILURE);
  }

  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(PORT);
  if (bind(outgoing_socket,
           (struct sockaddr *)&server_address,
           sizeof(server_address)) < 0) {
    perror("Bind failed");
    close(outgoing_socket);
    exit(EXIT_FAILURE);
  }
  if (listen(outgoing_socket, 5) < 0) {
    perror("Listen failed");
    close(outgoing_socket);
    exit(EXIT_FAILURE);
  }
  printf("Server is listening on port %d\n", PORT);

  while (true) {
    incoming_socket = accept(outgoing_socket,
                             (struct sockaddr *)&client_address,
                             &client_address_length);
    if (incoming_socket < 0) {
      perror("Accept failed");
      continue; // Continue to the next iteration
    }

    printf("Connection accepted from %s:%d\n",
           inet_ntoa(client_address.sin_addr),
           ntohs(client_address.sin_port));

    send(incoming_socket,
         welcomemsg,
         strlen(welcomemsg),
         0); // Send a welcome message
    sb reqbuff;
    sbinit(&reqbuff, 64);
    reqrecieve(&reqbuff, incoming_socket, MAX_REQUEST_SIZE);
    printf("REQ_RECIEVE buffer of size %lu:\n%s\n", reqbuff.size, reqbuff.data);
    sbfree(&reqbuff);

    printf("Client disconnected\n");

    close(incoming_socket);
  }

  return 0;
}