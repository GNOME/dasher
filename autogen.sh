#!/bin/sh

cat m4/* >acinclude.m4
aclocal
libtoolize --force
intltoolize --force
autoconf
automake -c -a
./configure $@
