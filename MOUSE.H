// DeHackEd version 3.0a
// Written by Greg Lewis, gregl@umich.edu
// If you release any versions of this code, please include
// the author in the credits.  Give credit where credit is due!

#define MOUSE_MASK CURSOR_LOCATION_CHANGED | LEFT_BUTTON_PRESSED |   \
				LEFT_BUTTON_RELEASED | RIGHT_BUTTON_PRESSED |       		\
				RIGHT_BUTTON_RELEASED | MIDDLE_BUTTON_PRESSED |      		\
				MIDDLE_BUTTON_RELEASED

#define MOUSEINT intr(0x33, &reg)

void CloseMouse(void);
void huge defaultHandler(void);
int  doMouse(void);
EBool getLastEvent(unsigned &x, unsigned &y, EButton &left_button,
						 EButton &right_button);
void hideMouseCursor(void);
int  InitMouse(void);
void setEventHandler(unsigned mask ,void huge (*handler)());
void showMouseCursor(void);
void swapEventHandler(unsigned mask ,void huge (*handler)());

EBool mouse_present;
unsigned eventX, eventY, eventButtons; /* any event handler should update */
unsigned LastEventButtons;
EBool eventHappened;	/* these vars to use getLastEvent   */
long  lclicktime;
struct REGPACK reg;	     /* general registers used */
signed int mouseCursorLevel;	/* if > 0 mouse cursor is visiable,
													otherwise not, containes the level
													of showMouseCursor/hideMouseCursor */

