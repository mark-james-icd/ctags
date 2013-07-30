/*
 * Copyright (c) 2011-2012, NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 *
 * version: 0.2
 * <Cotigao>
 */


=> PREREQUISITES:

1. The nvgstcapture application supports gstreamer version 0.10.32 by default.

2. Install GStreamer-0.10 on the target board with following command:

   sudo apt-get install gstreamer-tools gstreamer0.10-alsa gstreamer0.10-plugins-good gstreamer0.10-plugins-base gstreamer0.10-plugins-ugly gstreamer0.10-plugins-bad gstreamer0.10-ffmpeg

3. To ensure the device is running gstreamer version 0.10.32,use the following command before you start using the nvgstcapture application:

   gst-inspect --version

4. Execute the following commands on the target board's Ubuntu command line:

   export DISPLAY=:0
   xinit &

5.  Make the following amixer settings (for K3.1) to enable audio capture using the built-in microphone on Cardhu before running the nvgstcapture application:

  amixer cset name='Int Mic Switch' on
  amixer cset name='ADC Input' 1
  amixer cset name='Left Input PGA Switch' on
  amixer cset name='Right Input PGA Switch' on
  amixer cset name='Left Input Mux' on
  amixer cset name='Right Input Mux' on
  amixer cset name='Headphone Volume' 63,63
  amixer cset name='Left Input PGA Volume' 31,31
  amixer cset name='Right Input PGA Volume' 31,31

=> NvGstCapture Usage:

* The nvgstcapture application can capture audio and video data using the microphone and camera on the device and encapsulate encoded AV data in a container file.

* The nvgstcapture application supports both command-line (with automation) and runtime options. For complete usage infomration, use the following command:

   ./nvgstcapture --help

=> NvGstCapture Application Options:

* Command-line options:

    Usage: ./nvgstcapture --option

           --or--

           ./nvgstcapture -option

  -V, --vcap_res                    Video Capture width and height                                         eg. -V 720x480
  -I, --icap_res                    Image Capture width and height                                         eg. -I 720x480
  -m, --mode                        Capture mode value (2=video 1=still)                                   eg. -m 2
  -e, --exposure                    Capture exposure value                                                 eg. -e 0
  -i, --iso                         Capture ISO value                                                      eg. -i 200
  -n, --contrast                    Capture contrast value                                                 eg. -n 33
  -o, --metermode                   Capture metermode value                                                eg. -o 1
  -f, --flicker                     Capture flicker value                                                  eg. -f 2
  -b, --brightness                  Capture brightness value                                               eg. -b 33
  -s, --saturation                  Capture saturation value                                               eg. -s 33
  -h, --hue                         Capture hue value                                                      eg. -h 33
  -y, --edgeenhancement             Capture edgeenhancement value                                          eg. -y 0
  -g, --imagefilter                 Capture imagefilter value                                              eg. -g 0
  -w, --whitebalance                Capture whitebalance value                                             eg. -w 0
  -d, --stereo                      Stereo mode (0=Off 1=Left Only 2=Right Only 3=Stereo full)             eg. -d 1
  -v, --video_enc                   Video encoder type (0=H.264 1=H.263 2=MPEG-4)                          eg. -v 0
  -u, --audio_enc                   Audio encode type (0=AAC 1=AMR)                                        eg. -u 0
  -k, --file_type                   Container file type (0=MP4 1=3GP 2=AVI)                                eg. -k 0
  -x, --voice_control               Voice control value (1=on 0=off)                                       eg. -x 0
  -q, --sensorid                    Camera sensor ID (0=Primary(Left-back) 1=Secondary(Front) 3=stereo)    eg. -q 0
  -F, --fl_mode                     Camera Flash mode                                                      eg. -F 2
  --frate                           Capture frame rate (min = 5, max = 30)                                 eg. --frate 30
  --svs                             Chain for video preview (nv_omx_videosink: for overlay,
                                       nvxvimagesink: for xvimagesink)
  --rt                              Render Target, 0=(Mixer + EGL), 1=(EGL), 2=(Mixer + Overlay)           eg. --rt 1

