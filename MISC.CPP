// SeHackEd version 0.3
// Modified by Mike Fredericks, chohatsu@yahoo.com
// Thanks to Janis Legzdinsh for info, http://www.vavoom-engine.com
// based on DeHackEd version 3.0a
// Written by Greg Lewis, gregl@umich.edu
// If you release any versions of this code, please include
// the author in the credits.  Give credit where credit is due!

#include <conio.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "dehacked.h"
#include "misc.h"

// Changes to a new editing mode

void Changemode(EModes newmode)
{
	// Redraw only the data if the person tries to switch the mode he's
	// currently in.
	if (newmode != mode)
		redraw = ALL;
	else
		redraw = DATA;

	mode = newmode;
	Modeptr = &Screen[mode];

	// Verify that the current item is onscreen for the different
	// lists... ie, if we're on Frame #562, make sure that one is somewhere
	// on the screen.  Adjust TOPROW accordingly.
	if (Modeptr->current < Modeptr->toprow ||
		 Modeptr->current > Modeptr->toprow + 37)
	{
		if (Modeptr->current < 14)
			Modeptr->toprow = 0;
		else if (Modeptr->current > Modeptr->max - Modeptr->min - 37)
			Modeptr->toprow = Modeptr->max - Modeptr->min - 37;
		else
			Modeptr->toprow = Modeptr->current - 12;
	}
}

// Converts a cheat string to its decrypted equivalent.  If cheat is not
// as long as length, string will be padded with FFs.  Does NOT append a
// 0.

void Cheatconvert(char *string, char *cheat, int length)
{
	int i = 0;

	// Continue converting until we run into the end of the string, or
	// we've converted length characters.
	while (i<length && cheat[i] != 0)
	{
		string[i]  =  cheat[i] & 36;
		string[i] |= (cheat[i] & 128) >> 7;
		string[i] |= (cheat[i] & 64) >> 5;
		string[i] |= (cheat[i] & 16) >> 1;
		string[i] |= (cheat[i] & 8) << 1;
		string[i] |= (cheat[i] & 2) << 5;
		string[i] |= (cheat[i] & 1) << 7;
		i++;
	}

	// And pad the rest of the string with FFs.
	while (i < length)
		string[i++] = 0xFF;
}

// Confirms that the user wants to quit if changes have been made.

EBool ConfirmQuit(void)
{
	char buffer[2];

	// Ask the user if he wants to quit, only if changes have actually been
	// made.
	if (changes == NO)
		return YES;

	if ((Printinputwindow(buffer, "You have unwritten changes.  Quit?", YESNO, 1) < 0)
		 || (buffer[0] == 'n'))
		return NO;
	else
		return YES;
}

// Copies from one object to another.

