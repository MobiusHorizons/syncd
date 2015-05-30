#!/bin/sh
LIBTOOLIZE=$(which libtoolize) || $(which glibtoolize)
$LIBTOOLIZE --ltdl -c -i
aclocal
automake -a
autoreconf --no-recursive
