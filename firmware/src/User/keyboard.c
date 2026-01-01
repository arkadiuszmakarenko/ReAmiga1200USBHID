#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "keyboard.h"
#include "usb_keyboard.h"
#include "gpio.h"

/* ===== Type Definitions ===== */

typedef enum {
    LOCK_STATE_INACTIVE = 0,
    LOCK_STATE_ACTIVE   = 1
} lock_state_e;

typedef struct {
    uint8_t caps_lock;
    uint8_t num_lock;
    uint8_t scroll_lock;
} lock_flags_t;

typedef struct {
    HID_KEYBD_Info_TypeDef previous_state;
    uint8_t last_transmitted_code;
    lock_flags_t lock_flags;
    bool is_ready;
} keyboard_state_t;

/* ===== Static State ===== */

static keyboard_state_t g_kbd_state = {
    .previous_state = {0},
    .last_transmitted_code = 0xFF,
    .lock_flags = {0},
    .is_ready = false
};

static const uint8_t scancodeamiga[KEYCODE_TAB_SIZE][2] = {
    {KEY_GRAVE_ACCENT_AND_TILDE,          0x00},
    {KEY_1_EXCLAMATION_MARK,              0x01}, // 1!
    {KEY_2_AT,                            0x02}, // 2@
    {KEY_3_NUMBER_SIGN,                   0x03}, // 3#
    {KEY_4_DOLLAR,                        0x04}, // 4$
    {KEY_5_PERCENT,                       0x05}, // 5%
    {KEY_6_CARET,                         0x06}, // 6^
    {KEY_7_AMPERSAND,                     0x07}, // 7&
    {KEY_8_ASTERISK,                      0x08}, // 8*
    {KEY_9_OPARENTHESIS,                  0x09}, // 9(
    {KEY_0_CPARENTHESIS,                  0x0A}, // 0)
    {KEY_MINUS_UNDERSCORE,                0x0B}, // -_
    {KEY_EQUAL_PLUS,                      0x0C}, // +=
    {KEY_BACKSLASH_VERTICAL_BAR,          0x0D}, // |
    {KEY_KEYPAD_0_INSERT,                 0x0F}, // NUM 0
    {KEY_Q,                               0x10}, // Q
    {KEY_W,                               0x11}, // W
    {KEY_E,                               0x12}, // E
    {KEY_R,                               0x13}, // R
    {KEY_T,                               0x14}, // T
    {KEY_Y,                               0x15}, // Y
    {KEY_U,                               0x16}, // U
    {KEY_I,                               0x17}, // I
    {KEY_O,                               0x18}, // O
    {KEY_P,                               0x19}, // P
    {KEY_OBRACKET_AND_OBRACE,             0x1A}, // [{
    {KEY_CBRACKET_AND_CBRACE,             0x1B}, // }]
    {KEY_KEYPAD_1_END,                    0x1D}, // NUM 1
    {KEY_KEYPAD_2_DOWN_ARROW,             0x1E}, // NUM 2
    {KEY_KEYPAD_3_PAGEDN,                 0x1F}, // NUM 3
    {KEY_A,                               0x20}, // A
    {KEY_S,                               0x21}, // S
    {KEY_D,                               0x22}, // D
    {KEY_F,                               0x23}, // F
    {KEY_G,                               0x24}, // G
    {KEY_H,                               0x25}, // H
    {KEY_J,                               0x26}, // J
    {KEY_K,                               0x27}, // K
    {KEY_L,                               0x28}, // L
    {KEY_SEMICOLON_COLON,                 0x29}, // :;
    {KEY_SINGLE_AND_DOUBLE_QUOTE,         0x2A}, // "'
    {KEY_ENTER,                           0x44}, // <Enter>
    {KEY_KEYPAD_4_LEFT_ARROW,             0x2D}, // NUM 4
    {KEY_KEYPAD_5,                        0x2E}, // NUM 5
    {KEY_KEYPAD_6_RIGHT_ARROW,            0x2F}, // NUM 6
    {KEY_INTERNATIONAL2,                  0x30}, // <SHIFT> international?
    {KEY_Z,                               0x31}, // Z
    {KEY_X,                               0x32}, // X
    {KEY_C,                               0x33}, // C
    {KEY_V,                               0x34}, // V
    {KEY_B,                               0x35}, // B
    {KEY_N,                               0x36}, // N
    {KEY_M,                               0x37}, // M
    {KEY_COMMA_AND_LESS,                  0x38}, // <,
    {KEY_KEYPAD_COMMA,                    0x38}, // NUM ,
    {KEY_DOT_GREATER,                     0x39}, // >.
    {KEY_SLASH_QUESTION,                  0x3A}, // ?/
    {KEY_KEYPAD_7_HOME,                   0x3D}, // NUM 7
    {KEY_KEYPAD_8_UP_ARROW,               0x3E}, // NUM 8
    {KEY_KEYPAD_9_PAGEUP,                 0x3F}, // NUM 9
    {KEY_SPACEBAR,                        0x40}, // SPACE
    {KEY_BACKSPACE,                       0x41}, // BACKSPACE
    {KEY_TAB,                             0x42}, // TAB
    {KEY_KEYPAD_ENTER,                    0x43}, // ENTER
    {KEY_RETURN,                          0x2B}, // RETURN
    {KEY_ESCAPE,                          0x45}, // ESC
    {KEY_DELETE,                          0x46}, // DEL
    {KEY_KEYPAD_MINUS,                    0x4A}, // NUM -
    {KEY_UPARROW,                         0x4C}, // CURSOR U
    {KEY_DOWNARROW,                       0x4D}, // CURSOR D
    {KEY_RIGHTARROW,                      0x4E}, // CURSOR R
    {KEY_LEFTARROW,                       0x4F}, // CURSOR L
    {KEY_F1,                              0x50}, // F1
    {KEY_F2,                              0x51}, // F2
    {KEY_F3,                              0x52}, // F3
    {KEY_F4,                              0x53}, // F4
    {KEY_F5,                              0x54}, // F5
    {KEY_F6,                              0x55}, // F6
    {KEY_F7,                              0x56}, // F7
    {KEY_F8,                              0x57}, // F8
    {KEY_F9,                              0x58}, // F9
    {KEY_F10,                             0x59}, // F10
    {KEY_KEYPAD_SLASH,                    0x5C}, // /
    {KEY_KEYPAD_ASTERIKS,                 0x5D}, // NUM *
    {KEY_KEYPAD_PLUS,                     0x5E}, // NUM +
    {KEY_F12,                             0x5F}, // HELP
    {KEY_LEFTSHIFT,                       0x60}, // LSHIFT
    {KEY_RIGHTSHIFT,                      0x61}, // RSHIFT
    {KEY_CAPS_LOCK,                       0x62}, // CAPS
    {KEY_LEFTCONTROL,                     0x63}, // LCTRL
    {KEY_LEFTALT,                         0x64}, // LALT
    {KEY_RIGHTALT,                        0x65}, // RALT
    {KEY_LEFT_GUI,                        0x66}, // LWIN
    {KEY_RIGHT_GUI,                       0x67}, // RWIN
    {KEY_APPLICATION,                     0x5F}, // APP - HELP
    {KEY_KEYPAD_DECIMAL_SEPARATOR_DELETE, 0x3C}, // KEYPAD '.'
    {KEY_KEYPAD_NUM_LOCK_AND_CLEAR,       0x68}, // NUMLOCK & CLEAR
    {KEY_PRINTSCREEN,                     0x0E}, // SPARE
    {KEY_SCROLL_LOCK,                     0x1C}, // SPARE
    {KEY_PAUSE,                           0x2C}, // SPARE
    {KEY_HOME,                            0x3B}, // SPARE
    {KEY_PAGEUP,                          0x3F}, // PGUP
    {KEY_PAGEDOWN,                        0x1F}, // PGDOWN
    {KEY_END1,                            0x49}, // SPARE
    {KEY_INSERT,                          0x4B}, // SPARE
    {KEY_NONE,                            0x5B}, // SPARE
    {KEY_NONE,                            0x6A}, // SPARE
    {KEY_NONE,                            0x6B}, // SPARE
    {KEY_NONE,                            0x6C}, // SPARE
    {KEY_NONE,                            0x6D}, // SPARE
    {KEY_NONE,                            0x6E}, // SPARE
    {KEY_NONE,                            0x6F}, // SPARE
};

