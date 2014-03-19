# preamble

EXTRA_DIST=
BUILT_SOURCES=
DISTCLEANFILES=
CLEANFILES=
lib_LTLIBRARIES=
NULL=

AM_CPPFLAGS = \
	$(xcb_CFLAGS) \
	$(json_CFLAGS) \
	$(gobject_CFLAGS) \
	$(gio_CFLAGS) \
	$(NULL)

AM_CFLAGS = -std=gnu99 -Wall

source_h = \
	$(top_srcdir)/i3ipc-glib/i3ipc-con.h \
	$(top_srcdir)/i3ipc-glib/i3ipc-event-types.h \
	$(top_srcdir)/i3ipc-glib/i3ipc-reply-types.h \
	$(top_srcdir)/i3ipc-glib/i3ipc-connection.h \
	$(NULL)

source_h_private = \
	$(top_srcdir)/i3ipc-glib/i3ipc-con-private.h \
	$(NULL)

source_c = \
	i3ipc-con.c \
	i3ipc-event-types.c \
	i3ipc-reply-types.c \
	i3ipc-connection.c \
	$(NULL)

# glib-mkenums rules
glib_enum_h = i3ipc-enum-types.h
glib_enum_c = i3ipc-enum-types.c
glib_enum_headers = $(source_h)
include $(top_srcdir)/i3ipc-glib/Makefile.am.enums

lib_LTLIBRARIES += libi3ipc-glib-1.0.la
libi3ipc_glib_1_0_la_LIBADD = $(xcb_LIBS) $(json_LIBS) $(gobject_LIBS) $(gio_LIBS)
libi3ipc_glib_1_0_la_SOURCES = $(source_c) $(source_h) $(source_h_private) $(BUILT_SOURCES)

i3ipcincludedir = $(includedir)/i3ipc-glib
i3ipcinclude_DATA = \
	$(source_h) \
	$(top_srcdir)/i3ipc-glib/i3ipc-enum-types.h \
	$(top_srcdir)/i3ipc-glib/i3ipc-glib.h \
	$(NULL)

EXTRA_DIST += i3ipc-glib.h

pcfiles = i3ipc-glib-1.0.pc

i3ipc-glib-1.0.pc: i3ipc-glib.pc
	$(AM_V_GEN)cp -f $< $@

pkgconfig_DATA = $(pcfiles)
pkgconfigdir = $(libdir)/pkgconfig
CLEANFILES += $(pcfiles)
EXTRA_DIST += i3ipc-glib.pc.in

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS = i3ipc_1_0_gir
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir) --warn-all
INTROSPECTION_COMPILER_ARGS =

if HAVE_INTROSPECTION
INTROSPECTION_GIRS += i3ipc-1.0.gir

i3ipc-1.0.gir: libi3ipc-glib-1.0.la
i3ipc_1_0_gir_INCLUDES = GObject-2.0
i3ipc_1_0_gir_CFLAGS = $(AM_CPPFLAGS)
i3ipc_1_0_gir_PACKAGES =
i3ipc_1_0_gir_LIBS = libi3ipc-glib-1.0.la
i3ipc_1_0_gir_FILES = $(source_h) $(source_c)
i3ipc_1_0_gir_NAMESPACE = i3ipc
i3ipc_1_0_gir_SCANNERFLAGS = --warn-all --pkg-export i3ipc-1.0 --c-include "i3ipc-glib/i3ipc-glib.h"

girdir = $(datadir)/gir-1.0
gir_DATA = i3ipc-1.0.gir

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = i3ipc-1.0.typelib

CLEANFILES += $(gir_DATA) $(typelib_DATA) $(BUILT_SOURCES)
endif
