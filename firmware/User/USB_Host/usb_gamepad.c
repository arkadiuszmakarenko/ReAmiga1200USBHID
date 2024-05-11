#include <usb_gamepad.h>
#include <stdio.h>
#include <stdlib.h>

#define JOYSTICK_AXIS_MIN           0
#define JOYSTICK_AXIS_MID           127
#define JOYSTICK_AXIS_MAX           255
#define JOYSTICK_AXIS_TRIGGER_MIN   64
#define JOYSTICK_AXIS_TRIGGER_MAX   192

#define JOY_RIGHT       0x01
#define JOY_LEFT        0x02
#define JOY_DOWN        0x04
#define JOY_UP          0x08
#define JOY_BTN_SHIFT   4
#define JOY_BTN1        0x10
#define JOY_BTN2        0x20
#define JOY_BTN3        0x40
#define JOY_BTN4        0x80
#define JOY_MOVE        (JOY_RIGHT|JOY_LEFT|JOY_UP|JOY_DOWN)



HID_gamepad_Info_TypeDef    gamepad_info;
static uint8_t 			gamepad_report_data[64];




HID_gamepad_Info_TypeDef *GetGamepadInfo(Interface *Itf)
{
	//refresh value of joymap and return value
	if(GamepadDecode(Itf)== USBH_OK)
	{

		return &gamepad_info;
	}
	else
	{
		return NULL;
	}
}



USBH_StatusTypeDef GamepadDecode(Interface *Itf)
{


	  if (Itf->HidRptLen == 0U)
	  {
	    return USBH_FAIL;
	  }

	  if (FifoRead(&Itf->buffer, &gamepad_report_data, Itf->HidRptLen) !=0)
	  {


		uint8_t jmap = 0;
		uint8_t btn = 0;
		uint8_t btn_extra = 0;
		int16_t a[2];
		uint8_t i;



		hid_report_t conf = Itf->HIDRptDesc;
				//HID_Handle->HID_Desc.RptDesc;

		// skip report id if present
		uint8_t *p = gamepad_report_data+(conf.report_id?1:0);


		//process axis
		// two axes ...
				for(i=0;i<2;i++) {
					// if logical minimum is > logical maximum then logical minimum
					// is signed. This means that the value itself is also signed
					int is_signed = conf.joystick_mouse.axis[i].logical.min >
					conf.joystick_mouse.axis[i].logical.max;
					a[i] = collect_bits(p, conf.joystick_mouse.axis[i].offset,
								conf.joystick_mouse.axis[i].size, is_signed);
				}

		//process 4 first buttons
		for(i=0;i<4;i++)
			if(p[conf.joystick_mouse.button[i].byte_offset] &
			 conf.joystick_mouse.button[i].bitmask) btn |= (1<<i);

		// ... and the eight extra buttons
		for(i=4;i<12;i++)
			if(p[conf.joystick_mouse.button[i].byte_offset] &
			 conf.joystick_mouse.button[i].bitmask) btn_extra |= (1<<(i-4));



	for(i=0;i<2;i++) {

		int hrange = (conf.joystick_mouse.axis[i].logical.max - abs(conf.joystick_mouse.axis[i].logical.min)) / 2;
		int dead = hrange/63;

		if (a[i] < conf.joystick_mouse.axis[i].logical.min) a[i] = conf.joystick_mouse.axis[i].logical.min;
		else if (a[i] > conf.joystick_mouse.axis[i].logical.max) a[i] = conf.joystick_mouse.axis[i].logical.max;

		a[i] = a[i] - (abs(conf.joystick_mouse.axis[i].logical.min) + conf.joystick_mouse.axis[i].logical.max) / 2;

		hrange -= dead;
		if (a[i] < -dead) a[i] += dead;
		else if (a[i] > dead) a[i] -= dead;
		else a[i] = 0;

		a[i] = (a[i] * 127) / hrange;

		if (a[i] < -127) a[i] = -127;
		else if (a[i] > 127) a[i] = 127;

		a[i]=a[i]+127; // mist wants a value in the range [0..255]
	}

				if(a[0] < JOYSTICK_AXIS_TRIGGER_MIN) jmap |= JOY_LEFT;
				if(a[0] > JOYSTICK_AXIS_TRIGGER_MAX) jmap |= JOY_RIGHT;
				if(a[1] < JOYSTICK_AXIS_TRIGGER_MIN) jmap |= JOY_UP;
				if(a[1] > JOYSTICK_AXIS_TRIGGER_MAX) jmap |= JOY_DOWN;
				jmap |= btn << JOY_BTN_SHIFT;      // add buttons

				gamepad_info.gamepad_data = jmap;
				gamepad_info.gamepad_extraBtn = btn_extra;

		  return USBH_OK;
	    }



	  return USBH_FAIL;

}