static inline uint8_t translate_usb_to_amiga(uint8_t usb_key) {
    for (uint8_t i = 0; i < KEYCODE_TAB_SIZE; i++) {
        if (usb_key == scancodeamiga[i][0]) {
            return scancodeamiga[i][1];
        }
    }
    return usb_key;
}

static led_status_t process_lock_key_toggle(uint8_t *lock_bit, bool is_press,
                                             led_status_t on_status, led_status_t off_status,
                                             bool *transmit_flag) {
    *transmit_flag = false;
    g_kbd_state.last_transmitted_code = 0;
    
    const bool current_lock_state = *lock_bit;
    
    if (is_press && !current_lock_state) {
        *transmit_flag = true;
        return on_status;
    }
    
    if (!is_press && current_lock_state) {
        *lock_bit = 0;
        *transmit_flag = true;
        return off_status;
    }
    
    if (!is_press && !current_lock_state) {
        *lock_bit = 1;
    }
    
    return NO_LED;
}

static inline void set_data_line(BitAction state) {
    GPIO_WriteBit(KBD_DATA_GPIO_Port, KBD_DATA_Pin, state);
}

static inline void pulse_clock_line(void) {
    GPIO_WriteBit(KBD_CLOCK_GPIO_Port, KBD_CLOCK_Pin, Bit_RESET);
    Delay_Us(TIMING_CLOCK_PULSE_US);
    GPIO_WriteBit(KBD_CLOCK_GPIO_Port, KBD_CLOCK_Pin, Bit_SET);
    Delay_Us(TIMING_CLOCK_PULSE_US);
}

