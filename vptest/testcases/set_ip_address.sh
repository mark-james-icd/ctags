#!/bin/bash

# param #1 the ip address to set
# sets the IP address
# checks it got set OK

echo "Running Set IP Address test..."

passed=0
interface=$1
ip=$2

echo $1 $2

if [[ ${#ip} -eq 0 ]]
then
	echo "Enter New IP Address: "
	read ip
fi

if [[ ${#interface} -eq 0 ]]
then
	echo "Enter Interface (e.g eth0,wlan0): "
	read interface
fi


# removing possible previous temp file
rm list.temp 2>/dev/null

ifconfig $interface $ip

#pass or fail	
OUT=$?
if [[ OUT -eq 0 ]]
then
	# passed
	let "passed += 1"
else
	# failed
	exit 1
fi

# to be sure read back the now set IP
ifconfig $interface0 2>/dev/null > list.temp
newip=$(grep inet list.temp | awk '/inet/{print $2}')

rm list.temp 2>/dev/null

if [[ $newip == *"$ip"* ]]
then
	let "passed += 1"
fi

#pass or fail	
if [[ $passed == 2 ]]
then
   # passed
   echo "IP Address set passed"
   exit 0
else
   # failed
   echo "IP Address set failed"
   exit 1
fi