NOTE: the "-d" option is only valid if sensorid=3 is set, for example:

    ./nvgstcapture -q 3 -d 2

  * VIP-specific options:

  --vip                             Use Video input
  --format_type                     Capture format type: 0=NTSC, 1=PAL
  --frame_type                      Capture frame type: 0=Progressive, 1=Interlaced

* Runtime options:

   * Quit : 'q'

   * Set Capture Mode:
      mo:<val>
          (2): video
          (1): image

     Get Capture Mode:
      gmo

   * Capture: enter 'j'

      --or--

      'j' followed by a timer (eg. jx5000, capture after 5 seconds) OR
      'j' followed by multishot count (eg. j:6, capture 6 images)

      Timer and multishot values are optional. Capture defaults to single shot when the timer is set to 0s.

   * Start Recording: '1'
     Stop Recording: '0'

   * Set Zoom value:
      z:<val> (for example, z:1.5)

     Get Zoom value:
      gz

   * Set Container file type (0=MP4 1=3GP 2=AVI):
      k:<val> (for example, k:0)

     Get Container file type:
      gk

   * Set Image Capture Resolution (only in image mode):
      icr:<w>x<h> (for example, icr:640x480)

     Get Image Capture Resolution:
      gicr

   * Set Video Capture Resolution (only in video mode):
      vcr:<w>x<h> (for example, vcr:640x480)

     Get Video Capture Resolution:
      gvcr

   * Set Flash:
      xx:<val>, val =
          (0) : auto
          (1) : off
          (2) : on
          (3) : fill-in (Not supported)
          (4) : red-eye (Not supported)
          (5) : torch   (Not supported)

     Get Flash:
      gxx

   * Rotation Preview:
      px:<val>, val=0, 90, 180, 270
     Rotation Capture:
      cx:<val>, val=0, 90, 180, 270

   * Set Exposure:
      ex:<val>, Range val= -1 - 2147483647, 0=auto

     Get Exposure:
      gex

   * Set White Balance:
      wb:<val>, val =
          (0)  : auto
          (1)  : daylight
          (2)  : cloudy
          (3)  : sunset
          (4)  : tungsten
          (5)  : fluorescent
          (6)  : shade
          (7)  : incandescent
          (8)  : flash
          (9)  : horizon
          (10) : off

     Get White Balance:
      gwb

   * Set ISO Mode:
      io:<val>, val = 100, 200, 400, 800, 0 (auto)

     Get ISO Mode:
      gio

   * Set Metering:
      mt:<val>,
          (0) : average
          (1) : spot
          (2) : matrix

     Get Metering:
      gmt

   * Set EV Compensation:
      ev:<val>, Range: -2.5 - 2.5

     Get EV Compensation:
      gev

   * Set Brightness:
      bt:<val>, Range: 0 - 100

     Get Brightness:
      gbt

   * Set Saturation:
      st:<val>, Range: -100 - 100

     Get Saturation:
      gst

   * Set Contrast:
      ct:<val>, Range: 0 - 100

     Get Contrast:
      gct

   * Set Hue:
      ht:<val>, Range: 0 - 100

     Get Hue:
      ght

   * Set Flicker Control:
      fl:<val>, val =
          (0) : off
          (1) : 50Hz
          (2) : 60Hz
          (3) : auto

     Get Flicker Control:
      gfl

   * Set colour-tone-mode Control:
      ft:<val>, val =
          (0)  : normal
          (1)  : sepia
          (2)  : negative
          (3)  : grayscale (Not supported)
          (4)  : natural (Not supported)
          (5)  : vivid (Not supported)
          (6)  : colorswap (Not supported)
          (7)  : solarize
          (8)  : out-of-focus (Not supported)
          (9)  : sky-blue (Not supported)
          (10) : grass-greeen (Not supported)
          (11) : skin-whiten (Not supported)
          (12) : noise (Not supported)
          (13) : emboss
          (14) : sketch (Not supported)
          (15) : oilpaint (Not supported)
          (16) : hatch(Not supported)
          (17) : gpen(Not supported)
          (18) : antialias(Not supported)
          (19) : dering(Not supported)
          (20) : posterize
          (21) : bw
          (22) : manual
          (23) : aqua
     Get colour-tone-mode Control:
      gft

   * Set Scene Mode:
      scm:<val>, val =
          (0)  : Manual
          (1)  : Closeup(Not supported)
          (2)  : Portrait
          (3)  : Landscape
          (4)  : Sports
          (5)  : Night
          (6)  : Auto
          (7)  : Action
          (8)  : Beach
          (9)  : Candlelight
          (10) : Fireworks
          (11) : NightPortrait
          (12) : Party
          (13) : Snow
          (14) : Sunset
          (15) : Theatre
          (16) : Barcode
     Get Scene Mode:
      gscm

   * Set Focus Mode:
      fm:<val>, val =
          (0) : auto
          (1) : macro
          (2) : portrait(Not supported)
          (3) : infinity
          (4) : hyperfocal(Not supported)
          (5) : extended(Not supported)
          (6) : continuous-normal(Not supported)
          (7) : continuous-extended(Not supported)
          (8) : continuous-picture
          (9) : continuous-video
          (10) : fixed
     Get Focus Mode:
      gfm

   * Set Noise-reduction Mode:
      nr:<val>, val =
          (1) : on
          (0) : off
     Get Noise-reduction Mode:
      gnr

   * Set Edge Enhancement:
      ee:<val>, Range: -1.0 - 1.0,
     Get Edge Enhancement:
      gee

   *  Set Stereo Mode: (Only applicable if sensorid=3)
      stm:<val>, val =
          (0) : Off
          (1) : Left Only
          (2) : Right Only
          (3) : Stereo Full
     Get Stereo Mode:
      gstm

   * Set Audio Encoder:
      ae:<val>, val= 0 (aac), 1 (amr)
     Get Audio Encoder:
      gae

   * Set Video Encoder:
      ve:<val>, val= 0(h264), 1(h263)[Set Default Resolution:704x576(4CIF)], 2(mpeg4)
     Get Video Encoder:
      gve

   For information on these options, use the --help option.

