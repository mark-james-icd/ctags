#!/bin/bash

# Test an IP address for validity:
# Usage:
#      ip_test IP_ADDRESS
#      return 0 if ip is valid

echo "Running IP test..."
stat=1
ip=$(ip a s|sed -ne '/127.0.0.1/!{s/^[ \t]*inet[ \t]*\([0-9.]\+\)\/.*$/\1/p}')
echo $ip
if [[ $ip =~ ^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$ ]]; then
	OIFS=$IFS
	IFS='.'
	ip=($ip)
	IFS=$OIFS
	[[ ${ip[0]} -le 255 && ${ip[1]} -le 255 \
	    && ${ip[2]} -le 255 && ${ip[3]} -le 255 ]]
	stat=$?
fi

if [[ $state -eq 0 ]]
then
   # passed
   echo "IP Address Valid"
   exit 0
else
   # failed
   echo "IP Address Invalid"
   exit 1
fi




