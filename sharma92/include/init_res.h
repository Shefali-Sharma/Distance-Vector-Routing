#ifndef INIT_RES_H_
#define INIT_RES_H_
#include <string.h>
#include <string>
#include <iostream>
#include <vector>
//#include <cstdint>

struct myDetails{
    uint16_t routerID;
    uint16_t routerPort1; //This is my port
    uint32_t router_IP;
    char routerIP[256];
};

struct Neighbours{
    uint16_t routerID;
    uint16_t routerPort1;
    uint32_t routerIP;
    uint16_t router_cost;
    char router_IP[256];
};

struct RouterStruct{
    uint16_t routerID;
    uint16_t routerPort1;
    uint16_t routerPort2;
    uint16_t router_cost;
    char routerIP[256];
    uint32_t router_IP;
    
};


struct Packet{
    RouterStruct router[5];
};

struct updateRouter{
    uint32_t routerIP;
    uint16_t routerPort;
    uint16_t padding;
    uint16_t routerID;
    uint16_t router_cost;
};

struct updatePacket{
    uint16_t num_update_fields;
    uint16_t source_router_port;
    uint32_t sourceIP;
    updateRouter router[5];
};

extern Packet *new_packet;
extern myDetails my_router;
extern std::vector<RouterStruct> my_neighbours;
extern updatePacket my_updatepacket;
extern updatePacket received_packet;
extern uint16_t num_routers;
extern uint16_t periodic_interval;

std::string getMyPort();

void init_response(int sock_index, char *cntrl_payload);

#endif
