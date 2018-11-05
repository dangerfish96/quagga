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
static void test2(){

	 struct listnode *node;
	 struct listnode *ifnode, *ifnnode;
	 struct connected *connected;
	 struct interface *ifp;
	 for (ALL_LIST_ELEMENTS_RO (iflist, node, ifp))
	 {
		 printf("%s",ifp->name);
	 }

}
void test(struct zclient *zclient){
	  zclient_send_requests (zclient, VRF_DEFAULT);
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
static int noop(int a,struct zclient *zclient ,unsigned short b,unsigned short c){
	printf("test");
	return 0;
}
int main(){
	master = thread_master_create ();
	zclient = zclient_new (master);
	zclient_init (zclient, ZEBRA_ROUTE_STATIC);
	zclient->zebra_connected = test;
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
