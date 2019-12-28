# \brief	GNU make include file for AVRs
# \author	Circuit Chaos
# \date		2017-11-18
#
# Building for AVR includes these steps:
#
# Creation of .out, .bin, .hex, .lst files in build directory
# Programming AVR with .hex file, fuses and lock bits
#
# Building files is delegated to SCons -- see avr.scons. Programming
# is done in this makefile.
#
# Make targets:
#
# - build: build everything
# - clean: clean everything
# - poke: upload to MCU
#
# Needed variables:
#
# - NAME	- project name
# - MCU		- target MCU
# - FREQ	- clock frequency in Hz (see MFREQ)
# - FUSEH	- high fuse in hex
# - FUSEL	- low fuse in hex
#
# Prerequisites:
#
# - src: source directory
# - build: build directory

OURPATH		= avrbuild

SCONS		= scons
AVRDUDE		= avrdude

# PROG		= buspirate
PROG		= usbasp
PORT		= /dev/ttyUSB0

.PHONY: all
all: build

.PHONY: build
build:
	$(SCONS) -f $(OURPATH)/avr.scons NAME=$(NAME) MCU=$(MCU) FREQ=$(FREQ) ADDCFLAGS="$(CFLAGS)" ADDCXXFLAGS="$(CXXFLAGS)" ADDLDFLAGS="$(LDFLAGS)"

.PHONY: poke
poke: build
	$(AVRDUDE) -V -i 5 -p $(MCU) -c $(PROG) -P $(PORT) -e -U lfuse:w:0x$(FUSEL):m -U hfuse:w:0x$(FUSEH):m -U flash:w:build/$(NAME).hex

.PHONY: clean
clean:
	$(SCONS) -c -f $(OURPATH)/avr.scons NAME=$(NAME) MCU=$(MCU) FREQ=$(FREQ) ADDCFLAGS="$(CFLAGS)" ADDCXXFLAGS="$(CXXFLAGS)" ADDLDFLAGS="$(LDFLAGS)"

