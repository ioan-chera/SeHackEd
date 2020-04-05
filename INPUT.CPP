// SeHackEd version 0.3
// Modified by Mike Fredericks, chohatsu@yahoo.com
// Thanks to Janis Legzdinsh for info, http://www.vavoom-engine.com
// based on DeHackEd version 3.0a
// Written by Greg Lewis, gregl@umich.edu
// If you release any versions of this code, please include
// the author in the credits.  Give credit where credit is due!

#include <conio.h>
#include <ctype.h>
#include <dir.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "dehacked.h"
#include "input.h"

// Gets an input string with certain specifications.

int Getinput(int promptlen, char *string, int maxlen, int type, int x1, char firstchar)
{
	int curx, cury, curpos = 0;
	unsigned char inputchar;
	unsigned int x, y;					// Mouse x and y
	EButton lbutton, rbutton;			// State of left and right buttons
	EBool ExitLoop = NO;
	EBool escape = NO;

	// Find where to position the cursor
	if (type == STRING)
	{
		cury = 26;
		curx = x1 + 2;
	}
	else
	{
		curx = x1+4+promptlen;
		cury = 25;
	}

	// A little kludge to handle the fact that when a user is in the
	// Thing name pick-list, and types a number, it must be entered into
	// the box as the first number.  So if there is a character passed to
	// this function, make it the first one in the string.
	if (firstchar)
	{
		curpos = 1;

		string[0] = firstchar;
		gotoxy(curx, cury);
		curx++;
		putch(firstchar);
	}

	// Keep getting input until done
	while (!ExitLoop)
	{
		string[curpos] = 0;
		gotoxy(curx, cury);
		_setcursortype(_NORMALCURSOR);

		// Get some input
		if (Waitforevent(NO))
		{
			// OK, it was a keypress
			_setcursortype(_NOCURSOR);
			inputchar = getch();

			// If it's an extended key, ignore it.
			if (inputchar == 0)
			{
				getch();
				continue;
			}
			else if (inputchar == ESC)
				// Escape exits
				escape = ExitLoop = YES;
			else if (inputchar == RET)
			{
				// Return acts like Escape if there hasn't been anything
				// entered at all.
				ExitLoop = YES;
				if (curpos == 0)
					escape = YES;
			}
			else if (inputchar == BKSP)
			{
				// Get rid of the last thing typed.
				if (curpos > 0)
				{
					string[--curpos] = 0;
					gotoxy(--curx, cury);
					putch(' ');
				}
			}
		}
		else
		{
			// It was a mouse-click.
			_setcursortype(_NOCURSOR);
			getLastEvent(x, y, lbutton, rbutton);

			if (LastEventButtons & RIGHTBUTTON && rbutton == buttonUp)
				escape = ExitLoop = YES;
			else if (LastEventButtons & LEFTBUTTON && lbutton == buttonUp)
			{
				// If we're in the yes-or-no screen, check to make sure the
				// yes and no buttons weren't clicked on.  Exit if they were.
				if (type == YESNO && y == 26)
				{
					if (x > 28 && x < 36)
					{
						strcpy(string, "y");
						ExitLoop = YES;
					}
					else if (x > 42 && x < 49)
					{
						strcpy(string, "n");
						ExitLoop = YES;
					}
				}
			}
		}

		// Depending on what type of input we're accepting, check to see if
		// the user entered a valid character and process it.
		switch (type)
		{
			case STRING:
				// We're inputting a string, which means anything above a
				// space and not extended is OK.  Also make sure we haven't
				// reached the max string length.
				if ((inputchar >= ' ') && (inputchar <= 127) && (curpos != maxlen))
				{
					string[curpos++] = inputchar;
					curx++;
					putch(inputchar);
				}
				break;
			case LONGINT:
				// We're inputting a long integer, which may or may not have
				// a leading sign indicator.
				if (((inputchar == '+' || inputchar == '-') && (curpos == 0))
					 || isdigit(inputchar))
				{
					// Check to be sure that we haven't overflowed the maximum
					// or minimum value of a long integer...  do this by checking
					// the current data and seeing if it is within the range of
					// LONG_MAX/10 and LONG_MIN/10.
					if (atol(string) <=  214748364L &&
						 atol(string) >= -214748364L &&
						 curpos < 11)
					{
						string[curpos++] = inputchar;
						curx++;
						putch(inputchar);
					}
				}
				break;
			case CHARINT:
				// This is inputting a char-range (-128 - 127) integer.
				if (((inputchar == '+' || inputchar == '-') && (curpos == 0))
					 || isdigit(inputchar))
				{
					string[curpos] = inputchar;
					string[curpos+1] = 0;

					if (atoi(string) > 127 || atoi(string) < -128 || curpos > 3)
					{
						gotoxy(curx-1, cury);
						string[curpos-1] = inputchar;
						string[curpos] = 0;
					}
					else
					{
						curx++;
						curpos++;
					}
					putch(inputchar);
				}
				break;
			case YESNO:
				// We want one character, only one, no more, no less, and it
				// has got to be a 'y' or an 'n', no doubts about it.
				if (tolower(inputchar) == 'y' || tolower(inputchar) == 'n')
				{
					putch(inputchar);
					string[curpos] = tolower(inputchar);
					string[1] = 0;
					ExitLoop = YES;
				}
				break;
		}
	}

	// Escape is true if Esc was hit (or right button clicked)
	if (escape)
	{
		string[0] = 0;
		if (inputchar == RET)
			return -2;
		else
			return -1;
	}
	else
		return 0;
}

