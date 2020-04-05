// SeHackEd version 0.4
// Modified by Mike Fredericks, chohatsu@yahoo.com
// Thanks to Janis Legzdinsh for info, http://www.vavoom-engine.com
// based on DeHackEd version 3.0a
// Written by Greg Lewis, gregl@umich.edu
// If you release any versions of this code, please include
// the author in the credits.  Give credit where credit is due!

#include <alloc.h>
#include <conio.h>
#include <ctype.h>
#include <dir.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "dehacked.h"
#include "data.h"
#include "dheinit.h"

int main (int argc, char *argv[])
{
	EBool ExitLoop = NO;					// Are we out of the main loop?

	// Load config file
	Parseconfigfile();

	// Find what version to initialize memory for
	if (DetectDoomver(argc, argv) == -1)
		return 0;

	// Initialize memory according to current Doom version
	alldata[0] = thingdata  = new long[Data[THING].numobj][THING_FIELDS];
	alldata[1] = framedata  = new long[Data[FRAME].numobj][FRAME_FIELDS];
	alldata[2] = weapondata = new long[Data[WEAPON].numobj][WEAPON_FIELDS];
	alldata[3] = sounddata  = new long[Data[SOUND].numobj][SOUND_FIELDS];
	alldata[4] = spritedata = new long[Data[SPRITE].numobj];
	alldata[5] = textdata   = new char[Data[TEXT].length];
	alldata[6] = codepdata  = new long[Data[CODEP].numobj];
	alldata[7] = cheatdata  = new char[Data[CHEAT].length];
	alldata[8] = ammodata   = new long[Data[AMMO].numobj];
	alldata[9] = miscdata   = new long[Data[MISC].numobj];

	// Make sure we've got the necessary memory
	for (int i=0; i<NUMDATA; i++)
		if (alldata[i] == NULL)
			AbortProg("in Main");

	getcwd(curdir, MAXDIR);

	// Check out command line arguments
	if (Parsecommandline(argc, argv) == -1)
		ExitLoop = YES;
	else
	{
		// If it's not command-line only, change to 50 line mode, draw
		// the Thing windows, and the intro screen.
		batch = NO;
		InitMouse();
		if (loadlogo)
			Displaylogo();
		textmode(C4350);
		_setcursortype(_NOCURSOR);
		WipeScreen();
		Modeptr = &Screen[mode];
		Screen[mode].printfunc();
		redraw = NOT;
		if (loadlogo == NO)
			Printintro();
	}

        while (ExitLoop == NO)
        {
		// Redraw the correct screen
		if (redraw != NOT)
		{
			if (redraw == ALL)
				WipeScreen();

			// Draw the correct screen according to current mode
			Modeptr->printfunc();
			redraw = NOT;
		}

		// Highlight the current field
		Highlight(NHILIT);

		// The Process functions return a YES if the user is exiting...
		if (Waitforevent(NO))
			ExitLoop = ProcessKeypress();
		else
			ExitLoop = ProcessMouse();

		// Close the mouse, clear the screen, print exiting message.
		if (ExitLoop == YES)
		{
                        CloseMouse();
                        textmode(C80);
                        textattr(0x7);
                        gotoxy(1, 1);
                        puts("Exiting...");
                        puts("Bye!");
		}
        }

        // Delete allocated memory   
        for (i=0; i<NUMDATA; i++)
                delete[] alldata[i];

	// Close open files
	fclose(doomexefp);
        fclose(doomwadfp);
	fclose(doombakfp);

	return 0;
}

// Aborts program when out of memory, and asks if the user wants to
// write the changes first.  I hate it when DEU just crashes!

void AbortProg(char *func)
{
	int result;

	// Crash to text mode, let the user know what happened, and how much
	// memory is still left.
	textmode(C80);
	textattr(0x7);
	printf("Out of memory %s!\n", func);
	printf("Current farcoreleft: %d\n", farcoreleft());

	// If there are changes to write, ask the user if he/she wants to write
	// them.  This *should* work even if we have 0 memory free.
	if (changes == YES)
	{
		puts("Do you want to write all changes to the exe file?  ");
		result = getch();
		if (tolower(result) == 'y')
		{
			Writedoom();
			puts("Changes written.\n");
		}
		else
			puts("Changes not written.\n");
	}

	puts("Have a nice day.  Try again later with a bit more memory");
	puts("free...");
	exit(1);
}

