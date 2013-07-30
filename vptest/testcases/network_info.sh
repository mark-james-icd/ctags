#!/bin/bash

echo "Network information \n"

HOSTNAME=`hostname`
NS=$(cat /etc/resolv.conf  | grep -v '^#' | grep nameserver | awk '{print $2}')
SEARCH=$(cat /etc/resolv.conf  | grep -v '^#' | grep search | awk '{print $2}')


echo "hostname :" $HOSTNAME
echo "Named Server :" $NS
echo "Search :" $SEARCH


#gets a list fo the interfaces
LIST=`ip link show | grep BROADCAST | awk {' print $2 '} | awk -F: {' print $1 '}`

# for each interface
for INTERFACE in $LIST
do
        if [[ $INTERFACE == "eth0" || $INTERFACE == "eth1" || $INTERFACE == "wlan0" || $INTERFACE == "wlan1" ]]
        then
	echo "==============================================================="
	./network_info_interface.sh $INTERFACE
	ethtool $INTERFACE	
        echo ""
        fi
done