// Gets the current "window", which smaller window in the big Thing or
// Misc editing screen we're on.

int GetWindow(int curfield)
{
	int i;

	for (i=0; i < Modeptr->numfields; i++)
		if (curfield <= Modeptr->windowdata[i][1])
			return i;

	return 0;
}

// The file load box

int Loadfilelist(char *filename)
{
	char (*filenames)[13];
	EBool ExitLoop = NO;
	int i;
	int curnumber = 0, firstnum = 0;
	int maxnum = 0;
	char key;
	unsigned int x, y;					// Mouse x and y
	EButton lbutton, rbutton;			// State of left and right buttons
	char searchkey[18] = "";
	char curpos = 0;
	struct ffblk fileblock;
	char path[80];

	strcpy(path, patchdir);
        strcat(path, "\\*.seh"); // changed deh to seh

	i = findfirst(path, &fileblock, 0);
	while (!i)
	{
		maxnum++;
		i = findnext(&fileblock);
	}

	if (maxnum == 0)
	{
		Printwindow("No files found in patch directory!", ERROR);
		return -1;
	}

	// Get some memory for an array of all the text strings
	filenames = new char[maxnum][13];

	// Abort if we can't get the memory
	if (filenames == NULL)
		AbortProg("in LoadFile");

	// Fill in the array of filenames with the list of the directory
	// names.
	findfirst(path, &fileblock, 0);
	strcpy(filenames[0], fileblock.ff_name);
	i = 1;
	while (!findnext(&fileblock))
	{
		strcpy(filenames[i], fileblock.ff_name);
		i++;
	}

	// Sort the array
	qsort(filenames, maxnum, 13, Sortfunction);

	// Save the background text, and draw the window frame up there, with
	// the scroll arrows.
	Getwindow(27, 9, 55, 42);
	Drawframe(1, INPUT, 27, 9, 54, 41);
	textattr(INPUT);
	CPutsXY(54, 13, "");
	CPutsXY(54, 38, "");

	// This is the main input loop for this function
	while (ExitLoop == NO)
	{
		// Just draw everything every time, rather than computing
		// what needs to be done.  My drawing routines are fast enough
		// anyways.  :)
		textattr(INPUT);
		CPutsXY(29, 11, "Choose a file to load or");
		CPutsXY(29, 12, "hit Escape to cancel:");

		// This is as good a place as any to verify this
		if (maxnum <= 26)
			firstnum = 0;

		// Loop through, draw each Thing name
		for (i = 0; i < 26; i++)
		{
			if (curnumber == i + firstnum)
				textattr(INPHILIT);
			else
				textattr(INPUT);
			if (i+firstnum < maxnum)
				CPrintfXY(31, i+14, "%-13s", filenames[i + firstnum]);
		}

		// Get a key/mouse click
		if (Waitforevent(NO))
		{
			key = getch();

			// !key means that a special (function, arrow) key was pressed.
			if (!key)
			{
				key = getch();
				Listkeyinput(key, firstnum, curnumber, maxnum-1, 26);
			}
			else
			{
				if (key == ESC)
				{
					// Escape = exit without selecting a new filename
					curnumber = -1;
					ExitLoop = YES;
				}
				else if (key == RET)
					// Return = select this filename
					ExitLoop = YES;
				else
				{
					// Backspace = chop a letter off the current search key
					if (key == BKSP)
					{
						if (curpos > 0)
							curpos--;
						searchkey[curpos] = 0;
					}
					else
					{
						// Otherwise, the default is just add it to the search key
						searchkey[curpos] = key;
						if (curpos < 19)
							curpos++;
					}

					// Find the closest match to what was typed.
					curnumber = -1;
					for (i=0; i<maxnum; i++)
						if (stricmp(searchkey, filenames[i]) <= 0)
						{
							curnumber = i;
							break;
						}
					if (curnumber == -1)
						curnumber = maxnum - 1;

					// Make sure the new curnumber is onscreen.
					if ((curnumber < firstnum) || (curnumber > firstnum + 25))
					{
						if (curnumber < 13)
							firstnum = 0;
						else if (curnumber > maxnum-26)
							firstnum = maxnum-26;
						else
							firstnum = curnumber - 13;
					}
				}
			}
		}
		else
		{
			getLastEvent(x, y, lbutton, rbutton);

			// Handle arrow input
			Listmouseinput(x, y, 53, 12, 37, firstnum, curnumber, maxnum, 26);

			// Right button = abort
			if (LastEventButtons & RIGHTBUTTON && rbutton == buttonUp)
			{
				curnumber = -1;
				ExitLoop = YES;
			}
			else if ((LastEventButtons & LEFTBUTTON) && (lbutton == buttonUp) &&
						 x > 29 && x < 47 && y > 12 && y < 39)
			{
				if (firstnum + y - 13 < maxnum)
				{
					curnumber = firstnum + y - 13;
					ExitLoop = YES;
				}
			}
		}
	}

	// Slap the background text back up onscreen.
	Putwindow();

	// If the curnumber is valid (not -1), copy the current filename into
	// the filename string.
	if (curnumber != -1)
		strcpy(filename, filenames[curnumber]);

	// Free memory, return value.
	delete filenames;

	if (curnumber != -1)
		return 0;
	else
		return -1;
}
// The 'Go to <Thing>' window

