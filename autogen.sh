#!/bin/sh

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

(test -f "$srcdir/configure.ac") || {
        echo "*** ERROR: Directory '$srcdir' does not look like the top-level directory of the project ***"
        exit 1
}

PKG_NAME=`autoconf --trace 'AC_INIT:$1' "$srcdir/configure.ac"`

if [ "$#" = 0 -a "x$NOCONFIGURE" = "x" ]; then
        echo "*** WARNING: I am going to run \`configure' with no arguments." >&2
        echo "If you wish to pass any to it, please specify them on the" >&2
        echo \`$0\'" command line. ***" >&2
        echo "" >&2
fi

set -x
aclocal --install || exit $?
glib-gettextize --force --copy || exit $?
intltoolize --force --copy --automake || exit $?
autoreconf --verbose --force --install -Wno-portability || exit $?

if [ "x$NOCONFIGURE" = x ]; then
        $srcdir/configure "$@" || exit $?
else
        echo "Skipping configure process."
fi

{ set +x; } 2>/dev/null
