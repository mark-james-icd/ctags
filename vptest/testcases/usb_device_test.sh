#!/usr/bin/python

# on the board there are 2 usb buses 001 and 002
# This test work by checking that on each bus we find 2 devices
# 001 hub + one device
# 002 hub + one device
# requires usbutils

import sys
import os
import re
import subprocess

class DiskTest():
    def __init__(self):
        self.process = None       
        self.returnCode = None
       
    def get_usb_info(self, bus):
	device_re = re.compile("Bus\s+(?P<bus>\d+)\s+Device\s+(?P<device>\d+).+ID\s(?P<id>\w+:\w+)\s(?P<tag>.+)$", re.I)
	df = subprocess.check_output("lsusb -s %s:" % bus, shell=True)
	count = 0
	for i in df.split('\n'):
	    if i:
		count = count + 1
        	info = device_re.match(i)
        	if info:
        	    dinfo = info.groupdict()
        	    dinfo['device'] = '/dev/bus/usb/%s/%s' % (dinfo.pop('bus'), dinfo.pop('device'))
#		    print dinfo['device']
	return count

def main():

    print "Running USB tests"
    test = DiskTest()
    bus1 = test.get_usb_info("001")
    bus2 = test.get_usb_info("002")

    if bus1>=2 and bus1>=2:
	print "Usb test passed"
	return 1

    print "Usb test failed"
    return 0

if __name__ == '__main__':
    sys.exit(main())