int Printgowindow(void)
{
	char (*thingnames)[18];
	EBool ExitLoop = NO;
	EBool gotnum = NO;
	int i, curnumber = 0, firstnum = 0;
	int maxnum = Data[THING].numobj;
	char key;
	unsigned int x, y;					// Mouse x and y
	EButton lbutton, rbutton;			// State of left and right buttons
	char searchkey[18] = "";
	char curpos = 0;
	int thingnum;

	// Get some memory for an array of all the text strings
	thingnames = new char[maxnum][18];

	// Abort if we can't get the memory
	if (thingnames == NULL)
		AbortProg("in PrintGoWindow");

	// Fill in the array of Thing names with the normal array of Thing
	// names.
	memcpy(thingnames, namelist, 18*maxnum);

	// Sort the array
	qsort(thingnames, maxnum, 18, Sortfunction);

	// Save the background text, and draw the window frame up there, with
	// the scroll arrows.
	Getwindow(27, 9, 55, 42);
	Drawframe(1, INPUT, 27, 9, 54, 41);
	textattr(INPUT);
	CPutsXY(54, 13, "");
	CPutsXY(54, 38, "");

	// This is the main input loop for this function
	while (ExitLoop == NO)
	{
		// Just draw everything every time, rather than computing
		// what needs to be done.  My drawing routines are fast enough
		// anyways.  :)
		textattr(INPUT);
		CPutsXY(29, 11, "Choose an item to go to");
		CPutsXY(29, 12, "or enter a number:");

		// Loop through, draw each Thing name
		for (i = 0; i < 26; i++)
		{
			if (curnumber == i + firstnum)
				textattr(INPHILIT);
			else
				textattr(INPUT);
			CPrintfXY(31, i+14, "%-18s", thingnames[i + firstnum]);
		}

		// Get a key/mouse click
		if (Waitforevent(NO))
		{
			key = getch();

			// !key means that a special (function, arrow) key was pressed.
			if (!key)
			{
				// Get the extended part of the keypress and go to the generic
				// list-input routine.
				key = getch();
				Listkeyinput(key, firstnum, curnumber, maxnum-1, 26);
			}
			else
			{
				if (key == ESC)
				{
					// Escape = exit without selecting a new Thing
					// Set gotnum to YES so that we don't try to find the
					// thing number, but leave it at -1 instead.
					gotnum = YES;
					thingnum = -1;
					ExitLoop = YES;
				}
				else if (key == RET)
					// Return = select this Thing
					ExitLoop = YES;
				else if (isdigit(key))
				{
					// Typing a digit brings up the Goto Thing box.  Provided
					// the user didn't escape out of it, return the number
					// Also remember to put the character back into the input
					// stream.
					thingnum = GotoObject(key);
					if (thingnum != -1)
					{
						gotnum = YES;
						ExitLoop = YES;
					}
				}
				else
				{
					// Backspace = chop a letter off the current search key
					if (key == BKSP)
					{
						if (curpos > 0)
							curpos--;
						searchkey[curpos] = 0;
					}
					else
					{
						// Otherwise, the default is just add it to the search key
						searchkey[curpos] = key;
						if (curpos < 19)
							curpos++;
					}

					// Find the closest match to what was typed.
					curnumber = -1;
					for (i=0; i<maxnum; i++)
						if (stricmp(searchkey, thingnames[i]) <= 0)
						{
							curnumber = i;
							break;
						}
					if (curnumber == -1)
						curnumber = maxnum - 1;

					// Make sure the new curnumber is onscreen.
					if ((curnumber < firstnum) || (curnumber > firstnum + 25))
					{
						if (curnumber < 13)
							firstnum = 0;
						else if (curnumber > maxnum-26)
							firstnum = maxnum-26;
						else
							firstnum = curnumber - 13;
					}
				}
			}
		}
		else
		{
			getLastEvent(x, y, lbutton, rbutton);

			// Handle arrow input
			Listmouseinput(x, y, 53, 12, 37, firstnum, curnumber, maxnum, 26);

			// Right button = abort
			if (LastEventButtons & RIGHTBUTTON && rbutton == buttonUp)
			{
				thingnum = -1;
				ExitLoop = YES;
				gotnum = YES;
			}
			else if ((LastEventButtons & LEFTBUTTON) && (lbutton == buttonUp) &&
						 x > 29 && x < 47 && y > 12 && y < 39)
			{
				curnumber = firstnum + y - 13;
				ExitLoop = YES;
			}
		}
	}

	// Slap the background text back up onscreen.
	Putwindow();

	// If the curnumber is valid (not -1), compares the chosen string with
	// each string in the regular Thing name list to find the correct Thing
	// to go to.
	if (gotnum == NO)
	{
		thingnum = -1;
		for (i=0; i<maxnum; i++)
			// Compares the chosen string with each string in the Thing name
			// list to find the correct Thing to go to.
			if (strcmp(thingnames[curnumber], namelist[i]) == 0)
			{
				thingnum = i;
				break;
			}

		// Need to do an addition step in Frame mode
		if (mode == FRAME_ED)
			thingnum = thingdata[thingnum][NORMALFRAME];
	}

	// Free memory, return value.
	delete thingnames;
	return thingnum;
}

