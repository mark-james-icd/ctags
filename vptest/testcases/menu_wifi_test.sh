#!/bin/bash

output="wifi.log"
echo "Testing Wifi" | tee $output

for (( ; ; ))
do
	echo "======================"
	echo "Wifi Tests"
	echo "======================"
	echo ""
	echo "1) Wifi Connect"
	echo "2) Check for valid IP"
	echo "3) Ping"
	echo "4) Output Network Info"
	echo "5) Speedtest.net test"
	echo "6) Exit"
	echo "Enter Option: "
	read choice

	echo ""
	echo "======================"

	case $choice in
	1) ./wifi_connect.sh | tee -a $output ;;
	2) ./ip_test.sh | tee -a $output ;;
	3) ./ping_test.sh www.google.com | tee -a $output ;;
	4) iwconfig wlan0 | tee -a $output ;;
	5) ./speedtest-cli  | tee -a $output ;;
	6) break ;;
	esac
done

