#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "keyboard.h"
#include "usb_keyboard.h"
#include "gpio.h"

/* ===== Type Definitions ===== */

typedef enum {
    LOCK_STATE_INACTIVE = 0,
    LOCK_STATE_ACTIVE = 1
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
    .is_ready = false};


/* ===== Static State ===== */

/* Replace O(n) table with a direct 256-entry lookup table for O(1) translation */
#define AMIGA_KEY_UNMAPPED 0xFF

static const uint8_t usb_to_amiga_lut[256] = {
    [0x00] = AMIGA_KEY_UNMAPPED, /* KEY_NONE */
    [0x01] = AMIGA_KEY_UNMAPPED,
    [0x02] = AMIGA_KEY_UNMAPPED,
    [0x03] = AMIGA_KEY_UNMAPPED,
    /* Letters */
    [0x04] = 0x20, /* A */
    [0x05] = 0x35, /* B */
    [0x06] = 0x33, /* C */
    [0x07] = 0x22, /* D */
    [0x08] = 0x12, /* E */
    [0x09] = 0x23, /* F */
    [0x0A] = 0x24, /* G */
    [0x0B] = 0x25, /* H */
    [0x0C] = 0x17, /* I */
    [0x0D] = 0x26, /* J */
    [0x0E] = 0x27, /* K */
    [0x0F] = 0x28, /* L */
    [0x10] = 0x37, /* M */
    [0x11] = 0x36, /* N */
    [0x12] = 0x18, /* O */
    [0x13] = 0x19, /* P */
    [0x14] = 0x10, /* Q */
    [0x15] = 0x13, /* R */
    [0x16] = 0x21, /* S */
    [0x17] = 0x14, /* T */
    [0x18] = 0x16, /* U */
    [0x19] = 0x34, /* V */
    [0x1A] = 0x11, /* W */
    [0x1B] = 0x32, /* X */
    [0x1C] = 0x15, /* Y */
    [0x1D] = 0x31, /* Z */
    /* Numbers */
    [0x1E] = 0x01, /* 1 */
    [0x1F] = 0x02, /* 2 */
    [0x20] = 0x03, /* 3 */
    [0x21] = 0x04, /* 4 */
    [0x22] = 0x05, /* 5 */
    [0x23] = 0x06, /* 6 */
    [0x24] = 0x07, /* 7 */
    [0x25] = 0x08, /* 8 */
    [0x26] = 0x09, /* 9 */
    [0x27] = 0x0A, /* 0 */
    /* Common keys */
    [0x28] = 0x44,               /* Enter */
    [0x29] = 0x45,               /* Escape */
    [0x2A] = 0x41,               /* Backspace */
    [0x2B] = 0x42,               /* Tab */
    [0x2C] = 0x40,               /* Space */
    [0x2D] = 0x0B,               /* -_ */
    [0x2E] = 0x0C,               /* =+ */
    [0x2F] = 0x1A,               /* [{ */
    [0x30] = 0x1B,               /* ]} */
    [0x31] = 0x0D,               /* \| */
    [0x32] = AMIGA_KEY_UNMAPPED, /* Non-US */
    [0x33] = 0x29,               /* ;: */
    [0x34] = 0x2A,               /* '" */
    [0x35] = 0x00,               /* `~ */
    [0x36] = 0x38,               /* ,< */
    [0x37] = 0x39,               /* .> */
    [0x38] = 0x3A,               /* /? */
    [0x39] = 0x62,               /* Caps Lock */
    /* Function keys */
    [0x3A] = 0x50,
    [0x3B] = 0x51,
    [0x3C] = 0x52,
    [0x3D] = 0x53,
    [0x3E] = 0x54,
    [0x3F] = 0x55,
    [0x40] = 0x56,
    [0x41] = 0x57,
    [0x42] = 0x58,
    [0x43] = 0x59,
    [0x44] = AMIGA_KEY_UNMAPPED,
    [0x45] = 0x5F,
    /* Print/Scroll/Pause */
    [0x46] = 0x0E,
    [0x47] = 0x1C,
    [0x48] = 0x2C,
    /* Navigation cluster */
    [0x49] = 0x4B,
    [0x4A] = 0x3B,
    [0x4B] = 0x3F,
    [0x4C] = 0x46,
    [0x4D] = 0x49,
    [0x4E] = 0x1F,
    [0x4F] = 0x4E,
    [0x50] = 0x4F,
    [0x51] = 0x4D,
    [0x52] = 0x4C,
    /* Keypad */
    [0x53] = 0x68,
    [0x54] = 0x5C,
    [0x55] = 0x5D,
    [0x56] = 0x4A,
    [0x57] = 0x5E,
    [0x58] = 0x43,
    [0x59] = 0x1D,
    [0x5A] = 0x1E,
    [0x5B] = 0x1F,
    [0x5C] = 0x2D,
    [0x5D] = 0x2E,
    [0x5E] = 0x2F,
    [0x5F] = 0x3D,
    [0x60] = 0x3E,
    [0x61] = 0x3F,
    [0x62] = 0x0F,
    [0x63] = 0x3C,
    /* Misc */
    [0x64] = AMIGA_KEY_UNMAPPED,
    [0x65] = 0x5F,
    [0x85] = 0x38,
    [0x88] = 0x30,
    [0x9E] = 0x2B,
    /* Modifiers */
    [0xE0] = 0x63,
    [0xE1] = 0x60,
    [0xE2] = 0x64,
    [0xE3] = 0x66,
    [0xE4] = AMIGA_KEY_UNMAPPED,
    [0xE5] = 0x61,
    [0xE6] = 0x65,
    [0xE7] = 0x67,
};

