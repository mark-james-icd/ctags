============================================
Requirements
============================================

1) Python 2.4-2.7

2) net-tool
sudo apt-get install ethtool net-tools

3) checkbox qt (pre installed on ubuntu)
sudo apt-get install checkbox-qt checkbox

4) Bluetooth
sudo apt-get install bluez-utils
sudo apt-get install bluez-compat

5) usb utils
sudo apt-get install usbutils

============================================
USB Tests
============================================

1) Check something plugged into each external USB
Command : usb_device_check.sh

2) Removable Storage Test
Command : removable_storage_test.sh usb

3) List the removable devices and mounting status
Command : removable_storage_test.sh usb -l

============================================
WiFi Tests
============================================

1) Wifi Connect
Command : wifi_connect.sh
Optional Parms : #1 SSIS  #2 key  #3 security type  e.g. wifi_connect.sh ICD_UK password123 WPA

2) Check for valid IP
Command : ip_test.sh

3) Ping
Command : ping_test.sh
Parms : url to ping e.g. ping_test.sh www.google.com

4) Output Network Info
Command : iwconfig wlan0

5) Speedtest.net test
Command :speedtest-cli

============================================
Network Tests
============================================

1) Select hardwired
Command : wifi_onoff.sh off
Parms : on/off to swith wifi on/off

2) Output Network Info
Command : network_info.sh
Requires sudo

3) Set IP address
Command : set_ip_address.sh
optional Parms : iface ip e.g set_ip_address.sh eth0 10.1.11.200

4) Force link negotiation 10/100/1000mbps
Command : linkspeed.sh

5) Check for valid IP
Command : ip_test.sh

6) Ping
Command : ping_test.sh www.google.com

7) Speedtest.net test
Command : speedtest-cli

8) HTTP Test
Command : network_check

9) Select wifi
Command : wifi_onoff.sh on
Parms : on/off to swith wifi on/off

============================================
Bluetooth Tests
============================================

1) connect bluetooth device
command : bz_connect.sh 
optional params : address of the device to connect to

2) disconnect bluetooth device
command : bz_disconnect.sh

============================================
Memory Tests
============================================

1) Outputs memory usage data
command : mem_ram_storage_usage

============================================
Lightring tests
============================================

1) Test flashers
command : lightringtest 1
Requires SUDO

2) Test indicator lights
command : lightringtest 2
Requires SUDO

3) Test light ring
command : lightringtest 3
Requires SUDO

=============================================
hdmi
=============================================

1) Show edid information
command : show_hdmi_edid.sh
Requires SUDO

=============================================
audio
=============================================

1) Play sound to listen through headset
command : play_1000hz_sound.sh

2) play sound through hdmi
command : play_hdmi_audio.sh

3) record sound with headset mic, then plays it back
command : record_sound.sh

4) record sound with rcu mic, then plays it back
command : record_sound.sh

5) Play 1000khz sound
command : play_1000hz_sound.sh

=============================================
resolution tests
=============================================

1) Test set resolution (sets 1280x720 then 1980x1080)
command : set_output_res.sh





