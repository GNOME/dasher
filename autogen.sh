#!/bin/sh

aclocal -I m4
intltoolize --copy --force --automake
libtoolize --force
autoconf
automake -c -a
./configure $@
