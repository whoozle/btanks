#!/bin/sh
VER=`svnversion -cn .`
REV=`echo "$VER" | cut -d: -f 2`
DIR=btanks-0.4.$REV
echo "revision $REV"
echo "cleaning up..."
rm -rf ../$DIR
echo "exporting..."
svn export . ../$DIR
echo -n $VER > ../$DIR/.svnversion
rm -f ../$DIR/make-dist.sh
