#!/bin/bash

# wifi connect
#
# requires sudo to run
#
# param #1 = the output log file name
# optional params #2 #3 #4
# ssid password security (WAP or WEP)
# e.g ./wifi_test.sh ICD_UK password123 WEP
#
# scans for the avaialble wifi networks
# prompts the user to select a network and connects

wifi=$1
key=$2
security=$3

# removing possible previous temp file
rm list.temp 2>/dev/null

# list available wifi connections

# scan the wifi networks and store them in a tmpFile for processing
echo "Running Wifi Connect test..."
echo "Scanning available networks..." 

sudo iwlist scan 2>/dev/null > list.temp

# create a list of essid's and signal levels
eval essidlist=( $(grep -i ESSID list.temp | awk -F":" '/ESSID/{print $2}') )
eval levellist=( $(grep -i level list.temp | awk '/level/{print $3}') )

# tests for number of wifi connections, exits if none
if [ -z "${essidlist[0]}" ]; then
	echo "No available wifi connection"
	exit 1
fi


if [[ ${#wifi} -eq 0 ]]
then
	#display the list with the signal level
	echo "List of available networks"
	len=${#essidlist[@]}
	for (( i=0; i<${len}; i++ ));
	do
		echo $i") "${essidlist[i]}" ("${levellist[i]}")"
	done   

	echo $len") Enter ESSID Manually"

	# get the users choice
	echo -n "Your choice? : "
	read choice

	# process the result
	if [[ $choice -eq $len ]]
	then
		echo -n "Enter ESSID: "
		read wifi

		echo "Security"
		echo "========"
		echo "1) None"
		echo "2) WEP"
		echo "3) WPA"
		echo -n "Enter Security: "
		read choice

		case $choice in
		2) security="WEP" ;;
		3) security="WPA" ;;
		esac

	else 

		#sets essid as value for WIFI variable and displays information about the AP
		wifi=${essidlist[$choice]}

		#put the info about the selected wifi into the temp file
		sudo iwlist scan 2>/dev/null | sed -n "/$wifi"\""/, +29p" > list.temp

		#checks encryption algorithm
		IE=$(grep IE list.temp | sed 's/^ .*IE: \(...\).*/\1/')
		if [[ $IE == *WPA* ]]
		then
		  	security="WPA"
		elif [[ $IE == *WEP* ]]
		then
		  	security="WEP"
		fi	
	fi
fi

# connect
len=${#security}
if [[ $len != 0 ]]
then 
	# prompt for key if not provided
	if [[ ${#key} == 0 ]]
	then
		echo -n "Enter encryption key: "
		read key
	fi
	sudo /usr/share/checkbox/scripts/create_connection -S $security -K $key $wifi
else
	sudo /usr/share/checkbox/scripts/create_connection $wifi
fi


# remove the temp file
rm list.temp 2>/dev/null

#pass or fail	
OUT=$?
if [[ OUT -eq 0 ]]
then
   # passed
   echo "WiFi Test Passed"
   exit 0
else
   # failed
   echo "WiFi Test Failed"
   exit 1
fi

