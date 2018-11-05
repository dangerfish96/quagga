#include <arpa/inet.h>
#include "i2rs.h"
#include "../lib/zclient.h"
#include "../lib/command.h"
#include "../lib/table.h"
#include "../lib/vty.h"
#include "../lib/linklist.h"
#include "../lib/if.h"



/* All information about zebra. */
struct zclient *zclient = NULL;

// I2RS Structure
struct i2rs *i2rs = NULL;


void i2rs_terminate(){return;}
void i2rs_clean(){return;}
  
  /* I2RS node structure. */
static struct cmd_node i2rs_node =
  {
   I2RS_NODE,
   "%s(config-router)# ",
   1
};



 DEFUN (router_i2rs,
         router_i2rs_cmd,
         "router i2rs",
         "Enable a routing process\n"
         "Routing Information Protocol (RIP)\n")
  {
    //vty->node = I2RS_NODE;
    //vty->index = i2rs;
  
    return CMD_SUCCESS;
  }
  
  DEFUN (no_router_i2rs,
         no_router_i2rs_cmd,
         "no router i2rs",
         NO_STR
         "Enable a routing process\n"
         "Routing Information Protocol (RIP)\n")
  {
    if (i2rs)
      i2rs_clean ();
    return CMD_SUCCESS;
  }

 DEFUN (no_i2rs_route,
         no_i2rs_route_cmd,
         "no route A.B.C.D/M",
         NO_STR
         "I2RS static route configuration\n"
         "IP prefix <network>/<length>\n")
  {return CMD_SUCCESS;}
 DEFUN (i2rs_route,
         i2rs_route_cmd,
         "route A.B.C.D/M",
         NO_STR
         "I2RS static route configuration\n"
         "IP prefix <network>/<length>\n")
  {return CMD_SUCCESS;}


/* Send new route to the zebra daemon.*/
void
i2rs_zebra_ipv4_add (struct prefix_ipv4 *p, struct in_addr *nexthop,
                 u_int32_t metric, u_int32_t ifindex)
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

      zapi_ipv4_route (ZEBRA_IPV4_ROUTE_ADD, zclient, p, &api);
    }
}

/* Tell zebra to delete a route. */
void
i2rs_zebra_ipv4_delete (struct prefix_ipv4 *p, struct in_addr *nexthop,
                   u_int32_t metric)
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

      zapi_ipv4_route (ZEBRA_IPV4_ROUTE_DELETE, zclient, p, &api);
    }
}

void test(struct thread_master * master){
    zclient = zclient_new (master);
    zclient_init (zclient, ZEBRA_ROUTE_STATIC);

		struct in_addr *next_hop = (struct in_addr *) malloc(sizeof(struct in_addr*));
	struct prefix_ipv4 *p = (struct prefix_ipv4*) malloc(sizeof(struct prefix_ipv4));
	p->family = AF_INET;
	p->prefixlen = IPV4_MAX_PREFIXLEN;
	unsigned int ifindex = 0;
	int dist = 2;

	inet_aton("192.168.178.1",next_hop);
	inet_aton("0.0.0.0",&p->prefix);
	i2rs_zebra_ipv4_add ((struct prefix_ipv4 *) p, next_hop,
			                             dist,ifindex);
}

 static void
 rip_zebra_connected (struct zclient *zclient)
 { 
      zclient_send_requests (zclient, VRF_DEFAULT);
 }

  int
  i2rs_cmd (struct vty *vty)
  { 
        vty_out (vty, "!%s", VTY_NEWLINE);
        vty_out (vty, "interface test%s",VTY_NEWLINE);
        
          vty_out (vty, " description test%s",VTY_NEWLINE);
     
    
    return 0;
  }



void i2rs_init (struct thread_master *master)

  {
    srandom (time (NULL));

    /* Set default value to the zebra client structure. */
    zclient = zclient_new (master);
    zclient_init (zclient, ZEBRA_ROUTE_STATIC);
	zclient->zebra_connected = rip_zebra_connected;

	install_node (&i2rs_node, &i2rs_cmd);

//    install_element (CONFIG_NODE, &router_i2rs_cmd);
//    install_element (CONFIG_NODE, &no_router_i2rs_cmd);
	install_default (I2RS_NODE);

	install_element (I2RS_NODE, &i2rs_route_cmd);
	/*install_element (I2RS_NODE, &no_i2rs_route_cmd);
*/

  }

