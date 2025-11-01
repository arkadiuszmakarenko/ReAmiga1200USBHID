#include "mouse.h"
#include "gpio.h"
#include <stdio.h>
#include <stdlib.h>

volatile int8_t mouseDirectionX = 0;		// X direction (0 = decrement, 1 = increment)
volatile int8_t mouseEncoderPhaseX = 0;		// X Quadrature phase (0-3)
volatile int8_t mouseDirectionY = 0;		// Y direction (0 = decrement, 1 = increment)
volatile int8_t mouseEncoderPhaseY = 0;		// Y Quadrature phase (0-3)
volatile int16_t mouseDistanceX = 0;		// Distance left for mouse to move
volatile int16_t mouseDistanceY = 0;		// Distance left for mouse to move
volatile uint8_t xTimerTop = 1;				// X axis timer TOP value
volatile uint8_t yTimerTop = 1;				// Y axis timer TOP value
FIFO_Utils_TypeDef ScrollBuffer;
uint8_t code = 0;
volatile uint8_t AmigaACK = 0;
volatile uint8_t previousMMB = 0;



void InitMouse()
{
    //Init circular buffer
    FifoInit(&ScrollBuffer);

}

uint8_t processMouseMovement(int8_t movementUnits, uint8_t axis, int limitRate,
		int dpiDivide) {




	uint16_t timerTopValue = 0;

	// Set the mouse movement direction and record the movement units
	if (movementUnits > 0) {
		// Moving in the positive direction

		// Apply DPI limiting if required
		if (dpiDivide) {
			movementUnits /= DPI_DIVIDER;
			if (movementUnits < 1)
				movementUnits = 1;
		}

		// Add the movement units to the quadrature output buffer
		if (axis == MOUSEX)
			mouseDistanceX += movementUnits;
		else
			mouseDistanceY += movementUnits;
	} else if (movementUnits < 0) {
		// Moving in the negative direction

		// Apply DPI limiting if required
		if (dpiDivide) {
			movementUnits /= DPI_DIVIDER;
			if (movementUnits > -1)
				movementUnits = -1;
		}

		// Add the movement units to the quadrature output buffer
		if (axis == MOUSEX)
			mouseDistanceX += -movementUnits;
		else
			mouseDistanceY += -movementUnits;
	} else {
		if (axis == MOUSEX)
			mouseDistanceX = 0;
		else
			mouseDistanceY = 0;
	}

	// Apply the quadrature output buffer limit
	if (axis == MOUSEX) {
		if (mouseDistanceX > Q_BUFFERLIMIT)
			mouseDistanceX = Q_BUFFERLIMIT;
	} else {
		if (mouseDistanceY > Q_BUFFERLIMIT)
			mouseDistanceY = Q_BUFFERLIMIT;
	}

	// Get the current value of the quadrature output buffer
	if (axis == MOUSEX)
		timerTopValue = mouseDistanceX;
	else
		timerTopValue = mouseDistanceY;

	// Range check the quadrature output buffer
	if (timerTopValue > 127)
		timerTopValue = 127;

	// Since the USB reports arrive at 100-125 Hz (even if there is only
	// a small amount of movement, we have to output the quadrature
	// at minimum rate to keep up with the reports (otherwise it creates
	// a slow lag).  If we assume 100 Hz of reports then the following
	// is true:
	//
	// 127 movements = 12,700 interrupts/sec
	// 100 movements = 10,000 interrupts/sec
	//  50 movements =  5,000 interrupts/sec
	//  10 movements =  1,000 interrupts/sec
	//   1 movement  =    100 interrupts/sec
	//
	// Timer speed is 15,625 ticks per second = 64 uS per tick
	//
	// Required timer TOP values (0 is fastest so all results are x-1):
	// 1,000,000 / 12,700 = 78.74 / 64 uS = 1.2 - 1
	// 1,000,000 / 10,000 = 100 / 64 uS = 1.56 - 1
	// 1,000,000 / 5,000 = 200 / 64 uS = 3.125 - 1
	// 1,000,000 / 1,000 = 1000 uS / 64 uS = 15.63 - 1
	// 1,000,000 / 100 = 10000 uS / 64 uS = 156.25 - 1
	//
	// So:
	//   timerTopValue = 10000 / timerTopValue; // i.e. 1,000,000 / (timerTopValue * 100)
	//   timerTopValue = timerTopValue / 64;
	//   timerTopValue = timerTopValue - 1;
	if (timerTopValue != 0) {
		timerTopValue = ((10000 / timerTopValue) / 64) - 1;
	} else {
		timerTopValue = 255;
	}
	// If the 'Slow' configuration jumper is shorted; apply the quadrature rate limit
	if (limitRate) {
		// Rate limit is on

		// Rate limit is provided in hertz
		// Each timer tick is 64 uS
		//
		// Convert hertz into period in uS
		// 1500 Hz = 1,000,000 / 1500 = 666.67 uS
		//
		// Convert period into timer ticks (* 4 due to quadrature)
		// 666.67 us / (64 * 4) = 2.6 ticks
		//
		// Timer TOP is 0-255, so subtract 1
		// 10.42 ticks - 1 = 9.42 ticks

		uint32_t rateLimit = ((1000000 / Q_RATELIMIT) / 256) - 1;

		// If the timerTopValue is less than the rate limit, we output
		// at the maximum allowed rate.  This will cause addition lag that
		// is handled by the quadrature output buffer limit above.
		if (timerTopValue < (uint16_t) rateLimit)
			timerTopValue = (uint16_t) rateLimit;
	}

	// Return the timer TOP value
	return (uint8_t) timerTopValue;
}

