#!/bin/bash
LOGFILE=/var/log/yd2vcan.log
source /etc/yd2vcan.conf

/sbin/ip link add dev $CANIFACE type vcan 1>> $LOGFILE 2>> $LOGFILE
/sbin/ip link set up $CANIFACE 1>> $LOGFILE 2>> $LOGFILE
/bin/sleep 3
/usr/bin/yd2vcan -i $YDIP -p $YDPORT -c $CANIFACE 1>> $LOGFILE 2>> $LOGFILE &
