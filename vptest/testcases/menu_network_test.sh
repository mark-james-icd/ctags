#!/bin/bash

output="network.log"
echo "Testing Network" | tee $output

for (( ; ; ))
do
	echo "======================"
	echo "Network Tests"
	echo "======================"
	echo ""
	echo "1) Select hardwired"
	echo "2) Output Network Info"     
	echo "3) Set IP address"
	echo "4) Force link negotiation 10/100/1000mbps"
	echo "5) Check for valid IP"
	echo "6) Ping"
	echo "7) Speedtest.net test"
	echo "8) HTTP Test"

	echo "9) Select wifi"
	echo "10) Exit"
	echo "Enter Option: "
	read choice

	echo ""
	echo "======================"

	case $choice in
	1) ./wifi_onoff.sh off | tee -a $output ;;
	2) ./network_info.sh | tee -a $output ;;
	3) ./set_ip_address.sh eth0 | tee -a $output ;;
	4) ./linkspeed.sh | tee -a $output ;;
	5) ./ip_test.sh | tee -a $output ;;
	6) ./ping_test.sh www.google.com | tee -a $output ;;
	7) ./speedtest-cli  | tee -a $output ;;
	8) ./network_check  | tee -a $output ;;
	9) ./wifi_onoff.sh on | tee -a $output ;;
	10) break ;;
	esac
done
