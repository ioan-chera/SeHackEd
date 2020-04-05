// SeHackEd version 0.4
// Modified by Mike Fredericks, chohatsu@yahoo.com
// Thanks to Janis Legzdinsh for info, http://www.vavoom-engine.com
// based on DeHackEd version 3.0a
// Written by Greg Lewis, gregl@umich.edu
// If you release any versions of this code, please include
// the author in the credits.  Give credit where credit is due!

#include <conio.h>
#include <dos.h>
#include <stdarg.h>
#include <stdio.h>

//#include <stdlib.h>

#include "dehacked.h"
#include "screen.h"

void Blankwindow(char attr, int left, int top, int right, int bot)
{
	int i, size, temp;

	size = right - left - 1;
	bot--;

	// Point to the text screen.
	_DX = 0xb800;
	_ES = _DX;

	// Offset into the data to begin blanking
	temp = (top*160) + (left << 1);

	for (i=top; i<bot; i++, temp+=160)
	{
		asm {
			mov di, temp
			mov ah, attr
			mov al, 32
			mov cx, size
			rep stosw
		}
	}
}

// Clears the screen with attr and a fill character.  Does NOT clear
// the first and last rows (status rows).

void clearscreen(char attr, char fill)
{
	// Use mov dx, 0xb800 and
	// mov cx, 4000 to clear the whole screen.
	asm {
		mov dx, 0xb80a
		mov es, dx
		mov di, 0
		mov ah, attr
		mov al, fill
		mov cx, 3840
		rep stosw
	}
}

// Clears the MCGA screen

void clsMCGA(void)
{
	asm {
		mov dx, 0xa000
		mov es, dx
		mov di, 0
		mov ax, 0
		mov cx, 0x7D00
		rep stosw
	}
}

// Direct I/O -- puts the string at x, y, with full formatting.

void CPrintfXY(int x, int y, char *string, ...)
{
	va_list ap;
	char buffer[80];

	va_start(ap, string);
	gotoxy(x, y);
	vsprintf(buffer, string, ap);
	cputs(buffer);
	va_end(ap);
}

// Direct I/O -- puts the string at x, y.

void CPutsXY(int x, int y, char *string)
{
	gotoxy(x, y);
	cputs(string);
}

// Draw the frame for a window at the specified coordinates, with the
// specified attribute.

void Drawframe(int type, unsigned char attr, int left, int top, int right,
					int bot)
{
	unsigned cul, cur, cll, clr, chz, cvt;
	unsigned uattr = attr * 256;
	int i;

	if (type < 0 || type > 4)
		type = 0;

	cul = uattr + (unsigned char)ul[type];
	cur = uattr + (unsigned char)ur[type];
	cll = uattr + (unsigned char)ll[type];
	clr = uattr + (unsigned char)lr[type];
	chz = uattr + (unsigned char)hz[type];
	cvt = uattr + (unsigned char)vt[type];

	pokew(--left, --top,  cul);
	pokew(--right, top, cur);
	pokew(left, --bot,  cll);
	pokew(right, bot, clr);

	for (i=left+1; i<right; i++)
	{
		pokew(i, top, chz);
		pokew(i, bot, chz);
	}

	for (i=top+1; i<bot; i++)
	{
		pokew(left,  i, cvt);
		pokew(right, i, cvt);
	}

	// Draw the "shadowing"
	if (++right < 80)
	{
		for (i=top+1; i<=bot; i++)
			screenbptr[i*160+right*2+1] &= 0x7;
	}

	if (++bot < 50)
	{
		for (i=left+1; i<right; i++)
			screenbptr[bot*160+i*2+1] &= 0x7;

		// If bot < 50 and right < 80...
		if (right < 80)
			screenbptr[bot*160+right*2+1] &= 0x7;
	}

	Blankwindow(attr, left+1, top+1, right, bot);
}

// Get the background section of the screen.

void Getwindow(int x1, int y1, int x2, int y2)
{
	int amount = (x2-x1+1)*(y2-y1+1);
	Background *current = &backhead;

	// If background1 is non-NULL we have to swap with background2.
	while (current->next != NULL)
		current = current->next;

	// Set up the next link in the linked list.
	current->next = new Background;
	current = current->next;

	// Make sure params are in-bounds.
	if (x1 < 1)
		x1 = 1;
	if (y1 < 1)
		y1 = 1;
	if (x2 > 80)
		x2 = 80;
	if (y2 > 50)
		y2 = 50;

	// Continue to fill in the struct with relevant data.
	current->x1 = x1;
	current->y1 = y1;
	current->x2 = x2;
	current->y2 = y2;
	current->next = NULL;

	// Get memory
	current->image = new int[amount];
	if (current->image == NULL)
		AbortProg("in Getwindow");

	// Get the actual data itself.
	gettext(x1, y1, x2, y2, current->image);
}

// Outputs a string to the MCGA screen at x,y, with a given color and
// background color.

void MCGAPutsXY (int x, int y, char *string, unsigned char col)
{
	int i = 0;
	struct REGPACK regs;

	while (string[i] != '\0')
	{
		setpos(x, y);

		regs.r_ax = (9 << 8) + string[i];
		regs.r_bx = col;
		regs.r_cx = 1;
		regs.r_dx = 0;
		intr(0x10, &regs);

		i++;
		x++;
	}
}

// Slap the background back up on the screen.

void Putwindow(void)
{
	Background *current = &backhead;
	Background *parent  = NULL;

	// If background1 is non-NULL we have to swap with background2.
	while (current->next != NULL)
	{
		parent = current;
		current = current->next;
	}

	// If the pointer is NULL there is an error.  Abort.
	if (parent == NULL)
		return;

	// Slap it up at the pre-saved coords.
	puttext(current->x1, current->y1, current->x2, current->y2, current->image);

	// Set to NULL.
	delete[] current->image;
	delete current;
	parent->next = NULL;
}

// Sets the MCGA palette.

void setpalette(unsigned char palette[768])
{
	_AX = 0x1012;
	_BX = 0;
	_CX = 256;
	_ES = FP_SEG(palette);
	_DX = FP_OFF(palette);
	__int__(0x10);
}

// Goes to a position on the MCGA screen

void setpos(int x, int y)
{
	REGS regs;
	regs.h.ah = 2;
	regs.h.bh = 0;
	regs.h.dh = y;
	regs.h.dl = x;
	int86(0x10, &regs, &regs);
}

// Switch to MCGA mode

void toMCGA(void)
{
	_AX = 0x13;
   _BX = 0;
	_CX = 0;
	_DX = 0;
	__int__(0x10);
}

// Switch to text mode after MCGA mode

void totext(void)
{
	_AX = 3;
	__int__(0x10);
}

// Clears the screen and prints a little status bar

void WipeScreen(void)
{
	clearscreen(0x71, '°');
	textattr(BARS);
	CPrintfXY(1,  1, "    F2 Things    F3 Frames    F4 Misc    F5 Sounds    F6 Sprites    F7 Text     ");
        CPrintfXY(1, 50, "        F1 Help       Load       SeHackEd v0.4       Save       Esc Quit       ");
        pokew(79, 49, BARS*256 + ' ');  // Changed Dehacked to Sehacked
}
