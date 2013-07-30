#!/bin/bash

# parm #1 on or off

##http://manpages.ubuntu.com/manpages/maverick/man1/nmcli.1.html
#nmcli con up id "Wired connection 1"

echo "Running Wifi "$1" test..."

# get the wired connection
nmcli -f TYPE con > list.temp
mapfile -t connections < <(grep '^[^#;]' list.temp)

ethernet=0
len=${#connections[@]}
for (( i=0; i<${len}; i++ ));
do
	if [[ ${connections[i]} -eq "802-3-ethernet" ]]
	then
		echo ${connections[i]}
		ethernet=$i
	fi	
done

nmcli -f NAME con > list.temp
mapfile -t connections < <(grep '^[^#;]' list.temp)

ethernetName=${connections[$ethernet]}
# strip off  railing white space
ethernetName=$(echo "$ethernetName" | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')

# removing possible previous temp file
rm list.temp 2>/dev/null

# turn wifi on/off
nmcli nm wifi $1

if [[ $1 == "on" ]]
then
	nmcli con down id "$ethernetName"
#	nmcli dev connect iface eth0
	ifconfig eth0 down
else
	ifconfig eth0 up
	nmcli con up id "$ethernetName"
fi

# test pass or fail
nmcli dev > list.temp
echo $1
line=""
if [[ $1 == "on" ]]
then
	line=$(grep -i wlan list.temp)
else
	line=$(grep -i eth list.temp)
fi

# removing possible previous temp file
rm list.temp 2>/dev/null

if [[ $line == *"connected"* ]]
then
	echo "Wifi "$1" passed"
	exit 0
fi

exit 1

