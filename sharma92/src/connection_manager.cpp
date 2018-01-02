/**
 * @connection_manager
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
 * @section DESCRIPTION
 *
 * Connection Manager listens for incoming connections/messages from the
 * controller and other routers and calls the desginated handlers.
 */

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "../include/connection_manager.h"
#include "../include/global.h"
#include "../include/control_handler.h"
#include "../include/update_node.h"
#include "../include/init_res.h"

fd_set watch_list, master_list;;
int head_fd;
int control_socket, router_socket = -1, data_socket;
struct timeval objtime;
int set_UDP_server_socket(){
    
    struct sockaddr_in udpAddr;
    
    //int server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    int addr_len = sizeof(udpAddr);
    memset((char *)&udpAddr, 0, sizeof (udpAddr));
    /*
     udpAddr.sin_family = AF_INET;
     udpAddr.sin_port = htons(53);
     
     inet_pton(AF_INET, "8.8.8.8", &udpAddr.sin_addr);
     
     connect(server_socket, (struct sockaddr *)&udpAddr, sizeof(udpAddr));
     
     getsockname(server_socket, ((struct sockaddr *)&udpAddr), (unsigned int *) &addr_len);
     
     char *ip = (char*)malloc(100);
     inet_ntop(AF_INET, &(udpAddr.sin_addr), ip, addr_len);
     */
    struct sockaddr_in rout_addr;
    int server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    int my_port_to_bindto = my_router.routerPort1;
    
    if(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (int[]){1}, sizeof(int))<0){
        printf("bind failed.");
    }
    
    rout_addr.sin_family = AF_INET;
    
    //printf("\n My port to bind = %d \n", my_port_to_bindto);
    rout_addr.sin_port = htons(my_port_to_bindto);
    //inet_pton(AF_INET, ip, &rout_addr.sin_addr);
    rout_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    //memset(rout_addr.sin_zero, '\0', sizeof(rout_addr.sin_zero));
    bind(server_socket, (struct sockaddr *) &rout_addr, sizeof(rout_addr));
    router_socket = server_socket;
    
    FD_SET(router_socket, &master_list);
    if(router_socket>head_fd){
        //printf("\n Updating head_fd\n");
        head_fd = router_socket;
    }
    
    //printf("\n Router Sock = %d Head_fd = %d ", router_socket, head_fd);
    
    return server_socket;
    
}


void main_loop()
{
    int selret, sock_index, fdaccept;
    
    while(1){
        watch_list = master_list;
        if(head_fd < router_socket){
            head_fd = router_socket;
        }
        if(router_socket != -1)
        {
            selret = select(head_fd+1, &watch_list, NULL, NULL, &objtime);
        }
        else{
            selret = select(head_fd+1, &watch_list, NULL, NULL, NULL);
        }
        
        if(selret < 0)
        ERROR("select failed.");
        
        if(selret == 0){
            update_sendto_nodes();
            objtime.tv_sec = periodic_interval;
        }
        /* Loop through file descriptors to check which ones are ready */
        for(sock_index=0; sock_index<=head_fd; sock_index+=1){
            //printf("\n Sock_index = %d, Router Socket = %d  Head_fd = %d \n", sock_index, router_socket, head_fd);
            if(FD_ISSET(sock_index, &watch_list)){
                
                /* control_socket */
                if(sock_index == control_socket){
                    //printf("\n Inside control sock \n");
                    fdaccept = new_control_conn(sock_index);
                    
                    /* Add to watched socket list */
                    FD_SET(fdaccept, &master_list);
                    if(fdaccept > head_fd) head_fd = fdaccept;
                }
                
                /* router_socket */
                else if(sock_index == router_socket){
                    //printf("\n Packet received. \n");
                    //objtime.tv_sec = 100;
                    update_receivefrom_nodes(sock_index);
                }
                
                /* data_socket */
                else if(sock_index == data_socket){
                    //new_data_conn(sock_index);
                }
                
                /* Existing connection */
                else{
                    if(isControl(sock_index)){
                        if(!control_recv_hook(sock_index)) FD_CLR(sock_index, &master_list);
                    }
                    //else if isData(sock_index);
                    else ERROR("Unknown socket index");
                }
            }
        }
    }
    //}
}

void init()
{
    control_socket = create_control_sock();
    
    //router_socket and data_socket will be initialized after INIT from controller
    
    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);
    
    /* Register the control socket */
    FD_SET(control_socket, &master_list);
    head_fd = control_socket;
    
    main_loop();
}
