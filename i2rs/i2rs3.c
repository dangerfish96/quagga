#include "../lib/zclient.h"
#include "../lib/thread.h"

struct thread_master *master ;
struct zclient *zclient = NULL;


void
  i2rs_zebra_ipv4_add (struct prefix_ipv4 *p, struct in_addr *nexthop,
                 u_int32_t metric, u_int32_t ifindex,struct  zclient *zclient)
  {
    struct zapi_ipv4 api;
  
    if (zclient->redist[ZEBRA_ROUTE_STATIC])
      {
  
        api.type = ZEBRA_ROUTE_STATIC;
        api.flags = 0;
        api.message = 0;
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
static void test(struct zclient *zclient){
	  zclient_send_requests (zclient, VRF_DEFAULT);
      struct in_addr *next_hop = (struct in_addr *) malloc(sizeof(struct in_addr*));
      struct prefix_ipv4 *p = (struct prefix_ipv4*) malloc(sizeof(struct prefix_ipv4));
      p->family = AF_INET;
      p->prefixlen = IPV4_MAX_PREFIXLEN;
      unsigned int ifindex = 0;
      int dist = 2;
  
      inet_aton("192.168.178.1",next_hop);
      inet_aton("0.0.0.0",&p->prefix);
      i2rs_zebra_ipv4_add ((struct prefix_ipv4 *) p, next_hop,
                                           dist,ifindex,zclient);
  }

int main(){
master = thread_master_create ();
zclient = zclient_new (master);
zclient_init (zclient, ZEBRA_ROUTE_STATIC);
zclient->zebra_connected = &test;
thread_main (master);

}
