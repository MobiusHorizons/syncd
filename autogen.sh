#!/bin/sh
aclocal
automake -a
glibtoolize --ltdl -c -i
autoreconf --no-recursive
