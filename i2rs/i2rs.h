#include "zclient.h"
#include "thread.h"
extern struct zclient * zclient;
struct i2rs {
	struct zclient * zclient;
	int argc;
	int netconfd_fd;
	char ** argv;
	struct thread_master * master;
	struct socket * socket;
};

extern void signal_init2(void);
extern void signal_init3(struct thread_master * master);
extern void signal_cleanup(void);
extern void agt_signal_handler (int intr);
//extern void i2rs_init(struct thread_master *master);
extern void i2rs_terminate(void);
//extern void test(struct thread_master *master);


extern void connected (struct zclient *zclient);
extern int noop (int command, struct zclient *zclient, zebra_size_t length,vrf_id_t vrf_id);
extern int i2rs_interface_add (int command, struct zclient *zclient, zebra_size_t length,
        vrf_id_t vrf_id);
extern int i2rs_interface_delete (int command, struct zclient *zclient,
        zebra_size_t length, vrf_id_t vrf_id);
extern int i2rs_interface_state_down (int command, struct zclient *zclient, zebra_size_t length, vrf_id_t vrf_id);
extern int i2rs_interface_state_up (int command, struct zclient *zclient, zebra_size_t length, vrf_id_t vrf_id);
extern int i2rs_interface_address_add (int command, struct zclient *zclient, zebra_size_t length,vrf_id_t vrf_id);
extern int i2rs_interface_address_delete (int command, struct zclient *zclient, zebra_size_t length,vrf_id_t vrf_id);
extern  int i2rs_zebra_route_manage (int command, struct zclient *zclient, zebra_size_t length, vrf_id_t vrf_id);

void
  i2rs_zebra_ipv4_add (struct prefix_ipv4 *p, struct in_addr *nexthop,
                   u_int32_t metric, ifindex_t ifindex,struct  zclient *zclient);
void
i2rs_zebra_ipv4_delete (struct prefix_ipv4 *p, struct in_addr *nexthop,
		                   u_int32_t metric, struct zclient * zclient);



extern int run_netconfd(struct thread * thread);
extern int run_netconfd2(struct thread * thread);
extern void shutdown_netconfd(void);
struct socket_command{
	struct zapi_ipv4 zapi;
	int type;
};
