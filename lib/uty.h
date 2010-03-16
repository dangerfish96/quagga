/* VTY internal stuff -- header
 * Copyright (C) 1997 Kunihiro Ishiguro
 *
 * Copyright (C) 2009 Chris Hall (GMCH), Highwayman
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _ZEBRA_UTY_H
#define _ZEBRA_UTY_H

#include <stdbool.h>

#include "qpthreads.h"
#include "qpnexus.h"
#include "thread.h"
#include "list_util.h"
#include "vty.h"
#include "node_type.h"

/* Macro in case there are particular compiler issues.    */
#ifndef Inline
  #define Inline static inline
#endif

/*==============================================================================
 * This is stuff which is used by the close family of:
 *
 *   vty
 *   command
 *   log
 *
 * and which is not for use elsewhere.
 *
 * The two: vty_io and vty_cli are treated as private to vty.  So anything
 * there that is used in command or log is published here.  (Nobody other
 * than vty.c/vty_io.c/vty_cli.c will include either vty_io.h or vty_cli.h.)
 */

/*==============================================================================
 * To make vty qpthread safe we use a single mutex.
 *
 * vty and log recurse through each other, so the same mutex is used
 * for both, i.e. they are treated as being part of the same monitor.
 *
 * A recursive mutex is used.  This simplifies the calling from log to vty and
 * back again.  It also allows for the vty internals to call each other.
 *
 * There are some "uty" functions which assume the mutex is locked.
 *
 * vty is closely bound to the command handling -- the main vty structure
 * contains the context in which commands are parsed and executed.
 */

extern qpt_mutex_t vty_mutex ;

#ifdef  NDEBUG
# define  VTY_DEBUG 0           /* NDEBUG override                      */
#else
# ifndef  VTY_DEBUG
#  define VTY_DEBUG 1           /* Set to 1 to turn on debug checks     */
# endif
#endif

#if VTY_DEBUG

extern int vty_lock_count ;
extern int vty_lock_assert_fail ;

#endif

Inline void
VTY_LOCK(void)
{
  qpt_mutex_lock(&vty_mutex) ;
  if (VTY_DEBUG)
    ++vty_lock_count ;
} ;

Inline void
VTY_UNLOCK(void)
{
  if (VTY_DEBUG)
    --vty_lock_count ;
  qpt_mutex_lock(&vty_mutex) ;
} ;

#if VTY_DEBUG

Inline void
VTY_ASSERT_LOCKED(void)
{
  if (vty_lock_count == 0 && !vty_lock_assert_fail)
    {
      vty_lock_assert_fail = 1;
      assert(0);
    }
} ;

#else

#define VTY_ASSERT_LOCKED()

#endif

/*==============================================================================
 * Shared definitions
 */

enum cli_do
{
  cli_do_nothing  = 0,  /* no action required                   */

  cli_do_command,       /* dispatch the current command line    */
  cli_do_ctrl_c,        /* received ^c                          */
  cli_do_ctrl_d,        /* received ^d on empty line            */
  cli_do_ctrl_z,        /* received ^z                          */

  cli_do_count          /* number of different cli_do_xxx       */
} ;

/*==============================================================================
 * Variables in vty.c -- used in any of the family
 */
extern vty_io vio_list_base ;
extern vty_io vio_monitors_base ;
extern vty_io vio_death_watch ;

union vty_watch_dog
{
  qtimer          qnexus ;      /* when running qnexus  */
  struct thread*  thread;       /* when running threads */
  void*           anon ;
};

extern union vty_watch_dog vty_watch_dog ;

extern struct thread_master* vty_master ;

extern unsigned long vty_timeout_val ;

extern       bool vty_config ;

extern       bool no_password_check ;
extern const bool restricted_mode_default ;
extern       bool restricted_mode ;

char *vty_accesslist_name ;
char *vty_ipv6_accesslist_name ;

extern qpn_nexus vty_cli_nexus ;
extern qpn_nexus vty_cmd_nexus ;

/*==============================================================================
 * Functions in vty.c -- used in any of the family
 */

extern int uty_command(struct vty *vty, const char *buf) ;
extern int uty_auth (struct vty *vty, const char *buf, enum cli_do cli_do) ;
extern int vty_cmd_exit(struct vty* vty) ;
extern int vty_cmd_end(struct vty* vty) ;
extern int uty_stop_input(struct vty *vty) ;
extern int uty_end_config (struct vty *vty) ;
extern int uty_down_level (struct vty *vty) ;

extern bool vty_config_lock (struct vty *, enum node_type node);
extern void vty_config_unlock (struct vty *, enum node_type node);
extern void uty_config_unlock (struct vty *vty, enum node_type node) ;

/*==============================================================================
 * Functions in vty_cli
 */
extern void
vty_queued_result(struct vty* vty, int ret, int action);

extern void
uty_set_host_name(const char* name) ;

/*==============================================================================
 * Functions in vty_io
 *
 * Send a fixed-size message to all vty terminal monitors; this should be
 * an async-signal-safe function.
 */
extern void vty_log_fixed (const char *buf, size_t len);

extern void uty_log (struct logline* ll, struct zlog *zl, int priority,
                                                const char *format, va_list va);

#endif /* _ZEBRA_UTY_H */