// Act on a keypress

EBool ProcessKeypress(void)
{
	int  result;				// The result of inputs, used for error testing
	int key;						// For input
	int i;
	int last = Modeptr->max - Modeptr->min;
	int &curfield = Modeptr->field;
	int &curobj = Modeptr->current;

	key = getch();

	if (!key)
	{
		key = getch();

		// Unhighlight the current field
		Highlight(NORMAL);

		// If we are on the THING or MISC editing screens, do one set of
		// movements.
		if (mode == THING_ED || mode == MISC_ED)
		{
			i = GetWindow(curfield);

			// Take care of arrow keys etc.
			switch (key)
			{
				case RIGHT:
					// Thing editing mode, when we check the fieldinfo array
					// for the new position...
					if (i == Modeptr->numfields-1)
						curfield = Modeptr->windowdata[0][0];
					else
						curfield = Modeptr->windowdata[i+1][0];
					break;
				case LEFT:
					if (i == 0)
						curfield = Modeptr->windowdata[Modeptr->numfields-1][0];
					else
						curfield = Modeptr->windowdata[i-1][0];
					break;
				case UP:
					// If this is the Thing mode, Up means go to a different
					// window.
					if (curfield > Modeptr->windowdata[i][0])
						curfield--;
					else
						curfield = Modeptr->windowdata[i][1];
					break;
				case DOWN:
					if (curfield < Modeptr->windowdata[i][1])
						curfield++;
					else
						curfield = Modeptr->windowdata[i][0];
					break;
				case PGUP:
					GoPageUp();
					break;
				case PGDN:
					GoPageDown();
					break;
				case HOME:
					// Go to the first object, if we're not already there, and
					// redraw.
					if (curobj != 0)
					{
						curobj = 0;
						redraw = DATA;
					}
					break;
				case END:
					// Go to the last object, if we're not already there, and
					// set the top row on the screen to 37 higher.
					if (curobj != last)
					{
						curobj = last;
						redraw = DATA;
					}
					break;
			}
		}
		else
		{
			Listkeyinput(key, Modeptr->toprow, Modeptr->current, last, 38);

			// Handle LEFT and RIGHT arrows here.  Merely a matter of
			// incrementing/decrementing the current field.
			if (key == LEFT)
			{
				if (curfield > 1)
					curfield--;
				else
					curfield = Modeptr->numfields;
			}
			else if (key == RIGHT)
			{
				if (curfield < Modeptr->numfields)
					curfield++;
				else
					curfield = 1;
			}
		}

		// Take care of function keys
		if (key == F1)
			Printhelp();
		else if (key > F1 && key < F8)
			Changemode(key-F2);
	}
	else
	{
		// Handle normal keypresses
		switch (tolower(key))
		{
			case ESC:
				// Quit.  Return, do exit checking in Main.
				return ConfirmQuit();
			case RET:
				// Edit the current field, if applicable.
				if (Modeptr->updatefunc() == 0)
				{
					changes = YES;
					redraw = DATA;
				}
				break;
			case ' ':
				// Show frames, text, and play sound, if applicable.
				result = -1;
				switch (mode)
				{
					case THING_ED:
						if (curfield >= Modeptr->windowdata[2][0] &&
							 curfield <  Modeptr->windowdata[3][0])
							result = Showframe(thingdata[curobj][thingorder[curfield-1]]);
						else if (curfield >= Modeptr->windowdata[1][0] &&
									curfield <  Modeptr->windowdata[2][0])
							Playsound(thingdata[curobj][thingorder[curfield-1]]);
						break;
					case FRAME_ED:
						if (curfield == 4)
							result = Showframe(framedata[curobj][NEXTFRAME]);
						else
							result = Showframe(curobj);
						break;
					case MISC_ED:
                                                if (curfield >= 4 && curfield <= 8)
							result = Showframe(weapondata[curobj][curfield-3]);
						break;
					case SOUND_ED:
						Playsound(curobj+1);
						break;
					case TEXT_ED:
						Printtextstring(NO);
						break;
				}
				if (result != -1)
					redraw = ALL;
				break;
			case 'a':
				// Print an about/status box.
				Printintro();
				break;
			case 'c':
				// Copy an entry.
				redraw = DATA;
				if (Copydata() != -1)
					changes = YES;
				break;
			case 'g':
				// Go to a specific object, by a list of names.

				// Unhighlight the current field
				Highlight(NORMAL);

				// Print the list of Thing names if applicable, otherwise
				// do the generic Go to box.
				if (mode == THING_ED || mode == FRAME_ED)
				{
					if ((result = Printgowindow()) != -1)
					{
						curobj = result;
						redraw = DATA;
						Changemode(mode);
					}
				}
				else if (GotoObject(0) != -1)
					redraw = DATA;
				break;
			case 'j':
				// Jump to the info in whatever field is highlighted.

				// Unhighlight the current field first, the jump wherever
				// necessary.
				Highlight(NORMAL);
				Jump();

				break;
			case 'l':
				// Load a patch file.
				Loadpatchname();
				break;
			case 'r':
				// Run Doom
				if (RunExe() != -1)
					redraw = ALL;
				break;
			case 's':
				// Save a patch
				Savepatchname();
				break;
			case 'u':
				// Reload from the Doom.exe file.
				Loaddoom(doomexefp);
                                Printwindow("Strife exe data reloaded.", INFO);
				changes = NO;
				redraw = DATA;
				break;
			case 'w':
				// Write changes (if we've made any).
				if (changes == NO)
					Printwindow("No changes made!", INFO);
				else
				{
					Writedoom();
					Printwindow("Changed data written to exe.", INFO);
					changes = NO;
				}
				break;
			case 'z':
				// Reload from original Doom.exe file.
				Loaddoom(doombakfp);
                                Printwindow("Original Strife exe data loaded.", INFO);
				changes = YES;
				redraw = DATA;
				break;
		}
	}

	return NO;
}

