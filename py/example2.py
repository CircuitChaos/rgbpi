#!/usr/bin/env python3

from rgbpi import RgbPi
from time import sleep
from enum import Enum
from random import seed, randint

class Mode(Enum):
    rg = 0
    rb = 1
    gr = 2
    gb = 3
    br = 4
    bg = 5

class Example:
    rgbpi = RgbPi()
    mode = Mode.rg
    phase = 0
    rgb = [ 0, 0, 0 ]

    def __init__(self):
        self.rgb[0] = 0
        self.rgb[1] = 0
        self.rgb[2] = 0
        seed()
        shuffle_count = randint(1, 8)
        for i in range(shuffle_count):
            self.next_mode()

    def next_phase(self):
        if self.phase < 256:
            if self.mode == Mode.rg or self.mode == Mode.rb:
                index = 0
            elif self.mode == Mode.gr or self.mode == Mode.gb:
                index = 1
            else:
                index = 2
            self.rgb[index] = self.phase
        elif self.phase < 512:
            if self.mode == Mode.gr or self.mode == Mode.br:
                index = 0
            elif self.mode == Mode.rg or self.mode == Mode.bg:
                index = 1
            else:
                index = 2
            self.rgb[index] = 255 - (self.phase - 256)
        else:
            self.next_mode()

        self.phase += 1
        self.update()

    def next_mode(self):
        self.phase = 0
        choice = randint(0, 1) == 1

        if self.mode == Mode.rg or self.mode == Mode.rb:
            if choice:
                self.mode = Mode.br
            else:
                self.mode = Mode.gr
        elif self.mode == Mode.gr or self.mode == Mode.gb:
            if choice:
                self.mode = Mode.bg
            else:
                self.mode = Mode.rg
        else:
            if choice:
                self.mode = Mode.gb
            else:
                self.mode =  Mode.rb

        if self.mode == Mode.gr or self.mode == Mode.br:
            self.rgb[0] = 255
        else:
            self.rgb[0] = 0

        if self.mode == Mode.rg or self.mode == Mode.bg:
            self.rgb[1] = 255
        else:
            self.rgb[1] = 0

        if self.mode == Mode.rb or self.mode == Mode.gb:
            self.rgb[2] = 255
        else:
            self.rgb[2] = 0

    def update(self):
        self.rgbpi.set(self.rgb[0], self.rgb[1], self.rgb[2])

example = Example()

while True:
    example.next_phase()
    sleep(1. / 1000.)
