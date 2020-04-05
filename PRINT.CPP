// SeHackEd version 0.4
// Modified by Mike Fredericks, chohatsu@yahoo.com
// Thanks to Janis Legzdinsh for info, http://www.vavoom-engine.com
// based on DeHackEd version 3.0a
// Written by Greg Lewis, gregl@umich.edu
// If you release any versions of this code, please include
// the author in the credits.  Give credit where credit is due!

#include <alloc.h>
#include <conio.h>
#include <stdio.h>
#include <string.h>

#include "dehacked.h"
#include "print.h"

// Prints the code pointer screen

void Printcodep(void)
{
	int i, current = Modeptr->toprow;

	textattr(NORMAL);

	if (redraw == ALL)
	{
		Drawframe(0, NORMAL, 26, 3, 54, 47);
		CPutsXY(31, 5, "Frame     Code");
		CPutsXY(31, 6, "Number    Pointer");
		CPutsXY(Modeptr->arrows[0], Modeptr->arrows[1], "");
		CPutsXY(Modeptr->arrows[2], Modeptr->arrows[3], "");
	}

	for (i=0; i<38; current++, i++)
	{
		CPrintfXY(31, i+8, "%4d", codepoff[current][0]);
		CPrintfXY(39, i+8, "%9ld ", *((long *)&(codepdata[codepoff[current][version+1]])));
	}
}

// Prints the frame screen

void Printframe(void)
{
	int i, j;
	int current = Modeptr->toprow;
	char buffer[6];
	char xloc[6] = {22, 28, 35, 41, 52, 61};
	char *pformat[6] = {"%5ld", "%3ld", "", "%7ld   ", "%6ld   ", "%9ld"};
	char order[6] = {SPRITENUM, SPRITESUB, SPRITESUB, NEXTFRAME, DURATION, ACTIONPTR};

	textattr(NORMAL);

	// Slap up the window frame, along with column headings and arrows
	// for mouse clicking.
	if (redraw == ALL)
	{
		Drawframe(0, NORMAL, 4, 3, 76, 47);
		CPutsXY(7, 5, "Frame	     Sprite   Bright   Next		  Code");
		CPutsXY(7, 6, "Number   Name     #  sub#  Sprite   Frame    Duration   Pointer");
		CPutsXY(Modeptr->arrows[0], Modeptr->arrows[1], "");
		CPutsXY(Modeptr->arrows[2], Modeptr->arrows[3], "");
	}

	// Cycle through every row, printing it as we go.
	for (i=8; i<46; current++, i++)
	{
		CPrintfXY(7, i, "%4d", current);

		Getframename(current, buffer);
		Highlightname(buffer);
		CPrintfXY(16, i, "%-5s", buffer);

		textattr(NORMAL);

		for (j=0; j<6; j++)
		{
			gotoxy(xloc[j], i);
			switch (j)
			{
				case 1:
					cprintf(pformat[j], framedata[current][order[j]] & 255);
					break;
				case 2:
					CPutsXY(35, i, "[ ]");
					if (framedata[current][SPRITESUB] & 32768L)
						CPutsXY(36, i, "X");
					break;
				case 5:
					if (framedata[current][ACTIONPTR] == 0)
						cputs("	0  ");
					else
						// Print the code pointer
						cprintf(pformat[j], framedata[current][order[j]]); // changed
						//cprintf(pformat[j], codepdata[current]);
					break;
				case 0:
				case 3:
				case 4:
					cprintf(pformat[j], framedata[current][order[j]]);
					break;
			}
		}
	}
}

// Prints the help screen.

