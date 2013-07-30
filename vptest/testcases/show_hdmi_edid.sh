#!/bin/bash

# requires
# sudo apt-get install read-edid
# requires newest version of read-edid
# http://www.polypux.org/projects/read-edid/
#

# read it
./read-edid-i2c > edid.dat

# display it
parse-edid < edid.dat

#clean up
rm edid.dat

exit 1

