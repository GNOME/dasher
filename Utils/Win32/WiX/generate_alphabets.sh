#!/bin/bash

idx=1;

for i in ../../../Data/alphabets/alphabet*.xml; do
  fname=`basename $i`

  if test $idx -lt 10; then
    echo "              <File Id=\"$fname\" Name=\"ALPHAB~$idx.XML\" LongName=\"$fname\" DiskId=\"1\" Source=\"$fname\" />"
  elif test $idx -lt 100; then
    echo "              <File Id=\"$fname\" Name=\"ALPHA~$idx.XML\" LongName=\"$fname\" DiskId=\"1\" Source=\"$fname\" />"
  else
    echo "              <File Id=\"$fname\" Name=\"ALPH~$idx.XML\" LongName=\"$fname\" DiskId=\"1\" Source=\"$fname\" />"
  fi
  idx=$(($idx+1));
done
