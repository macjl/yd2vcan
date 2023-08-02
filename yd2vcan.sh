#!/bin/bash
LOGFILE=/var/log/yd2vcan.log

date 2>&1 | tee -a $LOGFILE
echo "Starting yd2vcan" 2>&1 | tee -a $LOGFILE
killall /usr/bin/yd2vcan 2>&1 | tee -a $LOGFILE

source /etc/yd2vcan.conf

/sbin/ip link add dev $CANIFACE type vcan 2>&1 | tee -a $LOGFILE
/sbin/ip link set up $CANIFACE 2>&1 | tee -a $LOGFILE
/bin/sleep 3
/usr/bin/yd2vcan -i $YDIP -p $YDPORT -c $CANIFACE 2>&1 | tee -a $LOGFILE &

echo "yd2vcan started" 2>&1 | tee -a $LOGFILE
