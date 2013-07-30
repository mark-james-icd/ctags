#!/bin/bash

echo "Running set resolution test..."

xrandr --output HDMI-1 --mode 1280x720

sleep 5

xrandr --output HDMI-1 --mode 1920x1080
