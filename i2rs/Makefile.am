## Process this file with automake to produce Makefile.in.

AM_CPPFLAGS = -g3 -Wall -I.. -I$(top_srcdir) -I$(top_srcdir)/lib -I$(top_builddir)/lib -I$(top_builddir)/yuma/netconf/src/ -I$(top_builddir)/yuma/netconf/src/platform -I$(top_srcdir)/yuma/netconf/src/mgr -I$(top_builddir)/yuma/netconf/src/agt -I$(top_builddir)/yuma/netconf/src/ncx -I$(top_srcdir)/yuma/netconf/src/ydump $(XML_CPPFLAGS)


DEFS = @DEFS@ -DSYSCONFDIR=\"$(sysconfdir)/\"
INSTALL_SDATA=@INSTALL@ -m 600
LIBS = @LIBS@ @CURSES@ @LIBPAM@
i2rsd_LDADD = ../yuma/netconf/src/agt/libyumaagt.la ../yuma/netconf/src/ncx/libyumancx.la ../yuma/netconf/src/mgr/libyumamgr.la ../yuma/netconf/src/yangrpc/libyangrpc.la ../lib/libzebra.la @LIBCAP@ @LIBREADLINE@ 


AM_CFLAGS = $(WERROR)

sbin_PROGRAMS = i2rsd

noinst_HEADERS = i2rs.h socket/PassiveServer.h

#i2rs_SOURCES = i2rs3.c i2rs_zebra.c i2rs_netconf.c i2rs_signals.c
i2rsd_SOURCES = i2rs.c i2rs_zebra.c i2rs_signals.c socket/PassiveServer.c
#libi2rs_la_SOURCES = i2rs_zebra.c i2rs_signals.c socket/PassiveServer.c
#libi2rs_la_SOURCES = i2rs3.c i2rs_zebra.c i2rs_netconf.c i2rs_signals.c
##	                i2rs.c i2rs_main.c

#lib_LTLIBRARIES = libi2rs.la
#examplesdir = $(exampledir)
#dist_examples_DATA = i2rs.conf.sample

#EXTRA_DIST = RIPv2-MIB.txt