void Printhelp(void)
{
	char *keys[19] = {"Esc", "Enter", "Space", "a", "c", "g", "j",	"l", "r",
							"s", "u", "w", "z", "F2", "F3", "F4", "F5", "F6", "F7"};
	char *effects[19] = {"quit SeHackEd",
								"edit the current field",
								"view/play the current field",
								"about SeHackEd",
								"copy from one object to another",
								"go to object (searches in Text editor)",
								"jump to the current item in its editor",
								"load a patch file",
								"run Strife",
								"save a patch file",
								"undo all changes--reload hacked Strife",
								"write current changes to hacked Strife",
								"zap all changes; reload original data",
								"Thing editor",
								"Frame/Code Pointer editor",
								"Weapon/Cheat code/Misc editor",
								"Sound editor",
								"Sprite editor",
								"Text editor"};
	int i;

	Getwindow(10, 8, 71, 40);
	Drawframe(1, INFO, 10, 8, 70, 38);
	textattr(INFO);

	CPutsXY(35, 10, "Help Screen");
	CPutsXY(14, 12, "Refer to the SHEHELP.TXT file for general user help");
	CPutsXY(12, 13, "and complete key information.  Use arrow keys or mouse");
	CPutsXY(12, 14, "to move between fields.  The left mouse button selects,");
	CPutsXY(12, 15, "and the right mouse button is Escape.");
	CPutsXY(14, 16, "Here is a list of available keys:");

	for (i=0; i<19; i++)
		CPrintfXY(16, i+18, "%-7s- %s", keys[i], effects[i]);

	Waitforevent(YES);

	Putwindow();
}

// Prints the window to get input

int Printinputwindow(char *buffer, char *prompt, EInptype type, int length, char firstchar)
{
	int result, x1, x2, total;
	int y1 = 23, y2 = 27;

	// Set the max length according to the 'type' argument.
	switch (type)
	{
		case YESNO:
			y2 += 2;
			length = 1;
			break;
		case CHARINT:
			length = 4;
			break;
		case LONGINT:
			length = 11;
			break;
		case STRING:
			// Take some extra precautions for strings (ie, put them on a
			// new line)
			y2++;
			if (length < strlen(prompt))
				total = strlen(prompt);
			else
				total = length;

			x1 = 38 - total/2;
			x2 = 42 + total/2;
			break;
	}

	// Set the x-coord start and end of the input box, only if this is not
	// a STRING type.
	if (type != STRING)
	{
		total = 6+strlen(prompt)+length;
		x1 = 40 - total/2;
		x2 = 40 + total/2;
	}

	// Store the screen in memory
	Getwindow(x1, y1, x2+1, y2+1);

	// Draw the frame up, put up the prompt and yes/no buttons if necessary.
	Drawframe(1, INPUT, x1, y1, x2, y2);
	textattr(INPUT);
	CPutsXY(x1+2, y1+2, prompt);
	if (type == YESNO)
	{
		// Put the clickable buttons up there.
		textattr(INFDGRAY);
		CPutsXY(30, y1+4, "  Yes  ");
		CPutsXY(44, y1+4, "  No  ");
		textattr(INFO);
		CPutsXY(32, y1+4, "Y");
		CPutsXY(46, y1+4, "N");
	}

	// Get the input
	result = Getinput(strlen(prompt), buffer, length, type, x1, firstchar);

	// Paste background back up
	Putwindow();

	return result;
}

// Prints the intro window
// changed Dehacked to Sehacked
void Printintro(void)
{
	char *about[11] = {"SeHackEd 0.4",
							 "The Strife exe Hack Editor",
							 "",
							 "modified by MikeFredericks",
							 "Email: chohatsu@yahoo.com",
							 "DeHackEd by GregLewis",
							 "Email: gregl@umich.edu",
							 "",
							 "Special thanks to:",
							 "The many others who have helped",
							 "in the creation of this program!"};
	int xloc[11] = {34, 26, 30, 28, 28, 28, 28, 30, 24, 24, 24};
	char *vers[6] = {"Strife v1.0",
						  "Strife v1.1",
						  "Strife v1.2",
						  "Strife v1.3",
						  "Strife v1.31",
						  "User Defined"}; // Changed Ultimate Doom to Strife
	char *stats[8] = {"Version:",
							"EXE name:",
							"WAD name:",
							"Patch directory:",
							"SB Address:",
							"SB IRQ:",
							"SB DMA channel:",
							"Free Memory:"};
	int i;

	Getwindow(15, 12, 66, 42);
	Drawframe(1, INFO, 20, 12, 60, 26);
	Drawframe(1, INFO, 15, 30, 65, 41);
	textattr(INFO);

	for (i=0; i<11; i++)
		CPutsXY(xloc[i], 14+i, about[i]);

	for (i=0; i<8; i++)
		CPutsXY(18, i+32, stats[i]);

	CPutsXY(35, 32, vers[version]);
	CPutsXY(35, 33, doomexe);
	CPutsXY(35, 34, doomwad);
	CPutsXY(35, 35, patchdir);
	CPrintfXY(35, 36, "%X", dev.addr);
	CPrintfXY(35, 37, "%d", dev.irq);
	CPrintfXY(35, 38, "%d", SB_DMA_CHAN);
	CPrintfXY(35, 39, "%lu", farcoreleft());

	Waitforevent(YES);

	Putwindow();
}

