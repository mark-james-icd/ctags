#!/bin/bash

# ping test
# pass in the host to ping as a parameter
# e.g ping_test www.google.com
echo "Running Ping test..."

count=$(ping -c 1 $1 | grep 'received' | awk -F',' '{ print $2 }' | awk '{ print $1 }')
if [[ $count -eq 0 ]]
then
	# failed
	echo "Ping failed"
	exit 1
else
	# success
	echo "Ping passed"
	exit 0
fi
