# Raspberry Pi Python-based RGB LED Strip Controller

A small and simple solution to control RGB LED strip from Python on Raspberry Pi.

The solution consists of two parts: a controller board which translates two-wire commands from the Raspberry into PWM for directly driving the strip, and software on the Raspberry Pi side to generate color patterns.

## Protocol

Protocol used between Raspberry Pi and the board is simple, a bit inspired by the widely-adopted I2C, and can be split into three layers.

### Layer 1 (physical)

On the physical layer, there are two signals:

* GC (clock)
* GD (data)

Signals are unidirectional and, in idle state, both are high. Inputs are protected with optocouplers, which also do the level shifting, as Raspberry Pi's GPIO (transmitter) uses 3V3 logic and controller (receiver) works with 5V.

### Layer 2 (transport)

Transport protocol is synchronous and information is sent during bit transitions. There are three bit types that can be sent:

* **Sync bit**: GD falling when GC is high (GC=H; GD=H->L)
* **1 bit**: GD high when GC is rising (GC=L->H; GD=1)
* **0 bit**: GD low when GC is rising (GC=L->H; GD=0)

There's no minimum clock frequency (it works down to DC), and maximum frequency has not been defined. Current implementation works reliably at around 20 kHz – it's high enough to quicikly transfer RGB data, and low enough to be sure that the MCU will catch up. As the protocol is unidirectional, there's no clock stretching.

Sync bit resets the internal state machine and signals a new transfer. All bus states are summarized in the table below.

|  GC  |  GD  | Meaning |
|:----:|:----:| ------- |
| H    | H->L | Sync    |
| H    | L->H | Ignored |
| L    | H->L | Ignored |
| L    | L->H | Ignored |
| H->L | H    | Ignored |
| H->L | L    | Ignored |
| L->H | H    | Bit 1   |
| L->H | L    | Bit 0   |
| H->L | H->L | Error   |
| H->L | L->H | Error   |
| L->H | H->L | Error   |
| L->H | L->H | Error   |
| H    | H    | Ignored |
| H    | L    | Ignored |
| L    | H    | Ignored |
| L    | L    | Ignored |

Sync bit transfer can be achieved with these steps:

* Set GD high (will be ignored regardless of GC state)
* Set GC high (transfer bit 1)
* Set GD low (send sync bit)
* Set GC low (ignored)

After sync bit has been sent, both wires are in low state and data bit transfer can be achieved with these steps:

* Set GD to the desired bit (low or high)
* Set GC high (actual bit transfer)
* Set GC low (ignored – preparation for the next bit)

### Layer 3 (application)

On this layer, actual RGB data transfer takes place. Transfer consists of:

* One sync bit, which resets bit counters
* Eight bits for the red color, LSB-first
* Eight bits for the green color, LSB-first
* Eight bits for the blue color, LSB-first

After 24 data bits are transferred, PWM controller is updated with them.

## Controller

Controller is a simple application of ATtiny13, which receives RGB data on *GC* and *GD* pins, and drives three transistors with a PWM signal. There are 256 possible PWM levels and LED driving frequency is 292 Hz. It also has three LEDs on the case, one for each PWM channel, to aid in testing and troubleshooting.

Upon startup and before first data frame from Raspberry Pi is received, controller controls LEDs with its own algorithm, smoothly fading between two colors. This mode of operation is called *standalone*. When signal from Raspberry Pi is received, controller smoothly terminates current phase (fades out LEDs) before switching to the *controlled* mode – mode in which Raspberry Pi controls the color.

### Firmware

Firmware has been written in C++ for AVRs, using avr-gcc, in the C++98 dialect. It needs GNU Make and scons to compile. In the project directory, simply type:

```make```

and everything should be built. Use:

```make poke```

to program the AVR using *avrdude*, with an *usbasp* programmer accessible at */dev/ttyUSB0* (it can be changed in *avrbuild/avr.mak* file), and:

```make clean```

to clear the build directory.

### PCB

PCB is single-layer, with THT and SMD components. Its dimensions match the case (see below), but can be shrunk down to 2100 x 2400 mils (53.4 x 61 millimeters).

### Case

A *Z23A* case has been selected. It's produced by the Polish manufacturer, Kradex, and I'm not sure if it's available outside of Poland. Its specifications are available below:

* [PDF file](https://www.kradex.com.pl/products/125/Z23A.pdf)
* [Product page](https://www.kradex.com.pl/product/obudowy_kolorowe_polprzezroczyste/z23a?lang=en)

## Python software

Python files reside in the *py* directory. There are two modules to be used – *simp2c.py* and *rgbpi.py* (the latter uses the former) – and two examples. One is a simple example for toggling colors, and one is a more sophisticated program that runs on the video shown below.

Controlling colors is as simple as:

```python
from rgbpi import RgbPi
rgbpi = RgbPi()
# Set green color
rgbpi.set(0, 255, 0)
```

## Video

Video showing operation of the controller can be seen on my YouTube channel: https://youtu.be/4_UW1dAGv-o

<a href="http://www.youtube.com/watch?feature=player_embedded&v=4_UW1dAGv-o" target="_blank"><img src="http://img.youtube.com/vi/4_UW1dAGv-o/0.jpg" alt="Video" width="240" height="180" border="10" /></a>

## License

Project is licensed under Creative Commons Attribution license (CC-BY).

## Files

* *README.md* – this readme file
* *Makefile* – file for GNU Make, used to build and load the controller firmware
* *avrbuild/* – generic helper files for building the controller firmware
* *doc/* – project documentation
    * *doc/ic.txt* – pin definitions for the AVR
    * *doc/how-to-connect.png* – label to be put on the case (in Polish)
    * *doc/rgbpi-pcb.pcb* – PCB file for AutoTrax
    * *doc/rgbpi-pcb.pdf* – PDF file with bottom layer of the PCB, ready to be printed
    * *doc/rgbpi-sch.sch* – schematic file for Eagle
    * *doc/rgbpi-sch.png* – image of the schematic
* *py/* – Python sources, to be used on Raspberry Pi
    * *py/simp2c.py* – implementation of the transport layer of the communication protocol
    * *py/rgbpi.py* – implementation of the application layer of the protocol, and class to be used to implement own programs
    * *py/example1.py* – a simple example to quickly get started with RgbPi
    * *py/example2.py* – a more sophisticated example
* *src/* – C++ sources, to be used on the controller
    * *src/main.cpp* – main entry point, basically initializes everything and enters sleep mode
    * *src/io.{h,cpp}* – input/output layer
    * *src/simp2c_l2.{h,cpp}* – implementation of tne transport layer of the communication protocol
    * *src/simp2c_l3.{h,cpp}* – implementation of the application layer of the protocol, it also controls the timer
    * *src/timer.{h,cpp}* – implementation of PWM and default (standalone) mode, running at around 75 kHz
