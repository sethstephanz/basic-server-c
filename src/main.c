#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../libs/cJSON.h" // https://github.com/DaveGamble/cJSON

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address; 
    int addr_len = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed.");
        exit(EXIT_FAILURE);
    }

    // configure server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // bind socket to address
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // start listening for incoming connections
    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is running on port %d\n", PORT);

    while (1) {
        // accept incoming connection
        if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addr_len)) < 0) {
            perror("Accept failed");
            continue;
        }

        // read request from client
        read(client_fd, buffer, BUFFER_SIZE);
        printf("Received request:\n%s\n", buffer);

        // send simple http response
        //const char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 13\r\n\r\nHello, world!";

        // let's try to use cJSON to send a JSON response instead of simple http/text
        
        // create cJSON object 
        cJSON *json = cJSON_CreateObject();
        cJSON_AddStringToObject(json, "name", "John Doe");
        cJSON_AddNumberToObject(json, "age", 30);
        cJSON_AddStringToObject(json, "email", "john.doe@email.com");

        // convert cJSON object to a JSON string
        char *json_str = cJSON_Print(json);
        
        size_t response_length = strlen(json_str) + 128; // Extra space for HTTP headers
        char *response = (char *)malloc(response_length);

        snprintf(response, sizeof(response),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json\r\n"
                "Content-Length: %zu\r\n"
                "\r\n"
                "%s",
                strlen(json_str), json_str);


        send(client_fd, response, strlen(response), 0);

        cJSON_Delete(json);
        free(json_str);
        free(response);

        // close client connection
        close(client_fd);
    }

    // clean up and close server
    close(server_fd);
    return 0;
}
