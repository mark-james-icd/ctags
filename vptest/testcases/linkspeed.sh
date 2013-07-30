#!/bin/bash

# requires net-tool
# apt-get install ethtool net-tools
# ethtool

echo "Running Link speed test..."

passed=0

# 10mb
mii-tool -F 10baseT-FD
mii-tool eth0 2>/dev/null > list.temp
newvalue=$(grep eth0: list.temp | awk '/eth0:/{print $2$4}')
if [[ $newvalue == "10full" ]]
then
	let "passed += 1"
	echo "Setting 10baseT-FD passed"
else
	echo "Setting 10baseT-FD failed"
fi


#100mb
mii-tool -F 100baseTx-FD
mii-tool eth0 2>/dev/null > list.temp
newvalue=$(grep eth0: list.temp | awk '/eth0:/{print $2$4}')
if [[ $newvalue == "100full" ]]
then
	let "passed += 1"
	echo "Setting 100baseT-FD passed"
else
	echo "Setting 100baseT-FD failed"
fi


#1000mb
mii-tool -F 1000baseT-FD
mii-tool eth0 2>/dev/null > list.temp
newvalue=$(grep eth0: list.temp | awk '/eth0:/{print $2$4}')
if [[ $newvalue == "1000full" ]]
then
	let "passed += 1"
	echo "Setting 1000baseT-FD passed"
else
	echo "Setting 1000baseT-FD failed"
fi

# removing possible previous temp file
rm list.temp 2>/dev/null

if [[ $passed -eq 3 ]]
then
	echo "Link Speed Test Passed"
	exit 1
else
	echo "Link Speed Test Failed"
	exit 0
fi

