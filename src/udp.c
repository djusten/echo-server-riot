#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "net/sock/udp.h"
#include "net/ipv6/addr.h"
#include "thread.h"
#include "xtimer.h"

#define SERVER_MSG_QUEUE_SIZE   (8)
#define SERVER_BUFFER_SIZE      (64)

static bool server_running = false;
static sock_udp_t sock;
static char server_buffer[SERVER_BUFFER_SIZE];
static char server_stack[THREAD_STACKSIZE_DEFAULT];
static msg_t server_msg_queue[SERVER_MSG_QUEUE_SIZE];

void *_udp_server(void *args)
{
    sock_udp_ep_t server = { .port = atoi(args), .family = AF_INET6 };
    msg_init_queue(server_msg_queue, SERVER_MSG_QUEUE_SIZE);

    if(sock_udp_create(&sock, &server, NULL, 0) < 0) {
        return NULL;
    }

    server_running = true;
    printf("Success: started UDP server on port %u\n", server.port);

    while (1) {
        int res;
        sock_udp_ep_t remote;

        if ((res = sock_udp_recv(&sock, server_buffer,
                                 sizeof(server_buffer) - 1, SOCK_NO_TIMEOUT,
                                 &remote)) < 0) {
            puts("Error while receiving");
        }
        else if (res == 0) {
            puts("No data received");
        }
        else {
            server_buffer[res] = '\0';
            printf("Recvd: %s\n", server_buffer);
            xtimer_usleep(1000000);
            if (sock_udp_send(&sock, server_buffer, res, &remote) < 0) {
                puts("Error sending reply");
            }
            puts("Sent");
        }
    }

    return NULL;
}

int udp_server(int argc, char **argv)
{
    if (argc != 2) {
        puts("Usage: udps <port>");
        return -1;
    }

    if ((server_running == false) &&
        thread_create(server_stack, sizeof(server_stack), THREAD_PRIORITY_MAIN - 1,
                      THREAD_CREATE_STACKTEST, _udp_server, argv[1], "UDP Server")
        <= KERNEL_PID_UNDEF) {
        return -1;
    }

    return 0;
}