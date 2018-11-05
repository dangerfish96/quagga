
#include <unistd.h>

#include "../lib/zebra.h"
#include "i2rs.h"
#include "../lib/log.h"
#include "../lib/vty.h"
#include "../lib/getopt.h"
#include "../lib/version.h"
#include "../lib/filter.h"
#include "../lib/memory.h"
#include "../lib/privs.h"
#include "../lib/sigevent.h"
#include "../lib/command.h"
#include "../lib/thread.h"
#include "../lib/version.h"
#include <getopt.h>
#include <signal.h>

/* just some includes, as usual */

/* you might want to put the following #defines on a zapd.h file 
 */

#define I2RS_DEFAULT_CONFIG "i2rs.conf"
/* this will be the name of the config file in the zebra
 *  config directory (could be /usr/local/etc/)
 */

#define I2RS_VTY_PORT 26666
/* telnet to this port to login to the zapd vty
 */


#define I2RS_VTYSH_PATH "/tmp/.i2rs"
/* name of the unix socket to communicate with the vtysh
 */


/* i2rsd privileges */
zebra_capabilities_t _caps_p [] = 
  {
  ZCAP_NET_RAW,
  ZCAP_BIND
};
struct zebra_privs_t i2rs_privs =
{
#if defined(QUAGGA_USER)
  .user = QUAGGA_USER,
#endif
#if defined QUAGGA_GROUP
  .group = QUAGGA_GROUP,
#endif
#ifdef VTY_GROUP
  .vty_group = VTY_GROUP,
#endif
  .caps_p = _caps_p,
  .cap_num_p = 2,
  .cap_num_i = 0
};


/*
 * *  GLOBALS
 */

char config_default[] = SYSCONFDIR I2RS_DEFAULT_CONFIG;
char *config_file = NULL;
/* zebra does #define  SYSCONFDIR
 */

/* SIGHUP handler. */
static void 
sighup (void)
{
  zlog (NULL, LOG_INFO, "SIGHUP received");
}

/* SIGINT handler. */
static void
sigint (void)
{
  zlog (NULL, LOG_INFO, "Terminating on signal");

//  i2rs_terminate ();
/* this is your clean-up function */

  exit (0);
}

/* SIGUSR1 handler. */
static void
sigusr1 (void)
{
  zlog_rotate (NULL);
}


static struct quagga_signal_t sighandlers[] = {
	  { .signal = SIGUSR1, .handler = &sigusr1, },
	  { .signal = SIGINT,  .handler = &sigint, },
	  { .signal = SIGHUP, .handler = &sighup, },
};


struct option longopts[] = 
{
  { "daemon",      no_argument,       NULL, 'd'},
  { "config_file", required_argument, NULL, 'f'},
  { "help",        no_argument,       NULL, 'h'},
  { "vty_addr",    required_argument, NULL, 'A'},
  { "vty_port",    required_argument, NULL, 'P'},
  { "version",     no_argument,       NULL, 'v'},
};


char* progname;
/* will contain (mangled) argv[0]
 */


struct thread_master *master;
/* needed by the thread implementation
 */


/* These 2 are defined somewhere else, say in libzapd.a
 */
/* #ifdef REALLY_DUMMY
void i2rs_init(void) {return;};
void i2rs_terminate(void) {return;};
#else
i2rs_init(master);
i2rs_terminate(void);
#endif */

/* Help information display. */
static void
usage (int status)
{
  if (status != 0)
    fprintf (stderr, "Try `%s --help' for more information.\n", progname);
  else
    {    
      printf ("Usage : %s [OPTION...]\n\
Daemon for I2RS.\n\n\
-d, --daemon       Runs in daemon mode\n\
-f, --config_file  Set configuration file name\n\
-A, --vty_addr     Set vty's bind address\n\
-P, --vty_port     Set vty's port number\n\
-v, --version      Print program version\n\
-h, --help         Display this help and exit\n\
\n\
Report bugs to %s\n", progname, ZEBRA_BUG_ADDRESS);
    }
  exit (status);
}

int vty_port = 0;
char *vty_addr = NULL;

int main(int argc, char** argv, char** envp) {
  char *p;
  int daemon_mode = 0;
  char *config_file = NULL;
  struct thread thread;

          
  umask(0027);
 
 
  progname =  ((p = strrchr (argv[0], '/')) ? ++p : argv[0]);
 
  zlog_default = openzlog (progname, ZLOG_I2RS,
			   LOG_CONS|LOG_NDELAY|LOG_PID, LOG_DAEMON);

/* initialize the log subsystem, you will have to include
 * ZLOG_ZAP in the zlog_proto_t enum type definition in 
 * lib/log.h
 */

     
/* this while just reads the options */                       
  while (1) 
    {
      int opt;
            
      opt = getopt_long (argc, argv, "dlAf:hP:v", longopts, 0);
                      
      if (opt == EOF)
	break;
                                    
      switch (opt) 
	{
	case 0:
	  break;
	case 'd':
	  daemon_mode = 1;
	  break;
	case 'f':
	  config_file = optarg;
	  break;
	case 'P':
	  vty_port = atoi (optarg);
	  break;
    case 'A':
      vty_addr = optarg;
      break;
	case 'v':
	  printf("0.1\n");
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
/* one to control them all, ... */
  master = thread_master_create ();
/* this the main thread controlling structure,
 * nothing to remember.
 */


  signal_init (master, 3, sighandlers);
/* before you start the engine, put your safety belt on
 */


/* 
 * * Library inits.
 */
  cmd_init (1);
/* initializes the command sub-system, if arg, add some commands
 * which are mostly only useful for humans on the vty
 */

  i2rs_init(master);
  vty_init (master);
  access_list_init ();
  memory_init ();
  zprivs_init (&i2rs_privs);


  //
//  prefix_list_init ();
/* these are all from libzebra
 */

              
/*
 I2RS*  inits
 */
  test(master);
  //i2rs_init(master);
/* this is implemented somewhere, e.g. on libi2rs.a
 * here, you could start some threads (the thread subsystem
 * is not running yet), register some commands, ...
 */

//  sort_node();
/* This is needed by the command subsystem to finish initialization.
 */

  /* Get configuration file. */
  vty_read_config (config_file, config_default);
/* read the config file, your commands should be defined before this
 */



  /* Change to the daemon program. */
  if (daemon_mode)
    daemon (0, 0);

  /* Create VTY socket */
  vty_serv_sock (vty_addr,vty_port ? vty_port : I2RS_VTY_PORT, I2RS_VTYSH_PATH);
/* start the TCP and unix socket listeners
 */

  /* Print banner. */
zlog_notice ("I2RS %s starting: vty@%d", QUAGGA_VERSION, vty_port);
  /* Fetch next active thread. */
    thread_main (master);
/* this is the main event loop */

/* never reached */
  return 0;
}