// Act on a mouse action

EBool ProcessMouse(void)
{
	int  result;				// The result of inputs, used for error testing
	unsigned int x, y;		// Mouse x and y
	EButton lbutton, rbutton;// State of left and right buttons
	int i;
	EBool Foundspot = NO;
	long elapsed;
	static int didthisone = -1;
	static long prevlclick = 0;

	getLastEvent(x, y, lbutton, rbutton);

	if (LastEventButtons != eventButtons)
		didthisone = -1;

	// OK, we're holding down the left button, so it's probably on an arrow
	// key somewhere.
	if (eventButtons & LEFTBUTTON)
	{
		// Get the current time, and subtract the starting time from
		// it to find elapsed time (in clock ticks).
		elapsed = clock()-lclicktime;

		// If this is the third tick and we haven't done this tick's
		// action yet, then do it... slight pause at first for slow
		// clicking people.
		if (((elapsed == 0) || ((elapsed > 6) && (elapsed % 3 == 0))) &&
			 (didthisone != elapsed))
		{
			// Test the pageup/pagedown arrows.  There might be a better way.
			if (x == Modeptr->arrows[0]-1 && y == Modeptr->arrows[1]-1)
			{
				// This is the up button

				// Set didthisone to the elapsed time, so that we know
				// we've performed the action on this tick already.
				didthisone = elapsed;

				// And, of course, do the correct thing for whichever
				// editing screen.
				GoPageUp();
			}
			else if (x == Modeptr->arrows[2]-1 && y == Modeptr->arrows[3]-1)
			{
				didthisone = elapsed;
				GoPageDown();
			}
		}
	}

	if ((lbutton == buttonUp) && (LastEventButtons & LEFTBUTTON))
	{
		if (y == 49)
		{
			// The bar on the bottom of the screen.
			if (x > 7 && x < 15)
				Printhelp();
			else if (x > 21 && x < 26)
				Loadpatchname();
			else if (x > 32 && x < 47)
				Printintro();
			else if (x > 52 && x < 57)
				Savepatchname();
			else if (x > 63 && x < 72)
				return ConfirmQuit();
		}
		else if (y == 0)
		{
			// The mode bar at the top of the screen.
			if (x > 3 && x < 13)
				Changemode(THING_ED);
			else if (x > 16 && x < 26)
				Changemode(FRAME_ED);
			else if (x > 29 && x < 37)
				Changemode(MISC_ED);
			else if (x > 40 && x < 50)
				Changemode(SOUND_ED);
			else if (x > 53 && x < 64)
				Changemode(SPRITE_ED);
			else if (x > 67 && x < 75)
				Changemode(TEXT_ED);
		}
		else
		{
			// Switch to any applicable field that was clicked
			// (for editing).

			// Unhighlight the current field first
			Highlight(NORMAL);

			for (i=0; i < posinfo[mode][1]; i++)
			{
				if (x >= Fielddata[i+posinfo[mode][0]][1] &&
					 x <= Fielddata[i+posinfo[mode][0]][2])
				{
					if (posinfo[mode][2] == 1 && y > 6 && y < 75)
					{
						Foundspot = YES;
						Modeptr->field = i+1;
						Modeptr->current = Modeptr->toprow+y-7;
						if ((lclicktime-prevlclick < 6) && (Modeptr->updatefunc() == 0))
						{
							changes = YES;
							redraw = DATA;
						}
					}
					else if (posinfo[mode][2] == 0 &&
								y == Fielddata[i+posinfo[mode][0]][0])
					{
						// Edit the current field, if applicable.
						Modeptr->field = i+1;
						Foundspot = YES;
						if ((lclicktime-prevlclick < 6) && (Modeptr->updatefunc() == 0))
						{
							changes = YES;
							redraw = DATA;
						}
					}
				}

				if (Foundspot == YES)
					break;
			}

			// If the mouse click still hasn't been handled,
			// take care of special cases.
			if (Foundspot == NO)
			{
				switch (mode)
				{
					case THING_ED:
						if (x > 3 && x < 34 && y == 4)
						{
							if ((result = Printgowindow()) != -1)
							{
								Modeptr->current = result;
								redraw = DATA;
							}
						}
						else if (x > 63 && x < 75 && y > 3 && y < 9)
							Playsound(thingdata[Modeptr->current][thingorder[y+6]]);
                                                else if (x > 67 && x < 75 && y > 14 && y < 24) // changed 23 to 24
						{
							if (Showframe(thingdata[Modeptr->current][thingorder[y]]) != -1)
								redraw = ALL;
						}
						break;
					case FRAME_ED:
						if (x > 14 && x < 20 && y > 6 && y < 75)
							if (Showframe(Modeptr->toprow+y-7) != -1)
								redraw = ALL;
						break;
					case MISC_ED:
                                                if (x > 50 && x < 58 && y > 35 && y < 42) // changed 41 to 42
                                                        if (Showframe(weapondata[Modeptr->current][y-35]) != -1)
								redraw = ALL;
						break;
					case SOUND_ED:
						if (x > 24 && x < 32 && y > 6 && y < 75)
							Playsound(Modeptr->current+y-6);
						break;
				}
			}
		}
		prevlclick = lclicktime;
	}

	return NO;
}

