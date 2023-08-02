#!/bin/bash

source /etc/yd2vcan.conf

/sbin/ip link add dev $CANIFACE type vcan
/sbin/ip link set up $CANIFACE
/bin/sleep 1
/usr/bin/yd2vcan -i $YDIP -p $YDPORT -c $CANIFACE >/dev/null 2>/dev/null &
