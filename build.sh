#!/bin/sh

BUILDDIR=.builddir

if [ ! -d $BUILDDIR ]; then
    meson setup $BUILDDIR
fi
ninja -C $BUILDDIR