// Called by qsort to compare strings

int Sortfunction(const void *a, const void *b)
{
	return (stricmp((char *)a, (char *)b));
}

// Returns when key is pressed or mouse is clicked (or left mouse button
// is held down).

EBool Waitforevent(EBool eatevent)
{
	EBool ExitLoop = NO;

	showMouseCursor();

	// Flush the input stream
	while (kbhit() != 0)
		if (!getch())
			getch();

	do
	{
		// Wait until a key is pressed or the mouse is used.
		while (kbhit() == 0 && eventHappened == NO && !(eventButtons & LEFTBUTTON)) ;

		// If a key was pressed...
		if (kbhit() == 0)
		{
			unsigned int x, y;
			EButton lbutton, rbutton;

			getLastEvent(x, y, lbutton, rbutton);

			// Otherwise, if the left or right button was let up, or
			// the left button is down, quit.
			if (((eventButtons & LEFTBUTTON) && !eatevent) ||
				((lbutton == buttonUp) && (LastEventButtons & LEFTBUTTON)) ||
				((rbutton == buttonUp) && (LastEventButtons & RIGHTBUTTON)))
				ExitLoop = YES;
		}
		else
			ExitLoop = YES;

	} while (!ExitLoop);

	hideMouseCursor();

	// Remove the event if we are supposed to.
	if (eatevent)
		if (kbhit())
		{
			if (!getch())
				getch();
		}
		else
			LastEventButtons = eventButtons;

	// Return if it was a keyboard press or not.
	if (kbhit())
		return YES;
	else
		return NO;
}
