/*
 * Copyright (C) 2018  Diogo Justen. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include "net/sock/udp.h"

#define SERVER_BUFFER_SIZE      (64)

static bool server_running = false;
static char server_stack[THREAD_STACKSIZE_DEFAULT];

void *_udp_server(void *args)
{
    sock_udp_t sock;
    sock_udp_ep_t remote;
    char server_buffer[SERVER_BUFFER_SIZE];
    int res;

    if (!args) {
        printf("Invalid arguments. Thread finished\n");
        return NULL;
    }

    sock_udp_ep_t server = { .port = (int)args, .family = AF_INET6 };
    memset(&server_buffer, '\0', sizeof(server_buffer));

    if(sock_udp_create(&sock, &server, NULL, 0) < 0) {
        printf("Unable create udp socket. Thread finished\n");
        return NULL;
    }

    server_running = true;
    printf("Success: started UDP server on port %u\n", server.port);

    while (server_running) {

        if ((res = sock_udp_recv(&sock, server_buffer,
                                 sizeof(server_buffer) - 1, SOCK_NO_TIMEOUT,
                                 &remote)) < 0) {
            printf("Error while receiving udp packet\n");
        }
        else {
            server_buffer[res] = '\0';
            if (sock_udp_send(&sock, server_buffer, res, &remote) < 0) {
                printf("Error sending reply\n");
            }
        }
    }

    printf("Success: stoped UDP server on port %u\n", server.port);

    return NULL;
}

int server_start(int argc, char **argv)
{
    int port = 0;

    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return -1;
    }

    port = strtol(argv[1], NULL, 10);
    if (port <= 0) {
        printf("Invalid port %d\n", port);
        return -1;
    }

    if (server_running == true) {
        printf("Server already running\n");
        return -1;
    }

    memset(&server_stack, '\0', sizeof(server_stack));

    if (thread_create(server_stack, sizeof(server_stack), THREAD_PRIORITY_MAIN - 1,
                      THREAD_CREATE_STACKTEST, _udp_server, (void *)port, "UDP Server")
        <= KERNEL_PID_UNDEF) {
        return -1;
    }

    return 0;
}

int server_stop(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    if (server_running == false) {
        printf("Server not running\n");
        return 0;
    }

    server_running = false;

    return 0;
}
