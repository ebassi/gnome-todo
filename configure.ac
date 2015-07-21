dnl -*- mode: m4 -*-
AC_PREREQ([2.69])
AC_INIT([gnome-todo],[3.17.4],[https://github.com/GeorgesStavracas/gnome-todo/issues],[gnome-todo])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([1.11.1 foreign -Wno-portability dist-xz no-dist-gzip tar-ustar subdir-objects])
AM_MAINTAINER_MODE([enable])
AM_SILENT_RULES([yes])

# Releases do not have git directories
AX_IS_RELEASE([git-directory])

## don't rerun to this point if we abort
AC_CACHE_SAVE

# We require additional macros later on
AX_REQUIRE_DEFINED([APPSTREAM_XML])
AX_REQUIRE_DEFINED([IT_PROG_INTLTOOL])

# Add debug symbol
AX_CHECK_ENABLE_DEBUG([yes], [GNOME_TODO_ENABLE_DEBUG])

# Enable compiler warnings
AX_COMPILER_FLAGS([GNOME_TODO_WARN_CFLAGS], [GNOME_TODO_WARN_LDFLAGS])

AC_PROG_CC_C_O
AC_PROG_INSTALL

# enable libtool
LT_PREREQ([2.2.6])
LT_INIT([disable-static])

## don't rerun to this point if we abort
AC_CACHE_SAVE

PKG_PROG_PKG_CONFIG

GLIB_GSETTINGS
AM_GLIB_GNU_GETTEXT

dnl ***************************************************************************
dnl Internationalization
dnl ***************************************************************************
IT_PROG_INTLTOOL([0.40.6])

GETTEXT_PACKAGE=AC_PACKAGE_TARNAME
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])

dnl ================================================================
dnl Misc
dnl ================================================================

GLIB_COMPILE_RESOURCES=`$PKG_CONFIG --variable glib_compile_resources gio-2.0`
AC_SUBST([GLIB_COMPILE_RESOURCES])

PKG_CHECK_MODULES(GNOME_TODO,
                  gmodule-export-2.0
                  gio-2.0 >= 2.43.4
                  glib-2.0 >= 2.43.4
                  goa-1.0 >= 3.2.0
                  gtk+-3.0 >= 3.16.0
                  libecal-1.2 >= 3.13.90
                  libedataserver-1.2 >= 3.16.4
                  libedataserverui-1.2 >= 3.16.4
                  libical >= 0.43)

APPSTREAM_XML

AC_CONFIG_FILES([
      Makefile
      src/Makefile
      data/Makefile
      data/org.gnome.Todo.desktop.in
      po/Makefile.in
])

AC_OUTPUT

dnl ==========================================================================
echo "

        gnome-todo $VERSION
        ==================

        source at:     ${srcdir}
        prefix:        ${prefix}
        compiler:      ${CC}
	global flags:  ${CFLAGS} ${CPPFLAGS} ${LDFLAGS}
	project flags: ${GNOME_TODO_CFLAGS} ${GNOME_TODO_LIBS} ${LIBS}
        warning flags: ${GNOME_TODO_WARN_CFLAGS} ${GNOME_TODO_WARN_LDFLAGS}
        release:       ${ax_is_release}

        Now type 'make' to build $PACKAGE
"
