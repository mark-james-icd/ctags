Requirements
=============

1. The latest kernel
2. Update the nvidia drivers on the board (see below)
3. Install the following libraries on the board
	
	sudo apt-get install libgtk-3-dev libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev

then run make

Note : the above assumes the test application is built on the target board.
To build on a ubuntu PC you would need to setup the cross complier tools and libraries and modify th make file accordingly. 

Running the application
=======================

On a board with no desktop installed, run X first

startx

then run the test application

overlay parm1 parm2 parm3 (all parm's optional)

parm1 - percentage (between 0-1) of screen to use for test bars (default 0.6)
parm2 - time to display test  (default 5 seconds)
parm3 - pattern to use with gstreamer (default 18)

Alternatively, run the overlay_test.sh script file provided that runs the over test 4 times at different resolutions. 

Updating Nvidia drivers
=======================

You need R16.3.1EA_HW_Compositing_T30.zip which contains the updated driver files. 

 - OpenMAX-IL, gst-omx, Tegra X Driver and the xorg.conf have been updated; 
   for ease of evaluation, this early access build provides replacement 
   versions of the following files included in the R16.3 
   public release:

    - /usr/lib/libnvomx.so
    - /usr/lib/gstreamer-0.10/libgstomx.so
    - /usr/lib/xorg/modules/drivers/tegra_drv.so
    - /etc/X11/xorg.conf
      	The xorg.conf file contains the Overlay Depth & Blend Options.
      	To revert to original configuration, disable the "OverlayDepth"
        and "OverlayCombineMode" options in xorg.conf and relaunch the X server.

 - Installation process:
   - Replace the above files provided in the standard R16.3 release
     with the equivalents provided by this early access build; as 
     an example from within the running target's file system:

       # tar -C / -xjf <path-to>/R16.3.1EA_HW_Compositing_T30.tbz2


Note that README_R16.3.1EA_HW_Compositing_T30.txt refers to the overlay as being supported by nv_omx_videosink.
In our case its the nv_omx_hdmi_videosink that the test is using. Run gst-inspect nv_omx_hdmi_videosink to see what it supports.




