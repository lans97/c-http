#include "http_internal.h"
#include "http.h"

#include <logger/logger.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>

int listen_and_serve(int port) {
    return 0;
}

void tutorial() {
    char buff[1024];
    size_t bytes;
    int status;

    struct addrinfo hints, *res;
    int server_socket, client_socket;
    int yes = 1;

    struct sockaddr_storage client_addr;
    socklen_t client_addr_size = sizeof(struct sockaddr_storage);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, "8080", &hints, &res)) != 0) {
        LOG_ERROR("Error filling addrinfo: %s", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    server_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &yes, sizeof yes) == -1){
        LOG_ERROR("Error setting socket options: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (bind(server_socket, res->ai_addr, res->ai_addrlen) == -1) {
        LOG_ERROR("Error binding socket: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    if (listen(server_socket, 5) == -1) {
        LOG_ERROR("Error starting listen on port 8080: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_size)) == -1) {
        LOG_ERROR("Error accepting connection from client: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    while ((bytes = recv(client_socket, buff, 1024, 0)) > 0) {
        printf("%s", buff);
    }
    if (bytes == 0) {
        LOG_INFO("Client has closed the connecion");
    }
    if (bytes == -1) {
        LOG_ERROR("Error recieving client bytes: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    close(server_socket);

}