// Detects the current Doom version, if one was not specified in the .ini
// file.  Also finds and open all necessary Doom files.

int DetectDoomver(int argc, char *argv[])
{
	char buffer[160] = "";
	long time = clock();

	Printtextintro();

	// If there was a command-line path for doom specified, make sure
	// the path ends in a backslash.
	if (argc > 1 && argv[1][0] != '-')
	{
		strcpy(buffer, argv[1]);
		if (buffer[strlen(buffer)-1] != '\\' && strlen(buffer) != 0)
			strcat(buffer, "\\");
	}

	if (GetDoomFiles(buffer) == -1)
		return -1;

	// Clear to end of line.
	clreol();
      
	// Version is set to -1 at the program start.  It will only be set to
        // something if the user has specified a version # in the sehacked.ini
	// file.
	if (version == NO_VER)
	{
		// Verify size
		fseek(doomexefp, 0, SEEK_END);

		switch (ftell(doomexefp))
		{
			case SIZE1_12:
				puts("Registered Doom v1.2 exe found...  DeHackEd no longer supports");
				puts("Doom 1.2.  I would highly recommend upgrading from Doom 1.2 to the");
				puts("newer Doom versions, such as the Ultimate Doom upgrade, which is");
				puts("supported by DeHackEd (and is free too!).");
				return -1;
			case SIZE1_16:
				version = DOOM1_16;
                                puts("Using Strife v1.0");
				break;
			case SIZE2_16:
				version = DOOM2_16;
                                puts("Using Strife v1.1");
				break;
			case SIZE2_17:
				version = DOOM2_17;
                                puts("Using Strife v1.2");
				break;
			case SIZE2_17A:
				version = DOOM2_17;
				puts("Using Doom 2 v1.7a");
				while (clock() - time < 180)
					;
				break;
			case SIZE2_19:
				version = DOOM2_19;
                                puts("Using Strife v1.3");
				break;
			case SIZE2_18:
				puts("\nDoom v1.8 exe found...  DeHackEd doesn't support Doom 1.8.");
				puts("Upgrade to version 1.9, and you'll be all set!");
				return -1;
			case SIZE1_19U:
				version = DOOM1_19U;
                                puts("Using Strife v1.31");  // Changed
				break;
			default:
				if (ftell(doomexefp) == doomsize)
                                        printf("Using user-specified Strife size: %ld\n", doomsize);
				else
				{
                                        puts("Unknown Strife exe file size!  You may have a modified Strife exe file,");
                                        puts("or you are using an unknown version of Strife.  Do you wish to continue?");
					puts("If you are not positive about this, answer no!");

					if (tolower(getch()) != 'y')
						return -1;
				}
		}
	}

	// Take care of data that differs between the different Doom versions:
	// Text length and number of text objects.
	Data[TEXT].length = textlength[version];
	Data[TEXT].numobj = textobjs[version];
	Screen[TEXT_ED].max = textobjs[version] - 1;

	return 0;
}

// Loads all of the data from the exe into the correct data structures.

