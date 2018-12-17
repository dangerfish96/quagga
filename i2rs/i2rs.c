#include "../lib/zclient.h"
#include "../lib/thread.h"
#include "../lib/log.h"
#include "i2rs.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdarg.h>

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "socket/PassiveServer.h"

#define PORT 8888


struct thread_master *master;
struct thread *test3;
struct i2rs* i2rs;
struct zclient *zclient = NULL;

int receive(struct thread * thread);
int socket_loop(struct thread * thread);
int socketSelect(struct thread * thread);

static 
void test(struct zclient * zclient){
//      struct i2rs * i2rs = THREAD_ARG(thread);
      struct in_addr *next_hop = (struct in_addr *) malloc(sizeof(struct in_addr*));
      struct prefix_ipv4 *p = (struct prefix_ipv4*) malloc(sizeof(struct prefix_ipv4));
      p->family = AF_INET;
      p->prefixlen = 24;
	ifindex_t ifindex = 0;
      int dist = 1;
      inet_aton("172.16.0.1",next_hop);
      inet_aton("8.8.8.8",&p->prefix);
      i2rs_zebra_ipv4_add ((struct prefix_ipv4 *) p, next_hop,
                                           dist,ifindex,zclient);
	zlog_debug ("%s: %s/%d nexthops %d",
                              "Install into zebra" ,
                          inet_ntoa (p->prefix), p->prefixlen, dist);

  }
static 
void test2(struct zclient * zclient){
      struct in_addr *next_hop = (struct in_addr *) malloc(sizeof(struct in_addr*));
      struct prefix_ipv4 *p = (struct prefix_ipv4*) malloc(sizeof(struct prefix_ipv4));
      p->family = AF_INET;
      p->prefixlen = 24;
      int dist = 1;
      inet_aton("172.16.0.1",next_hop);
      inet_aton("8.8.8.8",&p->prefix);
      i2rs_zebra_ipv4_delete ((struct prefix_ipv4 *) p, next_hop,
                                           dist,zclient);
	zlog_debug ("%s: %s/%d nexthops %d",
                              "Delete from zebra" ,
                          inet_ntoa (p->prefix), p->prefixlen, dist);

  }
void connected(struct zclient *zclient){
	zclient_send_requests (zclient, VRF_DEFAULT);
	printf("Zebra is connected\n");
	thread_add_read (master, receive, (void*)i2rs, i2rs->netconfd_fd);
}
void i2rs_terminate(){
	
	printf("Shutting down...");
	//test2(zclient);
	close(i2rs->netconfd_fd);
	zclient_stop(zclient);
	zclient_free(zclient);
	thread_master_free(master);
	exit(0);
}
int main(int argc, char ** argv){
	
	master = thread_master_create ();
	

	i2rs = (struct i2rs*)malloc(sizeof(struct i2rs));
    	i2rs->netconfd_fd = create_tcp_server(PORT);
	i2rs->argc = argc;
	i2rs->argv = argv;
	i2rs->master = master;
	i2rs->zclient = zclient;
	zclient = zclient_new (master);
	signal_init3(master);

	zclient_init (zclient, ZEBRA_ROUTE_STATIC);
	zclient->zebra_connected = connected;
	//zclient->interface_add = i2rs_interface_add;
	zclient->interface_add = noop;
    zclient->interface_delete = i2rs_interface_delete;
    //zclient->interface_address_add = i2rs_interface_address_add;
    zclient->interface_address_add = noop;
    zclient->interface_address_delete = i2rs_interface_address_delete;
    zclient->ipv4_route_add = i2rs_zebra_route_manage;
    zclient->ipv4_route_delete = i2rs_zebra_route_manage;
    zclient->interface_up = i2rs_interface_state_up;
    zclient->interface_down = i2rs_interface_state_down;
	thread_main (master);
}
int receive(struct thread * thread){
    union sockunion su;
    i2rs = THREAD_ARG(thread);
    int accept_sock = THREAD_FD(thread);
    int fd = sockunion_accept (accept_sock, &su);
    struct socket_command * msg = (struct socket_command *)malloc(sizeof(struct socket_command*));

    int	cc = read(fd, msg, sizeof(msg));
    if(cc > 0){
        if(write(fd, msg, sizeof(msg)) < 0)
            printf("write to client %d error, close!\n", fd);
	thread_add_read (i2rs->master, receive, (void*)i2rs, i2rs->netconfd_fd);
    } else if(cc == 0)
        printf("client %d disconnect\n", fd);
    else
        printf("read from client %d error, close!\n", fd);
    close (fd);
    free(msg);
    return 0;
}
