#!/bin/sh
libtoolize --ltdl -c -i
aclocal
automake -a
autoreconf --no-recursive
