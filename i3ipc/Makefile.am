CLEANFILES =

AM_CFLAGS=-std=gnu99
AM_CPPFLAGS=`pkg-config --cflags gobject-2.0 json-glib-1.0 xcb`
lib_LTLIBRARIES=libi3ipc-1.0.la
libi3ipc_1_0_la_LIBADD=${xcb_LIBS} ${json_LIBS} ${gobject_LIBS}
libi3ipc_1_0_la_SOURCES= \
  ipc-protocol.h \
	i3ipc-con.c \
	i3ipc-con.h \
	i3ipc-connection.c \
	i3ipc-connection.h

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =i3ipc_1_0_gir
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir)
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

if HAVE_INTROSPECTION
introspection_sources = $(libi3ipc_1_0_la_SOURCES)

i3ipc-1.0.gir: libi3ipc-1.0.la
i3ipc_1_0_gir_INCLUDES = GObject-2.0
i3ipc_1_0_gir_CFLAGS = $(AM_CPPFLAGS)
i3ipc_1_0_gir_PACKAGES = json-glib-1.0
i3ipc_1_0_gir_LIBS = libi3ipc-1.0.la
i3ipc_1_0_gir_FILES = $(introspection_sources)
i3ipc_1_0_gir_NAMESPACE = i3ipc
INTROSPECTION_GIRS += i3ipc-1.0.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)
endif