int Copydata(void)
{
	int minnum = Modeptr->min;
	int fromnum, tonum;
	char buffer[80];
	char temp[20];
	int i, result;

	// Text strings and cheat codes can't be copied due to the stringent
	// length restrictions.  It's possible to, but very very few of the
	// strings or cheats are = in length.
	if (mode != THING_ED && !(mode == MISC_ED && Modeptr->field < 9) &&
		 mode != FRAME_ED)
	{
		Printwindow("You can not copy at this location.", ERROR);
		return -1;
	}

	sprintf(buffer, "Enter the %s number to copy from (default is %d):",
			  Modeptr->name, Modeptr->current+minnum);
	result = Printinputwindow(temp, buffer, LONGINT, 0);

	if (result == -1)
		return -1;
	else if (result == -2)
		fromnum = Modeptr->current;
	else
	{
		// Verify the from number to make sure it's not invalid.
		fromnum = atol(temp);
		if (fromnum < minnum || fromnum > Modeptr->max)
		{
			sprintf(buffer, "Invalid %s number!", Modeptr->name);
			Printwindow(buffer, ERROR);
			return -1;
		}

		// Compensate for different starting indices in the objects, ie, Things
		// start at #1, whereas Frames start at #0.
		fromnum -= minnum;
	}

	sprintf(buffer, "Enter the %s number to copy to (default is %d):",
			  Modeptr->name, Modeptr->current+minnum);
	result = Printinputwindow(temp, buffer, LONGINT, 12);

	if (result == -1)
		return -1;
	else if (result == -2)
		tonum = Modeptr->current;
	else
	{
		// Verify the to number to make sure it's not invalid.
		tonum = atol(temp);
		if (tonum < minnum || tonum > Modeptr->max)
		{
			sprintf(buffer, "Invalid %s number!", Modeptr->name);
			Printwindow(buffer, ERROR);
			return -1;
		}

		// Compensate for different starting indices in the objects, ie, Things
		// start at #1, whereas Frames start at #0.
		tonum -= minnum;
	}

	// OK, the dope is trying to copy something over itself.
	if (fromnum == tonum)
	{
		Printwindow("Cannot copy an object onto itself!", ERROR);
		return -1;
	}

	// And do the copy.  Each case needs to be handled separately due to
	// peculiarities.
	if (mode == FRAME_ED)
	{
		// Copy all data except code pointers
		for (i=0; i<FRAME_FIELDS; i++)
			if (i != ACTIONPTR)
				framedata[tonum][i] = framedata[fromnum][i];

		// Check if both code pointers are non-zero so that they can be
		// copied too.
                /*if (codepdata[tonum] && codepdata[fromnum])
                        codepdata[tonum] = codepdata[fromnum];*/ //changed to disable copying of codep
	}
	else if (mode == THING_ED)
	{
		memcpy(thingdata[tonum], thingdata[fromnum], Data[THING].objsize);
		strcpy(namelist[tonum], namelist[fromnum]);
	}
	else if (mode == MISC_ED)
		memcpy(weapondata[tonum], weapondata[fromnum], Data[WEAPON].objsize);

	return 0;
}

// Gets the frame name and puts it into string.

void Getframename(int framenum, char *string)
{
	if (framenum == 0)
		strcpy(string, "none");
	else if (framenum > 0 && framenum <= Data[FRAME].numobj &&
				framedata[framenum][SPRITENUM] >= 0 &&
				framedata[framenum][SPRITENUM] < Data[SPRITE].numobj)
        {
                strncpy(string, textdata + spritedata[framedata[framenum][SPRITENUM]] - toff[version], 4);
		string[4] = (char)framedata[framenum][SPRITESUB] + 'A';
		string[5] = 0;
	}
	else
		strcpy(string, "ERROR");
}

// Gets the sound name and puts it into string.

void Getsoundname(int soundnum, char *string)
{
        soundnum--;  // changed, disabled

        if (soundnum == -1)  // Changed -1 to 0
		strcpy(string, "none");
        /*else if ((soundnum < 0) ||
				(soundnum > Data[SOUND].numobj) ||
				(sounddata[soundnum][TEXTP] < toff[version]) ||
				(sounddata[soundnum][TEXTP] > toff[version] + Data[TEXT].length))
                strcpy(string, "ERROR");*/  // disabled else if
        else
		strncpy(string, textdata + sounddata[soundnum][TEXTP] - toff[version], 6);
	string[6] = 0;
}

// Gets the sprite name and puts it into a string

void Getspritename(int spritenum, char *string)
{
/*        if ((spritenum < 0) ||
		 (spritenum > Data[SPRITE].numobj) ||
		 (spritedata[spritenum] < toff[version]) ||
		 (spritedata[spritenum] > toff[version] + Data[TEXT].length))
		strcpy(string, "ERR ");
        else*/ // Changed, disabled if-else
                strncpy(string, &(textdata[spritedata[spritenum] - toff[version]]), 4);
	string[4] = 0;
}

// Gets a text number from a text offset

int Gettextnum(int offset)
{
	int i = 0, stringlen, current = 0;

	offset -= toff[version];

	while (current <= offset)
	{
		stringlen = strlen(textdata+current);
		current += (stringlen & (~3)) + 4;
		i++;
	}

	return i-1;
}

// GoPageDown goes down (for Page-Down or arrow click)

