/*  Itsy-PAC
    by Bolrog <bolrog@protonmail.com>

    Uses NKRO keyboard implementation by Keyboard.io, Inc. and NicoHood.
*/

/*
Copyright (c) 2014-2015 NicoHood
Copyright (c) 2015-2018 Keyboard.io, Inc

See the readme for credit to other people.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "Keyboard.h"
#include "Config.h"

#define DEBOUNCE_MILLIS 10
#define START_HOLD_TIME 50                // Time to hold P1 Start after it has been released without being used as SHIFT.

#define PIN_P1_START (16 + 7)

#define PORT_MASK_B B11111110
#define PORT_MASK_C B11000000
#define PORT_MASK_D B11011111
#define PORT_MASK_E B01000000
#define PORT_MASK_F B11110011

uint32_t millis_now = 0;
uint8_t port_state[5] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
uint8_t port_shifted[5] = { 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t port_state_cur;
uint8_t port_shifted_cur;
uint8_t port_state_direct;
uint32_t pin_timestamp[40];
bool p1_start_held = false;
bool p1_start_used_as_shift = false;
uint32_t p1_start_held_since = 0;
bool is_shift_active;

void setup() 
{
  DDRB  &= ~PORT_MASK_B; // 7 pins
  DDRC  &= ~PORT_MASK_C; // 2 pins
  DDRD  &= ~PORT_MASK_D; // 7 pins
  DDRE  &= ~PORT_MASK_E; // 1 pins
  DDRF  &= ~PORT_MASK_F; // 6 pins
  PORTB |= PORT_MASK_B;
  PORTC |= PORT_MASK_C;
  PORTD |= PORT_MASK_D;
  PORTE |= PORT_MASK_E;
  PORTF |= PORT_MASK_F;

  port_state[0] = PINB & PORT_MASK_B;
  port_state[1] = PINC & PORT_MASK_C;
  port_state[2] = PIND & PORT_MASK_D;
  port_state[3] = PINE & PORT_MASK_E;
  port_state[4] = PINF & PORT_MASK_F;

  millis_now = millis();

  for (uint8_t i = 0; i < 40; ++i) {
    pin_timestamp[i] = millis_now;
  }

  Keyboard.begin();
}

static inline int32_t elapsed_ms(uint32_t start, uint32_t end)
{
  if (end < start) {
    return (0xFFFFFFFF - start) + 1 + end;
  } else {
    return end - start;
  }
}

static inline void handle_pin(const uint8_t pin, const uint8_t key, const uint8_t shifted_key)
{
  const uint8_t bitmask = 1U << (pin & 7);

  if ((port_state_direct & bitmask) == (port_state_cur & bitmask) || elapsed_ms(pin_timestamp[pin], millis_now) < DEBOUNCE_MILLIS) {
    return;
  }

  port_state_cur ^= bitmask;

  const uint8_t pin_state_cur = port_state_cur & bitmask;
  if (!pin_state_cur) {
    // Press
    if (pin == PIN_P1_START) {
      port_shifted_cur |= bitmask;
    } else if (is_shift_active && shifted_key != 0) {
      port_shifted_cur |= bitmask;
      Keyboard.press(shifted_key);
      p1_start_used_as_shift = true;
    } else {
      Keyboard.press(key);
    }
  } else {
    // Release
    if (pin == PIN_P1_START) {
      if (p1_start_used_as_shift) {
        p1_start_used_as_shift = false;
      } else {
        Keyboard.press(key);
        p1_start_held_since = millis_now;
        p1_start_held = true;
      }
      port_shifted_cur &= ~bitmask;
    } else {
      if (port_shifted_cur & bitmask) {
        Keyboard.release(shifted_key);
        port_shifted_cur &= ~bitmask;
      } else {
        Keyboard.release(key);
      }
    }
  }  

  pin_timestamp[pin] = millis_now;
}

void loop() 
{
  bool changed = false;

  millis_now = millis();

  if (p1_start_held && elapsed_ms(p1_start_held_since, millis_now) >= START_HOLD_TIME) {
    Keyboard.release(HID_KEYBOARD_1_AND_EXCLAMATION_POINT);
    p1_start_held = false;
    changed = true;
  }
  
  port_state_direct = PINB & PORT_MASK_B;
  port_state_cur = port_state[0];
  port_shifted_cur = port_shifted[0];
  if (port_state_direct != port_state_cur) {
    handle_pin(1, MAPPING_P1_BUTTON3);        // B1 (SCK)
    handle_pin(2, MAPPING_P1_BUTTON4);        // B2 (MOSI)
    handle_pin(3, MAPPING_P1_BUTTON5);        // B3 (MISO)
    handle_pin(4, MAPPING_P1_BUTTON6);        // B4 (8)
    handle_pin(5, MAPPING_P2_BUTTON1);        // B5 (9)
    handle_pin(6, MAPPING_P2_RIGHT);          // B6 (10)
    handle_pin(7, MAPPING_P2_LEFT);           // B7 (11)
    port_state[0] = port_state_cur;
    port_shifted[0] = port_shifted_cur;
    changed = true;
  }
  port_state_direct = PINC & PORT_MASK_C;
  port_state_cur = port_state[1];
  port_shifted_cur = port_shifted[1];
  if (port_state_direct != port_state_cur) {
    handle_pin(8 + 6, MAPPING_P2_BUTTON3);    // C6 (5)
    handle_pin(8 + 7, MAPPING_P2_UP);         // C7 (13)
    port_state[1] = port_state_cur;
    port_shifted[1] = port_shifted_cur;
    changed = true;
  }
  port_state_direct = PIND & PORT_MASK_D;
  port_state_cur = port_state[2];
  port_shifted_cur = port_shifted[2];
  if (port_state_direct != port_state_cur) {
    handle_pin(16 + 0, MAPPING_P2_BUTTON4);   // D0 (3)
    handle_pin(16 + 1, MAPPING_P2_BUTTON5);   // D1 (2)
    handle_pin(16 + 2, MAPPING_P2_START);     // D2 (0)
    handle_pin(16 + 3, MAPPING_P2_BUTTON6);   // D3 (1)
    handle_pin(16 + 4, MAPPING_P1_COIN);      // D4 (4)
    handle_pin(16 + 6, MAPPING_P2_DOWN);      // D6 (12)
    handle_pin(16 + 7, MAPPING_P1_START);     // D7 (6)
    port_state[2] = port_state_cur;
    port_shifted[2] = port_shifted_cur;
    is_shift_active = port_shifted_cur & 0x80;
    changed = true;
  }
  port_state_direct = PINE & PORT_MASK_E;
  port_state_cur = port_state[3];
  port_shifted_cur = port_shifted[3];
  if (port_state_direct != port_state_cur) {
    handle_pin(24 + 6, MAPPING_P2_BUTTON2);   // E6 (7)
    port_state[3] = port_state_cur;
    port_shifted[3] = port_shifted_cur;
    changed = true;
  }
  port_state_direct = PINF & PORT_MASK_F;
  port_state_cur = port_state[4];
  port_shifted_cur = port_shifted[4];
  if (port_state_direct != port_state_cur) {
    handle_pin(32 + 0, MAPPING_P1_BUTTON2);   // F0 (A5)
    handle_pin(32 + 1, MAPPING_P1_BUTTON1);   // F1 (A4)
    handle_pin(32 + 4, MAPPING_P1_RIGHT);     // F4 (A3)
    handle_pin(32 + 5, MAPPING_P1_LEFT);      // F5 (A1)
    handle_pin(32 + 6, MAPPING_P1_DOWN);      // F6 (A1)
    handle_pin(32 + 7, MAPPING_P1_UP);        // F7 (A0)
    port_state[4] = port_state_cur;
    port_shifted[4] = port_shifted_cur;
    changed = true;
  }

  if (changed) {
    Keyboard.sendReport();
  }
}
