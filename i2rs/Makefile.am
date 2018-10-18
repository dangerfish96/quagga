## Process this file with automake to produce Makefile.in.

AM_CPPFLAGS = -I.. -I$(top_srcdir) -I$(top_srcdir)/lib -I$(top_builddir)/lib
DEFS = @DEFS@ -DSYSCONFDIR=\"$(sysconfdir)/\"
INSTALL_SDATA=@INSTALL@ -m 600
LIBS = @LIBS@ @CURSES@ @LIBPAM@
i2rs_LDADD = ../lib/libzebra.la @LIBCAP@ @LIBREADLINE@



AM_CFLAGS = $(WERROR)

#noinst_LIBRARIES = librip.a
sbin_PROGRAMS = i2rs

noinst_HEADERS = \
		         i2rs.h i2rs_debug.h i2rs_interface.h

i2rs_SOURCES = \
	               i2rs.c

#examplesdir = $(exampledir)
#dist_examples_DATA = i2rs.conf.sample

#EXTRA_DIST = RIPv2-MIB.txt

