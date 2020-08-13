#! /bin/sh

A=`echo $1_ | tr '[:lower:]' '[:upper:]'`

sed -e s/newClass/$1/g -e s/NEWCLASS_/$A/g < newClassT.cc > tmp_newClass.cc
sed -e s/newClass/$1/g -e s/NEWCLASS_/$A/g < newClassT.hh > tmp_newClass.hh

mv -i tmp_newClass.cc $1.cc && echo $1.cc - ok
mv -i tmp_newClass.hh $1.hh && echo $1.hh - ok
