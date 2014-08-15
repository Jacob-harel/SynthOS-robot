#
# Project:       Arduino (DFRobot rover v2) robot
# File:          Makefile
# Author:        Igor Serikov
# Date:          07-29-2014
#
# Purpose:       The makefile to build the firmware.
#
# Copyright (c) 2014 Zeidman Technologies, Inc.
# 15565 Swiss Creek Lane, Cupertino California, 95014 
# All Rights Reserved
#
# Zeidman Technologies gives an unlimited, nonexclusive license to
# use this code  as long as this header comment section is kept
# intact in all distributions and all future versions of this file
# and the routines within it.
#

SRCS=robot.c temp.1.c util.c uart.c print.c synthos-support.c timer.c hardware.c

.PHONY: default
.PHONY: clean
.PHONY: upload

default: work/robot.out

-include work/.depends

work/synthos: $(SRCS) project.sop work/.done
	avr-gcc -MM -MT work/synthos -mmcu=atmega328p $(SRCS) > work/.depends
	$(HOME)/Projects/SynthOSDriver/synthos project.sop -o work/synthos -I avr-include -I avr-gcc-include

work/robot.out: work/synthos
	avr-gcc -Os -flto -mmcu=atmega328p work/synthos/*.c -o work/robot.out

work/robot.hex: work/robot.out
	avr-objcopy -O ihex -R .eeprom work/robot.out work/robot.hex

temp.1.c: motors.c work/.done
	sed -e '/--- cut ---/,$$w work/temp' motors.c > temp.1.c
	echo >> temp.1.c
	sed -e 's/left/tempxyz/' -e 's/right/left/' -e 's/tempxyz/right/' work/temp >> temp.1.c

work/.done:
	mkdir work
	touch work/.done

upload: work/robot.hex
	avrdude -F -V -c arduino -p ATMEGA328P -P /dev/ttyACM0 -b 115200 -U flash:w:work/robot.hex

clean:
	rm -rf work
	rm -f temp.1.c
