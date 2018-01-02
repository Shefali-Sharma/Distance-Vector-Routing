#ifndef ROUTUNG_TABLE_H_
#define ROUTING_TABLE_H_

struct RoutingTableStruct{
    uint16_t routerID;
    uint16_t padding;
    uint16_t nextHopID;
    uint16_t router_cost;
};
extern RoutingTableStruct routing_table_obj[5];

void send_routing_table(int sock_index);
void updateRoutingTable(int sock_index, char *cntrl_payload);
void crash_response(int sock_index);

#endif
