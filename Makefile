NAME	= rgbpi
MCU	= attiny13
FREQ	= 9600000

# 11111<bod><bod>1=11111001=f9
# 010110<ck><ck>=01011010=5a
FUSEH	= F9
FUSEL	= 5A

include avrbuild/avr.mak
