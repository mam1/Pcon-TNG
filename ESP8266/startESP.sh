#!/bin/sh
echo "setting up serial port"
ln -s /dev/tty.SLAB_USBtoUART /dev/tty.usbserial
echo "stating ESPlorer"
java -jar /Applications/ESPlorer/ESPlorer.jar