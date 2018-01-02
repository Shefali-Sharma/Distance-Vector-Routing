#ifndef UPDATE_NODE_H_
#define UPDATE_NODE_H_
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

extern struct addrinfo hints;
extern struct addrinfo *result, *rp;;

void update_receivefrom_nodes(int sock_index);
void update_sendto_nodes();

#endif