void ProcessMouse(HID_MOUSE_Info_TypeDef *mousemap) {



	if (mousemap == NULL) return;
		// +X = Mouse going right
		// -X = Mouse going left
		// +Y = Mouse going down
		// -Y = Mouse going up
		//
		// X and Y have a range of -127 to +127

		// If the mouse movement changes direction then disregard any remaining
		// movement units in the previous direction.



		if (mousemap->x > 0 && mouseDirectionX == 0) {
			mouseDistanceX = 0;
			mouseDirectionX = 1;
		} else if (mousemap->x < 0 && mouseDirectionX == 1) {
			mouseDistanceX = 0;
			mouseDirectionX = 0;
		} else if (mousemap->y > 0 && mouseDirectionY == 0) {
			mouseDistanceY = 0;
			mouseDirectionY = 1;
		} else if (mousemap->y < 0 && mouseDirectionY == 1) {
			mouseDistanceY = 0;
			mouseDirectionY = 0;
		}

		// Process mouse X and Y movement -------------------------------------

		xTimerTop = processMouseMovement(mousemap->x, MOUSEX, 0U, 0U);
		yTimerTop = processMouseMovement(mousemap->y, MOUSEY, 0U, 0U);

		// Process mouse buttons ----------------------------------------------

		GPIO_WriteBit(LB_GPIO_Port, LB_Pin, !(mousemap->buttons[0]));

		uint8_t writeBuff = 0;
		uint8_t  numberTics = abs(mousemap->wheel);
		if (previousMMB == 0 && mousemap->buttons[2] == 1)
		{
		    writeBuff = CODE_MMB_DOWN;
		    FifoWrite(&ScrollBuffer,&writeBuff , 1);
		    previousMMB = 1;
		}
		if (previousMMB == 1 &&mousemap->buttons[2] == 0) {
            writeBuff = CODE_MMB_UP;
            FifoWrite(&ScrollBuffer,&writeBuff , 1);
            FifoWrite(&ScrollBuffer,&writeBuff , 1);
            previousMMB = 0;
	   }


		if (mousemap->wheel !=0)
		{
		    if (mousemap->wheel > 0)
		    {
		        writeBuff = CODE_WHEEL_UP;
		        for( uint8_t i = 0; i<numberTics ;i++)
		        {
		            FifoWrite(&ScrollBuffer,&writeBuff , 1);
		        }
		    }
		    else {
		        writeBuff = CODE_WHEEL_DOWN;
                for( uint8_t i = 0; i<numberTics ;i++)
                {
                    FifoWrite(&ScrollBuffer,&writeBuff , 1);
                }
            }

		}

		GPIO_WriteBit(RB_GPIO_Port, RB_Pin, !(mousemap->buttons[1]));

}

void ProcessX_IRQ() {



	// Process X output
	if (mouseDistanceX > 0) {
		// Set the output pins according to the current phase BH RHQ FV LVQ

		if (mouseEncoderPhaseX == 0)
			GPIO_WriteBit(BH_GPIO_Port, BH_Pin, !(1));	// Set X1 to 1
		if (mouseEncoderPhaseX == 1)
			GPIO_WriteBit(RHQ_GPIO_Port, RHQ_Pin, !(1));	// Set X2 to 1
		if (mouseEncoderPhaseX == 2)
			GPIO_WriteBit(BH_GPIO_Port, BH_Pin, !(0));	// Set X1 to 0
		if (mouseEncoderPhaseX == 3)
			GPIO_WriteBit(RHQ_GPIO_Port, RHQ_Pin, !(0));	// Set X2 to 0

		// Change phase
		if (mouseDirectionX == 0)
			mouseEncoderPhaseX--;
		else
			mouseEncoderPhaseX++;

		// Decrement the distance left to move
		mouseDistanceX--;

		// Range check the phase
		if ((mouseDirectionX == 1) && (mouseEncoderPhaseX > 3))
			mouseEncoderPhaseX = 0;
		if ((mouseDirectionX == 0) && (mouseEncoderPhaseX < 0))
			mouseEncoderPhaseX = 3;
	} else {
		// Reset the phase if the mouse isn't moving
		mouseEncoderPhaseX = 0;
	}

	// Set the timer top value for the next interrupt
	if (xTimerTop == 0) {
		TIM2->ATRLR = 1;
	} else {
		TIM2->ATRLR = xTimerTop;
	}

}

