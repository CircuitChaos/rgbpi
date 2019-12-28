#!/usr/bin/env python3

from simp2c import Simp2c

class RgbPi:
    simp2c = Simp2c()

    def __init__(self):
        self.set(0, 0, 0)

#    def __del__(self):
#        self.set(0, 0, 0)

    def set(self, r, g, b):
        self.simp2c.sync()
        self.simp2c.byte(r)
        self.simp2c.byte(g)
        self.simp2c.byte(b)
