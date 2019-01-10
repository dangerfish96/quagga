#include "../lib/zclient.h"
#include "../lib/stream.h"
#include "../lib/log.h"
#include "../lib/if.h"
#include "i2rs.h"
void i2rs_zebra_ipv4 (int mtype, struct prefix_ipv4 *p, struct in_addr *nexthop,
                   u_int32_t metric, struct zclient * zclient){
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

	 zapi_ipv4_route (mtype, zclient, p, &api);
	printf("Changing route setting for prefix %s",inet_ntoa(p->prefix));
	printf("/%i ",p->prefixlen);
	printf("with nexthop %s\n",inet_ntoa(**api.nexthop));
}
}

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
      api.ifindex = 0;
      SET_FLAG (api.message, ZAPI_MESSAGE_METRIC);
      api.metric = metric;

      zapi_ipv4_route (ZEBRA_IPV4_ROUTE_DELETE, zclient, p, &api);
    }
}
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

/* Interface addition message from zebra. */
  int
  i2rs_interface_add (int command, struct zclient *zclient, zebra_size_t length,
      vrf_id_t vrf_id)
  {
    struct interface *ifp;
  
    ifp = zebra_interface_add_read (zclient->ibuf, vrf_id);
  
    zlog_debug ("interface add %s index %d flags %#llx metric %d mtu %d",
                  ifp->name, ifp->ifindex, (unsigned long long) ifp->flags,
                  ifp->metric, ifp->mtu);
	return 0;
  }
int
  i2rs_interface_delete (int command, struct zclient *zclient,
                        zebra_size_t length, vrf_id_t vrf_id)
  {
    struct interface *ifp;
    struct stream *s;
  
  
    s = zclient->ibuf;
    /* zebra_interface_state_read() updates interface structure in iflist */
    ifp = zebra_interface_state_read (s, vrf_id);
  
    if (ifp == NULL){
      return 0;
    }
    zlog_info("interface delete %s index %d flags %#llx metric %d mtu %d",
              ifp->name, ifp->ifindex, (unsigned long long) ifp->flags,
              ifp->metric, ifp->mtu);
    return 0;
}
  int i2rs_interface_state_up (int command, struct zclient *zclient,
                         zebra_size_t length, vrf_id_t vrf_id)
  {
    struct interface *ifp;

    ifp = zebra_interface_state_read (zclient->ibuf,vrf_id);
  
    if (ifp == NULL){
      return 0;
    }
      zlog_debug ("Zebra: Interface[%s] state change to up.", ifp->name);
  
    return 0;
  }

  int i2rs_interface_state_down (int command, struct zclient *zclient,
                         zebra_size_t length, vrf_id_t vrf_id)
  {
    struct interface *ifp;

  
    ifp = zebra_interface_state_read (zclient->ibuf,vrf_id);
  
    if (ifp == NULL){
      return 0;
  	}
      zlog_debug ("Zebra: Interface[%s] state change to down.", ifp->name);
  
    return 0;
  }
int i2rs_interface_address_add (int command, struct zclient *zclient,
                            zebra_size_t length,vrf_id_t vrf_id)
{
  struct connected *ifc;
  struct prefix *p;

  ifc = zebra_interface_address_read (command, zclient->ibuf, vrf_id);
  
  if (ifc == NULL){
    return 0;
  }
  p = ifc->address;
  if (p->family == AF_INET)
    {
	zlog_debug ("Zebra: new IPv4 address %s/%d added on interface %s.",
		    inet_ntoa(p->u.prefix4), p->prefixlen, ifc->ifp->name);
    }
  return 0;
}
int i2rs_interface_address_delete (int command, struct zclient *zclient,
			    zebra_size_t length, vrf_id_t vrf_id)
{
struct connected *ifc;

ifc = zebra_interface_address_read (command, zclient->ibuf, vrf_id);

if (ifc == NULL){
	return 0;
}
zlog_debug ("Zebra: Address deleted.");
connected_free (ifc);

return 0;
}

static void zclient_read_zapi_ipv4( struct zclient* zclient,
 struct zapi_ipv4 *zapi, struct prefix_ipv4* p,
 unsigned int* ifindex,  struct in_addr* nexthop) 
{
  struct stream *s;


  s = zclient->ibuf;

/* read the header */
  zapi->type = stream_getc (s);
  zapi->flags = stream_getc (s);
  zapi->message = stream_getc (s);

/* and the prefix */
  memset (p, 0, sizeof (struct prefix_ipv4));
  p->family = AF_INET;
  p->prefixlen = stream_getc (s);
  stream_get (&p->prefix, s, PSIZE (p->prefixlen));

  if (CHECK_FLAG (zapi->message, ZAPI_MESSAGE_NEXTHOP))
    {
      zapi->nexthop_num = stream_getc (s);
      nexthop->s_addr = stream_get_ipv4 (s);
    }
  if (CHECK_FLAG (zapi->message, ZAPI_MESSAGE_IFINDEX))
    {
      zapi->ifindex_num = stream_getc (s);
      *ifindex = stream_getl (s);
    }
  if (CHECK_FLAG (zapi->message, ZAPI_MESSAGE_DISTANCE))
    zapi->distance = stream_getc (s);
  if (CHECK_FLAG (zapi->message, ZAPI_MESSAGE_METRIC))
    zapi->metric = stream_getl (s);

}


int i2rs_zebra_route_manage (int command, struct zclient *zclient,
                            zebra_size_t length, vrf_id_t vrf_id) {

  struct prefix_ipv4 p;
  struct zapi_ipv4 zapi;
  unsigned int ifindex = 0;
  struct in_addr nexthop;

  zclient_read_zapi_ipv4( zclient, &zapi, &p,&ifindex,&nexthop);

  if (command == ZEBRA_IPV4_ROUTE_ADD) {
   zlog_info (" new IPv4 route %s/%d on interface ifindex %i",
                inet_ntoa (p.prefix), p.prefixlen, ifindex);

  } else { 

   zlog_info ("deleted IPv4 route %s/%d on interface ifindex %i",
                inet_ntoa (p.prefix), p.prefixlen, ifindex);
  }
  return 0;
}
//void connected(struct zclient *zclient){                                                                      
 //    zclient_send_requests (zclient, VRF_DEFAULT);                                                            
//  run_netconfd(zclient,argcSave,argvSave);                                                                  
//  test(zclient);                                                                                            
//}  
int noop(int a,struct zclient *zclient ,unsigned short b,unsigned short c){                            
      return 0;                                                                                                 
  }