// Prints all of the misc data in the correct places.

void Printmisc(void)
{
	Printmisccheat();
	Printmiscmisc();
	Printmiscweapon();
	textattr(NORMAL);
	CPutsXY(Modeptr->arrows[0], Modeptr->arrows[1], "");
	CPutsXY(Modeptr->arrows[2], Modeptr->arrows[3], "");
}

// Prints the cheat code info on the misc screen

void Printmisccheat(void)
{
	int i;
	char string[16];

	textattr(NORMAL);

	if (redraw == ALL)
		Drawframe(0, NORMAL, 44, 6, 75, 26);

	for (i=0; i<17; i++)
	{
		switch (redraw)
		{
			case ALL:
				CPutsXY(46, i+8, cheatfields[i]);
			case DATA:
				// Convert the cheat code
				Cheatconvert(string, cheatdata+cheatinfo[i][0], cheatinfo[i][1]);
				string[cheatinfo[i][1]] = 0;

				// Display it
				CPutsXY(64, i+8, string);
				break;
		}
	}
}

// Prints the miscellaneous stuff on the misc screen

void Printmiscmisc(void)
{
	//int i;  // changed

	textattr(NORMAL);
	/*
	if (redraw == ALL)
		Drawframe(0, NORMAL, 5, 7, 38, 26);

	for (i=0; i<16; i++)
	{
		switch (redraw)
		{
			case ALL:
				CPutsXY(7, i+9, miscfields[i]);
			case DATA:
				// Display it
				if (i == INFIGHTING)
				{
					if ((miscdata[i] & 255) == 0xDD)
						CPrintfXY(26, i+9, "[X]");
					else
						CPrintfXY(26, i+9, "[ ]");
				}
				else
					CPrintfXY(26, i+9, "%-11ld", miscdata[i]);
				break;
		}
	}*/
}

// Prints the weapon info on the misc screen

void Printmiscweapon(void)
{
	char order[6] = {BOB1FRAME, BOB2FRAME, BOB3FRAME, SHOOTFRAME, FIREFRAME, UNKNOWNW};
	char temp[11];  // changed 10 to 11
	char na[10] = "N/A     ";  // changed 9 to 10
	int curweapon = Modeptr->current;
	int i;

	textattr(NORMAL);

	if (redraw == ALL)
		Drawframe(0, NORMAL, 20, 31, 60, 45);

	CPutsXY(22, 33, "Weapon number");
	CPutsXY(22, 34, "Weapon name");
	CPutsXY(22, 35, "Ammo type");
	//CPutsXY(22, 44, "? Unknown");


	for (i=0; i<12; i++)  // changed 11 to 12
	{
		textattr(NORMAL);
		switch (redraw)
		{
			case ALL:
				if (i > 2)
					CPutsXY(22, i+33, fullwepfields[i-3]);
				if (i > 5 && i < 11)
				{
					CPutsXY(52, i+33, "[");
					CPutsXY(58, i+33, "]");
				}
			case DATA:
				gotoxy(39, i+33);
				switch (i)
				{
					case 0:
						cprintf("%-2d", curweapon+1);
						break;
					case 1:
						cprintf("%-16s", weaponlist[curweapon]);
						break;
					case 2:
						if (weapondata[curweapon][AMMOTYPE] > 8) // changed 5 to 8
							cputs(na);
						else
							cprintf("%-9s", ammolist[weapondata[curweapon][AMMOTYPE]]);  // 9 = length of ammo name
						break;
					case 3:
						cprintf("%-11ld", weapondata[curweapon][AMMOTYPE]);
						break;
					case 4:
						if (weapondata[curweapon][AMMOTYPE] >= 7) // changed 4 to 7
							cputs(na);
						else
							cprintf("%-11ld", ammodata[weapondata[curweapon][AMMOTYPE]]);
						break;
					case 5:
						if (weapondata[curweapon][AMMOTYPE] >= 7) // changed 4 to 7
							cputs(na);
						else
							cprintf("%-11ld", ammodata[weapondata[curweapon][AMMOTYPE]+7]);  // changed 4 to 7
						break;
					case 6:
					case 7:
					case 8:
					case 9:
					case 10:
					case 11: // changed added case 11
						cprintf("%-11ld", weapondata[curweapon][order[i-6]]);

						if (i != 11)
						     Getframename(weapondata[curweapon][order[i-6]], temp);
						Highlightname(temp);
						if (i != 11)
						     CPrintfXY(53, i+33, "%-5s", temp);
						// break;
					// case 11:
						// CPrintfXY(39, i+33, "%-5s", weapondata[curweapon][UNKNOWNW]);

				}
		}
	}
}

