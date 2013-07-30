#!/bin/bash
# This script will disconnect the bluetooth device connection
# www.convergeddevices.net new tests specific for Bluetooth
# --
# script optionally accepts paramaters:
#   bz_disconnect [device address]
#      if the device address isn't passed, then the address is assumed it's in the temp file created from the scan
#   e.g. bz_disconnect 84:85:06:2F:FE:78
# --


if [ $# != 1  ]; then
   # try and get the device address from the temp file
   if [ ! -f device.temp ];then
      # cannot proceed with device address
      echo "Failed - Device file not found!"
      exit 1
   fi
   read -r addr < device.temp
else
   addr=$1
fi

#echo $addr

len=${addr}
if [ len == 0 ];then
   echo "No device address"
   exit 1
fi

OUT=1

# disconnecting the device
echo "Disconnecting ${addr}"
hidd --kill ${addr}

OUT=$?

if [ $OUT == 0 ];then
   echo "Disconnection successful"
   exit 0
else
   echo "Disconnection failed"
   exit 1
fi

rm device.temp


