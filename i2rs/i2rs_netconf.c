#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef MEMORY_DEBUG
#include <mcheck.h>
#endif

#include <pthread.h>

#define _C_main 1

#include  "procdefs.h"
#include  "ncx/log.h"
#include  "agt.h"
#include  "agt_ncxserver.h"
#include  "agt_util.h"
#include  "agt_cli.h"
#include  "help.h"
#include  "ncx.h"
#include  "ncxconst.h"
#include  "ncxmod.h"
#include  "status.h"
#include  "xmlns.h"
#include "i2rs.h"
#include "zclient.h"


#define NETCONFD_MOD       (const xmlChar *)"netconfd"
#define NETCONFD_EX_MOD       (const xmlChar *)"netconfd-ex"
#define NETCONFD_CLI       (const xmlChar *)"netconfd"

#define START_MSG          "Starting netconfd...\n"

  static fd_set active_fd_set;
  static fd_set read_fd_set;
  static fd_set write_fd_set;



static status_t load_base_schema (void)
{   
    status_t res;
    
    /* load in the NETCONF data types and RPC methods */ 
    res = ncxmod_load_module( NCXMOD_YUMA_NETCONF, NULL, NULL, NULL );
    if (res != NO_ERR) {
        return res;
    }
    
    /* load in the server boot parameter definition file */
    res = ncxmod_load_module( NCXMOD_NETCONFD_EX, NULL, NULL, NULL );
    if (res != NO_ERR) {
        return res;
    }
    
    return res;
}   /* load_base_schema */

static status_t load_core_schema ( agt_profile_t *profile )
{
    log_debug2("\nnetconfd: Loading NCX Module");

    /* load in the with-defaults extension module */
    status_t res = ncxmod_load_module( NCXMOD_WITH_DEFAULTS, NULL,
                                      &profile->agt_savedevQ, NULL );
    if (res != NO_ERR) {
        return res;
    }
 
#ifdef NETCONFD_DEBUG_LOAD_TEST
    res = load_debug_test_module( profile );
#endif
    
    return res;


}

static status_t cmn_init ( int argc, char**argv,boolean *showver,
                           help_mode_t *showhelpmode, boolean *validate_config_only_mode)
{
#define BUFFLEN 256

    status_t     res;
    log_debug_t  dlevel;
    int          len;
    char         buff[BUFFLEN];
    val_value_t* val;
    val_value_t* cli_valset;

	*validate_config_only_mode = FALSE;
    /* set the default debug output level */
    dlevel = LOG_DEBUG_INFO;

    /* initialize the NCX Library first to allow NCX modules to be processed.  
     * No module can get its internal config until the NCX module parser and 
     * definition registry is up */
    len = strlen(START_MSG) + strlen(COPYRIGHT_STRING_LINE0) + strlen(COPYRIGHT_STRING_LINE1) + 2;

  //  if (len < BUFFLEN) {
//        strcpy(buff, START_MSG);
//        strcat(buff, COPYRIGHT_STRING_LINE0);
 //       strcat(buff, COPYRIGHT_STRING_LINE1);
  //  } else {
   //     return ERR_BUFF_OVFL;
  //  }

    res = ncx_init( FALSE, dlevel, TRUE, buff, argc, argv);

    if (res != NO_ERR) {
        return res;
    }

    log_debug2("\nnetconfd: Loading Netconf Server Library");

    /* at this point, modules that need to read config params can be 
     * initialized */

    /* Load the core modules (netconfd and netconf) */
    res = load_base_schema();
    if (res != NO_ERR) {
        return res;
    }

    /* Initialize the Netconf Server Library with command line and conf file 
     * parameters */
    res = agt_init1(argc, argv, showver, showhelpmode);
    if (res != NO_ERR) {
        return res;
    }

    /* check quick-exit mode */
    if (*showver || *showhelpmode != HELP_MODE_NONE) {
        return NO_ERR;
    }

    cli_valset = agt_cli_get_valset();
    val = val_find_child(cli_valset, NETCONFD_EX_MOD, "validate-config-only");
    if(val!=NULL) {
        *validate_config_only_mode=TRUE;
    }

    /* Load the core modules (netconfd and netconf) */
    res = load_core_schema(agt_get_profile());
    if (res != NO_ERR) {
        return res;
    }

    /* finish initializing server data structures */
    res = agt_init2();
    if (res != NO_ERR) {
        return res;
    }

    log_debug("\nnetconfd init OK, ready for sessions\n");

    return NO_ERR;

}
static status_t
    netconfd_run (void)
{
    status_t  res;

    show_server_banner();
    res = agt_ncxserver_run();
    if (res != NO_ERR) {
        log_error("\nncxserver failed (%s)", get_error_string(res));
    }
    return res;

}  /* netconfd_run */


int run_netconfd(struct thread* thread){
	struct i2rs * i2rs = THREAD_ARG(thread);
      boolean            showver = FALSE;
      boolean            done = FALSE;
      help_mode_t        showhelpmode;
      boolean            validate_config_only_mode = FALSE;
	status_t  res;
  
  #ifdef MEMORY_DEBUG
      mtrace();
  #endif


	cmn_init(i2rs->argc, i2rs->argv,&showver, &showhelpmode, &validate_config_only_mode);
	res = agt_ncxserver_run();
	if (res != NO_ERR) {
          log_error("\nncxserver failed (%s)", get_error_string(res));
		shutdown_netconfd();
      }
	return NO_ERR;
}
void shutdown_netconfd(){
	/* Cleanup the Netconf Server Library */
      agt_cleanup();
  
      /* cleanup the NCX engine and registries */
      ncx_cleanup();

	agt_request_shutdown(NCX_SHUT_EXIT);
}
