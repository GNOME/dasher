#!/bin/sh

aclocal -I m4
intltoolize --copy --force --automake
libtoolize --force
intltoolize
autoconf
automake -c -a
./configure $@
