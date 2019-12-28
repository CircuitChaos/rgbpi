#!/usr/bin/env python3

from rgbpi import RgbPi
from time import sleep

rgbpi = RgbPi()

while True:
    print("Red half")
    rgbpi.set(128, 0, 0)
    sleep(1)

    print("Red and green full")
    rgbpi.set(255, 255, 0)
    sleep(1)

    print("Green full")
    rgbpi.set(0, 255, 0)
    sleep(1)

    print("Blue full")
    rgbpi.set(0, 0, 255)
    sleep(1)
