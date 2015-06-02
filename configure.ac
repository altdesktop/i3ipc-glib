m4_define([i3ipc_major_version], [0])
m4_define([i3ipc_minor_version], [6])
m4_define([i3ipc_micro_version], [0])

m4_define([i3ipc_version], [i3ipc_major_version.i3ipc_minor_version.i3ipc_micro_version])

AC_PREREQ(2.69)

AC_INIT([i3ipc], [i3ipc_version], [tony@dubstepdish.com])

AC_CONFIG_AUX_DIR([build])
AC_CONFIG_MACRO_DIR([build/autotools])
AC_CONFIG_SRCDIR([i3ipc-glib/i3ipc-glib.h])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.
PKG_CHECK_MODULES([xcb], [xcb])
PKG_CHECK_MODULES([json], [json-glib-1.0])
PKG_CHECK_MODULES([gobject], [gobject-2.0 >= 2.32])
PKG_CHECK_MODULES([gio], [gio-2.0])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h sys/socket.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset socket strerror])

AM_INIT_AUTOMAKE([-Wall -Werror foreign -Wno-portability])
AM_PATH_GLIB_2_0
AM_PROG_AR
AM_SILENT_RULES([yes])
LT_INIT

# Versioning
I3IPC_MAJOR_VERSION=i3ipc_major_version
I3IPC_MINOR_VERSION=i3ipc_minor_version
I3IPC_MICRO_VERSION=i3ipc_micro_version
I3IPC_VERSION=i3ipc_version
AC_SUBST(I3IPC_MAJOR_VERSION)
AC_SUBST(I3IPC_MINOR_VERSION)
AC_SUBST(I3IPC_MICRO_VERSION)
AC_SUBST(I3IPC_VERSION)

GOBJECT_INTROSPECTION_CHECK([1.32.0])

# Check for gtk-doc
GTK_DOC_CHECK([1.14],[--flavour no-tmpl])

AC_CONFIG_FILES([
                 Makefile
                 i3ipc-glib/Makefile
                 i3ipc-glib/i3ipc-glib.pc
                 doc/Makefile
                 doc/reference/Makefile
                 doc/reference/version.xml
                 ])

AC_OUTPUT
