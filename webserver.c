#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void serve_static_content(int client_socket, char *uri){
  // TODO: Implement the function to serve static content
}

void serve_dynamic_content(int client_socket, char *uri){
  // TODO: Implement the function to serve dynamic content
}

void serve_request(int client_socket) {
  char buffer[BUFFER_SIZE];
  char response[BUFFER_SIZE];
  char method[BUFFER_SIZE], uri[BUFFER_SIZE], version[BUFFER_SIZE];

  // Read from the socket
  int valread = read(client_socket, buffer, BUFFER_SIZE);
  if (valread == -1) {
    perror("Socket read failed");
    close(client_socket);
    return;
  }

  // Process the request
  sscanf(buffer, "%s %s %s", method, uri, version);
  printf("%s %s %s\n", method, version, uri);

  // Serve static files
  if (strcmp(method, "GET") == 0 && strcmp(uri, "/") != 0) {
    // Get the filename by removing the leading slash
    char *filename = uri + 1;

    // Check if file exists
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
      // File not found
      sprintf(response,
              "HTTP/1.0 404 Not Found\r\nServer: webserver-c\r\n\r\n");
      if (write(client_socket, response, strlen(response)) < 0) {
        perror("Socket write failed");
      }
      close(client_socket);
      return;
    }

    // Read the contents of the file
    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *file_content = malloc(fsize + 1);
    size_t bytes_read = fread(file_content, fsize, 1, file);
    if (bytes_read != 1) {
      perror("Error reading file");
      free(file_content);
      fclose(file);
      close(client_socket);
      return;
    }
    fclose(file);

    // Send the response with the file content
    sprintf(response,
            "HTTP/1.0 200 OK\r\nServer: webserver-c\r\nContent-type: "
            "text/html\r\nContent-length: %ld\r\n\r\n%s",
            fsize, file_content);
    ssize_t valwrite = write(client_socket, response, strlen(response));
    if (valwrite == -1) {
      perror("Socket write failed");
    }
    close(client_socket);
    free(file_content);
    return;
  }

  // Serve dynamic content
  sprintf(
      response,
      "HTTP/1.0 200 OK\r\nServer: webserver-c\r\nContent-type: "
      "text/html\r\n\r\n<html>This is a simple dynamic web page.</html>\r\n");
  ssize_t valwrite = write(client_socket, response, strlen(response));
  if (valwrite < 0) {
    perror("write");
    exit(EXIT_FAILURE);
  }
  close(client_socket);
}

int main() {
    char buffer[BUFFER_SIZE];
    char resp[] = "HTTP/1.0 200 OK\r\n"
                  "Server: webserver-c\r\n"
                  "Content-type: text/html\r\n\r\n"
                  "<html>This a simple static webserver.</html>\r\n";

    // Create a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
      //editing belowe line to help with 
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
    }
    printf("socket created successfully\n");

    // Create the address to bind the socket to
    struct sockaddr_in host_addr;
    int host_addrlen = sizeof(host_addr);
  
    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(PORT);
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Create client address
    struct sockaddr_in client_addr;
    int client_addrlen = sizeof(client_addr);

    // Bind the socket to the address
    if (bind(sockfd, (struct sockaddr *)&host_addr, host_addrlen) != 0) {
    perror("Socket bind failed");
    exit(EXIT_FAILURE);
    }
    printf("socket successfully bound to address\n");

    // Listen for incoming connections
    if (listen(sockfd, SOMAXCONN) != 0) {
    perror("Socket listen failed");
    exit(EXIT_FAILURE);
    }
    printf("server listening for connections\n");

    for (;;) {
        // Accept incoming connections
        int client_socket = accept(sockfd, (struct sockaddr *)&client_addr,&client_addrlen);
        if (client_socket < 0) {
      perror("Socket accept failed");
            continue;
        }
        printf("connection accepted\n");

        // Get client address 
        int sockn = getsockname(client_socket, (struct sockaddr *)&client_addr,
                                (socklen_t *)&client_addrlen);
        if (sockn < 0) {
            perror("webserver (getsockname)");
            continue;
        }

        // Read from the socket
        int valread = read(client_socket, buffer, BUFFER_SIZE);
        if (valread < 0) {
      perror("Socket read failed");
      close(client_socket);
            continue;
        }

        // Read the request
        char method[BUFFER_SIZE], uri[BUFFER_SIZE], version[BUFFER_SIZE];
        sscanf(buffer, "%s %s %s", method, uri, version);
    printf("%s %s %s\n", method, version, uri);

    // Serve dynamic or static content depending on the URI
    if (strstr(uri, ".html") || strstr(uri, ".php")) {
      // Serve dynamic content
      serve_dynamic_content(client_socket, uri);
    } else {
      // Serve static content
      serve_static_content(client_socket, uri);
    }


        close(client_socket);
    }

    return 0;
}
