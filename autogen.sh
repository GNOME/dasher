#!/bin/sh

libtoolize
intltoolize
aclocal
autoconf
automake
./configure $@
