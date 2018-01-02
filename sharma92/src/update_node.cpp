/**
 * @update_node
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <vector>
#include <inttypes.h>
#include <sys/time.h>
#include <iostream>
#include "../include/global.h"
#include "../include/network_util.h"
#include "../include/control_header_lib.h"
#include "../include/author.h"
#include "../include/init_res.h"
#include "../include/update_node.h"
#include "../include/connection_manager.h"
#include "../include/routing_table.h"
using namespace std;

//Reference: http://man7.org/linux/man-pages/man3/getaddrinfo.3.html
//Reference: https://www.cse.buffalo.edu/faculty/dimitrio/courses/cse4589_f17/material/recs/server.c
struct addrinfo hints;
struct addrinfo *result, *rp;
updatePacket received_packet;

int neighbour_index(int j){
    int k;
    for (k = 0; k < num_routers; k++){
        if(my_updatepacket.router[k].routerID == j)
        break;
    }
    return k;
    
}


void update_receivefrom_nodes(int sock_index){
    
    //Sending Updates to all the neighbours
    int udpSocket, nBytes;
    int sz = sizeof (my_updatepacket);
    char buffer[sz];
    struct sockaddr_in serverAddr, clientAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size;
    
    
    addr_size = sizeof serverStorage;
    
    //printf("Before receive\n");
    nBytes = recvfrom(sock_index, buffer, sz, 0, (struct sockaddr *)&serverStorage, &addr_size);
    if(nBytes<=0)
    {
        printf("error in receiving\n");
    }
    
    struct updatePacket *received_packet = (struct updatePacket *) buffer;
    
    
    uint16_t source_cost, sourceID;
    int idx;
    
    for( int i = 0; i < num_routers; i++){
        //cout << "\n my_updatepacket.source_router_port : " << my_updatepacket.source_router_port;
        //cout << "\n received_packet.router[i].routerPort : " << ntohs(received_packet.router[i].routerPort);
        //cout << "\n received_packet.router[i].router_cost : " << ntohs(received_packet.router[i].router_cost);
        
        if(my_updatepacket.source_router_port == ntohs(received_packet->router[i].routerPort)){
            source_cost = ntohs(received_packet->router[i].router_cost);
            //cout<< "\n Source cost " << source_cost << endl;
        }
        if(ntohs(received_packet->router[i].router_cost) == (0)){
            sourceID = ntohs(received_packet->router[i].routerID);
            idx = i;
        }
    }
    
    cout << "\n Source Cost: " << source_cost << endl;
    cout << "\n SourceID: " << sourceID << endl;
    
    
    for(int x =0; x < num_routers; x++){
        
        if(my_updatepacket.router[x].router_cost > source_cost + ntohs(received_packet->router[x].router_cost))
        {
            my_updatepacket.router[x].router_cost = source_cost + ntohs(received_packet->router[x].router_cost);
            routing_table_obj[x].router_cost = htons(my_updatepacket.router[x].router_cost);
            routing_table_obj[x].nextHopID = htons(sourceID);
        }
        
    }
    
    for(int i = 0; i < num_routers ; i++){
        std::cout << routing_table_obj[i].routerID << "\t";
        std::cout << routing_table_obj[i].router_cost << "\t";
        std::cout << routing_table_obj[i].nextHopID << "\t";
        std::cout << "\n";
    }
    
    std::cout << "\n";
    
}




void update_sendto_nodes(){
    
    int udpSocket1 = socket(AF_INET, SOCK_DGRAM, 0);
    char *buffer = (char *)malloc(68);
    
    struct updatePacket *send_packet = (struct updatePacket *) buffer;
    send_packet->num_update_fields = htons(num_routers);
    send_packet->source_router_port = htons(my_updatepacket.source_router_port);
    send_packet->sourceIP = htons(my_updatepacket.sourceIP);
    
    for( int i = 0; i< num_routers; i++){
        send_packet->router[i].routerIP = htonl(my_updatepacket.router[i].routerIP);
        send_packet->router[i].routerPort = htons(my_updatepacket.router[i].routerPort);
        send_packet->router[i].padding = htons(my_updatepacket.router[i].padding);
        send_packet->router[i].routerID = htons(my_updatepacket.router[i].routerID);
        send_packet->router[i].router_cost = htons(my_updatepacket.router[i].router_cost);
    }
    
    for( int i = 0; i < num_routers; i++){
        cout<<"\n Next hop ID: " << ntohs(routing_table_obj[i].nextHopID) << endl;
        cout<<"\n My Router ID: " << my_router.routerID << endl;
        if(((ntohs(routing_table_obj[i].nextHopID)) != my_router.routerID) && (ntohs(routing_table_obj[i].nextHopID) != 65535)){
            //cout<<"Next hop ID: "<<ntohs(routing_table_obj[i].nextHopID) <<endl;
            int y = neighbour_index(ntohs(routing_table_obj[i].nextHopID));
            cout<< "\n Y: "<<y;
            cout<< "\n Port: "<< my_updatepacket.router[y].routerPort<<endl;
            struct sockaddr_in serverAddr;
            //struct sockaddr_storage serverStorage;
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_port = htons(my_updatepacket.router[y].routerPort);
            
            serverAddr.sin_addr.s_addr = htonl(my_updatepacket.router[y].routerIP);//inet_addr(my_updatepacket.router[y].routerIP);
            //serverAddr.sin_addr.s_addr = inet_addr((char*)my_ip_str.c_str());
            
            socklen_t addr_size;
            addr_size = sizeof serverAddr;
            
            //std::cout<< "IP = " << my_neighbours[i].router_IP << std::endl;
            
            // memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
            
            //printf("sending data\n");
            int test = sendto(udpSocket1, buffer, 68, 0, (struct sockaddr *)&serverAddr, addr_size);
            //printf("\n testing send = %d", test);
        }
    }
    free(buffer);
}

/*
 int udpSocket1 = socket(AF_INET, SOCK_DGRAM, 0);
 char *buffer = (char *)malloc(68);
 
 struct updatePacket *send_packet = (struct updatePacket *) buffer;
 send_packet->num_update_fields = htons(num_routers);
 send_packet->source_router_port = htons(my_updatepacket.source_router_port);
 send_packet->sourceIP = htons(my_updatepacket.sourceIP);
 
 for( int i = 0; i< num_routers; i++){
 send_packet->router[i].routerIP = htonl(my_updatepacket.router[i].routerIP);
 send_packet->router[i].routerPort = htons(my_updatepacket.router[i].routerPort);
 send_packet->router[i].padding = htons(my_updatepacket.router[i].padding);
 send_packet->router[i].routerID = htons(my_updatepacket.router[i].routerID);
 send_packet->router[i].router_cost = htons(my_updatepacket.router[i].router_cost);
 }
 
 for( int i = 0; i < my_neighbours.size(); i++){
 if(routing_table_obj[i].nextHopID != my_router.routerID && routing_table_obj[i].nextHopID != 65535){
 int y = neighbour_index(routing_table_obj[i].nextHopID);
 struct sockaddr_in serverAddr;
 //struct sockaddr_storage serverStorage;
 serverAddr.sin_family = AF_INET;
 serverAddr.sin_port = htons(my_updatepacket.router[y].routerPort);
 
 //Reference: https://stackoverflow.com/questions/1680365/integer-to-ip-address-c
 uint32_t routerIP_uint32 =my_updatepacket.router[y].routerIP;
 unsigned char bytes[4];
 bytes[0] = routerIP_uint32 & 0xFF;
 bytes[1] = (routerIP_uint32 >> 8) & 0xFF;
 bytes[2] = (routerIP_uint32 >> 16) & 0xFF;
 bytes[3] = (routerIP_uint32 >> 24) & 0xFF;
 //printf("%d.%d.%d.%d\n", bytes[3], bytes[2], bytes[1], bytes[0]);
 
 
 std::stringstream ss;
 ss<<bytes[0];
 std::string my_ip_str = ss.str();
 ss.str(std::string());
 
 ss<<bytes[1];
 my_ip_str.append(ss.str());
 ss.str(std::string());
 
 ss<<bytes[2];
 my_ip_str.append(ss.str());
 ss.str(std::string());
 
 ss<<bytes[3];
 my_ip_str.append(ss.str());
 ss.str(std::string());
 
 //serverAddr.sin_addr.s_addr = inet_addr(my_updatepacket.router[y].routerIP);
 serverAddr.sin_addr.s_addr = inet_addr((char*)my_ip_str.cpp.c_str());
 
 socklen_t addr_size;
 addr_size = sizeof serverAddr;
 
 //std::cout<< "IP = " << my_neighbours[i].router_IP << std::endl;
 
 // memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
 
 //printf("sending data\n");
 int test = sendto(udpSocket1, buffer, 68, 0, (struct sockaddr *)&serverAddr, addr_size);
 //printf("\n testing send = %d", test);
 }
 */



