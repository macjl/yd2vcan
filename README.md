# yd2vcan

Bi-directionnal gateway between YachD NMEA 2000 RAW and a vcan interface.

# Installation
````
cd yd2vcan
make
cp yd2vcan-sample.conf yd2vcan.conf
edit yd2vcan.conf
echo "$PWD/startup.sh" >> /data/rc.local
chmod +x /data/rc.local
````
