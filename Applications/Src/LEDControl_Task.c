#include "LEDControl_Task.h"
#include "udp_client.h"
#include "udp_protocol.h"
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>

extern sem_t led_semaphore;
extern udp_client_t client;
uint8_t resp[64];
char *led_on_msg = "LED_DS1=1\n";
char *led_off_msg = "LED_DS1=0\n";
uint8_t data[2] = {1, 0};

void *LEDControl_Task(void *argument)
{
    void **thread_arg = (void **)argument;
    while (1)
    {
        /*
        sem_wait(&led_semaphore);
        int res = udp_client_send_and_wait(&client, led_on_msg, resp, sizeof(resp));
        sem_post(&led_semaphore);
        if(res < 0) {
            perror("LED ON 发送或接收失败");
        }
        else
            printf("LED ON 收到响应: %s\n", resp);
        
        usleep(500 * 1000);
        sem_wait(&led_semaphore);
        res = udp_client_send_and_wait(&client, led_off_msg, resp, sizeof(resp));
        sem_post(&led_semaphore);
        if(res < 0) {
            perror("LED OFF 发送或接收失败");
        }
        else
            printf("LED OFF 收到响应: %s\n", resp);
        
        usleep(500 * 1000);
        */
        data[0] = 1;
        data[1] = 0;
        udp_msg_send_resv(&client,
                          UDP_DEV_LED,
                          data,
                          sizeof(data),
                          (uint8_t *)resp,
                          sizeof(uint8_t) * 2);
        printf("LED ON 收到响应: %d%d\n", resp[0],resp[1]);
        usleep(500 * 1000);
        data[0] = 0;
        data[1] = 1;
        udp_msg_send_resv(&client,
                          UDP_DEV_LED,
                          data,
                          sizeof(data),
                          (uint8_t *)resp,
                          sizeof(uint8_t) * 2);
        printf("LED OFF 收到响应: %d%d\n", resp[0],resp[1]);
        usleep(500 * 1000);
    }
}