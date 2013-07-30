#!/bin/bash
# This script will connect to a bluetooth device
# www.convergeddevices.net new tests specific for Bluetooth
# --
# script optionally accepts paramaters:
#   bz_connect [device address]
#      if the device address isn't passed, then a scan is done giving the user a choice
#   e.g. bz_connect 84:85:06:2F:FE:78
# --

#if bluetooth device address wasn't added as a paramater, scan devices & allow choice
if [ $# != 1  ]; then

  # scans for Bluetooth connections
  echo "Scanning for Bluetooth devices..."

  sudo hcitool scan --class > list.temp

  eval namelist=( $(grep -i name list.temp | awk '/Device name/ {print $3}') )
  eval namelist2=( $(grep -i name list.temp | awk '/Device name/ {print $4}') )
  eval addrlist=( $(grep -i Address list.temp | awk '/BD/ {print $3}') )

  len=${#namelist[@]}
  echo "Number of devices: ${len}"

  #tests for number of bluetooth connections, exits if none
  if [ -z "${namelist[0]}" ]; then
	echo "No available bluetooth devices"
	echo -n "FAILED : Bluetooth failed${newline}"
	exit 1
  fi

  #display the list
  len=${#namelist[@]}
  for (( i=0; i<${len}; i++ ));
  do
	echo $i") "${namelist[i]} ${namelist2[i]}" ("${addrlist[i]}")"
  done   

  # get the users choice
  echo -n " Select device to connect? : "
  read choice

  # process the result
  addr=${addrlist[$choice]}

else

  addr=$1

fi

echo $addr > device.temp

OUT=1

# connect to the device
echo "Connecting to ${addr}"
hidd --connect ${addr}

OUT=$?

if [ $OUT == 0 ];then
   echo "Connection successful"
   exit 0
else
   echo "Connection failed"
   exit 1
fi

rm list.temp 

