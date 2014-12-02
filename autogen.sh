#!/bin/sh
glibtoolize --ltdl -c -i
aclocal
automake -a
autoreconf --no-recursive
