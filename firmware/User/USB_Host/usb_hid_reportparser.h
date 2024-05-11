#ifndef HIDREPORTPARSER_H
#define HIDREPORTPARSER_H

#define REPORT_TYPE_NONE     0
#define REPORT_TYPE_MOUSE    1
#define REPORT_TYPE_KEYBOARD 2
#define REPORT_TYPE_JOYSTICK 3

// currently only joysticks are supported
typedef struct {
  uint8_t type: 2;             // REPORT_TYPE_...
  uint8_t report_id;
  uint8_t report_size;

  // for downstream mapping
  uint16_t vid;
  uint16_t pid;

  union {
    struct {
      struct {
				uint16_t offset;
				uint8_t size;
				struct {
					uint16_t min;
					uint16_t max;
				} logical;
      } axis[2];               // x and y axis

      struct {
				uint8_t byte_offset;
				uint8_t bitmask;
      } button[12];             // 12 buttons max

      struct {
				uint16_t offset;
				uint8_t size;
      } hat;                   // 1 hat (joystick only)

      struct {
                uint16_t offset;
                uint8_t size;
                struct {
                    uint16_t min;
                    uint16_t max;
                } logical;
      } wheel;

			uint8_t button_count;

    } joystick_mouse;
  };
} hid_report_t;

int parse_report_descriptor(uint8_t *rep, uint16_t rep_size,hid_report_t *conf);

#endif // HIDPARSER_H