// Prints a text intro

void Printtextintro(void)
{
	puts("\nWelcome to SeHackEd v0.4\n");
	puts("modified by MikeFredericks, email at chohatsu@yahoo.com.");
	puts("DeHackEd by GregLewis, email at gregl@umich.edu.");
	puts("Vital Doom exe specs written by Matt Fell.");
	puts("Thanks to Janis Legzdinsh and Randy Heit for very helpful info.");
	puts("Thanks to iD for creating the Doom engine.");
	puts("Thanks to Rogue and Velocity for creating Strife.\n");

}

// Prints command line options.

void Printoptions(void)
{       // changed dehacked and doom
	puts("\n  Here is the proper command line syntax for SeHackEd:\n");
	puts("\tsehacked [path] [-reload] [-load <patch1> ...] [-save <patch>]\n");
	puts("  path	     specify a different directory for Strife.exe");
	puts("  -reload	  load original exe data into hacked exe");
	puts("  -load patch(es)  load one or more patch files into the Strife exe file.");
	puts("  -save patch      save the current Strife exe to a patch file");
}

// Prints the sound screen

void Printsound(void)
{
	int i, current = Modeptr->toprow;
	char buffer[7];

	textattr(NORMAL);

	if (redraw == ALL)
	{
		Drawframe(0, NORMAL, 16, 3, 64, 47);
		CPutsXY(19, 5, "Sound	    Text");
		CPutsXY(19, 6, "Number  Name     Offset   0 / 1    \"Value\"");
		CPutsXY(Modeptr->arrows[0], Modeptr->arrows[1], "");
		CPutsXY(Modeptr->arrows[2], Modeptr->arrows[3], "");
	}

	for (i=8; i<46; current++, i++)
	{
		Getsoundname(current+1, buffer);
		Highlightname(buffer);
		CPrintfXY(27, i, "%-6s", buffer);

		textattr(NORMAL);
		CPrintfXY(19, i, "%4d", current+1);
		CPrintfXY(36, i, "%5ld", sounddata[current][TEXTP] - toff[version]);
		CPrintfXY(44, i, "%4ld", sounddata[current][ZERO_ONE]);
		CPrintfXY(55, i, "%4ld", sounddata[current][VALUE]);
	}
}

// Prints the Sprite list

void Printsprite(void)
{
	int i, current = Modeptr->toprow;
	char buffer[5];

	textattr(NORMAL);

	if (redraw == ALL)
	{
		Drawframe(0, NORMAL, 26, 3, 54, 47);
		CPutsXY(29, 5, "Sprite   Text    Sprite");
		CPutsXY(29, 6, "Number   Offset  Name");
		CPutsXY(Modeptr->arrows[0], Modeptr->arrows[1], "");
		CPutsXY(Modeptr->arrows[2], Modeptr->arrows[3], "");
	}

	for (i=8; i<46; current++, i++)
	{
		textattr(NORMAL);
		CPrintfXY(29, i, "%4d", current);
		CPrintfXY(38, i, "%5ld", spritedata[current] - toff[version]);

		Getspritename(current, buffer);
		Highlightname(buffer);
		CPrintfXY(46, i, "%-4s", buffer);
	}
}