void Loaddoom(FILE *exefp)
{
	int i;
	char *codepraw;
        //int tempver;

	if ((codepraw = new char[5855]) == NULL)
		AbortProg("in Loaddoom");

	// Initialize the thingdata to all 0's, mostly for the Clipboard.
	// Also initialize the misc data to 0's, since we only read single
	// bytes into a few of the array slots.
        //memset(thingdata, 0, 33120L); // changed
        memset(thingdata, 0, Data[THING].length);
	memset(miscdata,  0, Data[MISC].length);
	memset(codepdata, 0, Data[CODEP].length);

	// Read in the rest of the data
	for (i=0; i<NUMDATA; i++)
	{
		// These need to be read in special-case
		if (i == THING || i == CODEP || i == MISC)
			continue;

		fseek(exefp, Data[i].offset[version], SEEK_SET);
		fread(alldata[i], Data[i].length, 1, exefp);
	}

	// Read Thing data
	fseek(exefp, Data[THING].offset[version], SEEK_SET);
	fread(thingdata, Data[THING].objsize, Data[THING].numobj-1, exefp);
          
	// Get that one lonely iddt cheat.
        //fseek(exefp, Data[CHEAT].offset[version]-5692, SEEK_SET);
        //fread(cheatdata+143, 4, 1, exefp); // changed 3624 to 5692

	// Get that pesky misc data
        //for (i=0; i<16; i++)
        //{
        //        fseek(exefp, miscoffs[i][version], SEEK_SET);
                //fread(miscdata+i, miscsize[i], 1, exefp);
        //}

	// Get the code pointers
        //fseek(exefp, Data[CODEP].offset[version], SEEK_SET); // disabled
        //fread(codepraw, 5855, 1, exefp);

	// Parse the code pointers into the data structure
        //if (version == 0 || version == 1 || version == 2)
        //        tempver = 0;
        //else
        //        tempver = version - 2;

        //for (i=0; i<NUMCODEP; i++) // changed
                //codepdata[i] = *((long *)&(codepraw[codepoff[i][tempver]]));
                //codepdata[codepconv[i]] = *((long *)&(codepraw[i]));
        //        codepdata[codepconv[i]] = *((long *)&(codepraw[codepoff[i][tempver]]));
        
        delete[] codepraw;
}

// Handles the command line arguments.  Also opens the doom.exe file.
// Current verification of doom.exe is the file size.

int Parsecommandline(int argc, char *argv[])
{
	int i = 1;
	EBool quit = NO, loadon = NO;
	char buffer[160] = "";

	// If there was a command-line path for doom specified, increment
	// the current-argument variable, i.
	if (argc > 1 && argv[1][0] != '-')
		i++;

	// OK, load the stuff
	Loaddoom(doomexefp);

	// Parse all the command line args
	for (; i<argc; i++)
	{
		if (stricmp(argv[i], "-save") == 0)
		{
			int x, y, result;

			if (++i == argc)
			{
				puts("\nError: A patch file must be specified after -save!");
				return -1;
			}

			strcpy(buffer, argv[i]);
			printf("\nSaving patch file:  %s\n", buffer);
			x = wherex();
			y = wherey();

			result = Savepatch(buffer, NO);
			if (result == -1)
			{
				cputs("File exists!  Overwrite?  ");
				result = getch();
				gotoxy(x, y);
				clreol();
				if (tolower(result) != 'y')
					strcpy(buffer, "Write canceled.");
				else
					Savepatch(buffer, YES);
			}
			puts(buffer);
			quit = YES;
			loadon = NO;
		}
		else if (stricmp(argv[i], "-reload") == 0)
		{
			Loaddoom(doombakfp);
			Writedoom();
                        printf("\nStrife data reloaded from %s.\n", doombak);
			quit = YES;
			loadon = NO;
		}
		else if ((stricmp(argv[i], "-load") == 0) || loadon)
		{
			if (!loadon && (++i == argc))
			{
				puts("\nError: At least one patch file must be specified after -load!");
				return -1;
			}

			strcpy(buffer, argv[i]);
			printf("\nLoading patch file:  %s\n", buffer);
			if (Loadpatch(buffer) != ERROR)
				Writedoom();
			quit = YES;
			loadon = YES;
		}
		else
		{
			printf("  Cannot parse command \"%s\"!\n", argv[i]);
			Printoptions();
			return -1;
		}
	}

	// quit will be set if we are working on some command line arguments
	// and don't actually want to edit interactively.
	if (quit)
		return -1;
	else
		return 0;
}

// Parses the config file

