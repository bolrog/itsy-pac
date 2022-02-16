/*  Itsy-PAC
 *  by Bolrog <bolrog@protonmail.com>
 */

#include <Keyboard.h>

#define DEBOUNCE_MILLIS 10
#define START_HOLD_TIME 50                // Time to hold P1 Start after it has been released without being used as SHIFT.

#define PIN_P1_START (16 + 7)

uint32_t millis_now = 0;
uint8_t  port_state[5] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
uint8_t  port_shifted[5] = { 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t  port_masks[5] = { B11111110, B11000000, B11011111, B01000000, B11110011 };
uint8_t port_state_cur;
uint8_t port_shifted_cur;
uint8_t port_state_direct;
uint32_t pin_timestamp[5 * 8];
bool p1_start_held = false;
bool p1_start_used_as_shift = false;
uint32_t p1_start_held_since = 0;
bool is_shift_active;

void setup() 
{
  DDRB  &= ~port_masks[0]; // 7 pins
  DDRC  &= ~port_masks[1]; // 2 pins
  DDRD  &= ~port_masks[2]; // 7 pins
  DDRE  &= ~port_masks[3]; // 1 pins
  DDRF  &= ~port_masks[4]; // 6 pins
  PORTB |=  port_masks[0];
  PORTC |=  port_masks[1];
  PORTD |=  port_masks[2];
  PORTE |=  port_masks[3];
  PORTF |=  port_masks[4];

  port_state[0] = PINB & port_masks[0];
  port_state[1] = PINC & port_masks[1];
  port_state[2] = PIND & port_masks[2];
  port_state[3] = PINE & port_masks[3];
  port_state[4] = PINF & port_masks[4];

  millis_now = millis();

  for (uint8_t i = 0; i < sizeof(pin_timestamp); ++i) {
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
  millis_now = millis();

  if (p1_start_held && elapsed_ms(p1_start_held_since, millis_now) >= START_HOLD_TIME) {
    Keyboard.release('1');
    p1_start_held = false;
  }

  for (uint8_t i = 0; i < 1; ++i) {
    port_state_direct = PINB & port_masks[0];
    port_state_cur = port_state[0];
    port_shifted_cur = port_shifted[0];
    if (port_state_direct != port_state_cur) {
      handle_pin(1, ' ', 0);                            // B1 (SCK) P1 Button 3
      handle_pin(2, KEY_LEFT_SHIFT, 0);                 // B2 (MOSI) P1 Button 4
      handle_pin(3, 'z', 0);                            // B3 (MISO) P1 Button 5
      handle_pin(4, 'x', 0);                            // B4 (8) P1 Button 6
      handle_pin(5, 'a', 0);                            // B5 (9) P2 Button 1
      handle_pin(6, 'g', 0);                            // B6 (10) P2 Right
      handle_pin(7, 'd', 0);                            // B7 (11) P2 Left
      port_state[0] = port_state_cur;
      port_shifted[0] = port_shifted_cur;
    }
    port_state_direct = PINC & port_masks[1];
    port_state_cur = port_state[1];
    port_shifted_cur = port_shifted[1];
    if (port_state_direct != port_state_cur) {
      handle_pin(8 + 6, 'q', 0);                        // C6 (5) P2 Button 3
      handle_pin(8 + 7, 'r', 0);                        // C7 (13) P2 Up
      port_state[1] = port_state_cur;
      port_shifted[1] = port_shifted_cur;
    }
    port_state_direct = PIND & port_masks[2];
    port_state_cur = port_state[2];
    port_shifted_cur = port_shifted[2];
    if (port_state_direct != port_state_cur) {
      handle_pin(16 + 0, 'w', 0);                       // D0 (3) P2 Button 4
      handle_pin(16 + 1, 'i', 0);                       // D1 (2) P2 Button 5
      handle_pin(16 + 2, '2', 0);                       // D2 (0) P2 Start  
      handle_pin(16 + 3, 'k', 0);                       // D3 (1) P2 Button 6
      handle_pin(16 + 4, '5', 0);                       // D4 (4) P1 Coin
      handle_pin(16 + 6, 'f', 0);                       // D6 (12) P2 Down
      handle_pin(16 + 7, '1', 0);                       // D7 (6) P1 Start (and SHIFT) 
      port_state[2] = port_state_cur;
      port_shifted[2] = port_shifted_cur;
      is_shift_active = port_shifted_cur & 0x80;
    }
    port_state_direct = PINE & port_masks[3];
    port_state_cur = port_state[3];
    port_shifted_cur = port_shifted[3];
    if (port_state_direct != port_state_cur) {
      handle_pin(24 + 6, 's', 0);                       // E6 (7) P2 Button 2
      port_state[3] = port_state_cur;
      port_shifted[3] = port_shifted_cur;
    }
    port_state_direct = PINF & port_masks[4];
    port_state_cur = port_state[4];
    port_shifted_cur = port_shifted[4];
    if (port_state_direct != port_state_cur) {
      handle_pin(32 + 0, KEY_LEFT_ALT, 0);              // F0 (A5) P1 Button 2
      handle_pin(32 + 1, KEY_LEFT_CTRL, '5');           // F1 (A4) P1 Button 1
      handle_pin(32 + 4, KEY_RIGHT_ARROW, KEY_RETURN);  // F4 (A3) P1 Right
      handle_pin(32 + 5, KEY_LEFT_ARROW, KEY_ESC);      // F5 (A1) P1 Left
      handle_pin(32 + 6, KEY_DOWN_ARROW, 0);            // F6 (A1) P1 Down
      handle_pin(32 + 7, KEY_UP_ARROW, KEY_TAB);        // F7 (A0) P1 Up
      port_state[4] = port_state_cur;
      port_shifted[4] = port_shifted_cur;
    }
  }
}
