#!/bin/bash
arduino-cli compile -b adafruit:avr:itsybitsy32u4_5V --build-property "build.extra-flags=\"-DUSBCON\"" --build-property "compiler.cpp.extra_flags=-O3" --output-dir build/
#~/.arduino15/packages/arduino/tools/avr-gcc/7.3.0-atmel3.6.1-arduino7/bin/avr-objdump -d -S build/kbdenc.ino.elf > kbdenc.asm
