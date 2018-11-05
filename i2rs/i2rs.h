#include "../lib/thread.h"
extern void i2rs_init(struct thread_master *master);
extern void i2rs_terminate();
//extern void test(struct thread_master *master);


extern int i2rs_interface_add (int command, struct zclient *zclient, zebra_size_t length,
        vrf_id_t vrf_id);
extern int i2rs_interface_delete (int command, struct zclient *zclient,
        zebra_size_t length, vrf_id_t vrf_id);
extern int i2rs_interface_state_down (int command, struct zclient *zclient, zebra_size_t length, vrf_id_t vrf_id);
extern int i2rs_interface_state_up (int command, struct zclient *zclient, zebra_size_t length, vrf_id_t vrf_id);
extern int i2rs_interface_address_add (int command, struct zclient *zclient, zebra_size_t length,vrf_id_t vrf_id);
extern int i2rs_interface_address_delete (int command, struct zclient *zclient, zebra_size_t length,vrf_id_t vrf_id);
extern  int i2rs_zebra_route_manage (int command, struct zclient *zclient, zebra_size_t length, vrf_id_t vrf_id);