void ProcessY_IRQ() {


// Process Y output
	if (mouseDistanceY > 0) {
		// Set the output pins according to the current phase
		if (mouseEncoderPhaseY == 3)
		{
			GPIO_WriteBit(FV_GPIO_Port, LVQ_Pin, !(0));	// Set Y1 to 0
		}
		if (mouseEncoderPhaseY == 2)
			GPIO_WriteBit(LVQ_GPIO_Port, FV_Pin, !(0));	// Set Y2 to 0
		if (mouseEncoderPhaseY == 1)
		{
			GPIO_WriteBit(FV_GPIO_Port, LVQ_Pin, !(1));	// Set Y1 to 1
		}
			if (mouseEncoderPhaseY == 0)
			GPIO_WriteBit(LVQ_GPIO_Port, FV_Pin, !(1));	// Set Y2 to 1

		// Change phase
		if (mouseDirectionY == 0)
			mouseEncoderPhaseY--;
		else
			mouseEncoderPhaseY++;

		// Decrement the distance left to move
		mouseDistanceY--;

		// Range check the phase
		if ((mouseDirectionY == 1) && (mouseEncoderPhaseY > 3))
			mouseEncoderPhaseY = 0;
		if ((mouseDirectionY == 0) && (mouseEncoderPhaseY < 0))
			mouseEncoderPhaseY = 3;
	} else {
		// Reset the phase if the mouse isn't moving
		mouseEncoderPhaseY = 0;
	}

// Set the timer top value for the next interrupt
	if (yTimerTop == 0) {
		TIM4->ATRLR = 1;
	} else {
		TIM4->ATRLR = yTimerTop;
	}

}



void ProcessScrollIRQ()
{

    uint8_t code = 0;
    uint16_t PortCurrentValue = GPIO_ReadOutputData(GPIOB);

    FifoRead(&ScrollBuffer, &code, 1);

    // if (code == 0) return;

    GPIO_WriteBit(RB_GPIO_Port, RB_Pin, 0);

    if (code == 0)
    {
        GPIO_WriteBit(RHQ_GPIO_Port, RHQ_Pin, 1);
        GPIO_WriteBit(LVQ_GPIO_Port, LVQ_Pin,1);
        GPIO_WriteBit(BH_GPIO_Port, BH_Pin,1);
        GPIO_WriteBit(FV_GPIO_Port,FV_Pin, 1);
    }

    if (code == CODE_WHEEL_UP)
    {
        GPIO_WriteBit(RHQ_GPIO_Port, RHQ_Pin, 0);
        GPIO_WriteBit(LVQ_GPIO_Port, LVQ_Pin,0);
        GPIO_WriteBit(BH_GPIO_Port, BH_Pin,1);
        GPIO_WriteBit(FV_GPIO_Port,FV_Pin, 1);
    }

    if (code == CODE_WHEEL_DOWN)
    {
        GPIO_WriteBit(RHQ_GPIO_Port, RHQ_Pin, 1);
        GPIO_WriteBit(LVQ_GPIO_Port, LVQ_Pin, 0);
        GPIO_WriteBit(BH_GPIO_Port, BH_Pin,0);
        GPIO_WriteBit(FV_GPIO_Port,FV_Pin, 1);
    }

    if (code == CODE_MMB_DOWN)
    {
        GPIO_WriteBit(RHQ_GPIO_Port, RHQ_Pin, 0);
        GPIO_WriteBit(LVQ_GPIO_Port, LVQ_Pin, 0);
        GPIO_WriteBit(BH_GPIO_Port, BH_Pin,1);
        GPIO_WriteBit(FV_GPIO_Port,FV_Pin, 0);
    }

    if (code == CODE_MMB_UP)
    {
        GPIO_WriteBit(RHQ_GPIO_Port, RHQ_Pin, 1);
        GPIO_WriteBit(LVQ_GPIO_Port, LVQ_Pin, 0);
        GPIO_WriteBit(BH_GPIO_Port, BH_Pin,0);
        GPIO_WriteBit(FV_GPIO_Port,FV_Pin, 0);
    }

   while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3) != 1);
   GPIO_Write(GPIOB,PortCurrentValue);
}


