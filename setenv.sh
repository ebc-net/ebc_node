#!/bin/bash

LIBDIR=lib
SYSLIBDIR=/usr/lib
CDIR=`pwd`

echo $CDIR
cd $SYSLIBDIR

if [ ! -e libudt4.so ]
then
	sudo ln $CDIR/$LIBDIR/libudt4.so .
fi

if [ ! -e  libprotobuf-lite.so.15 ]
then
sudo ln $CDIR/$LIBDIR/libprotobuf-lite.so libprotobuf-lite.so.15
fi


if [ ! -e libebcCryptoLib.so ]
then
	sudo ln $CDIR/$LIBDIR/libebcCryptoLib.so .
fi
