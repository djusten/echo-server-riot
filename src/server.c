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
#include "event/timeout.h"
#include "net/sock/udp.h"

#define SERVER_BUFFER_SIZE  (64)
#define MAIN_QUEUE_SIZE     (64)
#define MAX_EVENTS          (128)
#define MAX_BUFFER_LEN      (64)
#define REPLY_DELAY         (1 * US_PER_SEC)
#define REPLY_PORT          (1111)
#define SOCKET_TIMEOUT      (1 * US_PER_SEC)

typedef struct {
    event_t super;
    event_timeout_t event_timeout;
    sock_udp_ep_t remote;
    sock_udp_t *sock;
    char data[MAX_BUFFER_LEN];
} server_event_t;

static bool server_running;
static char server_stack[THREAD_STACKSIZE_DEFAULT];
static char delay_stack[THREAD_STACKSIZE_DEFAULT];
static int num_events;
server_event_t custom_event_list[MAX_EVENTS];

static void custom_callback(event_t *event)
{
    assert(event);

    server_event_t *custom_event = (server_event_t *) event;

    int buffer_len = strlen(custom_event->data);

    if (sock_udp_send(custom_event->sock, custom_event->data,
                buffer_len, &custom_event->remote) < 0) {
        printf("Error sending reply delayed\n");
    }
}

void *_delayed_server(void *args)
{
    assert(args);

    event_queue_t *queue = (event_queue_t *) args;

    while(server_running){

        event_t *event = event_get (queue);
        if (event) {
            event->handler(event);
        }

        xtimer_usleep(1000); //FIXME Use thread_wakeup()
    }

    printf("Success: stoped udp delayed server thread.\n");

    return NULL;
}

int add_queue_delay(event_queue_t *queue, sock_udp_ep_t *remote,
                        sock_udp_t *sock, char *server_data)
{
    assert(server_data);
    assert(remote);
    assert(sock);
    assert(queue);

    memset(&custom_event_list[num_events], 0, sizeof(server_event_t));

    custom_event_list[num_events].super.handler = custom_callback;
    custom_event_list[num_events].sock = sock;
    memcpy(&custom_event_list[num_events].remote, remote, sizeof(sock_udp_ep_t));

    snprintf(custom_event_list[num_events].data,
            sizeof(custom_event_list[num_events].data),
            "%s", server_data);

    event_timeout_init(&custom_event_list[num_events].event_timeout,
                         queue, (event_t *) &custom_event_list[num_events]);

    event_timeout_set(&custom_event_list[num_events].event_timeout, REPLY_DELAY);

    num_events++;
    num_events &= (MAX_EVENTS - 1);

    return 0;
}

void *_udp_server(void *args)
{
    assert(args);

    sock_udp_t sock_rx, sock_tx;
    sock_udp_ep_t remote;
    event_queue_t queue;
    char server_data[SERVER_BUFFER_SIZE];
    int res;

    if (!args) {
        printf("Invalid arguments. Thread finished\n");
        return NULL;
    }

    memset(&server_data, '\0', sizeof(server_data));

    sock_udp_ep_t server_rx = { .port = (int)args, .family = AF_INET6 };
    if(sock_udp_create(&sock_rx, &server_rx, NULL, 0) < 0) {
        printf("Unable create RX udp socket. Thread finished\n");
        return NULL;
    }

    server_running = true;

    printf("Success: started UDP server on port %u\n", server_rx.port);

    sock_udp_ep_t server_tx = { .port = REPLY_PORT, .family = AF_INET6 };
    if(sock_udp_create(&sock_tx, &server_tx, NULL, 0) < 0) {
        printf("Unable create TX udp socket. Thread finished\n");
        sock_udp_close(&sock_rx);
        return NULL;
    }

    event_queue_init(&queue);

    if (thread_create(delay_stack, sizeof(delay_stack), THREAD_PRIORITY_MAIN - 1,
                      THREAD_CREATE_STACKTEST, _delayed_server, &queue, "Delay Server")
        <= KERNEL_PID_UNDEF) {
        printf("Unable to create deleyed reply thread. Stopping threads\n");
        server_running = false;
    }

    while (server_running) {

        if ((res = sock_udp_recv(&sock_rx, server_data,
                                 sizeof(server_data) - 1, SOCKET_TIMEOUT,
                                 &remote)) < 0) {
            if (res != -ETIMEDOUT) {
                printf("Error while receiving udp packet\n");
                continue;
            }

            if (!server_running) {
                break;
            }
        }
        else {
            server_data[res] = '\0';

            if (add_queue_delay(&queue, &remote, &sock_tx, server_data) < 0) {
                printf("Unable add event in the queue");
            }
        }
    }

    sock_udp_close(&sock_tx);
    sock_udp_close(&sock_rx);

    printf("Success: stoped udp server thread.\n");

    return NULL;
}

int server_start(int argc, char **argv)
{
    int port = 0;

    num_events = 0;
    server_running = false;

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
        printf("Unable create udp_server thread\n");
        return -1;
    }

    return 0;
}

int server_stop(void)
{
    if (server_running == false) {
        printf("Server not running\n");
        return 0;
    }

    server_running = false;

    return 0;
}
