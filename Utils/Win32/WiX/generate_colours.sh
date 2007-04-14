#!/bin/bash

idx=1;

for i in ../../../Data/colours/colour*.xml; do
  fname=`basename $i`

  if test $idx -lt 10; then
    echo "              <File Id=\"$fname\" Name=\"COLOUR~$idx.XML\" LongName=\"$fname\" DiskId=\"1\" Source=\"$fname\" />"
  elif test $idx -lt 100; then
    echo "              <File Id=\"$fname\" Name=\"COLOU~$idx.XML\" LongName=\"$fname\" DiskId=\"1\" Source=\"$fname\" />"
  else
    echo "              <File Id=\"$fname\" Name=\"COLO~$idx.XML\" LongName=\"$fname\" DiskId=\"1\" Source=\"$fname\" />"
  fi
  idx=$(($idx+1));
done