void GoPageDown(void)
{
	int last = Modeptr->max - Modeptr->min;

	if (Modeptr->current < last)
		redraw = DATA;
	else
		redraw = NOT;

	// Note: I don't wanna comment this.  Take a look at GoPageUp for
	// details.  I'm lazy.
	if (mode == THING_ED || mode == MISC_ED)
	{
		if (Modeptr->current < last)
			Modeptr->current++;
	}
	else
	{
		if (Modeptr->current > last-38)
			Modeptr->current = last;
		else
			Modeptr->current += 38;

		if (Modeptr->toprow > last-76)
			Modeptr->toprow = last-37;
		else
			Modeptr->toprow += 38;
	}
}

// GoPageUp goes up (for Page-Up or arrow click)

void GoPageUp(void)
{
	// If we aren't already AT the top, redraw everything, otherwise
	// don't bother redrawing.
	if (Modeptr->current > 0)
		redraw = DATA;
	else
		redraw = NOT;

	// In the Thing and Weapon editing modes, "GoPageUp" actually means
	// "advance to next object".
	if (mode == THING_ED || mode == MISC_ED)
	{
		// If we're not at the top, go up one.
		if (Modeptr->current > 0)
			Modeptr->current--;
	}
	else
	{
		// If we're less than a page away from the top, go to the top.
		// If we're more than a page away from the top, go up by one page
		// (which is 38, which should be a global define BTW).
		if (Modeptr->current < 38)
			Modeptr->current = 0;
		else
			Modeptr->current -= 38;

		// TOPROW is the number of the object at the top of the screen.
		// Set it to the correct value.
		if (Modeptr->toprow < 38)
			Modeptr->toprow = 0;
		else
			Modeptr->toprow -= 38;
	}
}

// Go to a given object for any mode

int GotoObject(int firstdigit)
{
	char prompt[30];
	char buffer[20];
	long num;
	int min = Modeptr->min;
	EBool error = NO;

	if (mode == TEXT_ED)
		return Textsearch();

	sprintf(prompt, "Enter a new %s number:", Modeptr->name);
	if (Printinputwindow(buffer, prompt, LONGINT, 0, firstdigit) < 0)
		return -1;

	// Check for negative numbers, at least.
	num = atol(buffer);
	if (num < 0)
	{
		Printwindow("Negative numbers are invalid.", ERROR);
		return -1;
	}

	// Make sure it's a valid object to go to.  Note that for the Thing
	// and Frame editors we need to do a special return of the correct
	// number to go to. All other modes, just go there.
	switch (mode)
	{
		case THING_ED:
		case FRAME_ED:
			if (num > Modeptr->max)
				error = YES;
			else
				return num - min;
			break;
		case MISC_ED:
		case SOUND_ED:
		case SPRITE_ED:
			if (num > Modeptr->max)
				error = YES;
			else
				Modeptr->current = num - min;
			break;
	}

	// Invalid number to go to
	if (error == YES)
	{
		Printwindow("That value is too high!", ERROR);
		return -1;
	}

	// Make it so.
	Changemode(mode);

	return 0;
}

// Highlights the current field (equivalent to unhighlighting if the
// NORMAL attribute is given).

void Highlight(unsigned char attribute)
{
	int ref = posinfo[mode][0] + Modeptr->field - 1;
	int i, row;

	if (mode == THING_ED || mode == MISC_ED)
		row = Fielddata[ref][0];
	else
		row = Modeptr->current - Modeptr->toprow + 7;

	// Lump a few constants in here to speed it up in the loop.
	row = row*160 + 1;

	// Check the super duper master array of highlight locations, and
	// change the color of whatever information is there accordingly.
	for (i=Fielddata[ref][3]-1; i < Fielddata[ref][4]; i++)
		screenbptr[row + (i<<1)] = attribute;
}

// Turn strings certain colors if they are "none" or "ERROR".

void Highlightname(char *name)
{
	if ((strcmp(name, "ERROR") == 0) || (strcmp(name, "ERR ") == 0))
		textattr(NERROR);
	else if (strcmp(name, "none") == 0)
		textattr(NGRAY);
	else
		textattr(NORMAL);
}

// Jumps to an object in another editor based on the current selection
// in the current editor.

