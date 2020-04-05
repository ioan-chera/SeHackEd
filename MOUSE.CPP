// DeHackEd version 3.0a
// Written by Greg Lewis, gregl@umich.edu
// If you release any versions of this code, please include
// the author in the credits.  Give credit where credit is due!

#include <dos.h>
#include <time.h>

#include "dehacked.h"
#include "mouse.h"

void CloseMouse (void)
{
	reg.r_ax = 0x14;
	reg.r_cx = MOUSE_MASK;
	reg.r_es = _CS;
	reg.r_dx = FP_OFF(defaultHandler);
	MOUSEINT;
}

int doMouse (void)
{
	/* trick stupid mouse driver to know we are in graphic mode */
	reg.r_ax = 0;	  /* detect genius mouse */
	reg.r_bx = 0;	  /* be sure what mode we get */
	MOUSEINT;
	mouse_present = (reg.r_ax ? YES : NO)   ; /* not an iret.. */
	if (mouse_present == NO)
		return -1;

	mouseCursorLevel = eventX = eventY = eventButtons = LastEventButtons = 0;
	eventHappened = NO;
	return 0;
}

int InitMouse(void)
{
	eventX = eventY = 0;
	eventHappened = NO;

	if (doMouse() == -1)
		return -1;
	swapEventHandler(MOUSE_MASK, defaultHandler);
	return 0;
}

void showMouseCursor (void)
{
	 reg.r_ax = 1; /* enable cursor display */
	 MOUSEINT;
	 mouseCursorLevel++;
}

void hideMouseCursor (void)
{
	 reg.r_ax = 2; /* disable cursor display */
	 MOUSEINT;
	 mouseCursorLevel--;
}

EBool getLastEvent(unsigned &x, unsigned &y,
						 EButton &left_button, EButton &right_button)
{
	EBool event = eventHappened ;

	// clear to next read/event
	eventHappened = NO;
	x = eventX >> 3;
	y = eventY >> 3;

	if (eventButtons & LEFTBUTTON)
		left_button = buttonDown;
	else
		left_button = buttonUp;

	if (eventButtons & RIGHTBUTTON)
		right_button = buttonDown;
	else
		right_button = buttonUp;

	return event;
}

void huge defaultHandler (void)
{
/*	asm {
		mov eventX, cx;   // where in the x region did it occur
		mov eventY, dx;
		push ax;
		mov ax, eventButtons;
		mov LastEventButtons, ax;
		pop ax;
		mov eventButtons, bx;
		mov eventHappened, 1;  // eventhappened = TRUE
	}*/

	eventX = _CX;
	eventY = _DX;
	LastEventButtons = eventButtons;
	eventButtons = _BX;
	eventHappened = YES;

	if ((eventButtons & LEFTBUTTON) && !(LastEventButtons & LEFTBUTTON))
	{
		lclicktime = clock();
		LastEventButtons |= LEFTBUTTON;	// Kludge, otherwise it sucks
	}

}

void swapEventHandler (unsigned mask ,void huge (*handler)())
{
	reg.r_ax = 0x14;
	reg.r_cx = mask;
	reg.r_es = _CS;
	reg.r_dx = FP_OFF(handler);
	MOUSEINT;
}

