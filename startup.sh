#!/bin/bash
LOGFILE=/var/log/yd2vcan.log
BINFILE=/data/yd2vcan/build/yd2vcan
CONFFILE=/data/yd2vcan/yd2vcan.conf

date 2>&1 | tee -a $LOGFILE
echo "Starting $BINFILE" 2>&1 | tee -a $LOGFILE
killall $BINFILE 2>&1 | tee -a $LOGFILE

source $CONFFILE

/sbin/ip link add dev $CANIFACE type vcan 2>&1 | tee -a $LOGFILE
/sbin/ip link set up $CANIFACE 2>&1 | tee -a $LOGFILE
/bin/sleep 3
$BINFILE -i $YDIP -p $YDPORT -c $CANIFACE 2>&1 | tee -a $LOGFILE &

echo "$BINFILE started" 2>&1 | tee -a $LOGFILE