void Jump(void)
{
	int &curfield = Modeptr->field;
	int &curobj = Modeptr->current;

	// Depending on which mode and which field we're currently on, do a
	// bunch of different things...
	switch (mode)
	{
		case THING_ED:
			// Thing editor...  if we are on a Frame field, go to the
			// respective frame.
			if (curfield >= Modeptr->windowdata[2][0] &&
				 curfield <  Modeptr->windowdata[3][0])
			{
				Screen[FRAME_ED].current =	thingdata[curobj][thingorder[curfield-1]];
				Changemode(FRAME_ED);
			}
			else if (curfield >= Modeptr->windowdata[1][0] &&
						curfield <  Modeptr->windowdata[2][0] &&
						thingdata[curobj][thingorder[curfield-1]] != 0)
			{
				// Or a Sound field, go to the respective sound.
				Screen[SOUND_ED].current =	thingdata[curobj][thingorder[curfield-1]]-1;
				Changemode(SOUND_ED);
			}
			break;
		case FRAME_ED:
			// Frame editor, go to the Sprite editor if we're on a Sprite
			// field, or the Next Frame if we are there.
			if (curfield == 1 || curfield == 2)
			{
				// Sprite number and sprite subnumber
				Screen[SPRITE_ED].current = framedata[curobj][0];
				Changemode(SPRITE_ED);
			}
			else if (curfield == 4)
			{
				// Next Frame
				Screen[FRAME_ED].current = framedata[curobj][4];
				Changemode(FRAME_ED);
				redraw = DATA;
			}
			break;
		case MISC_ED:
			// Jump to the Frames if we are on one of those fields
			if (curfield >= 4 && curfield <= 8)
			{
				Screen[FRAME_ED].current = weapondata[curobj][curfield-3];
				Changemode(FRAME_ED);
			}
			break;
		case SPRITE_ED:
			// Go to the Text editor
			Screen[TEXT_ED].current = Gettextnum(spritedata[curobj]);
			Changemode(TEXT_ED);
			break;
		case SOUND_ED:
			// Go to the Text editor
			Screen[TEXT_ED].current = Gettextnum(sounddata[curobj][TEXTP]);
			Changemode(TEXT_ED);
			break;
	}
}

// Acts on various input keys for the list-type windows

void Listkeyinput(char key, int &topnum, int &curnum, int maxnum, int listsize)
{
	switch (key)
	{
		case HOME:
			if (curnum != 0)
			{
				curnum = 0;
				redraw = DATA;
			}
			topnum = 0;
			break;
		case END:
			if (curnum != maxnum)
			{
				curnum = maxnum;
				redraw = DATA;
			}
			topnum = maxnum - listsize + 1;
			break;
		case PGUP:
			if (curnum == 0)
				break;

			redraw = DATA;

			if (topnum < listsize)
				topnum = 0;
			else
				topnum -= listsize;

			if (curnum < listsize)
				curnum = 0;
			else
				curnum -= listsize;
			break;
		case PGDN:
			if (curnum == maxnum)
				break;

			redraw = DATA;

			if (topnum > maxnum - 2*listsize)
				topnum = maxnum - listsize + 1;
			else
				topnum += listsize;

			if (curnum > maxnum - listsize)
				curnum = maxnum;
			else
				curnum += listsize;
			break;
		case UP:
			if (curnum > 0)
				curnum--;

			if (curnum < topnum)
			{
				topnum = curnum;
				redraw = DATA;
			}
			break;
		case DOWN:
			if (curnum < maxnum)
				curnum++;

			if (curnum > topnum + listsize - 1)
			{
				topnum++;
				redraw = DATA;
			}
			break;
	}
}

// Checks for arrow key action for list boxes, perform action if
// appropriate.

