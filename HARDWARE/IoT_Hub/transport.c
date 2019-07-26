/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *    Sergio R. Caprile - "commonalization" from prior samples and/or documentation extension
 *******************************************************************************/
 
#include "stm32f10x_usart.h"
#include "usart.h"
#include "SIM800C_IoT_Hub.h"

/**
This simple low-level implementation assumes a single connection for a single thread. Thus, a static
variable is used for that connection.
On other scenarios, the user must solve this by taking into account that the current implementation of
MQTTPacket_read() has a function pointer for a function call to get the data to a buffer, but no provisions
to know the caller or other indicator (the socket id): int (*getfn)(unsigned char*, int)
*/


int transport_sendPacketBuffer(unsigned char* buf, int buflen)
{
    send_SIM800C_TCP_data(buf, buflen);

    return buflen;
}

int transport_getdata(unsigned char* buf, int count)
{   
    int i = 0,
        j = 0,
        len = 0;
    
    int check_disconnect_index = 0;
    
    for (i = 0; i < count; i++)
    {
        // check MQTT index from buffer end to buffer header
        if (MQTT_deal_with_buffer_index == SIM800C_BUFFER_SIZE)
            MQTT_deal_with_buffer_index = 0;
        
        // SIM800C_recv_buffer may be not have any date
        if (MQTT_deal_with_buffer_index == SIM800C_recv_buffer_index) {
            delay_ms(10);
            i--;
            continue;
        }
        
        len = sizeof(MQTT_disconnect_str) / sizeof(MQTT_disconnect_str[0]);
        check_disconnect_index = MQTT_deal_with_buffer_index;
        // if connect close, will get "\r\nCLOSED\r\n", we need to reconnect server
        for (j = 0; j < len; j++) {
            // check disconnect index from buffer end to buffer header
            if (check_disconnect_index == SIM800C_BUFFER_SIZE)
                check_disconnect_index = 0;
            
            // SIM800C_recv_buffer may be not have any date
            if (check_disconnect_index == SIM800C_recv_buffer_index) {
                delay_ms(10);
                j--;
                continue;
            }
            
            if (MQTT_disconnect_str[j] != SIM800C_recv_buffer[check_disconnect_index++])
                break;
        }
               
        if (j == len) {
            connected_server = false;
            Logln(D_ERROR, "Server CLOSED.");
        }
        
        len = sizeof(MQTT_disconnect_str) / sizeof(MQTT_disconnect_str[0]);
        check_disconnect_index = MQTT_deal_with_buffer_index;
        // if connect close, will get "\r\n+PDP DEACT\r\n", we need to reconnect server
        for (j = 0; j < len; j++) {
            // check disconnect index from buffer end to buffer header
            if (check_disconnect_index == SIM800C_BUFFER_SIZE)
                check_disconnect_index = 0;
            
            // SIM800C_recv_buffer may be not have any date
            if (check_disconnect_index == SIM800C_recv_buffer_index) {
                delay_ms(10);
                j--;
                continue;
            }
            
            if (MQTT_deact_connect_str[j] != SIM800C_recv_buffer[check_disconnect_index++])
                break;
        }
        
        if (j == len) {
            connected_server = false;
            Logln(D_ERROR, "Server Deactived.");
        }
        
        buf[i] = SIM800C_recv_buffer[MQTT_deal_with_buffer_index++];
    }
    
    return i;
}

/**
return >=0 for a socket descriptor, <0 for an error code
@todo Basically moved from the sample without changes, should accomodate same usage for 'sock' for clarity,
removing indirections
*/
int transport_open()
{
    return connect_server();
}

int transport_close()
{
    disconnect_server();
    return 0;
}
