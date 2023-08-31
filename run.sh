#!/bin/sh

BUILDDIR=.builddir

if [ ! -d $BUILDDIR ]; then
    ./build.sh
else
    ninja --quiet -C $BUILDDIR
fi

"$BUILDDIR/whichfont" "$@"
#$BUILDDIR/whichfont $*
