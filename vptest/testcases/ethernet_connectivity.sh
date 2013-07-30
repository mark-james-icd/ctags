#!/bin/bash

declare -i PASSED=0

./ip_test.sh
OUT=$?
if [ $OUT -eq 0 ];then
	let "PASSED += 1"
fi

./ping_test.sh www.google.com
OUT=$?
if [ $OUT -eq 0 ];then
	let "PASSED += 1"
fi

./network_check
OUT=$?
if [ $OUT -eq 0 ];then
	let "PASSED += 1"
fi


# check for pass or fail
if [ $PASSED -ge 3 ];then
   echo "Ethernet Test PASSED"
   exit 0
else
   echo "Ethernet Test FAILED"
   exit 1
fi
