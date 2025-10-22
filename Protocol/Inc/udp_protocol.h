#pragma once

#include <stdint.h>

#define UDP_LED_HEADER_1 0xA1
#define UDP_LED_HEADER_2 0xF1

#define UDP_SENSOR_HEADER_1 0xA2
#define UDP_SENSOR_HEADER_2 0xF2


typedef struct udp_protocol
{
    uint8_t header[2];
    uint8_t command;
    uint8_t dlc;
    void * data; 
}udp_protocol_t;

typedef enum
{
    UDP_CMD_LED_CONTROL = 0x00,
    UDP_CMD_SENSOR_DATA = 0x01,
    UDP_CMD_DEVICE_STATUS = 0x02,
}udp_commend_e;

typedef enum
{
    UDP_DEV_LED = 0,
    UDP_DEV_SENSOR = 1,
}udp_device_type_e;

int udp_msg_send_resv(udp_client_t *client,
                      udp_device_type_e dev,
                      uint8_t *send_data,
                      size_t send_len,
                      uint8_t *resv_data,
                      size_t resv_len);