// Prints the text section of strife.exe

void Printtext(void)
{
	int i, j, current = 0;
	int stringlen;
	char buffer[64];

	for (i=0; i < Modeptr->toprow; i++)
	{
		stringlen = strlen(textdata+current);
		current += (stringlen & (~3)) + 4;
	}

	textattr(NORMAL);

	if (redraw == ALL)
	{
		Drawframe(0, NORMAL, 3, 3, 77, 47);
		CPutsXY(5, 5, "Text");
		CPutsXY(5, 6, "Offset   Text");
		CPutsXY(Modeptr->arrows[0], Modeptr->arrows[1], "");
		CPutsXY(Modeptr->arrows[2], Modeptr->arrows[3], "");
	}

	for (i=8; i<46; i++)
	{
		CPrintfXY(5, i, "%6d", current);

		strncpy(buffer, textdata+current, 60);
		if (strlen(textdata+current) > 59)
		{
			buffer[59] = 0;
			strcat(buffer, "...");
		}
		for (j=0; j<strlen(buffer); j++)
			if (buffer[j] == '\n' || buffer[j] == '\r' || buffer[j] == '\b' ||
				 buffer[j] == '\f' || buffer[j] == '\t')
				buffer[j] = '~';
		CPrintfXY(14, i, "%-63s", buffer);

		stringlen = strlen(textdata+current);
		current += (stringlen & (~3)) + 4;
	}
}

// Prints one specific Text string.  It is the caller's obligation to
// call Putwindow when it is done with the Text string.

void Printtextstring(EBool leavewindow)
{
	char *origtext;
	int stringlen, textoff = 0;
	int i;
	int numlines = 0, longestline = 0, curline = 0;
	int x1, y1, x2, y2;

	// Get a concrete number for which string we're on, from the offset
	// in the text section.
	for (i=0; i<Modeptr->current; i++)
	{
		stringlen = strlen(textdata+textoff);
		textoff += (stringlen & (~3)) + 4;
	}

	// Get some memory for the old and new text strings, abort if none
	// is available.
	origtext = new char[strlen(textdata+textoff)+1];

	if (origtext == NULL)
		AbortProg("in Inputtext");

	strcpy(origtext, textdata+textoff);

	// Loop through text, finding characteristics.  Ignore last character.
	// Count the number of newlines and the longest line of text, in order
	// to determine size of the window that will be displayed.
	for (i=0; i<strlen(origtext)-1; i++)
	{
		switch(origtext[i])
		{
			case '\n':
				numlines++;
				curline += 2;
				if (curline > longestline)
					longestline = curline;
				curline = 0;
			case '\r':
			case '\t':
			case '\b':
			case '\f':
				curline++;
			default:
				curline++;
				break;
		}
	}

	curline += 2;
	if (curline > longestline)
		longestline = curline;
	if (numlines > 19)
		numlines = 19;
	if (longestline > 75)
		longestline = 75;

	x1 = 39 - (longestline+1)/2;
	x2 = 43 + longestline/2;
	y1 = 11 - (numlines+1)/2;
	y2 = 15 + numlines/2;

	// Get background, draw window frame.
	Getwindow(x1, y1, x2+1, y2+1);
	Drawframe(1, INFO, x1, y1, x2, y2);

	// Location of cursor while printing text
	x2 = x1+2;
	y2 = y1+2;

	// Go through the string character by character, and print "bad"
	// character in their C-style equivalent, in dark grey color.
	for (i=0; i<strlen(origtext); i++)
	{
		// Don't display lines that are too long (bad idea?)
		if (x2 > 78)
			x2--;

		gotoxy(x2, y2);
		switch(origtext[i])
		{

			case '\n':
				textattr(INFDGRAY);
				cputs("\\n");
				x2 = x1+2;
				y2++;
				break;
			case '\r':
				textattr(INFDGRAY);
				cputs("\\r");
				x2 += 2;
				break;
			case '\t':
				textattr(INFDGRAY);
				cputs("\\t");
				x2 += 2;
				break;
			case '\b':
				textattr(INFDGRAY);
				cputs("\\b");
				x2 += 2;
				break;
			case '\f':
				textattr(INFDGRAY);
				cputs("\\f");
				x2 += 2;
				break;
			default:
				textattr(INFO);
				putch(origtext[i]);
				x2++;
				break;
		}
	}

	// If all the user wants to do is see the current string, quit right
	// now.
	if (leavewindow == NO)
	{
		Waitforevent(YES);
		Putwindow();
		delete[] origtext;
	}
}