void Parseconfigfile(void)
{
	FILE *cfgfp;
	char nextline[80];
	char *line2;
	int i;
	int numlines = 1;
	EBool match = NO;
	int tempver, result;
	char *options[23] = {"pathname",
								"editname",
								"normalname",
								"wadname",
								"params",
								"patchdir",
								"version",
								"size",
								"thingoff",
								"frameoff",
								"weaponoff",
								"soundoff",
								"spriteoff",
								"textoff",
								"codepoff",
								"cheatoff",
								"ammooff",
								"sbaddress",
								"sbirq",
								"sbdma",
								"askatload",
								"textlength",
								"loadlogo"};
	char *strptrs[6] = {doompath, doomexe, doombak, doomwad, doomargs,
							  patchdir};

// changed to sehacked.ini here
        if ((cfgfp = fopen("sehacked.ini", "rt")) == NULL)
	{
                puts("SeHackEd.ini not found.");
		return;
	}

	while (GetNextLine(nextline, numlines, cfgfp))
	{
		// Parse the line the for spaces or equal signs.
		result = ProcessLine(nextline, &line2);

		switch (result)
		{
			case 1:
				for (i=0; i<23; i++)
				{
					if (strcmpi(nextline, options[i]) == 0)
					{
						match = YES;
						switch (i)
						{
							case 0:
							case 1:
							case 2:
							case 3:
							case 4:
							case 5:
								strcpy(strptrs[i], line2);
								break;
							case 6:
								sscanf(line2, "%d", &tempver);
								if (tempver == 0)
									version = DOOM1_16;
								else if (tempver == 1)
									version = DOOM2_16;
								else if (tempver == 2)
									version = DOOM2_17;
								else if (tempver == 3)
									version = DOOM2_19;
								else if (tempver == 4)
									version = DOOM1_19U;
								break;
							case 7:
								sscanf(line2, "%ld", &doomsize);
								break;
							case 8:
							case 9:
							case 10:
							case 11:
							case 12:
							case 13:
							case 14:
							case 15:
							case 16:
								sscanf(line2, "%ld", &(Data[i-8].offset[version]));
								break;
							case 17:
								sscanf(line2, "%x", &(dev.addr));
								break;
							case 18:
								sscanf(line2, "%d", &(dev.irq));
								break;
							case 19:
								sscanf(line2, "%d", &SB_DMA_CHAN);
								break;
							case 20:
								if (stricmp(line2, "false") == 0)
									askatload = NO;
								break;
							case 21:
								sscanf(line2, "%ld", &(Data[TEXT].length));
								break;
							case 22:
								if (stricmp(line2, "false") == 0)
									loadlogo = NO;
								break;
						}
					}
				}
				break;
			case -1: printf("Line %d: No value after equal sign.\n", numlines);
				break;
			case -2: printf("Line %d: No value before equal sign.\n", numlines);
				break;
			case 2:
			case -3: printf("Line %d: Invalid single-word line detected.\n", numlines);
				break;
		}

//changed to sehacked.ini here
		if (match == NO)
		{
                        printf("Line %d: Cannot match variable \"%s\" in sehacked.ini!\n", numlines, nextline);
			break;
		}
		else
			match = NO;
	}

	fclose(cfgfp);
}

// Run Doom.  This sucker's tricky.  Probably the wrong way to do it too,
// but I'm not sure of a better way.

int RunExe(void)
{
	char buffer[80];
	char *argv[20];
	int i=2, j;

	// Check if the doompath actually exists.
	if (chdir(doompath) == -1)
	{
		sprintf(buffer, "Could not switch to %s!", doompath);
		Printwindow(buffer, ERROR);
		return -1;
	}

	// Init 'em to NULL, if that helps at all.  Hopefully prevents garbage
	// arguments from getting passed to Doom.
	for (j=0; j<20; j++)
		argv[j] = NULL;

	strcpy(buffer, doomargs);
	argv[0] = doomexe;
	argv[1] = buffer;

	// Parse the doomargs into separate arguments.
	// Not sure if this is necessary, but it seems to work.
	for (j=0; j<strlen(doomargs); j++)
	{
		if (buffer[j] == ' ' || buffer[j] == '\t')
		{
			buffer[j] = 0;

			if (argv[i-1] == buffer+j)
				argv[i-1] = buffer+j+1;
			else
			{
				argv[i] = buffer+j+1;
				i++;
			}
		}
		else if (buffer[j] == '\r' || buffer[j] == '\n')
			break;
	}
	buffer[j] = 0;

	// Change the screen mode, close files, etc.
	clearscreen(0, ' ');
	textmode(C80);
	CloseMouse();
	fclose(doomexefp);
        fclose(doomwadfp);
	fclose(doombakfp);

	// Now try to actually run it.
	spawnv(P_WAIT, doomexe, argv);

	// Set things up again.  Clear screen, re-open Doom files, init
	// mouse, etc.
	clearscreen(0, ' ');
	if (GetDoomFiles("") == -1)
		exit (1);
	InitMouse();
	textmode(C4350);
	_setcursortype(_NOCURSOR);
	chdir(curdir);
	redraw = ALL;

	return 0;
}

