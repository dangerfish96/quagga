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

#include "i2rs_netconf_module2.h"


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

    y_i2rs2_init("i2rs2","22");
    y_i2rs2_init2();
    /* finish initializing server data structures */
    res = agt_init2();
    if (res != NO_ERR) {
        return res;
    }
    log_debug("\nnetconfd init OK, ready for sessions\n");

    return NO_ERR;

}
static void show_server_banner (void)
{
#define BANNER_BUFFLEN 32

    xmlChar buff[BANNER_BUFFLEN];
    status_t  res;

    if (LOGINFO) {
        res = ncx_get_version(buff, BANNER_BUFFLEN);
        if (res == NO_ERR) {
            log_info("\nRunning netconfd server (%s)\n", buff);
        } else {
            log_info("\nRunning netconfd server\n");
        }
    }

}  /* show_server_banner */
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
static void show_version(void)
{
    xmlChar versionbuffer[NCX_VERSION_BUFFSIZE];

    status_t res = ncx_get_version(versionbuffer, NCX_VERSION_BUFFSIZE);
    if (res == NO_ERR) {
        log_write( "\nnetconfd version %s\n", versionbuffer );
    } else {
        SET_ERROR( res );
    }
    agt_request_shutdown(NCX_SHUT_EXIT);
}

static void netconfd_cleanup (void)
{

    if (LOGINFO) {
        log_info("\nShutting down the netconfd server\n");
    }

    /* Cleanup the Netconf Server Library */
    agt_cleanup();

    /* cleanup the NCX engine and registries */
    ncx_cleanup();

}  /* netconfd_cleanup */


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

    /* this loop is used to implement the restart command the sw image is not 
     * reloaded; instead everything is cleaned up and re-initialized from 
     * scratch. If the shutdown operation (or Ctl-C exit) is used instead of 
     * restart, then the loop will only be executed once */
    while (!done) {
        res = cmn_init( i2rs->argc, i2rs->argv, &showver, &showhelpmode, &validate_config_only_mode);

        if (res != NO_ERR) {
            log_error( "\nnetconfd: init returned (%s)",
                       get_error_string(res) );
            agt_request_shutdown(NCX_SHUT_EXIT);
        } else {
            if (showver) {
                show_version();
            } else if (showhelpmode != HELP_MODE_NONE) {
                help_program_module( NETCONFD_MOD, NETCONFD_CLI, showhelpmode );
                agt_request_shutdown(NCX_SHUT_EXIT);
            } else if (validate_config_only_mode) {
                agt_request_shutdown(NCX_SHUT_EXIT);
            } else {
                res = netconfd_run();
                if (res != NO_ERR) {
                    agt_request_shutdown(NCX_SHUT_EXIT);
                }
            }
        }

        netconfd_cleanup();
        print_error_count();
if ( NCX_SHUT_EXIT == agt_shutdown_mode_requested() ) {
            done = TRUE;
        }
    }

    print_errors();
    print_error_count();

    if ( !log_is_open() ) {
        printf("\n");
    }
	i2rs_terminate();
#ifdef MEMORY_DEBUG
    muntrace();
#endif

    return res;
}
