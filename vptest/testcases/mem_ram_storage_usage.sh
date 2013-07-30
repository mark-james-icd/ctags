#!/bin/sh

echo ""
echo "Memory usage"
echo ""
egrep --color 'Mem|Cache|Swap' /proc/meminfo
echo ""
df -h

