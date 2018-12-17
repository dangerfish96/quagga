#include "i2rs.h"
#include "sigevent.h"
#include "thread.h"
#include "log.h"
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

  i2rs_terminate ();
/* this is your clean-up function */

  exit (0);
}

/* SIGUSR1 handler. */
static void
sigusr1 (void)
{
  zlog_rotate (NULL);
}

static void
sigterm (void)
{
  zlog (NULL, LOG_INFO, "Terminating on signal");
  i2rs_terminate ();
  exit(0);
}

 struct quagga_signal_t sighandlers[] = {
      { .signal = SIGUSR1, .handler = &sigusr1, },
      { .signal = SIGTERM, .handler = &sigterm, },
      { .signal = SIGINT,  .handler = &sigint, },
      { .signal = SIGHUP, .handler = &sighup, },
};
void signal_init3(struct thread_master * master){
	signal_init(master,3,sighandlers);
}
