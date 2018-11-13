#include "../lib/zclient.h"
#include "../lib/thread.h"
#include "../lib/log.h"
#include "i2rs.h"

struct thread_master *master ;
struct zclient *zclient = NULL;


void
  i2rs_zebra_ipv4_add (struct prefix_ipv4 *p, struct in_addr *nexthop,
                 u_int32_t metric, ifindex_t ifindex,struct  zclient *zclient)
  {
    struct zapi_ipv4 api;
  
    if (zclient->redist[ZEBRA_ROUTE_STATIC])
      {
  
        api.type = ZEBRA_ROUTE_STATIC;
        api.vrf_id = VRF_DEFAULT;
        api.flags = 0;
        api.message = 0;

		//needed?
        api.safi = SAFI_UNICAST;

        SET_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP);
        api.nexthop_num = 1;
        api.nexthop = &nexthop;
        api.ifindex_num = 0;
        SET_FLAG (api.message, ZAPI_MESSAGE_METRIC);
        api.metric = metric;
		printf("Sending message...\n");
  
        zapi_ipv4_route (ZEBRA_IPV4_ROUTE_ADD, zclient, p, &api);
      }
  }
/* Tell zebra to delete a route. */
void
i2rs_zebra_ipv4_delete (struct prefix_ipv4 *p, struct in_addr *nexthop,
                   u_int32_t metric, struct zclient * zclient)
{
  struct zapi_ipv4 api;

  if (zclient->redist[ZEBRA_ROUTE_STATIC])
    {
	  api.vrf_id = VRF_DEFAULT;
      api.type = ZEBRA_ROUTE_STATIC;
      api.flags = 0;
      api.message = 0;
      api.safi = SAFI_UNICAST;
      SET_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP);
      api.nexthop_num = 1;
      api.nexthop = &nexthop;
      api.ifindex_num = 0;
      SET_FLAG (api.message, ZAPI_MESSAGE_METRIC);
      api.metric = metric;

      zapi_ipv4_route (ZEBRA_IPV4_ROUTE_DELETE, zclient, p, &api);
    }
}

void test(struct zclient *zclient){
      struct in_addr *next_hop = (struct in_addr *) malloc(sizeof(struct in_addr*));
      struct prefix_ipv4 *p = (struct prefix_ipv4*) malloc(sizeof(struct prefix_ipv4));
      p->family = AF_INET;
      p->prefixlen = 24;
	ifindex_t ifindex = 0;
      int dist = 111;
      inet_aton("127.0.0.1",next_hop);
      inet_aton("192.168.178.0",&p->prefix);
      i2rs_zebra_ipv4_add ((struct prefix_ipv4 *) p, next_hop,
                                           dist,ifindex,zclient);
	zlog_debug ("%s: %s/%d nexthops %d",
                              "Install into zebra" ,
                          inet_ntoa (p->prefix), p->prefixlen, dist);

  }
void test2(struct zclient *zclient){
      struct in_addr *next_hop = (struct in_addr *) malloc(sizeof(struct in_addr*));
      struct prefix_ipv4 *p = (struct prefix_ipv4*) malloc(sizeof(struct prefix_ipv4));
      p->family = AF_INET;
      p->prefixlen = 24;
      int dist = 111;
      inet_aton("127.0.0.1",next_hop);
      inet_aton("192.168.178.0",&p->prefix);
      i2rs_zebra_ipv4_delete ((struct prefix_ipv4 *) p, next_hop,
                                           dist,zclient);
	zlog_debug ("%s: %s/%d nexthops %d",
                              "Delete from zebra" ,
                          inet_ntoa (p->prefix), p->prefixlen, dist);

  }
/*
static int noop(int a,struct zclient *zclient ,unsigned short b,unsigned short c){
	return 0;
}
void connected(struct zclient *zclient){
	 zclient_send_requests (zclient, VRF_DEFAULT);
//	run_netconfd(zclient,argcSave,argvSave);
//	test(zclient);
}
*/
void i2rs_terminate(){
	shutdown_netconfd();
	zclient_stop(zclient);
	zclient_free(zclient);
	thread_master_free(master);
	exit(0);
}
int main(int argc, char ** argv){
	
	master = thread_master_create ();
	
	struct i2rs* i2rs = (struct i2rs*)malloc(sizeof(struct i2rs));
	i2rs->argc = argc;
	i2rs->argv = argv;
	i2rs->master = master;
	i2rs->zclient = zclient;
	zclient = zclient_new (master);
    thread_execute (master, run_netconfd, (void*)i2rs, 0);
	
	signal_init2(master);
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
