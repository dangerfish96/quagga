## Process this file with automake to produce Makefile.in.

AM_CPPFLAGS = -g3 -Wall -I.. -I$(top_srcdir) -I$(top_srcdir)/lib -I$(top_builddir)/lib $(XML_CPPFLAGS) -I tlpi-dist/lib


DEFS = @DEFS@ -DSYSCONFDIR=\"$(sysconfdir)/\"
INSTALL_SDATA=@INSTALL@ -m 600
LIBS = @LIBS@ @CURSES@ @LIBPAM@
i2rsd_LDADD = ../lib/libzebra.la @LIBCAP@ @LIBREADLINE@ tlpi-dist/lib/create_pid_file.o tlpi-dist/lib/region_locking.o tlpi-dist/lib/error_functions.o 




AM_CFLAGS = $(WERROR)

sbin_PROGRAMS = i2rsd

noinst_HEADERS = i2rs.h socket/PassiveServer.h tlpi-dist/lib/create_pid_file.h tlpi-dist/lib/region_locking.h tlpi-dist/lib/tlpi_hdr.h


#i2rs_SOURCES = i2rs3.c i2rs_zebra.c i2rs_netconf.c i2rs_signals.c
i2rsd_SOURCES = i2rs.c i2rs_zebra.c i2rs_signals.c socket/PassiveServer.c
#libi2rs_la_SOURCES = i2rs_zebra.c i2rs_signals.c socket/PassiveServer.c
#libi2rs_la_SOURCES = i2rs3.c i2rs_zebra.c i2rs_netconf.c i2rs_signals.c
##	                i2rs.c i2rs_main.c

#lib_LTLIBRARIES = libi2rs.la
#examplesdir = $(exampledir)
#dist_examples_DATA = i2rs.conf.sample

#EXTRA_DIST = RIPv2-MIB.txt

