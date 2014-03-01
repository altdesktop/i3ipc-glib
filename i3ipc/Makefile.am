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
	$(top_srcdir)/i3ipc/i3ipc-con.h \
	$(top_srcdir)/i3ipc/i3ipc-connection.h \
	$(NULL)

source_h_private = \
	$(NULL)

source_c = \
	i3ipc-con.c \
	i3ipc-connection.c \
	$(NULL)

# glib-mkenums rules
glib_enum_h = i3ipc-enum-types.h
glib_enum_c = i3ipc-enum-types.c
glib_enum_headers = i3ipc-connection.h
include $(top_srcdir)/i3ipc/Makefile.am.enums

lib_LTLIBRARIES += libi3ipc-1.0.la
libi3ipc_1_0_la_LIBADD = $(xcb_LIBS) $(json_LIBS) $(gobject_LIBS) $(gio_LIBS)
libi3ipc_1_0_la_SOURCES = $(source_c) $(source_h) $(source_h_private) $(BUILT_SOURCES)

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS = i3ipc_1_0_gir
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir) --warn-all
INTROSPECTION_COMPILER_ARGS =

if HAVE_INTROSPECTION
INTROSPECTION_GIRS += i3ipc-1.0.gir
introspection_sources = $(libi3ipc_1_0_la_SOURCES)

i3ipc-1.0.gir: libi3ipc-1.0.la
i3ipc_1_0_gir_INCLUDES = GLib-2.0 GObject-2.0 Gio-2.0 Json-1.0
i3ipc_1_0_gir_CFLAGS = $(AM_CPPFLAGS)
i3ipc_1_0_gir_PACKAGES =
i3ipc_1_0_gir_LIBS = libi3ipc-1.0.la
i3ipc_1_0_gir_FILES = $(introspection_sources)
i3ipc_1_0_gir_NAMESPACE = i3ipc
i3ipc_1_0_gir_SCANNERFLAGS = --warn-all --pkg-export i3ipc-1.0

girdir = $(datadir)/gir-1.0
gir_DATA = i3ipc-1.0.gir

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = i3ipc-1.0.typelib

CLEANFILES += $(gir_DATA) $(typelib_DATA) $(BUILT_SOURCES)
endif