// Updates a Frame record and field with new info.

int Updateframe(void)
{
	char order[6] = {SPRITENUM, SPRITESUB, 0, NEXTFRAME, DURATION, ACTIONPTR};
	int curfield = Modeptr->field;
	int curnum = Modeptr->current;
	char buffer[20];
	char prompt[20];

	// Can't edit an invalid field
        if (curfield < 1 || curfield > 5) // changed 6 to 5
		return -1;

	curfield--;

	// Don't print the input box if we're on the "Bright Sprite" field, and
	// do a special input box if we're editing the code pointer.
	if (curfield == 5)
	{
		if (framedata[curnum][ACTIONPTR] != 0)
			if (Printinputwindow(buffer, "Enter the Frame number of the new code pointer:",
										LONGINT, 0) < 0)
				return -1;
	}
	else if (curfield != 2)
	{
		sprintf(prompt, "%s:", framefields[order[curfield]]);
		if (Printinputwindow(buffer, prompt, LONGINT, 0) < 0)
			return -1;
	}

	// "Bright Sprite" is really just the sprite sub-num's bit 15.
	if ((curfield == SPRITESUB) && (framedata[curnum][SPRITESUB] & 32768L))
		framedata[curnum][SPRITESUB] = atol(buffer) ^ 32768L;
	else if (curfield == 2)
		framedata[curnum][SPRITESUB] ^= 32768L;
	else if (curfield == 5)
	{
		if ((atol(buffer) < 0) ||
			 (atol(buffer) > Data[FRAME].numobj) ||
			 (framedata[atol(buffer)][ACTIONPTR] == 0))
		{
			Printwindow("Invalid Frame number!", ERROR);
			return -1;
		}

		// Change the code pointer to the given frame's code pointer
		codepdata[curnum] = framedata[atol(buffer)][ACTIONPTR];
	}
	else
		framedata[curnum][order[curfield]] = atol(buffer);

	return 0;
}

// Updates the misc screen with new information.

