#!/usr/bin/env python3

import RPi.GPIO as gpio
from time import sleep

class Simp2C:
    gc = 19
    gd = 26
    us = 50.

    def __init__(self):
        gpio.setmode(gpio.BCM)
        gpio.setup(self.gc, gpio.OUT)
        gpio.setup(self.gd, gpio.OUT)

    def __del__(self):
        gpio.cleanup()

    def sync(self):
        self.dh()
        self.ch()
        self.dl()
        self.cl()

    def byte(self, byte):
        for i in range(8):
            if byte & (1 << i):
                self.dh()
            else:
                self.dl()
            self.ch()
            self.cl()

    def cl(self):
        gpio.output(self.gc, gpio.HIGH)
        self.delay()

    def ch(self):
        gpio.output(self.gc, gpio.LOW)
        self.delay()

    def dl(self):
        gpio.output(self.gd, gpio.HIGH)
        self.delay()

    def dh(self):
        gpio.output(self.gd, gpio.LOW)
        self.delay()

    def delay(self):
        sleep(self.us / 1000. / 1000.)
