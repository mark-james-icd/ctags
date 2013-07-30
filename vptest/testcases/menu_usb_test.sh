#!/bin/bash

output="usb.log"
echo "Test USB" | tee $output

for (( ; ; ))
do
	echo "======================"
	echo "USB Tests"
	echo "======================"
	echo ""
	echo "1) Check something plugged into each external USB"
	echo "2) Removable Storage Test"     
	echo "3) List the removable devices and mounting status"     
	echo "4) Exit"
	echo "Enter Option: "
	read choice

	echo ""
	echo "======================"

	case $choice in
	1) ./usb_device_check.sh | tee -a $output ;;
	2) ./removable_storage_test.sh usb | tee -a $output ;;
	3) ./removable_storage_test.sh usb -l | tee -a $output ;;
	4) break ;;
	esac
done
