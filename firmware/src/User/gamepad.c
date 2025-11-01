#include "gamepad.h"

void ProcessGamepad(HID_gamepad_Info_TypeDef* joymap)
{

			if (joymap == NULL) return;

				GPIO_WriteBit(RHQ_GPIO_Port, RHQ_Pin, !(joymap->gamepad_data & 0x1));
				GPIO_WriteBit(LVQ_GPIO_Port, LVQ_Pin, !(joymap->gamepad_data >> 1 & 0x1));
				GPIO_WriteBit(BH_GPIO_Port, BH_Pin, !(joymap->gamepad_data >> 2 & 0x1));
				GPIO_WriteBit(FV_GPIO_Port, FV_Pin, !(joymap->gamepad_data >> 3 & 0x1));
				GPIO_WriteBit(LB_GPIO_Port, LB_Pin, !(joymap->gamepad_data >> 4 & 0x1));
				//GPIO_WriteBit(MB_GPIO_Port, RB_Pin, !(joymap->gamepad_data >> 5 & 0x1));
				GPIO_WriteBit(RB_GPIO_Port, RB_Pin, !(joymap->gamepad_data >> 6 & 0x1));

}
