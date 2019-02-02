#include "../lib/zclient.h"
#include "../lib/thread.h"
#include "../lib/log.h"
#include "../lib/table.h"
#include "i2rs.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdarg.h>

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "socket/PassiveServer.h"
#include "tlpi-dist/lib/create_pid_file.h"

#include <getopt.h>
#include "log.h"
#include "version.h"

#define PORT 8888


struct thread_master *master;
struct thread *test3;
struct i2rs* i2rs;
struct zclient *zclient = NULL;
const char * progname = "i2rsd";

int receive(struct thread * thread);
int socket_loop(struct thread * thread);
int socketSelect(struct thread * thread);


struct option longopts[] = 
{
  { "daemon",      no_argument,       NULL, 'd'},
  { "pid_file", required_argument, NULL, 'p'},
  { "help",        no_argument,       NULL, 'h'},
  { "version",     no_argument,       NULL, 'v'},
  { 0 }                                                                
};

usage (int status)
{
  if (status != 0)
    fprintf (stderr, "Try `%s --help' for more information.\n", progname);
  else
    {    
      printf ("Usage : %s [OPTION...]\n\
Daemon which manages ZAP.\n\n\
-d, --daemon       Runs in daemon mode\n\
-p, --pid_file  Set pid file name\n\
-v, --version      Print program version\n\
-h, --help         Display this help and exit\n\
\n\
Report bugs to %s\n", progname, ZEBRA_BUG_ADDRESS);
    }
  exit (status);
}
void print_version(){
	printf("1.0");
}
static 
void test(struct zclient * zclient){
//      struct i2rs * i2rs = THREAD_ARG(thread);
      struct in_addr *next_hop = (struct in_addr *) malloc(sizeof(struct in_addr*));
      struct prefix_ipv4 *p = (struct prefix_ipv4*) malloc(sizeof(struct prefix_ipv4));
      p->family = AF_INET;
      p->prefixlen = 24;
      ifindex_t ifindex = 0;
      int dist = 1;
      inet_aton("127.0.0.1",next_hop);
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
      inet_aton("127.0.0.1",next_hop);
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
	//test(zclient);
	//test2(zclient);
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
	int daemon_mode = 0;	
	char *pid_file;
	int netconfd_port = 8888;

 while (1) 
    {
      int opt;
      opt = getopt_long (argc, argv, "dlPp:h:v", longopts, 0);
                      
      if (opt == EOF)
	break;
                                    
      switch (opt) 
	{
	case 0:
	  break;
	case 'd':
	  daemon_mode = 1;
	  break;
	case 'P':
	    netconfd_port = atoi(optarg);
	  break;
	case 'p':
	  pid_file = optarg;
	  printf("pid file: %s\n", pid_file);
	  break;
	case 'v':
	  print_version ();
	  exit (0);
	  break;
	case 'h':
	  usage (0);
	  break;
	default:
	  usage (1);
	  break;
	}
    }
	master = thread_master_create ();
	
     	i2rs = (struct i2rs*)malloc(sizeof(struct i2rs));
    	i2rs->netconfd_fd = create_tcp_server(netconfd_port);
	i2rs->argc = argc;
	i2rs->argv = argv;
	i2rs->master = master;
	zclient = zclient_new (master);
	i2rs->zclient = zclient;
	signal_init3(master);

	zclient_init (zclient, ZEBRA_ROUTE_STATIC);
	zclient->zebra_connected = connected;
	//zclient->interface_add = i2rs_interface_add;
	zclient->interface_add = noop;
    zclient->interface_delete = noop;
    //zclient->interface_address_add = i2rs_interface_address_add;
    zclient->interface_address_add = noop;
    //zclient->interface_address_delete = i2rs_interface_address_delete;
    zclient->interface_address_delete = noop;
    zclient->ipv4_route_add = i2rs_zebra_route_manage;
    zclient->ipv4_route_delete = i2rs_zebra_route_manage;
    //zclient->interface_up = i2rs_interface_state_up;
    zclient->interface_up = noop;
    //zclient->interface_down = i2rs_interface_state_down;
    zclient->interface_down = noop;
    if (daemon_mode){
	daemon (0, 0);
    }
    if (pid_file != NULL){
	createPidFile(progname, pid_file, 0);
    }
	thread_main (master);
    }


int receive(struct thread * thread){
    union sockunion su;
    struct i2rs * i2rs = THREAD_ARG(thread);
    int accept_sock = THREAD_FD(thread);
    int fd = sockunion_accept (accept_sock, &su);
    struct socket_command * msg = (struct socket_command *)malloc(sizeof(struct socket_command)+sizeof(struct prefix_ipv4) + sizeof(struct in_addr));

    int	cc = read(fd, msg, sizeof(struct socket_command) + sizeof(struct prefix_ipv4)+ sizeof(struct in_addr));
    if(cc > 0){
      i2rs_zebra_ipv4 (msg->mtype, &(msg->p), &(msg->nexthop), msg->metric, zclient);
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
