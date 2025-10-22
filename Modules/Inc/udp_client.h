#pragma once

#include <netinet/in.h>

typedef struct {
    int sockfd;
    struct sockaddr_in dest;
    int timeout_ms;
    int retries;
} udp_client_t;

int udp_client_init(udp_client_t *c, const char *ip, int port);

int udp_client_send_and_wait(udp_client_t *c, 
                             const void *data,
                             size_t data_len,
                             const void *resp_buf, 
                             size_t bufsize);

int udp_client_send_and_wait_str(udp_client_t *c, const char *msg, 
                             char *resp_buf, size_t bufsize);

int udp_client_send_nb(udp_client_t *c, const void *buf, size_t len);

int udp_client_try_recv(udp_client_t *c, char *resp_buf, size_t bufsize);

int udp_client_set_timeout(udp_client_t *c, int timeout_ms);

int udp_client_set_retry_count(udp_client_t *c, int retry_count);

void udp_client_close(udp_client_t *c);