=> Capture Resolution support

* Image Encode

    176 x 144
    320 x 240
    480 x 480
    640 x 480
    800 x 600
    720 x 480
    1280 x 720
    1280 x 960 (1 MP)
    1600 x 1200 (2 MP)
    2048 x 1536 (3 MP)
    2240 x 1680 (4 MP)
    2560 x 1920 (5 MP)

* Video Encode

  * H.264/MPEG-4 (supports "MP4/3GP/AVI" container)

    176 x 144
    320 x 240
    640 x 480
    720 x 480
    1280 x 720
    1920 x 1080

  * H.263 (supports "3GP/AVI" container)

    176 x 144 (QCIF)
    352 x 288 (CIF)
    704 x 576 (4CIF)


NOTES:

1. The nvgstcapture application generates image and video output files in the same directory as the application itself.

2. Filenames for image and video content are in the formats "nvgsttest(%d).jpg" and "nvgsttest(%d).mp4" respectively,
   where %d is a counter starting from 0 every time you run the application. Rename or move files between runs, to avoid
   overwriting results you want to save.

3. To encode with H.263 use 3GP or AVI container formats. The MP4 container format is not supported for H.263 encoding.

4. The nvgstcapture application also supports camerabin2 and Gstphotography, which require gstreamer version 0.10.36 at a minimum.
   Compile and install gstreamer version 0.10.36 on the device to use camerabin2 and Gstphotography.