void Listmouseinput(int mousex, int mousey, int arrowx, int arrowy1,
						  int arrowy2, int &topnum, int &curnum, int maxnum,
						  int listsize)
{
	long elapsed;
	static didthisone = 0;

	if (LastEventButtons != eventButtons)
		didthisone = -1;

	// Otherwise, if the left or right button was let up, quit.
	if (eventButtons & LEFTBUTTON)
	{
		elapsed = clock()-lclicktime;

		// Test the pageup/pagedown arrows if enough time has passed
		// since the last time we checked.
		if (((elapsed == 0) || ((elapsed > 6) && (elapsed % 4 == 0))) &&
			 (didthisone != elapsed))
		{
			if (mousex == arrowx && mousey == arrowy1)
			{
				didthisone = elapsed;
				if (topnum > listsize)
					topnum -= listsize;
				else
					topnum = 0;

				if (curnum > listsize)
					curnum -= listsize;
				else
					curnum = 0;
			}
			else if (mousex == arrowx && mousey == arrowy2)
			{
				didthisone = elapsed;
				if (topnum < maxnum - 2*listsize)
					topnum += listsize;
				else
					topnum = maxnum-listsize;

				if (curnum < maxnum - listsize)
					curnum += listsize;
				else
					curnum = maxnum-1;
			}
		}
	}
}

// Gets user input for loading a patch file

void Loadpatchname(void)
{
	char temp[2];
	int result;
	char buffer[80];

	// If we are supposed to ask the user about reloading the original
	// data, do it.
	if (askatload)
		if ((Printinputwindow(temp, "Reload original exe data first?", YESNO, 1) == 0) &&
			  tolower(temp[0]) == 'y')
		{
			Loaddoom(doombakfp);
                        Printwindow("Original Strife exe data loaded.", INFO);
			changes = YES;
			redraw = DATA;
		}

	// Attempt to get a filename
	result = Printinputwindow(buffer, "Enter a filename to load (enter for list-box):",
									  STRING, 70);

	// This means the user hit enter.  In that case, load up the list box.
	if (result == -2)
		result = Loadfilelist(buffer);

	// If the user hasn't canceled by now, load the patch.
	if (result != -1)
	{
		Loadpatch(buffer);
		redraw = DATA;
		changes = YES;
	}
}

// Gets user input for saving a patch file

void Savepatchname(void)
{
	char temp[2];
	char buffer[80];
	int result;

	if (Printinputwindow(buffer, "Enter save patch filename:", STRING, 70) == 0)
	{
		// Savepatch will return a result of -1 when a file by the same name
		// already exists.
		result = Savepatch(buffer, NO);
		if (result == -1)
		{
			// Verify that the user wants to overwrite the patch
			if ((Printinputwindow(temp, "File exists! Overwrite?", YESNO, 1) < 0) ||
				  tolower(temp[0]) != 'y')
			{
				strcpy(buffer, "Write canceled.");
				result = INFO;
			}
			else
				result = Savepatch(buffer, YES);
		}

		// THIS time the result would mean that the save failed for some
		// reason.
		if (result == -1)
			Printwindow("Unable to save patch file!", ERROR);
		else
			Printwindow(buffer, result);
	}
}

// Searches for text in the Text editor, starting from the current item.

int Textsearch(void)
{
	char searchstring[41];
	char yesno[2] = "";
	int i, j;
	int length, curpos;

	// Get something to search for.
	if (Printinputwindow(searchstring, "Enter Text to search for (max 40 chars):",
		 STRING, 40) < 0)
		return -1;

	length = strlen(searchstring);
	curpos = 0;

	// Convert the string to all lowercase (case-insensitive search)
	for (i=0; i<length; i++)
		searchstring[i] = tolower(searchstring[i]);

	// Yes, I took a Data Structures and Algorithms class and now I'm doing
	// a brute-force search.
	while (curpos < Data[TEXT].length-length)
	{
		for (j=0; j<length; j++)
			if (tolower(textdata[curpos+j]) != searchstring[j])
				break;

		// This means it is a match.
		if (j == length)
		{
			Modeptr->current = Gettextnum(curpos+toff[version]);
			redraw = ALL;
			Changemode(TEXT_ED);
			Modeptr->printfunc();
			Highlight(NHILIT);

			// Ask if the user wants to continue searching
			if ((Printinputwindow(yesno, "Continue searching?", YESNO, 0) == -1) ||
				 yesno[0] == 'n')
			{
				Highlight(NORMAL);
				break;
			}
			Highlight(NORMAL);
		}
		curpos++;
	}

	// Print a "text not found" message if the search reached the termination
	// condition.
	if (curpos >= Data[TEXT].length-length)
		Printwindow("Text not found.", INFO);

	return 0;
}

