#!/bin/sh

aclocal -I m4
libtoolize --force
intltoolize --force
autoconf
automake -c -a
./configure $@
