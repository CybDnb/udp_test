#include "udp_client.h"
#include "udp_protocol.h"
#include <stdlib.h>
#include <string.h>

int check_map[][2] = {
    {UDP_LED_HEADER_1, UDP_LED_HEADER_2},
    {UDP_SENSOR_HEADER_1, UDP_SENSOR_HEADER_2},
};

int udp_msg_send_resv(udp_client_t *client,
                      udp_device_type_e dev,
                      uint8_t *send_data,
                      size_t send_len,
                      uint8_t *resv_data,
                      size_t resv_len)
{
    if (send_len > 255) {
        return -2; 
    }

    // header(2) + dlc(1) + data(send_len)
    size_t total_packet_len = 3 + send_len;

    uint8_t *packet_buffer = (uint8_t *)malloc(total_packet_len);
    if (!packet_buffer) {
        return -1;
    }

    packet_buffer[0] = check_map[dev][0]; // Header 1
    packet_buffer[1] = check_map[dev][1]; // Header 2
    packet_buffer[2] = (uint8_t)send_len;  // DLC (Data Length Code)

    if (send_len > 0) {
        memcpy(packet_buffer + 3, send_data, send_len);
    }

    int res = udp_client_send_and_wait(client,
                                       packet_buffer,
                                       total_packet_len,
                                       resv_data,
                                       resv_len); 
    
    free(packet_buffer);
    
    return res;
}