static inline uint8_t translate_usb_to_amiga (uint8_t usb_key) {
    uint8_t amiga = usb_to_amiga_lut[usb_key];
    return (amiga == AMIGA_KEY_UNMAPPED) ? usb_key : amiga;
}

static void handle_ctrl_f11 (void) {
    GPIO_WriteBit (IRQ7_Port, IRQ7_Pin, Bit_RESET);
    Delay_Us (500);
    GPIO_WriteBit (IRQ7_Port, IRQ7_Pin, Bit_SET);
}

static void handle_ctrl_f12 (void) {
    /* Cycle through states 00, 01, 10, 11 on each call.
       bit0 -> K0, bit1 -> K1. */
    static uint8_t kstate = 0;
    kstate = (kstate + 1) & 0x03;

    const BitAction k0_level = (kstate & 0x01) ? Bit_RESET : Bit_SET;
    const BitAction k1_level = (kstate & 0x02) ? Bit_RESET : Bit_SET;

    GPIO_WriteBit (K0_Port, K0_Pin, k0_level);
    GPIO_WriteBit (K1_Port, K1_Pin, k1_level);

    Delay_Us (200);

    amikb_reset();
}

static led_status_t process_lock_key_toggle (uint8_t *lock_bit, bool is_press,
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

static inline void set_data_line (BitAction state) {
    GPIO_WriteBit (KBD_DATA_GPIO_Port, KBD_DATA_Pin, state);
}

static inline void pulse_clock_line (void) {
    GPIO_WriteBit (KBD_CLOCK_GPIO_Port, KBD_CLOCK_Pin, Bit_RESET);
    Delay_Us (TIMING_CLOCK_PULSE_US);
    GPIO_WriteBit (KBD_CLOCK_GPIO_Port, KBD_CLOCK_Pin, Bit_SET);
    Delay_Us (TIMING_CLOCK_PULSE_US);
}

static led_status_t handle_lock_keys (uint8_t amiga_code, bool is_press, bool *allow_transmit) {
    led_status_t result = NO_LED;
    *allow_transmit = true;

    switch (amiga_code) {
    case AMIGA_CAPS_LOCK:
        result = process_lock_key_toggle (&g_kbd_state.lock_flags.caps_lock, is_press,
                                          LED_CAPS_LOCK_ON, LED_CAPS_LOCK_OFF, allow_transmit);
        break;
    case AMIGA_NUM_LOCK:
        result = process_lock_key_toggle (&g_kbd_state.lock_flags.num_lock, is_press,
                                          LED_NUM_LOCK_ON, LED_NUM_LOCK_OFF, allow_transmit);
        break;
    case AMIGA_SCROLL_LOCK:
        result = process_lock_key_toggle (&g_kbd_state.lock_flags.scroll_lock, is_press,
                                          LED_SCROLL_LOCK_ON, LED_SCROLL_LOCK_OFF, allow_transmit);
        break;
    }

    return result;
}

static led_status_t transmit_keycode_to_amiga (uint8_t keycode, bool is_press) {
    bool allow_transmit = true;
    led_status_t led_result = handle_lock_keys (keycode, is_press, &allow_transmit);

    if (!allow_transmit) {
        return led_result;
    }

    const uint8_t encoded_code = (keycode << 1) | (is_press ? 0 : 1);

    if (encoded_code == g_kbd_state.last_transmitted_code) {
        return NO_LED;
    }
    g_kbd_state.last_transmitted_code = encoded_code;

    set_data_line (Bit_SET);
    set_data_line (Bit_RESET);
    Delay_Us (TIMING_HANDSHAKE_US);
    set_data_line (Bit_SET);
    Delay_Us (TIMING_HANDSHAKE_US);

    uint8_t bits_to_send = encoded_code;
    for (uint8_t bit_idx = 0; bit_idx < 8; bit_idx++) {
        const BitAction bit_value = (bits_to_send & 0x80) ? Bit_RESET : Bit_SET;
        set_data_line (bit_value);
        bits_to_send <<= 1;

        Delay_Us (TIMING_DATA_SETUP_US);
        pulse_clock_line();
    }

    Delay_Us (TIMING_HANDSHAKE_US);
    set_data_line (Bit_SET);
    Delay_Us (TIMING_HANDSHAKE_US);

    return led_result;
}

void amikb_startup (void) {
    Delay_Us (TIMING_SYNC_DELAY_US);
    Delay_Us (TIMING_SYNC_DELAY_US);
    transmit_keycode_to_amiga (AMIGA_INITPOWER, false);
    Delay_Us (TIMING_SYNC_DELAY_US);
    transmit_keycode_to_amiga (AMIGA_TERMPOWER, false);
}

void amikb_ready (int isready) {
    g_kbd_state.is_ready = (isready != 0);
}

void amikb_reset (void) {
    static const uint8_t RESET_PULSE_COUNT = 10;

    for (uint8_t pulse = 0; pulse < RESET_PULSE_COUNT; pulse++) {
        GPIO_WriteBit (KB_RESET_GPIO_Port, KB_RESET_GPIO_Pin, Bit_RESET);
    }
    GPIO_WriteBit (KB_RESET_GPIO_Port, KB_RESET_GPIO_Pin, Bit_SET);

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

static led_status_t handle_modifier_changes (const HID_KEYBD_Info_TypeDef *current_state) {
    led_status_t led_result = NO_LED;
    HID_KEYBD_Info_TypeDef *prev = &g_kbd_state.previous_state;

    const modifier_map_t mod_table[] = {
        {&prev->lshift, (uint8_t *)&current_state->lshift, KEY_LEFTSHIFT   },
        {&prev->lalt,   (uint8_t *)&current_state->lalt,   KEY_LEFTALT     },
        {&prev->lctrl,  (uint8_t *)&current_state->lctrl,  KEY_LEFTCONTROL },
        {&prev->lgui,   (uint8_t *)&current_state->lgui,   KEY_LEFT_GUI    },
        {&prev->rshift, (uint8_t *)&current_state->rshift, KEY_RIGHTSHIFT  },
        {&prev->ralt,   (uint8_t *)&current_state->ralt,   KEY_RIGHTALT    },
        {&prev->rctrl,  (uint8_t *)&current_state->rctrl,  KEY_RIGHTCONTROL},
        {&prev->rgui,   (uint8_t *)&current_state->rgui,   KEY_RIGHT_GUI   },
    };

    const uint8_t mod_count = sizeof (mod_table) / sizeof (modifier_map_t);

    for (uint8_t idx = 0; idx < mod_count; idx++) {
        const uint8_t prev_val = *mod_table[idx].previous;
        const uint8_t curr_val = *mod_table[idx].current;

        if (prev_val != curr_val) {
            *mod_table[idx].previous = curr_val;
            const uint8_t amiga_code = translate_usb_to_amiga (mod_table[idx].hid_code);
            led_result |= transmit_keycode_to_amiga (amiga_code, curr_val != 0);
        }
    }

    return led_result;
}

static uint8_t detect_key_transitions (const uint8_t *source_keys, const uint8_t *compare_keys,
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

void amikb_process (HID_KEYBD_Info_TypeDef *kbdata) {
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

    handle_modifier_changes (kbdata);

    uint8_t press_buffer[KEY_PRESSED_MAX] = {0};
    uint8_t release_buffer[KEY_PRESSED_MAX] = {0};

    detect_key_transitions (g_kbd_state.previous_state.keys, kbdata->keys, release_buffer, false);
    detect_key_transitions (g_kbd_state.previous_state.keys, kbdata->keys, press_buffer, true);

    bool f11_pressed = false;
    bool f12_pressed = false;

    for (uint8_t i = 0; i < KEY_PRESSED_MAX; i++) {
        if (press_buffer[i] == KEY_F11) {
            f11_pressed = true;
            break;
        }
    }

    for (uint8_t i = 0; i < KEY_PRESSED_MAX; i++) {
        if (press_buffer[i] == KEY_F12) {
            f12_pressed = true;
            break;
        }
    }

    if (ctrl_pressed && f12_pressed) {
        handle_ctrl_f12();
        return;
    }

    if (ctrl_pressed && f11_pressed) {
        handle_ctrl_f11();
        return;
    }

    for (uint8_t i = 0; i < KEY_PRESSED_MAX; i++) {
        const uint8_t key_to_release = release_buffer[i];
        if (key_to_release != 0x00) {
            const uint8_t amiga_code = translate_usb_to_amiga (key_to_release);
            transmit_keycode_to_amiga (amiga_code, false);
        }
    }

    for (uint8_t i = 0; i < KEY_PRESSED_MAX; i++) {
        const uint8_t key_to_press = press_buffer[i];
        if (key_to_press != 0x00) {
            const uint8_t amiga_code = translate_usb_to_amiga (key_to_press);
            transmit_keycode_to_amiga (amiga_code, true);
        }
    }

    memcpy (g_kbd_state.previous_state.keys, kbdata->keys, KEY_PRESSED_MAX);
}