int Updatemisc(void)
{
        char order[6] = {BOB1FRAME, BOB2FRAME, BOB3FRAME, SHOOTFRAME, FIREFRAME, UNKNOWNW};
	int curfield = Modeptr->field;
	int curnum = Modeptr->current;
	char buffer[20];
	char prompt[20];
	long num;

	// If this is a field that shouldn't be edited, return.
        if (curfield < 1 || curfield > 25)  // changed 41 to 25
		return -1;

	curfield--;

        if (curfield < 8)
	{
		//This is a weapon field.  Update it with new information.

		// If this is a field that shouldn't be edited, return.  Ammo needs a
		// special check, because the ammo can't be edited if it's "N/A", not
		// a know ammo value.
                if ((weapondata[curnum][AMMOTYPE] >= 8) &&
			 (curfield == 1 || curfield == 2))
                        return -1; // Changed 8 to 9

		// Get the new value
		sprintf(prompt, "%s:", fullwepfields[curfield]);
		if (Printinputwindow(buffer, prompt, LONGINT, 0) < 0)
			return -1;

		num = atol(buffer);

		// Update the correct ammo or weapon data array with the new info.
		if (curfield == 0)
			weapondata[curnum][AMMOTYPE] = num;
		else if (curfield == 1)
			ammodata[weapondata[curnum][AMMOTYPE]] = num;
		else if (curfield == 2)
                        ammodata[weapondata[curnum][AMMOTYPE]+7] = num; //changed 4 to 7 or 9
		else
			weapondata[curnum][order[curfield-3]] = num;
	}
        else if (curfield < 25)
        	{
		// This is a cheat field. Get the new value.
		sprintf(prompt, "%s:", cheatfields[curfield-8]);
		if (Printinputwindow(buffer, prompt, STRING, cheatinfo[curfield-8][1]) < 0)
			return -1;

		// Convert the user's value into cheat-code values.
		Cheatconvert(cheatdata+cheatinfo[curfield-8][0], buffer, cheatinfo[curfield-8][1]);
	}
	else
	{
		// Everything else is the misc info.
                if (curfield == 32 || curfield == 39)
		{
			// Change this to signed byte
                        sprintf(prompt, "%s:", miscfields[curfield-25]); // changed 25 to 26
			if (Printinputwindow(buffer, prompt, CHARINT, 0) < 0)
				return -1;

                        miscdata[curfield-25] = atoi(buffer); // changed 25 to 26
		}
                else if (curfield == 40)
		{
			if (miscdata[INFIGHTING] == 0xCA)
				miscdata[INFIGHTING] = 0xDD;
			else
				miscdata[INFIGHTING] = 0xCA;
		}
		else
		{
                        sprintf(prompt, "%s:", miscfields[curfield-25]); // changed 25 to 26
			if (Printinputwindow(buffer, prompt, LONGINT, 0) < 0)
				return -1;

                        miscdata[curfield-25] = atol(buffer); // changed 25 to 26
		}
	}

	return 0;
}

// Updates a Sound record and field with new info

int Updatesound(void)
{
	int curfield = Modeptr->field;
	int curnum   = Modeptr->current;
	char order[3] = {TEXTP, ZERO_ONE, VALUE};
	char buffer[20];
	char prompt[20];

	if (curfield < 1 || curfield > 3)
		return -1;

	curfield--;

	sprintf(prompt, "%s:", soundfields[order[curfield]]);
	if (Printinputwindow(buffer, prompt, LONGINT, 0) < 0)
		return -1;

	if (curfield == 0)
		sounddata[curnum][TEXTP] = atol(buffer) + toff[version];
	else
		sounddata[curnum][order[curfield]] = atol(buffer);

	return 0;
}

// Updates the Sprite array with new info

int Updatesprite(void)
{
	char buffer[20];

	if (Printinputwindow(buffer, "Enter a new offset:", LONGINT, 0) < 0)
		return -1;

	spritedata[Modeptr->current] = atol(buffer) + toff[version];

	return 0;
}

// Update the text section

