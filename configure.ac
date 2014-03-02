AC_PREREQ(2.69)

AC_INIT([i3-ipc], [0.0.1], [tony@dubstepdish.com])

m4_pattern_allow([AM_PROG_AR])
m4_pattern_allow([AM_INIT_AUTOMAKE])

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
PKG_CHECK_MODULES([glib], [glib-2.0 >= 2.38])
PKG_CHECK_MODULES([gobject], [gobject-2.0 >= 2.38])
PKG_CHECK_MODULES([gio], [gio-2.0])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h sys/socket.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset socket strerror])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_PATH_GLIB_2_0
AM_PROG_AR
AM_SILENT_RULES([yes])
LT_INIT

GOBJECT_INTROSPECTION_CHECK([1.38.0])

AM_PATH_PYTHON([3],, [:])

AC_ARG_WITH([pygi_overrides_dir],
            AC_HELP_STRING([--with-pygi-overrides-dir], [Path to pygobject overrides directory]))

AC_MSG_CHECKING(for pygobject overrides directory)
if test "x$with_pygi_overrides_dir" = "x" ; then
    overrides_dir="`$PYTHON -c 'import gi; print(gi._overridesdir)' 2>/dev/null`"
    # fallback if the previous failed
    if test "x$overrides_dir" = "x" ; then
        overrides_dir="${pyexecdir}/gi/overrides"
    fi
else
    overrides_dir="$with_pygi_overrides_dir"
fi

AM_CONDITIONAL([HAS_PYGI], [test -d "$overrides_dir"])

PYGI_OVERRIDES_DIR="$overrides_dir"
AC_SUBST(PYGI_OVERRIDES_DIR)

AM_COND_IF([HAS_PYGI], AC_MSG_RESULT($PYGI_OVERRIDES_DIR), AC_MSG_RESULT([No]))

AC_CONFIG_FILES([
                 Makefile
                 i3ipc-glib/Makefile
                 i3ipc-glib/i3ipc-glib.pc
                 overrides/Makefile
                 ])

AC_OUTPUT