// Prints all of the Thing data in the correct places.

void Printthing(void)
{
	Printthingmisc();
	Printthingsound();
	Printthingframe();
	Printthingbits();
	Printthinginfo();
	CPutsXY(Modeptr->arrows[0], Modeptr->arrows[1], "");
	CPutsXY(Modeptr->arrows[2], Modeptr->arrows[3], "");
}

void Printthingbits(void)
{
						  //12345678901234567890
	char *data[32] = {"Can be picked up",
							"Obstacle",
							"Shootable",
							"Total invisibility",
							"Can't be hit",
							"Standing",
							"? (Peasant/Beggars)",
							"Steps before attack",
							"Hangs from ceiling",
							"No Gravity",
							"Travels over cliffs",
							"? (Boss/QuestThing)",
							"No clipping",
							"Slides along walls",
							"Floating",
							"? (mostly enemies)",
							"Projectiles",
							"Disappearing weapon",
							"Translucent",
							"Sparks (vs. bleeds)",
							"Sliding helpless",
							"No auto-leveling",
							"Affects Kill %",
							"Affects Item %",
							"Bounces",
							"Not in deathmatch",
							"Goodguy/Ally",
							"Modify visibility",
							"Color 1",
							"Color 2",
							"Color 3",
							"Spectral"};
	char brackets[4] = "[ ]";
	int i;

	if (redraw == ALL)
		Drawframe(0, NORMAL, 10, 28, 70, 47);

	for (i=0; i<16; i++)
	{
		textattr(NORMAL);
		switch (redraw)
		{
			case ALL:
				CPrintfXY(12, 30+i, "%2d", i);
				CPutsXY(19, 30+i, data[i]);
			case DATA:
				CPutsXY(15, 30+i, brackets);
				gotoxy(16, 30+i);
				if (thingdata[Modeptr->current][BITS] & (1L << i))
					putch('X');
			case NOT:
				break;
		}
	}

	for (i=16; i<32; i++)
	{
		switch (redraw)
		{
			case ALL:
				CPrintfXY(42, 14+i, "%2d", i);
				CPutsXY(49, 14+i, data[i]);
			case DATA:
				CPutsXY(45, 14+i, brackets);
				gotoxy(46, 14+i);
				if (thingdata[Modeptr->current][BITS] & (1L << i))
					putch('X');
			case NOT:
				break;
		}
	}
}

// Prints out the frame part of the Thing editing screen

