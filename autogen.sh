#!/bin/sh

libtoolize
intltoolize
aclocal -I m4
autoconf
automake
./configure $@
