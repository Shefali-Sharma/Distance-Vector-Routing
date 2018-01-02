/**
 * @routing_table
 * @author  Shefali Sharma <sharma92@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sstream>
#include <string.h>
#include <netinet/in.h>
#include "../include/control_header_lib.h"
#include "../include/network_util.h"
#include "../include/init_res.h"
#include "../include/routing_table.h"
#include "../include/connection_manager.h"
using namespace std;

void send_routing_table(int sock_index){
    uint16_t payload_len, init_len;
    char *cntrl_init_header, *cntrl_init_payload, *cntrl_response;
    
    payload_len = sizeof(routing_table_obj);
    cntrl_init_payload = (char *) malloc(payload_len);
    memcpy(cntrl_init_payload, routing_table_obj, payload_len);
    
    cntrl_init_header = create_response_header(sock_index, 2, 0, payload_len);
    
    init_len = CNTRL_RESP_HEADER_SIZE+payload_len;
    cntrl_response = (char *) malloc(init_len);
    /* Copy Header */
    memcpy(cntrl_response, cntrl_init_header, CNTRL_RESP_HEADER_SIZE);
    free(cntrl_init_header);
    /* Copy Payload */
    memcpy(cntrl_response+CNTRL_RESP_HEADER_SIZE, cntrl_init_payload, payload_len);
    free(cntrl_init_payload);
    
    sendALL(sock_index, cntrl_response, init_len);
    
    free(cntrl_response);
    
}

void updateRoutingTable(int sock_index, char *contrl_payload){
    uint16_t routerID = (uint8_t)contrl_payload[1] | (uint8_t)contrl_payload[0]<< 8;
    uint16_t router_cost = (uint8_t)contrl_payload[2] | (uint8_t)contrl_payload[3]<< 8;
    
    uint16_t init_len;
    char *cntrl_init_header;
    
    cntrl_init_header = create_response_header(sock_index, 3, 0, 0);
    
    sendALL(sock_index, cntrl_init_header, 8);
    free(cntrl_init_header);
    
    
}

void crash_response(int sock_index){
    
    uint16_t payload_len, init_len;
    char *cntrl_init_header;
    
    cntrl_init_header = create_response_header(sock_index, 4, 0, 0);
    
    sendALL(sock_index, cntrl_init_header, 8);
    free(cntrl_init_header);
    
    close(sock_index);
    
}













