# itsy-pac
Open source I-PAC-like using an Adafruit ItsyBitsy 32u4

Two-player arcade cabinet inputs using a single AdaFruit ItsyBitsy 32u4 (5V version recommended).

For both players:
- Up, Down, Left, Right
- Buttons 1-6
- Start

Only one coin button is mapped (P1) because I ran out of GPIOs on the ItsyBitsy.

Written to be fast, not readable. Haven't measured latency much yet. Borrows debouncing idea from Daemonbite.
