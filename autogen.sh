#!/bin/sh

aclocal -I m4
libtoolize
intltoolize
autoconf
automake -c -a
./configure $@
