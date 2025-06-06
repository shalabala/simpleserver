#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>

#include "../com/coordination.h"
#include "../types/cmap.h"
#include "../controller/controller.h"
#include "../configuration/const.h"
#include "../dal/dal.h"

int outgoing_socket;
cmap controllers;

void destruct(){
  free_users();
  close(outgoing_socket);
  cmap_free(&controllers);
  printf("Program shut down gracefully\n");
}

void handle_sigint(int sig){
  printf("Caught signal %d\n", sig);
  exit(0);
}

int main(int argc, char *argv[]) {
  atexit(destruct);
  signal(SIGINT, handle_sigint);

  cmap_init(&controllers, 64);
  init_users();
  reg_controllers(&controllers);
  
  int incoming_socket;
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

    acceptreq(incoming_socket, &controllers,&client_address);
    
    close(incoming_socket);
  }
  return 0;
}