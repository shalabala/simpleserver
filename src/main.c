#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 7890
#define MAX_REQUEST_SIZE 1<<32 

int main(int argc, char *argv[]) {
    int outgoing_socket, incoming_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_length = sizeof(client_address);
    char buffer[1024];
    int recv_len, yes = 1;

    if((outgoing_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    if(setsockopt(outgoing_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        perror("setsockopt failed");
        close(outgoing_socket);
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);
    if(bind(outgoing_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Bind failed");
        close(outgoing_socket);
        exit(EXIT_FAILURE);
    }
    if(listen(outgoing_socket, 5) < 0) {
        perror("Listen failed");
        close(outgoing_socket);
        exit(EXIT_FAILURE);
    }
    printf("Server is listening on port %d\n", PORT);

    while(true){
        incoming_socket = accept(outgoing_socket, (struct sockaddr *)&client_address, &client_address_length);
        if(incoming_socket < 0) {
            perror("Accept failed");
            continue; // Continue to the next iteration
        }

        printf("Connection accepted from %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        send(incoming_socket, "Hello, client!\n", 15, 0); // Send a welcome message
        while((recv_len = recv(incoming_socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
            buffer[recv_len] = '\0'; // Null-terminate the received data
            printf("Received: %s\n", buffer);
        }

        if(recv_len < 0) {
            perror("Receive failed");
        } else {
            printf("Client disconnected\n");
        }

        close(incoming_socket);
    }


    return 0;
}