/*
 //Reference: https://stackoverflow.com/questions/1680365/integer-to-ip-address-c
 uint32_t routerIP_uint32 = my_updatepacket.router[y].routerIP;
 unsigned char bytes[4];
 bytes[0] = routerIP_uint32 & 0xFF;
 bytes[1] = (routerIP_uint32 >> 8) & 0xFF;
 bytes[2] = (routerIP_uint32 >> 16) & 0xFF;
 bytes[3] = (routerIP_uint32 >> 24) & 0xFF;
 //printf("%d.%d.%d.%d\n", bytes[3], bytes[2], bytes[1], bytes[0]);
 
 cout << "\n IP 32 int" <<(int)routerIP_uint32;
 std::stringstream ss;
 ss<<bytes[0];
 std::string my_ip_str = ss.str();
 ss.str(std::string());
 
 cout<< bytes[0] << endl;
 cout<< my_ip_str << endl;
 
 ss<<bytes[1];
 my_ip_str.append(ss.str());
 ss.str(std::string());
 
 cout<< bytes[0] << endl;
 cout<< my_ip_str << endl;
 
 ss<<bytes[2];
 my_ip_str.append(ss.str());
 ss.str(std::string());
 
 cout<< bytes[0] << endl;
 cout<< my_ip_str << endl;
 
 ss<<bytes[3];
 my_ip_str.append(ss.str());
 ss.str(std::string());
 
 cout<< bytes[0] << endl;
 cout<< my_ip_str << endl;
 
 cout<< "\n IP " << my_ip_str << endl;
 */
