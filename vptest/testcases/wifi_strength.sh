#!/bin/bash

# displays the wifi strength

echo "Wifi strength test"

iwconfig wlan1 | grep -i --color quality

exit 1
