#!/bin/sh
# this file must be copied to /etc/init.d on the BBB
# use movesh target in makefile to move
# set up GPIOs
echo  66 > /sys/class/gpio/export
echo  67 > /sys/class/gpio/export
echo  69 > /sys/class/gpio/export
echo  68 > /sys/class/gpio/export
echo  45 > /sys/class/gpio/export
echo  44 > /sys/class/gpio/export
echo  26 > /sys/class/gpio/export
echo  47 > /sys/class/gpio/export
echo  46 > /sys/class/gpio/export
echo  27 > /sys/class/gpio/export
echo  65 > /sys/class/gpio/export
echo  30 > /sys/class/gpio/export
echo  60 > /sys/class/gpio/export
echo  31 > /sys/class/gpio/export
echo  48 > /sys/class/gpio/export
echo  49 > /sys/class/gpio/export
echo 117 > /sys/class/gpio/export
echo 115 > /sys/class/gpio/export
echo 112 > /sys/class/gpio/export
echo  20 > /sys/class/gpio/export
