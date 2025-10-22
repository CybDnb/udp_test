#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include "udp_client.h"
#include "LEDControl_Task.h"

udp_client_t client;
sem_t led_semaphore;

int main(int argc, char *argv[]) {
    /*
    if (argc < 3) {
        fprintf(stderr, "用法: %s <IP> <PORT>\n", argv[0]);
        return 1;
    }
    */
    int atoi_res = atoi(argv[2]);
    if (atoi_res <= 0 || atoi_res > 65535) {
        fprintf(stderr, "无效的端口号: %s\n", argv[2]);
        return 1;
    }
    if (udp_client_init(&client, argv[1], atoi_res) < 0)
        return 1;
    sem_init(&led_semaphore, 0, 1);
    pthread_t led_thread;
    pthread_create(&led_thread, NULL, (void*)LEDControl_Task, (void *)argv);
    

    char gets_buf[256];
    char resp[256];
    printf("请输入要发送的信息或命令(输入quit退出):\n"); 
    udp_client_set_retry_count(&client, 3);
    while(1)
    {
        
        if(fgets(gets_buf, sizeof(gets_buf), stdin) == NULL) {
            printf("读取输入失败, 请重试.\n");
            continue;
        }

        size_t len = strlen(gets_buf);
        if (len > 0 && gets_buf[len - 1] == '\n')
        {
            gets_buf[len - 1] = '\0';   
        }
        if (strcmp(gets_buf, "quit") == 0) {
            break;
        }
        char sendbuf[256];
        int m = snprintf(sendbuf, sizeof(sendbuf), "%s\n", gets_buf);
        if(m < 0 || (size_t)m >= sizeof(sendbuf)) {
            puts("输入信息过长, 请重新输入.");
            continue;
        }
        sem_wait(&led_semaphore);
        int res = udp_client_send_and_wait_str(&client, sendbuf, resp, sizeof(resp));
        sem_post(&led_semaphore);
        if(res < 0) {
            perror("发送或接收失败");
            continue;
        }
        else
            printf("收到响应: %s\n", resp);
    }    
    pthread_cancel(led_thread);
    sem_destroy(&led_semaphore);
    udp_client_close(&client);
    return 0;
}
