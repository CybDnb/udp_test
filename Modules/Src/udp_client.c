#include "udp_client.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>

static int set_nonblocking(int fd) {
    int fl = fcntl(fd, F_GETFL, 0);
    if (fl < 0) return -1;
    return fcntl(fd, F_SETFL, fl | O_NONBLOCK);
}

int udp_client_init(udp_client_t *c, const char *ip, int port) {
    memset(c, 0, sizeof(*c));
    c->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (c->sockfd < 0) return -1;
    c->timeout_ms = 1000;
    c->retries = 0;  
    c->dest.sin_family = AF_INET;
    c->dest.sin_port   = htons((uint16_t)port);
    if (inet_pton(AF_INET, ip, &c->dest.sin_addr) != 1) {
        perror("inet_pton");
        close(c->sockfd);
        return -1;
    }
    if(connect(c->sockfd, (struct sockaddr*)&c->dest, sizeof(c->dest)) < 0) {
        perror("connect");
        close(c->sockfd);
        return -1;
    }
    set_nonblocking(c->sockfd);
    return 0;
}

int udp_client_send_and_wait(udp_client_t *c, 
                                 const void *data,
                                 size_t data_len,
                                 const void *resp_buf, 
                                 size_t bufsize)
{
    if(c->sockfd < 0 || !data || data_len == 0 || !resp_buf || bufsize == 0) 
    {
        errno = EINVAL;
        return -1;
    }

    int retries = c->retries;
    while (retries >= 0)
    {
        for (;;) {
            ssize_t s = send(c->sockfd, data, data_len, 0);
            if (s >= 0) break;
            if (errno == EINTR) continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) 
            {
                usleep(5 * 1000);      
                continue;
            }
            perror("send"); 
            return -1;
        }

        int select_status = 0;
        for (;;) 
        {
            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(c->sockfd, &rfds);

            struct timeval tv;
            tv.tv_sec  = c->timeout_ms / 1000;
            tv.tv_usec = (c->timeout_ms % 1000) * 1000;

            int r = select(c->sockfd + 1, &rfds, NULL, NULL, &tv);
            if (r > 0) 
            {
                if (!FD_ISSET(c->sockfd, &rfds)) { errno = EIO; return -1; }
                select_status = 0;
                break;                               
            }
            if (r == 0) 
            { 
                select_status = 1;
                break; 
            } 
            if (errno == EINTR) continue;       
            select_status = -1;
            break;
        }
        if (select_status == 1) { 
            retries--;
            if (retries < 0) {
                puts("等待响应超时，重试次数已用尽.");
                errno = ETIMEDOUT;
                return -1;
            }            
            printf("等待响应超时，准备重试（还剩 %d/%d 次）...\n", retries, c->retries);
            continue; 
        }
        if (select_status == -1) {
             return -1;  
        }

        for (;;) {
            ssize_t n = recv(c->sockfd, resp_buf, bufsize, 0);

            if (n >= 0) {
                return (int)n;
            }

            if (errno == EINTR) continue; 

            perror("recv");
            return -1; 
        }

    } 
    
    errno = ETIMEDOUT;
    return -1;
}

int udp_client_send_and_wait_str(udp_client_t *c, const char *msg,
                             char *resp_buf, size_t bufsize)
{
    if(c->sockfd < 0 || !resp_buf || bufsize == 0) 
    {
        errno = EINVAL;
        return -1;
    }
    int retries = c->retries;
    size_t msglen = strlen(msg);
    while (retries >= 0)
    {
        for (;;) {
            ssize_t s = send(c->sockfd, msg, msglen, 0);
            if (s >= 0) break;
            if (errno == EINTR) continue;   
            if (errno == EAGAIN || errno == EWOULDBLOCK) 
            {
                usleep(5 * 1000);               
                continue;
            }
            perror("send"); 
            return -1;
        }
        int select_status = 0;
        
        for (;;) 
        {
            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(c->sockfd, &rfds);

            struct timeval tv;
            tv.tv_sec  = c->timeout_ms / 1000;
            tv.tv_usec = (c->timeout_ms % 1000) * 1000;

            int r = select(c->sockfd + 1, &rfds, NULL, NULL, &tv);
            if (r > 0) 
            {
                if (!FD_ISSET(c->sockfd, &rfds)) { errno = EIO; return -1; }
                select_status = 0;
                break;                              
            }                    
            if (r == 0) 
            { 
                select_status = 1;
                break; 
            } 
            if (errno == EINTR) continue;       
            perror("select"); 
            select_status = -1;
            break;
        }

        if (select_status == 1) { 
            retries--;
            if (retries < 0) {
                puts("等待响应超时，重试次数已用尽.");
                errno = ETIMEDOUT;
                return -1;
            }
            printf("等待响应超时，准备重试（还剩 %d/%d 次）...\n", retries, c->retries);
            continue; 
        }
        if (select_status == -1) { 
             return -1;
        }

        for (;;) {
            ssize_t n = recv(c->sockfd, resp_buf, bufsize - 1, 0);
            if (n >= 0) {
                resp_buf[(n > 0) ? n : 0] = '\0';
                return (int)n;
            }
            if (errno == EINTR) continue;
            perror("recv");
            return -1;
        }
    }    
    errno = ETIMEDOUT;
    return -1;
}

void udp_client_close(udp_client_t *c) {
    if (c->sockfd >= 0) close(c->sockfd);
}

int udp_client_send_nb(udp_client_t *c, const void *buf, size_t len) {
    if (c->sockfd < 0) { errno = EBADF; return -1; }
    for (;;) {
        ssize_t n = send(c->sockfd, buf, len, 0);
        if (n >= 0) return (int)n;         
        if (errno == EINTR) continue;      
        
        return -1;
    }
}

int udp_client_try_recv(udp_client_t *c, char *resp_buf, size_t bufsize) {
    if (c->sockfd < 0 || bufsize == 0) { errno = EINVAL; return -1; }
    for (;;) {
        ssize_t n = recv(c->sockfd, resp_buf, bufsize - 1, 0);
        if (n > 0) { resp_buf[n] = '\0'; return (int)n; }  
        if (n == 0) { resp_buf[0] = '\0'; return 0; }      
        if (errno == EINTR) continue;                      
        if (errno == EAGAIN || errno == EWOULDBLOCK) return 0; 
    }
}

int udp_client_set_timeout(udp_client_t *c, int timeout_ms) {
    if (c->sockfd < 0 || timeout_ms < 0) {
        errno = EINVAL;
        return -1;
    }
    c->timeout_ms = timeout_ms;
    return 0;
}

int udp_client_set_retry_count(udp_client_t *c, int retry_count) {
    if (c->sockfd < 0 || retry_count < 0) {
        errno = EINVAL;
        return -1;
    }
    c->retries = retry_count;
    return 0;
}