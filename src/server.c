#include "http/request.h"
#include "http/results.h"
#include "logger/logger.h"
#include "sds.h"
#include "server_internal.h"
#include <errno.h>
#include <http/server.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define READ_CHUNK 4096

http_connection *http_connection_new() {
    http_connection *new_connection = malloc(sizeof(http_connection));
    new_connection->fd = 0;
    new_connection->buffer = sdsempty();
    new_connection->content_length = 0;
    new_connection->header_parsed = false;
    new_connection->keep_alive = false;

    return new_connection;
}

ErrorMessage http_connection_bindAndListen(http_connection *this, int port) {
    int client_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    this->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->fd == -1) {
        return "Connection error: could not create socket";
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if(bind(this->fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        close(this->fd);
        return "Connection error: could not bind socket";
    }

    if (listen(this->fd, 10) < 0) {
        close(this->fd);
        return "Connection error: could not listen through socket";
    }

    LOG_DEBUG("Server listening on port %d...\n", port);

    char tmp[READ_CHUNK];
    while (true) {
        client_fd = accept(this->fd, (struct sockaddr*)&address, &addrlen);
        if (client_fd < 0) {
            LOG_ERROR("Connection error: failed to accept connection: %s", strerror(errno));
            continue;
        }

        size_t nread;
        while((nread = read(client_fd, tmp, sizeof(tmp))) > 0) {
            this->buffer = sdscatlen(this->buffer, tmp, nread);
            // TODO: Look for complete request or keep_alive: false
            // probably change parse strategy to stream reading strategy
        }

        if (nread < 0) {
            LOG_ERROR("Error reading from client: %s", strerror(errno));
            sdsclear(this->buffer);
            close(client_fd);
            continue;
        }

        // PROCESS buffer
        HTTPRequestResult req_res = http_request_new();
        if (!req_res.Ok) {
            LOG_ERROR("Error allocating request object: %s", req_res.Err);
            sdsclear(this->buffer);
            close(client_fd);
            continue;
        }

        http_request* req = req_res.Value;
        ErrorMessage reqErr = http_request_parse(req, this->buffer, sdslen(this->buffer));
        if (reqErr){
            LOG_ERROR("Error parsing request: %s", reqErr);
            http_request_delete(req);
            sdsclear(this->buffer);// conn object conn.buffer gets reused
            close(client_fd);
            continue;
        }

        // TODO: Read METHOD and pass to funcition in TRIE (TRIE not implemented yet)

        // CLEAN
        sdsclear(this->buffer);
        http_request_delete(req);
        close(client_fd);
    }
}

void http_connection_delete(http_connection *this) {
    if (this) {
        close(this->fd);
        if (this->buffer)
            sdsfree(this->buffer);
        free(this);
    }
}
