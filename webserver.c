#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void serve_static_content(int client_socket, char *uri);

void serve_dynamic_content(int client_socket, char *uri);

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
      };
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
    ssize_t bytes_written = write(client_socket, response, strlen(response));
    if (bytes_written == -1) {
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
  ssize_t bytes_written = write(client_socket, response, strlen(response));
  if (bytes_written < 0) {
    perror("write");
    exit(EXIT_FAILURE);
  }
  close(client_socket);
}

int main() {
  // Create a socket
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Bind the socket to a specific address and port
  struct sockaddr_in host_address;
  host_address.sin_family = AF_INET;
  host_address.sin_addr.s_addr = htonl(INADDR_ANY);
  host_address.sin_port = htons(PORT);

  if (bind(sockfd, (struct sockaddr *)&host_address, sizeof(host_address)) !=
      0) {
    perror("Socket bind failed");
    exit(EXIT_FAILURE);
  }

  // Listen for incoming connections
  if (listen(sockfd, SOMAXCONN) != 0) {
    perror("Socket listen failed");
    exit(EXIT_FAILURE);
  }

  printf("Server listening for connections on port %d...\n", PORT);

  // Accept incoming connections
  struct sockaddr_in client_address;
  socklen_t addrlen = sizeof(client_address);
  char buffer[BUFFER_SIZE];

  for (;;) {
    int client_socket =
        accept(sockfd, (struct sockaddr *)&client_address, &addrlen);
    if (client_socket == -1) {
      perror("Socket accept failed");
      continue;
    }

    printf("Connection accepted from %s:%d\n",
           inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

    // Read from the socket
    int valread = read(client_socket, buffer, BUFFER_SIZE);
    if (valread == -1) {
      perror("Socket read failed");
      close(client_socket);
      continue;
    }

    // Process the request
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