static led_status_t handle_lock_keys(uint8_t amiga_code, bool is_press, bool *allow_transmit) {
    led_status_t result = NO_LED;
    *allow_transmit = true;
    
    switch (amiga_code) {
        case AMIGA_CAPS_LOCK:
            result = process_lock_key_toggle(&g_kbd_state.lock_flags.caps_lock, is_press,
                                             LED_CAPS_LOCK_ON, LED_CAPS_LOCK_OFF, allow_transmit);
            break;
        case AMIGA_NUM_LOCK:
            result = process_lock_key_toggle(&g_kbd_state.lock_flags.num_lock, is_press,
                                            LED_NUM_LOCK_ON, LED_NUM_LOCK_OFF, allow_transmit);
            break;
        case AMIGA_SCROLL_LOCK:
            result = process_lock_key_toggle(&g_kbd_state.lock_flags.scroll_lock, is_press,
                                             LED_SCROLL_LOCK_ON, LED_SCROLL_LOCK_OFF, allow_transmit);
            break;
    }
    
    return result;
}

static led_status_t transmit_keycode_to_amiga(uint8_t keycode, bool is_press) {
    bool allow_transmit = true;
    led_status_t led_result = handle_lock_keys(keycode, is_press, &allow_transmit);
    
    if (!allow_transmit) {
        return led_result;
    }
    
    const uint8_t encoded_code = (keycode << 1) | (is_press ? 0 : 1);
    
    if (encoded_code == g_kbd_state.last_transmitted_code) {
        return NO_LED;
    }
    g_kbd_state.last_transmitted_code = encoded_code;

    set_data_line(Bit_SET);
    set_data_line(Bit_RESET);
    Delay_Us(TIMING_HANDSHAKE_US);
    set_data_line(Bit_SET);
    Delay_Us(TIMING_HANDSHAKE_US);
    
    uint8_t bits_to_send = encoded_code;
    for (uint8_t bit_idx = 0; bit_idx < 8; bit_idx++) {
        const BitAction bit_value = (bits_to_send & 0x80) ? Bit_RESET : Bit_SET;
        set_data_line(bit_value);
        bits_to_send <<= 1;
        
        Delay_Us(TIMING_DATA_SETUP_US);
        pulse_clock_line();
    }
    
    Delay_Us(TIMING_HANDSHAKE_US);
    set_data_line(Bit_SET);
    Delay_Us(TIMING_HANDSHAKE_US);
    
    return led_result;
}

void amikb_startup(void) {
    Delay_Us(TIMING_SYNC_DELAY_US);
    Delay_Us(TIMING_SYNC_DELAY_US);
    transmit_keycode_to_amiga(AMIGA_INITPOWER, false);
    Delay_Us(TIMING_SYNC_DELAY_US);
    transmit_keycode_to_amiga(AMIGA_TERMPOWER, false);
}

void amikb_ready(int isready) {
    g_kbd_state.is_ready = (isready != 0);
}

void amikb_reset(void) {
    static const uint8_t RESET_PULSE_COUNT = 10;
    
    for (uint8_t pulse = 0; pulse < RESET_PULSE_COUNT; pulse++) {
        GPIO_WriteBit(KB_RESET_GPIO_Port, KB_RESET_GPIO_Pin, Bit_RESET);
    }
    GPIO_WriteBit(KB_RESET_GPIO_Port, KB_RESET_GPIO_Pin, Bit_SET);
    
    g_kbd_state.last_transmitted_code = 0xFF;
    g_kbd_state.lock_flags.caps_lock = 0;
    g_kbd_state.lock_flags.num_lock = 0;
    g_kbd_state.lock_flags.scroll_lock = 0;
}

