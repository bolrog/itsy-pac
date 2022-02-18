# itsy-pac
Open source I-PAC-like using an Adafruit ItsyBitsy 32u4

Two-player arcade cabinet inputs using a single AdaFruit ItsyBitsy 32u4 (5V version recommended).

## Features
- Inputs for two players: Joystick, Buttons 1-6, Start
- Input for a single Coin button
- Acts like a keyboard with N-key rollover
- Emits standard MAME keys (easy to change as you like)
- Fast debouncing

## How to wire it
![Pinout](itsy-pac.png?raw=true "Pinout")

## How to build/upload to board
- Install arduino-cli and AdaFruit AVR core
- Ensure ItsyBitsy 32u4 is connected
- Run ./build.sh
- Run ./upload.sh

## Notes
Written to be fast, not readable.
Haven't measured latency much yet.

Borrows debouncing idea from Daemonbite.
Uses N-key rollover implementation by Keyboardio Inc. and Nico Hood.
