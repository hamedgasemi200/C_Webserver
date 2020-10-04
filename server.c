#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#define DEFAULT_PORT_NUMBER 8080
#define CLIENT_REQUEST_SIZE 5000 * sizeof(char)

void prepend(char *s, const char *t) {
    size_t len = strlen(t);
    memmove(s + len, s, strlen(s) + 1);
    memcpy(s, t, len);
}

/*void fpm_connection() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, "socket", sizeof(addr.sun_path) - 1);
    bind(fd, (struct sockaddr *) &addr, sizeof(addr));

    send(sock);
}*/

void raise_error(char *error) {
    printf("Got an error: %s\n", error);
    exit(1);
}

int main(int argc, char *argv[]) {

    // open a document
    FILE *document = fopen("index.html", "r");
    fseek(document, 0, SEEK_END);
    long int file_size = ftell(document);
    fseek(document, 0, SEEK_SET);

    // Response Header
    char http_header[] = "HTTP/1.1 200 OK\x0a"
                         "Server:\x0dTD/1 (Ubuntu)\x0a"
                         "\x0a";

    // Connection Response
    char *response_data = (char *) malloc(file_size + sizeof(http_header));
    fread(response_data, file_size, 1, document);
    prepend(response_data, http_header);

    // Create the server socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Define the server addresss
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET; // IP-V4
    server_addr.sin_port = htons(argc > 1 ? atoi(argv[1]) : DEFAULT_PORT_NUMBER);
    server_addr.sin_addr.s_addr = INADDR_ANY; // Any ip address

    // Bind the socket to our specified IP and Port
    bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr));

    // Listen
    listen(server_socket, 5);

    while (1) {
        // Accept the client socket
        int client_socket = accept(server_socket, NULL, NULL);

        // Send message to the client
        send(client_socket, response_data, file_size + sizeof(http_header), 0);

        // Receive Client Message
        char *message;
        message = (char *) malloc(CLIENT_REQUEST_SIZE);
        if (message == NULL) raise_error("Couldn't allocate memory for the message.");
        else {
            recv(client_socket, message, CLIENT_REQUEST_SIZE, 0);
            printf("Message:\n%s\n", message);
        }

        // Close client socket
        close(client_socket);
    }
}
