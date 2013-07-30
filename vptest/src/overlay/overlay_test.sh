#!/bin/bash

xrandr --output HDMI-1 --mode 1920x1080
./overlay

xrandr --output HDMI-1 --mode 1280x720
./overlay

xrandr --output HDMI-1 --mode 720x480
./overlay

xrandr --output HDMI-1 --mode 640x480
./overlay

xrandr --output HDMI-1 --mode 1280x720
