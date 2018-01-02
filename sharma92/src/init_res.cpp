/**
 * @init_res
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
#include <vector>
#include <curl/curl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include "../include/control_header_lib.h"
#include "../include/network_util.h"
#include "../include/init_res.h"
#include "../include/connection_manager.h"
#include "../include/routing_table.h"
using namespace std;

Packet *new_packet;
myDetails my_router;
//Neighbours my_neighbours[5];
std::vector<RouterStruct> my_neighbours;
updatePacket my_updatepacket;
RoutingTableStruct routing_table_obj[5];
uint16_t num_routers;
uint16_t periodic_interval;

void displayMyPacket(){
    for(int i = 0; i<5; i++){
        cout << "\n ID : " << new_packet->router[i].routerID <<endl;
        cout << "\n routerPort1 : " << new_packet->router[i].routerPort1 <<endl;
        cout << "\n router_cost : " << new_packet->router[i].router_cost <<endl;
    }
}

void displayNeighbours(){
    printf("\n Neighbours created : ");
    for(int i=0; i<5; i++){
        cout << "\n " << my_neighbours[i].routerID << endl;
    }
}

void set_neighbours(){
    
    for(int i=0; i< num_routers; i++){
        if(new_packet->router[i].router_cost < 65535 && new_packet->router[i].router_cost > 0){
            my_neighbours.push_back(new_packet->router[i]);
        }
        
        
    }    //displayNeighbours();
    
}

void send_init_response(int sock_index){
    
    uint16_t payload_len, init_len;
    char *cntrl_init_header, *cntrl_init_payload, *cntrl_response;
    
    payload_len = 0;
    cntrl_init_payload = (char *) malloc(payload_len);
    memcpy(cntrl_init_payload, "", payload_len);
    
    cntrl_init_header = create_response_header(sock_index, 1, 0, payload_len);
    
    init_len = CNTRL_RESP_HEADER_SIZE + payload_len;
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

void init_response(int sock_index, char *contrl_payload){
    
    new_packet = (struct Packet*) malloc (sizeof(struct Packet));
    //my_router = (struct myDetails*) malloc (sizeof(struct myDetails));
    num_routers = (uint8_t)contrl_payload[1] | (uint8_t)contrl_payload[0]<< 8;
    periodic_interval = (uint8_t)contrl_payload[3] | (uint8_t)contrl_payload[2]<< 8;
    objtime.tv_sec = periodic_interval;
    //timeobj.tvsec = new_packet->periodic_interval
    my_updatepacket.num_update_fields = htons(num_routers);
    
    for(int i=0, k=0; i<(num_routers)*12 && k<5; i+=12, k++){
        
        //new_packet->router[k] = (struct RouterStruct*)malloc(sizeof(struct RouterStruct));
        
        new_packet->router[k].routerID = (uint8_t)contrl_payload[5+i] | (uint8_t)contrl_payload[4+i]<< 8;
        new_packet->router[k].routerPort1 = (uint8_t)contrl_payload[7+i] | (uint8_t)contrl_payload[6+i]<< 8;
        new_packet->router[k].routerPort2 = (uint8_t)contrl_payload[9+i] | (uint8_t)contrl_payload[8+i]<< 8;
        new_packet->router[k].router_cost = (uint8_t)contrl_payload[11+i] | (uint8_t)contrl_payload[10+i]<< 8;
        uint32_t ip = (uint8_t)contrl_payload[15+i] | (uint8_t)contrl_payload[14+i] << 8 | (uint8_t)contrl_payload[13+i] << 16 |  (uint8_t)contrl_payload[12+i] << 24;
        new_packet->router[k].router_IP = ip;
        
        
        routing_table_obj[k].routerID = new_packet->router[k].routerID;
        routing_table_obj[k].padding = 0x0000;
        routing_table_obj[k].router_cost = new_packet->router[k].router_cost;
        if(routing_table_obj[k].router_cost == 65535){
            routing_table_obj[k].nextHopID = 65535;
        }
        else{
            routing_table_obj[k].nextHopID = routing_table_obj[k].routerID;
        }
        
        //Updating Update packet
        my_updatepacket.router[k].routerIP = (new_packet->router[k].router_IP);
        my_updatepacket.router[k].routerPort = (new_packet->router[k].routerPort1);
        my_updatepacket.router[k].padding = 0x0000;
        my_updatepacket.router[k].routerID = (new_packet->router[k].routerID);
        my_updatepacket.router[k].router_cost = (new_packet->router[k].router_cost);
        
        
        
        uint16_t a = (uint8_t)contrl_payload[12+i];
        uint16_t b = (uint8_t)contrl_payload[13+i];
        uint16_t c = (uint8_t)contrl_payload[14+i];
        uint16_t d = (uint8_t)contrl_payload[15+i];
        stringstream sstr;
        sstr << a;
        string IP = sstr.str();
        sstr.str(std::string());
        IP.append(".");
        sstr << b;
        IP.append(sstr.str());
        sstr.str(std::string());
        IP.append(".");
        sstr << c;
        IP.append(sstr.str());
        sstr.str(std::string());
        IP.append(".");
        sstr << d;
        IP.append(sstr.str());
        char *temp = (char *)IP.c_str();
        //printf("\n IP = %s \n", (char *)IP.c_str());
        //printf("\n IP = %s ", (char *)IP.c_str());
        //new_packet->router[k].routerIP = (char *)malloc(256 * sizeof(char));
        strcpy(new_packet->router[k].routerIP, temp); //.assign(IP);
        //printf("\n IP = %s ", (char *)IP.c_str());
        
        if(new_packet->router[k].router_cost == 0x0000){
            my_router.routerID = new_packet->router[k].routerID;
            printf("\n My Router ID: %u \n", my_router.routerID);
            my_router.routerPort1 = new_packet->router[k].routerPort1;
            printf("\n My Port : %u \n", my_router.routerPort1);
            strcpy(my_router.routerIP, new_packet->router[k].routerIP);
            //printf("\n My IP : %s ", my_router.routerIP);
            my_router.router_IP = new_packet->router[k].router_IP;
            //printf("\n My IP (32uint) : %u ", my_router.router_IP);
            
            //Updating Update packet
            my_updatepacket.num_update_fields = num_routers;
            my_updatepacket.source_router_port = (my_router.routerPort1);
            my_updatepacket.sourceIP = (my_router.router_IP);
        }
        
    }
    
    //printf("\n Inside initRoutingTable.Number of routers: %u\n", new_packet->num_routers);
    for(int i = 0; i <num_routers; i++){
        //routing_table_obj[i] = (struct RoutingTableStruct *)malloc(sizeof(struct RoutingTableStruct));
        routing_table_obj[i].routerID = htons(new_packet->router[i].routerID); //printf("\n %u ", routing_table_obj[i].routerID);
        routing_table_obj[i].padding = htons(0); //printf("\n %u", routing_table_obj[i].padding);
        routing_table_obj[i].router_cost = htons(new_packet->router[i].router_cost); //printf("\n %u", routing_table_obj[i].router_cost);
        //routing_table_obj[i].nextHopID = htons(new_packet->router[i].routerID); printf("\n %u", routing_table_obj[i].nextHopID);
        
        if(routing_table_obj[i].router_cost == 65535){
            routing_table_obj[i].nextHopID = htons(0xFFFF);
            //printf("\n Next Hop ID : %u \n", routing_table_obj[i].nextHopID);
        }
        else{
            routing_table_obj[i].nextHopID = htons(new_packet->router[i].routerID);
            //printf("\n Next Hop ID : %u \n", ntohs(routing_table_obj[i].nextHopID));
        }
    }
    
    
    send_init_response(sock_index);
    set_neighbours();
}

std::string getMyPort(){
    stringstream sstr;
    sstr << my_router.routerPort1;
    return sstr.str();
    
}