typedef struct {
    uint8_t *previous;
    uint8_t *current;
    uint8_t hid_code;
} modifier_map_t;

static led_status_t handle_modifier_changes(const HID_KEYBD_Info_TypeDef *current_state) {
    led_status_t led_result = NO_LED;
    HID_KEYBD_Info_TypeDef *prev = &g_kbd_state.previous_state;
    
    const modifier_map_t mod_table[] = {
        {&prev->lshift, (uint8_t*)&current_state->lshift, KEY_LEFTSHIFT},
        {&prev->lalt,   (uint8_t*)&current_state->lalt,   KEY_LEFTALT},
        {&prev->lctrl,  (uint8_t*)&current_state->lctrl,  KEY_LEFTCONTROL},
        {&prev->lgui,   (uint8_t*)&current_state->lgui,   KEY_LEFT_GUI},
        {&prev->rshift, (uint8_t*)&current_state->rshift, KEY_RIGHTSHIFT},
        {&prev->ralt,   (uint8_t*)&current_state->ralt,   KEY_RIGHTALT},
        {&prev->rctrl,  (uint8_t*)&current_state->rctrl,  KEY_RIGHTCONTROL},
        {&prev->rgui,   (uint8_t*)&current_state->rgui,   KEY_RIGHT_GUI},
    };
    
    const uint8_t mod_count = sizeof(mod_table) / sizeof(modifier_map_t);
    
    for (uint8_t idx = 0; idx < mod_count; idx++) {
        const uint8_t prev_val = *mod_table[idx].previous;
        const uint8_t curr_val = *mod_table[idx].current;
        
        if (prev_val != curr_val) {
            *mod_table[idx].previous = curr_val;
            const uint8_t amiga_code = translate_usb_to_amiga(mod_table[idx].hid_code);
            led_result |= transmit_keycode_to_amiga(amiga_code, curr_val != 0);
        }
    }
    
    return led_result;
}

static uint8_t detect_key_transitions(const uint8_t *source_keys, const uint8_t *compare_keys,
                                       uint8_t *result_buffer, bool detect_new) {
    uint8_t count = 0;
    const uint8_t *scan_from = detect_new ? compare_keys : source_keys;
    const uint8_t *scan_against = detect_new ? source_keys : compare_keys;
    
    for (uint8_t i = 0; i < KEY_PRESSED_MAX; i++) {
        const uint8_t key = scan_from[i];
        
        if (key == 0x00) {
            continue;
        }
        
        bool exists_in_other = false;
        for (uint8_t j = 0; j < KEY_PRESSED_MAX && !exists_in_other; j++) {
            exists_in_other = (key == scan_against[j]);
        }
        
        if (!exists_in_other) {
            result_buffer[count++] = key;
        }
    }
    
    return count;
}

void amikb_process(HID_KEYBD_Info_TypeDef *kbdata) {
    if (!kbdata) {
        return;
    }
    
    const bool ctrl_pressed = (kbdata->lctrl != 0);
    const bool alt_pressed = (kbdata->lalt != 0);
    const bool del_pressed = (kbdata->keys[0] == KEY_DELETE);
    
    if (ctrl_pressed && alt_pressed && del_pressed) {
        amikb_reset();
        return;
    }
    
    handle_modifier_changes(kbdata);
    
    uint8_t press_buffer[KEY_PRESSED_MAX] = {0};
    uint8_t release_buffer[KEY_PRESSED_MAX] = {0};
    
    detect_key_transitions(g_kbd_state.previous_state.keys, kbdata->keys, release_buffer, false);
    detect_key_transitions(g_kbd_state.previous_state.keys, kbdata->keys, press_buffer, true);
    
    for (uint8_t i = 0; i < KEY_PRESSED_MAX; i++) {
        const uint8_t key_to_release = release_buffer[i];
        if (key_to_release != 0x00) {
            const uint8_t amiga_code = translate_usb_to_amiga(key_to_release);
            transmit_keycode_to_amiga(amiga_code, false);
        }
    }
    
    for (uint8_t i = 0; i < KEY_PRESSED_MAX; i++) {
        const uint8_t key_to_press = press_buffer[i];
        if (key_to_press != 0x00) {
            const uint8_t amiga_code = translate_usb_to_amiga(key_to_press);
            transmit_keycode_to_amiga(amiga_code, true);
        }
    }
    
    memcpy(g_kbd_state.previous_state.keys, kbdata->keys, KEY_PRESSED_MAX);
}