void Printthingframe(void)
{
	char temp[6];
	int i;

	if (redraw == ALL)
		Drawframe(0, NORMAL, 36, 14, 77, 25);

	for (i=15; i<24; i++) // changed 23 to 24
	{
		textattr(NORMAL);
		switch (redraw)
		{
			case ALL:
				CPutsXY(38, i+1, thingfields[thingorder[i]]);
				if (i < 23)
				{
				     CPutsXY(69, i+1, "[");
				     CPutsXY(75, i+1, "]");
				}
			case DATA:
				CPrintfXY(57, i+1, "%-11ld", thingdata[Modeptr->current][thingorder[i]]);
				textattr(NORMAL);

				if (i < 23)
				     Getframename(thingdata[Modeptr->current][thingorder[i]], temp);
				Highlightname(temp);
				if (i < 23)
				     CPrintfXY(70, i+1, "%-5s", temp);
			case NOT:
				break;
		}
	}
	if (thingdata[Modeptr->current][thingorder[23]] != 0) // Print Name String
	{
	int j;
	int stringlen;
	char buffer[32];
	stringlen = strlen(textdata+thingdata[Modeptr->current][thingorder[23]]);
	textattr(NORMAL);
	strncpy(buffer, textdata+thingdata[Modeptr->current][thingorder[23]], 31);
	for (j=0; j<strlen(buffer); j++)
		if (buffer[j] == '\n' || buffer[j] == '\r' || buffer[j] == '\b' ||
		    buffer[j] == '\f' || buffer[j] == '\t')
			buffer[j] = '~';
	CPrintfXY(25, 29, "%-31s", "Name String:");
	CPrintfXY(38, 29, "%-31s", buffer);
	j+=stringlen; // shut up stupid warning
	j-=stringlen; // about stringlen not being used when it is needed
	}
	else
		CPrintfXY(25, 29, "%-31s", "					    ");
}

// Prints out the little info part of the Thing editing screen

void Printthinginfo(void)
{
	if (redraw == ALL)
		Drawframe(0, NORMAL, 3, 3, 36, 8);

	textattr(NORMAL);

	if (redraw == ALL)
	{
		CPutsXY(5,  5, "Thing Number:");
		CPutsXY(5,  6, "Thing Name:");
	}

	CPrintfXY(19, 5, "%-5d", Modeptr->current+1);
	CPrintfXY(17, 6, "%-18s", namelist[Modeptr->current]);
}

// Prints out the miscellaneous part of the Thing editing screen

void Printthingmisc(void)
{
	int i;

	if (redraw == ALL)
		Drawframe(0, NORMAL, 3, 12, 33, 25);

	for (i=0; i<10; i++)
	{
		textattr(NORMAL);
		switch (redraw)
		{
			case ALL:
				CPutsXY(6, i+14, thingfields[thingorder[i]]);
			case DATA:
				gotoxy(21, i+14);
				if (thingorder[i] == WIDTH || thingorder[i] == HEIGHT ||
					((thingorder[i] == SPEED) &&
					((Modeptr->current != 0) && (thingdata[Modeptr->current][BITS] & 1024L))))
					cprintf("%-11ld", thingdata[Modeptr->current][thingorder[i]] >> 16);
				else
					cprintf("%-11ld", thingdata[Modeptr->current][thingorder[i]]);
			case NOT:
				break;
		}
	}
}

// Prints out the sound portion of the Thing editing screen

void Printthingsound(void)
{
	char temp[7];
	int i;

	if (redraw == ALL)
		Drawframe(0, NORMAL, 39, 3, 77, 11);

	for (i=5; i<10; i++)
	{
		textattr(NORMAL);
		switch (redraw)
		{
			case ALL:
				CPutsXY(41, i, thingfields[thingorder[i+5]]);
				CPutsXY(68, i, "[");
				CPutsXY(75, i, "]");
			case DATA:
				CPrintfXY(56, i, "%-11ld", thingdata[Modeptr->current][thingorder[i+5]]);
				textattr(NORMAL);

				Getsoundname(thingdata[Modeptr->current][thingorder[i+5]], temp);
				Highlightname(temp);
				CPrintfXY(69, i, "%-6s", temp);
			case NOT:
				break;
		}
	}
}

// Prints a message, color is dependant on content.

EBool Printwindow(char *message, int type)
{
	int x1, x2;
	EBool escape = NO;

	if (batch == YES)
	{
		puts(message);
		return NO;
	}

	x1 = 40 - (4+strlen(message))/2;
	x2 = 40 + (4+strlen(message))/2;

	// If x1 is < 1, x2 must be greater than 80.
	if (x1 < 1)
	{
		x1 = 1;
		x2 = 80;
	}

	// Get background and draw the window
	Getwindow(x1, 23, x2+1, 28);
	Drawframe(1, type, x1, 23, x2, 27);
	textattr(type);

	CPutsXY(x1+2, 25, message);

	if (Waitforevent(YES))
		escape = YES;

	Putwindow();

	return escape;
}
