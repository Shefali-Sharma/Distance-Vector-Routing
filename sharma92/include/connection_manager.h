#ifndef CONNECTION_MANAGER_H_
#define CONNECTION_MANAGER_H_

extern fd_set master_list, watch_list;
extern int head_fd;
extern int control_socket, router_socket, data_socket;
extern struct timeval objtime;

int set_UDP_server_socket();
void init();

#endif