int Updatetext(void)
{
	char *buffer;
	EBool ExitLoop = NO, changes = NO;
	int stringlen, textoff = 0;
	int i;
	int curpos = 0, maxlen;
	char xpos, ypos, inputchar;
	char answer[2] = {0, 0};
	unsigned int x, y;					// Mouse x and y
	EButton lbutton, rbutton;			// State of left and right buttons

	Printtextstring(YES);

	// Save the screen.
	Getwindow(1, 27, 80, 50);
	Drawframe(1, INPUT, 1, 27, 80, 48);

	// Get a concrete number for which string we're on, from the offset
	// in the text section.
	for (i=0; i<Modeptr->current; i++)
	{
		stringlen = strlen(textdata+textoff);
		textoff += (stringlen & (~3)) + 4;
	}
	maxlen = (strlen(textdata+textoff)/4)*4+3;

	// Get some memory for the old and new text strings, abort if none
	// is available.
	buffer = new char[maxlen+1];

	if (buffer == NULL)
		AbortProg("in Inputtext");

	memset(buffer, 0, maxlen+1);

	// OK, so he wants to enter some new text also...
	textattr(INPUT);

	gotoxy(3, 29);
	cprintf("Enter new text. ESC quits, %3d chars min, %3d chars max. Current len:",
			 maxlen-3, maxlen);

	xpos = 3;
	ypos = 31;

	// Stay in the loop until the user exits
	while (!ExitLoop)
	{
		textattr(INPUT);
		CPrintfXY(75, 29, "%3d", curpos);

		gotoxy(xpos, ypos);
		_setcursortype(_NORMALCURSOR);

		// Wait for something to happen
		if (Waitforevent(NO))
		{
			_setcursortype(_NOCURSOR);
			inputchar = getch();

			// Ignore extended keys
			if (!inputchar)
			{
				inputchar = getch();
				continue;
			}

			// Depending on what the character was...
			switch(inputchar)
			{
				case ESC:
					ExitLoop = YES;
					break;
				case BKSP:
					// Check if we're at the beginning of the string and handle
					// those cases.
					if (curpos == 0)
						break;
					else if (curpos == 1)
						changes = NO;
					curpos--;

					// Yuck.  Try to handle the backspace over a newline elegantly.
					// It's tough cause of the cheap way I do the current position.
					if (buffer[curpos] == '\n')
					{
						for (i = curpos-1; i>=-1; i--)
							if (i == -1 || buffer[i] == '\n')
							{
								xpos = curpos-i+3;
								break;
							}
						gotoxy(xpos, --ypos);
						putch(' ');
					}
					gotoxy(--xpos, ypos);
					putch(' ');
					break;
				case RET:
					// Go to a new line on a return.
					if (curpos >= maxlen)
						break;
					changes = YES;
					buffer[curpos++] = '\n';
					textattr(INPDGRAY);
					cputs("\\n");
					xpos = 3;
					ypos++;
					break;
				default:
					// Take the character and enter it in the string.
					if (curpos >= maxlen)
						break;
					changes = YES;
					buffer[curpos++] = inputchar;
					textattr(INPUT);
					xpos++;
					putch(inputchar);
					break;
			}
		}
		else
		{
			// Handle the mouse-clicks.
			_setcursortype(_NOCURSOR);
			getLastEvent(x, y, lbutton, rbutton);

			if (LastEventButtons & RIGHTBUTTON && rbutton == buttonUp)
				ExitLoop = YES;
			else if (LastEventButtons & LEFTBUTTON && lbutton == buttonUp)
			{
				// Whoops, haven't handled this yet.
			}
		}

		// Text for exit conditions, whether the user wants to
		// save the text changes.
		if (ExitLoop)
		{
			if (changes == YES)
				if (Printinputwindow(answer, "Save your text changes?", YESNO, 1) < 0)
					ExitLoop = NO;

			// Check to make sure it's not too short.
			if (tolower(answer[0]) == 'y')
			{
				if (curpos < maxlen-3)
				{
					Printwindow("Text does not meet minimum length!", ERROR);
					ExitLoop = NO;
				}
				else
				{
					buffer[curpos] = 0;
					strcpy(textdata+textoff, buffer);
				}
			}
		}
	}

	// Restore BOTH windows
	Putwindow();
	Putwindow();

	// Free memory
	delete buffer;

	// Return -1 if the user answered "no" to that question about saving
	// text changes.
	if (tolower(answer[0]) == 'y')
		return 0;
	else
		return -1;
}

// Updates a Thing record with new info to a certain field.

int Updatethings(void)
{
	int curfield = Modeptr->field;
	int curnum   = Modeptr->current;
	char buffer[20];
	char prompt[20];

	// Return on an invalid field
	if (curfield < 1 || curfield > 56)
		return -1;

	curfield--;

	// Check if this is a non-bit field that we're editing
        if (curfield < 23)
	{
		// If so, print the prompt and get a new value.
		sprintf(prompt, "%s:", thingfields[thingorder[curfield]]);
		if (Printinputwindow(buffer, prompt, LONGINT, 0) < 0)
			return -1;

		// Replace the Thing values, being very careful of the fields that
		// are multiplied by 65536.
		if (thingorder[curfield] == WIDTH || thingorder[curfield] == HEIGHT ||
			((thingorder[curfield] == SPEED) &&
			((curnum != 0) && (thingdata[curnum][BITS] & 65536L))))
			thingdata[curnum][thingorder[curfield]] = (atol(buffer)) << 16;
		else
			thingdata[curnum][thingorder[curfield]] = atol(buffer);
	}
        else if (curfield < 55)
		// Otherwise just update the correct bit in the correct Bits field
		// by XORing it with itself.
		thingdata[curnum][BITS] ^= (1L << (curfield-23));
	else
	{
		// Ask for a new Thing Name.
		if (Printinputwindow(buffer, "Thing Name:", STRING, 17) < 0)
			return -1;

		// Put it in place.
		strcpy(namelist[curnum], buffer);
	}

	return 0;
}

// Write only the changeable data structures to the doom.exe

void Writedoom(void)
{
	int i, tempver;

	// Write the rest of the data
	for (i=0; i<NUMDATA; i++)
	{
		// These need to be read in special-case
		if (i == THING || i == CODEP || i == MISC || i == CHEAT)
			continue;

		fseek(doomexefp, Data[i].offset[version], SEEK_SET);
                if (i == THING)
                     fwrite(alldata[i], 33120L, 1, doomexefp);
                else
                     fwrite(alldata[i], Data[i].length, 1, doomexefp);
	}

	// Write the Thing stuff
	fseek(doomexefp, Data[THING].offset[version], SEEK_SET);
	fwrite(thingdata, Data[THING].objsize, Data[THING].numobj-1, doomexefp);

	// Write the cheats, making sure NOT to write the last 5 bytes.
	fseek(doomexefp, Data[CHEAT].offset[version], SEEK_SET);
	fwrite(cheatdata, Data[CHEAT].length-5, 1, doomexefp);

	// Write that one lonely iddt cheat.
        //fseek(doomexefp, Data[CHEAT].offset[version]-5692, SEEK_SET);
        //fwrite(cheatdata+143, 4, 1, doomexefp); // changed 3624 to 5692

	// Write out the code pointer stuff
	if (version == 0 || version == 1 || version == 2)
		tempver = 0;
	else
		tempver = version - 2;

        //for (i=0; i<NUMCODEP; i++) // changed
        //{
        //        fseek(doomexefp, Data[CODEP].offset[version]+codepoff[i][tempver], SEEK_SET);
                //fseek(doomexefp, Data[CODEP].offset[version]+i, SEEK_SET);
                //fseek(doomexefp, Data[CODEP].offset[version]+codepconv[i], SEEK_SET);
                //fwrite(&(codepdata[codepconv[i]]), 4, 1, doomexefp);
                //fwrite(&(codepdata[i]), 4, 1, doomexefp);
        //}


	// Write that pesky misc data
        //for (i=0; i<16; i++)
        //{
        //        fseek(doomexefp, miscoffs[i][version], SEEK_SET);
                //fwrite(miscdata+i, miscsize[i], 1, doomexefp); // changed
        //}

        /*
	// Write the even more pesky misc data
        //fseek(doomexefp, miscoffs[16][version], SEEK_SET);
        //fwrite(miscdata+MAXHEALTH, 4, 1, doomexefp);

        //fseek(doomexefp, miscoffs[17][version], SEEK_SET);
        //fwrite(miscdata+MAXARMOR, 4, 1, doomexefp);

        //fseek(doomexefp, miscoffs[18][version], SEEK_SET);
        //fwrite(miscdata+MAXSOUL, 4, 1, doomexefp);

        //fseek(doomexefp, miscoffs[19][version], SEEK_SET);
        //fwrite(miscdata+GODHEALTH, 4, 1, doomexefp);

        //fseek(doomexefp, miscoffs[20][version], SEEK_SET);
        //fwrite(miscdata+BFGAMMO, 1, 1, doomexefp);

        //fseek(doomexefp, miscoffs[21][version], SEEK_SET);
        //fwrite(miscdata+BFGAMMO, 1, 1, doomexefp);*/
        tempver+=1; // Shut up warning
        tempver-=1;
}

