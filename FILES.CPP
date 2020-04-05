// SeHackEd version 0.4
// Modified by Mike Fredericks, chohatsu@yahoo.com
// Thanks to Janis Legzdinsh for info, http://www.vavoom-engine.com
// based on DeHackEd version 3.0a
// Written by Greg Lewis, gregl@umich.edu
// If you release any versions of this code, please include
// the author in the credits.  Give credit where credit is due!

#include <conio.h>
#include <ctype.h>
#include <dir.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "dehacked.h"
#include "files.h"

// Try to find a given Doom file.

int Checkforfile(char *doomfile, char *arg1, char *doomext, char *type, FILE **fp)
{
	char xpos = wherex();
	char ypos = wherey();
	EBool commandname = NO, found;
	char filename[80];

	// If there's nothing specified in the ini, use the command line.
	if (strlen(doomfile) == 0 || strlen(arg1) != 0)
		commandname = YES;

	// This (attempts to) find the doom.exe file
	if (commandname == YES)
	{
		strcpy(filename, arg1);
                strcat(filename, "strife"); // changed doom to strife
		strcat(filename, doomext);
	}
	else
		strcpy(filename, doomfile);

	// Get current x and y position first.
	printf("Checking for %s...   ", filename);
	if ((*fp = fopen(filename, type)) == NULL)
	{
                // Cheap kludge.  Swap a 1 in on the end of the filename (change from
                // "strife.exe" to "strife1.exe", if we're running command-line AND
		// the name extension isn't "hack.exe", which it will be for the
		// backup file.
		if ((commandname == YES) && (strlen(doomext) < 7))
		{
                        // Slip a "1" into the filename before the extension.
                        strcpy(strstr(filename, doomext), "1");
                        strcat(filename, doomext);
                        printf("not found!      \nChecking for %s...  ", filename); // changed
                        if ((*fp = fopen(filename, "r+b")) == NULL)
				found = NO;
			else
                                found = YES;
		}
		else
			found = NO;
	}
	else
		found = YES;

	strcpy(doomfile, filename);

	if (found == YES)
	{
		printf("found!");
		gotoxy(xpos, ypos);
		return 0;
	}
	else
	{
		printf("not found!      \n");
		return -1;
	}
}

// Converts a patch file from 1.2 to 1.666 format.

void Convertpatch(FILE *patchp, char patchformat)
{
	int i, j;
	long buffer[22];
	char forder[7] = {NORMALFRAME, MOVEFRAME, INJUREFRAME, CLOSEATTACKFRAME,
							FARATTACKFRAME, DEATHFRAME, EXPLDEATHFRAME};
	char sorder[5] = {ALERTSOUND, ATTACKSOUND, PAINSOUND, DEATHSOUND, ACTSOUND};

	// Cycle through every Thing in Doom 1.2, converting as we go.
	for (i=0; i<103; i++)
	{
		fread(buffer, 88, 1, patchp);
		for (j=0; j<5; j++)
			buffer[sorder[j]] = soundconvar[buffer[sorder[j]]];
		for (j=0; j<7; j++)
			buffer[forder[j]] = frameconvar[buffer[forder[j]]];
		memcpy(thingdata[thingconvar[i]], buffer, 88);
	}

	// Ammo is the same
        fread(ammodata, 8, 10, patchp);  // Changed 4 to 8 and 8 to 10

        for (i=0; i<14; i++)  // Changed 8 to 14
	{
                fread(buffer,  25, 1, patchp);  // changed 24 to 25
                for (j=0; j<8; j++)  // Changed 5 to 8
			buffer[j+1] = frameconvar[buffer[j+1]];
                memcpy(weapondata[i], buffer, 25);  // changed 24 to 25
	}

	if (patchformat == 2)
	{
		for (i=0; i<512; i++)
		{
			fread(buffer, 28, 1, patchp);
			buffer[SPRITENUM] = spriteconvar[buffer[SPRITENUM]];
			buffer[NEXTFRAME] = frameconvar[buffer[NEXTFRAME]];
			memcpy(framedata[frameconvar[i]], buffer, 28);
		}
	}
}

// Creates a save patch file that contains only differences between
// the current doom exe and the backup one.  Note: this function is
// long and unwieldy.  Inelegant code sucks.

int CreateDiffSave(FILE *patchp)
{
	long (*thingorig )[THING_FIELDS];
	long (*frameorig )[FRAME_FIELDS];
	long (*weaponorig)[WEAPON_FIELDS];
	long (*soundorig )[SOUND_FIELDS];
	long *spriteorig;
	char *textorig;
	long *codeporig;
	char *cheatorig;
	long *ammoorig;
	long *miscorig;
	void *allorig[NUMDATA];

	EBool nameprinted;
	int i, j, k;
	char buffer[80];

	// Initialize memory according to largest values needed, for loading
	// original data from the unaltered exe file.
	allorig[0] = thingorig  = new long[Data[THING].numobj][THING_FIELDS];
	allorig[1] = frameorig  = new long[Data[FRAME].numobj][FRAME_FIELDS];
	allorig[2] = weaponorig = new long[Data[WEAPON].numobj][WEAPON_FIELDS];
	allorig[3] = soundorig  = new long[Data[SOUND].numobj][SOUND_FIELDS];
	allorig[4] = spriteorig = new long[Data[SPRITE].numobj];
	allorig[5] = textorig   = new char[Data[TEXT].length];
        allorig[6] = codeporig  = new long[Data[CODEP].numobj];
	allorig[7] = cheatorig  = new char[Data[CHEAT].length];
	allorig[8] = ammoorig   = new long[Data[AMMO].numobj];
	allorig[9] = miscorig   = new long[Data[MISC].numobj];

	// Check if any of the memory didn't come though, and abort if so.
	// If the memory is OK, copy the current data to the new memory
	// locations.  This allows us to load up the data again into the
	// normal data structures.
	for (i=0; i<NUMDATA; i++)
	{
		if (allorig[i] == NULL)
			AbortProg("in CreateDiff");
		else
			memcpy(allorig[i], alldata[i], Data[i].length);
	}

	Loaddoom(doombakfp);

	// Check each element in each array of the original data vs the
	// info found in the backup exe.  If they're different, copy the old
	// value to the patch file.
	for (i=0; i<Data[THING].numobj-1; i++)
	{
		nameprinted = NO;
		for (j=0; j<THING_FIELDS; j++)
		{
			if (thingorig[i][j] != thingdata[i][j])
			{
				if (nameprinted == NO)
				{
					fprintf(patchp, "\nThing %d (%s)\n", i+1, namelist[i]);
					nameprinted = YES;
				}
				fprintf(patchp, "%s = %ld\n", thingfields[j], thingorig[i][j]);
			}
		}
	}

	for (i=0; i<Data[SOUND].numobj; i++)
	{
		nameprinted = NO;
		for (j=0; j<SOUND_FIELDS; j++)
		{
			if (soundorig[i][j] != sounddata[i][j])
			{
				if (nameprinted == NO)
				{
                                        fprintf(patchp, "\nSound %d\n", i+1); // changed i+1 to i
					nameprinted = YES;
				}
				fprintf(patchp, "%s = %ld\n", soundfields[j], soundorig[i][j]);
			}
		}
	}

	for (i=0; i<Data[FRAME].numobj; i++)
	{
		nameprinted = NO;
		for (j=0; j<FRAME_FIELDS; j++)
		{
			// I may draw flak for this one, but don't save the action
			// pointers to the .deh file.  They're different every version,
			// but make no difference at all.
			if ((frameorig[i][j] != framedata[i][j]) &&
				 (j != ACTIONPTR))
			{
				if (nameprinted == NO)
				{
					fprintf(patchp, "\nFrame %d\n", i);
					nameprinted = YES;
				}
				fprintf(patchp, "%s = %ld\n", framefields[j], frameorig[i][j]);
			}
		}
	}

	for (i=0; i<Data[SPRITE].numobj; i++)
		if (spriteorig[i] != spritedata[i])
			fprintf(patchp, "\nSprite %d\nOffset = %ld\n", i, spriteorig[i]);

        for (i=0; i<7; i++)  // Changed 4 to 7
	{
		nameprinted = NO;
		if (ammoorig[i] != ammodata[i])
		{
			fprintf(patchp, "\nAmmo %d (%s)\n", i, ammolist[i]);
			nameprinted = YES;
			fprintf(patchp, "Max ammo = %ld\n", ammoorig[i]);
		}

                if (ammoorig[i+4] != ammodata[i+7]) // Changed 4 to 7
		{
			if (nameprinted == NO)
				fprintf(patchp, "\nAmmo %d (%s)\n", i, ammolist[i]);
                        fprintf(patchp, "Per ammo = %ld\n", ammoorig[i+7]);  // changed 4 to 7
		}
	}

	for (i=0; i<Data[WEAPON].numobj; i++)
	{
		nameprinted = NO;
		for (j=0; j<WEAPON_FIELDS; j++)
		{
			if (weaponorig[i][j] != weapondata[i][j])
			{
				if (nameprinted == NO)
				{
					fprintf(patchp, "\nWeapon %d (%s)\n", i, weaponlist[i]);
					nameprinted = YES;
				}
				fprintf(patchp, "%s = %ld\n", weaponfields[j], weaponorig[i][j]);
			}
		}
	}

        /*for (i=0; i<Data[CODEP].numobj; i++) // changed
	{
		// Check if the code pointer has been changed.  If so, we need to
		// search through all of the frames to see if we can find the frame
		// that this code pointer has been changed to.
		if (codepdata[i] != codeporig[i])
		{
			// Find the corresponding Frame that it has been changed to.
			for (j=0; j<Data[FRAME].numobj; j++)
				if (framedata[j][ACTIONPTR] == codeporig[i])
					break;

			// Find out which code pointer this is (necessary for 3.0 patches
			// but I wish I could change it now)
			for (k=0; k<NUMCODEP; k++)
                                if (k == i)//if (codepconv[k] == i) // changed
					break;

			if (j == Data[FRAME].numobj)
			{
				sprintf(buffer, "Ignoring invalid code pointer data for Frame %d.", i);
				Printwindow(buffer, ERROR);
			}
			else
				fprintf(patchp, "\nPointer %d (Frame %d)\nCodep Frame = %d\n",
					  k, i, j);
		}
        } */

	nameprinted = NO;
	for (i=0; i<Data[CHEAT].numobj; i++)
	{
		if (strncmp(cheatorig+cheatinfo[i][0], cheatdata+cheatinfo[i][0],
			 cheatinfo[i][1]) != 0)
		{
			if (nameprinted == NO)
			{
				fprintf(patchp, "\nCheat 0\n");
				nameprinted = YES;
			}
			Cheatconvert(buffer, cheatorig+cheatinfo[i][0], cheatinfo[i][1]);
			buffer[cheatinfo[i][1]] = 0;
			fprintf(patchp, "%s = %s\n", cheatfields[i], buffer);
		}
	}

	nameprinted = NO;
	for (i=0; i<Data[MISC].numobj; i++)
	{
		if (miscorig[i] != miscdata[i])
		{
			if (nameprinted == NO)
			{
				fprintf(patchp, "\nMisc 0\n");
				nameprinted = YES;
			}
			fprintf(patchp, "%s = %ld\n", miscfields[i], miscorig[i]);
		}
	}

	// Write the text LAST, since it's the most error-prone part, and
	// hardest to recover from.  And skip the last few chars, since some-
	// thing wierd happens if I include them.  Hmmm..
	for (i=0; i<Data[TEXT].length-4; i += (strlen(textorig+i) & ~3) + 4)
		if (strcmp(textorig+i, textdata+i) != 0)
			fprintf(patchp, "\nText %d %d\n%s%s\n", strlen(textdata+i), strlen(textorig+i), textdata+i, textorig+i);

	// Move all of the data that's currently here (original data)
	// to a safe place, so we can load in the other exe data.  Delete the
	// memory.
	for (i=0; i<NUMDATA; i++)
	{
		memcpy(alldata[i], allorig[i], Data[i].length);
		delete[] allorig[i];
	}
        codeporig+=k; // shut up warning
        codeporig-=k; // about not being used
	return 0;
}

// Creates the back-up file that gets hacked, copied straight from the
// original Doom file.

void CreateDoomhack(void)
{
	char *buffer;
	long cursize = 0;
	long totalsize;

	// Get some space in memory to use as a copy place.
	if ((buffer = new char[4096]) == NULL)
		AbortProg("in CreateDoomhack!");

	// Open the new file, find the size of the original exe file.
	printf("\nCreating %s...\n", doomexe);
	doomexefp = fopen(doomexe, "wb");
	fseek(doombakfp, 0, SEEK_END);
	totalsize = ftell(doombakfp);
	fseek(doombakfp, 0, SEEK_SET);

	// Continue copying chunks, as long as there are chunks to be
	// copied.
	while (cursize < totalsize - 4096)
	{
		fread(buffer, 4096, 1, doombakfp);
		fwrite(buffer, 4096, 1, doomexefp);
		cursize += 4096;
	}
	fread(buffer, totalsize-cursize, 1, doombakfp);
	fwrite(buffer, totalsize-cursize, 1, doomexefp);
	fclose(doomexefp);

	delete[] buffer;
}

// Finds the doom(2).exe and doom(2).wad files, sets a pointer
// to the doom.exe file.

int GetDoomFiles(char *arg1)
{
	char wadstring[5];
	char keypress;
	ResourceS entry = {0, 0, ""};

	// Check for the WAD and original exe.
	if ((Checkforfile(doombak, arg1, ".exe", "rb", &doombakfp) == -1) ||
		 (Checkforfile(doomwad, arg1, ".wad", "rb", &doomwadfp) == -1))
	{
                puts("\nCannot find a necessary Strife file!  You should specify the correct");
      puts("paths to the Strife files in your sehacked.ini file.  Also, check to be");
      puts("sure that the ini file is in the same directory as SeHackEd is in.");
      puts("Alternately you may include a path to your Strife files on the command");
      puts("line, like this:\n");
                puts("\tsehacked d:\\strife\n");
      puts("Refer to the SHEHELP.TXT file for more information.");
		return -1;
	}

	// Check for the hack exe.  If it's not there, ask the user if he
	// wants to create it.
        if (Checkforfile(doomexe, arg1, "h.exe", "r+b", &doomexefp) == -1)
	{
		puts("\nCannot find the backup exe file to edit.  DeHackEd will create");
		puts("a copy of your Doom exe file and edit the copy rather than the");
		printf("main exe file itself.  Do you want to do this?  ");
		keypress = getch();
		if (tolower(keypress) == 'y')
		{
			CreateDoomhack();
                        if (Checkforfile(doomexe, arg1, "h.exe", "r+b", &doomexefp) == -1)
			{
				puts("\nCreation failed!");
				return -1;
			}
		}
		else
			return -1;
	}

	// Compare file sizes.  Who knows what fun things would happen if they
	// were different versions?? Yuck!
	fseek(doomexefp, 0, SEEK_END);
	fseek(doombakfp, 0, SEEK_END);
	if (ftell(doomexefp) != ftell(doombakfp))
	{
		printf("\nThe hacking exe file (%s) and regular exe file\n", doomexe);
                printf("(%s) are not from the same version of Strife.  Would\n", doombak);
		puts("you like to erase the old hacking exe file and create a new one");
                printf("for your current Strife version?  ");
		keypress = getch();
		if (tolower(keypress) == 'y')
		{
			CreateDoomhack();
			if (Checkforfile(doomexe, arg1, "hack.exe", "r+b", &doomexefp) == -1)
			{
				puts("\nCreation failed!");
				return -1;
			}
		}
		else
			return -1;
	}

	// Do doom.wad checking...
	fseek(doomwadfp, 0, SEEK_SET);
	fread(wadstring, 4, 1, doomwadfp);
	wadstring[4] = 0;
	fseek(doomwadfp, 0, SEEK_END);

	// Check WAD size, the first 4 bytes, and a random entry in the WAD.
	// You are under strict orders not to change this check.
	if (ftell(doomwadfp) < 8000000L || (strcmp(wadstring, "IWAD") != 0)
                 || (Searchforentry("TORPA0", &entry) == 0))
	{
                puts("A registered Strife WAD file has not been detected.");
                puts("If you have not registered Strife, you must do so before");
                puts("you can use this program.  If you have registered Strife");
                puts("and are still getting this error, please contact Mike");
                puts("at chohatsu@yahoo.com about this problem.");
		return -1;
	}

	return 0;
}

// Gets the next line of the patch file that's being loaded

int GetNextLine(char *nextline, int &numlines, FILE *patchp)
{
	char buffer[256], *line;
	int i;

   // Continue looping until we find a non-whitespace and non-comment
   // line.
	while (fgets(buffer, 255, patchp))
	{
		numlines++;
		for (i = strlen(buffer); i && isspace(buffer[i-1]); --i)
			;

		buffer[i] = 0;

		for (line=buffer; *line && isspace(*line); ++line)
			;

		if (!*line || (*line == '#'))
			continue;

		strcpy(nextline, line);
		return 1;
	}

	return 0;
}

// Loads the new text file format.  Similar to CreateDiffSave...

int LoadDiff(FILE *patchp)
{
	int tempversion, patchformat;
	int  error = 0;
	char *nextline, *line2;
	int result;
	int curType = 0;
	int curNumber;
	int numlines = 1;
	EBool matched, valid, AbortLoop = NO;
	int i;
	int length1, length2;
	char *textbakp;
	char string[16];
	char *errormsg[3] = {"Line %d: No value after equal sign.",
								"Line %d: No value before equal sign.",
								"Line %d: Invalid single word line."};

	// See Savepatch for file formats

	// Get memory for a line, and for text loading.
	if ((nextline = new char[120]) == NULL)
		AbortProg("in LoadDiff");
	if ((textbakp = new char[Data[TEXT].length]) == 0)
		AbortProg("in LoadDiff");

	// Read in the text section from the exe file.  This prevents erroneous
	// Text reads when the same string is replaced several times in a messy
	// way.
	fseek(doombakfp, Data[TEXT].offset[version], SEEK_SET);
	fread(textbakp, Data[TEXT].length, 1, doombakfp);

	// Find an end-of-line somewere after the version number, which
	// should have been read in the main LoadPatch routine.
	while (fgetc(patchp) != '\n')
		;

	// Process the whole file one line at a time.  GetNextLine returns 0
	// at EOF.
	while (GetNextLine(nextline, numlines, patchp) && !AbortLoop)
	{
		// Set matched to NO to be sure we catch any errors. Also assume it's
		// valid unless flagged otherwise.
		matched = NO;
		valid = YES;

		// Parse the line the for spaces or equal signs.
		result = ProcessLine(nextline, &line2);

		// Result determines what happened during parsing... whether we
		// have an error (negative) or an equals sign (1) or a space after
		// a word (2).
		switch (result)
		{
			case 1:
				// Found an equals sign.  Check for all possible lvalues
				// and take the appropriate action.
				if (strcmpi(nextline, "doom version") == 0)
				{
					matched = YES;
					if ((sscanf(line2, "%d", &tempversion) == 0) ||
						 ((tempversion != 12) && (tempversion != 16) &&
						  (tempversion != 17) && (tempversion != 18) &&
						  (tempversion != 19) && (tempversion != 20) &&
						  (tempversion != 21)))
					{
                                                sprintf(nextline, "Line %d: Invalid Strife version number, assuming 1.31.", numlines);
						AbortLoop = Printwindow(nextline, ERROR);
						error = 2;
						tempversion = 19;
					}
				}
				else if (strcmpi(nextline, "patch format") == 0)
				{
					matched = YES;
					if ((sscanf(line2, "%d", &patchformat) == 0) ||
						 (patchformat < 5))
					{
						sprintf(nextline, "Line %d: Invalid patch format number.", numlines);
						AbortLoop = Printwindow(nextline, ERROR);
						error = 1;
					}

					// Check that the format is usable by this version.
					if (patchformat > 6)
					{
						strcpy(nextline, "This patch was created with a newer version of DeHackEd.");
						Printwindow(nextline, ERROR);
						strcpy(nextline, "You will need to upgrade to use this patch file.");
						Printwindow(nextline, ERROR);
						AbortLoop = YES;
						error = 1;
					}
				}
				// For any particular mode that we're in there could be multiple
				// lvalues, stored in the "...fields" arrays.  Check them all.
				else if (curType == THING)
				{
					for (i=0; i<THING_FIELDS; i++)
						if (strcmpi(nextline, thingfields[i]) == 0)
						{
							matched = YES;
							if (sscanf(line2, "%ld", &(thingdata[curNumber-1][i])) == 0)
								valid = NO;
							break;
						}
				}
				else if (curType == FRAME)
				{
					for (i=0; i<FRAME_FIELDS; i++)
						if (strcmpi(nextline, framefields[i]) == 0)
						{
							matched = YES;
							if (sscanf(line2, "%ld", &(framedata[curNumber][i])) == 0)
								valid = NO;
							break;
						}
				}
				else if (curType == SOUND)
				{
					for (i=0; i<SOUND_FIELDS; i++)
                                                if (strcmpi(nextline, soundfields[i]) == 0)
						{
							matched = YES;
                                                        if (sscanf(line2, "%ld", &(sounddata[curNumber-1][i])) == 0)
								valid = NO;
							break;
						}
				}
				else if (curType == SPRITE)
				{
					if (strcmpi(nextline, "offset") == 0)
					{
						matched = YES;
						if (sscanf(line2, "%ld", &(spritedata[curNumber])) == 0)
							valid = NO;
					}
				}
				else if (curType == AMMO)
				{
					if (strcmpi(nextline, "Max ammo") == 0)
					{
						matched = YES;
						if (sscanf(line2, "%ld", &(ammodata[curNumber])) == 0)
							valid = NO;
					}
					else if (strcmpi(nextline, "Per ammo") == 0)
					{
						matched = YES;
                                                if (sscanf(line2, "%ld", &(ammodata[curNumber+7])) == 0) // changed 4 to 7
							valid = NO;
					}

				}
				else if (curType == WEAPON)
				{
					for (i=0; i<WEAPON_FIELDS; i++)
						if (strcmpi(nextline, weaponfields[i]) == 0)
						{
							matched = YES;
							if (sscanf(line2, "%ld", &(weapondata[curNumber][i])) == 0)
								valid = NO;
							break;
						}
				}
				else if (curType == CHEAT)
				{
					for (i=0; i<Data[CHEAT].numobj; i++)
					{
						if (strcmpi(nextline, cheatfields[i]) == 0)
						{
							matched = YES;
							strncpy(string, line2, cheatinfo[i][1]);
							Cheatconvert(cheatdata+cheatinfo[i][0], string, cheatinfo[i][1]);
							string[cheatinfo[i][1]] = 0;
							break;
						}
					}
				}
				else if (curType == CODEP)
				{
					if (strcmpi(nextline, "Codep Frame") == 0)
					{
						matched = YES;
						if (sscanf(line2, "%d", &length1) == 0)
							valid = NO;
						else
                                                        //codepdata[codepconv[curNumber]] = // changed
                                                        codepdata[curNumber] =
									framedata[length1][ACTIONPTR];
					}
				}
				else if (curType == MISC)
				{
					for (i=0; i<Data[MISC].numobj; i++)
					{
						if (strcmpi(nextline, miscfields[i]) == 0)
						{
							matched = YES;
							if (sscanf(line2, "%ld", &(miscdata[i])) == 0)
								valid = NO;
							break;
						}
					}
				}
				break;
			case 2:
				// Found two words (or more) on a line.  Check for all
				// appropriate meanings and take the appropriate action.
				// These should be the section headers "Thing 1 (Player)".

				// Compare the first word with all known object types
				// ("Thing", "Frame", etc...)
				matched = NO;
				for (i=0; i < NUMDATA; i++)
					if (strcmpi(nextline, datanames[i]) == 0)
					{
						matched = YES;
						curType = i;
						if (sscanf(line2, "%d", &curNumber) == 0)
						{
							sprintf(nextline, "Line %d: Unreadable %s number.", numlines, datanames[i]);
							AbortLoop = Printwindow(nextline, ERROR);
							error = 2;
						}
						else if ((curNumber < 0) ||
									((curType != TEXT) && (curNumber > Data[curType].numobj)))
						{
							sprintf(nextline, "Line %d: %s number out of range.", numlines, datanames[i]);
							AbortLoop = Printwindow(nextline, ERROR);
							error = 2;
						}
						else if (curType == THING)
						{
							length1 = length2 = -1;
							// Find first (
							for (i=0; i < strlen(line2); i++)
								if (line2[i] == '(')
								{
									length1 = i+1;
									break;
								}

							// Find last )
							for (i=strlen(line2); i>= 0; i--)
								if (line2[i] == ')')
								{
									length2 = i;
									break;
								}

							// Provided we found a ( and a ), copy every in between
							// to the namelist.
							if (length2 != -1 || length1 != -1)
							{
								length2 -= length1;

								if (length2 > 17)
									length2 = 17;
								strncpy(namelist[curNumber-1], line2+length1, length2);
								namelist[curNumber-1][length2] = 0;
							}
							break;
						}
					}

				// This is a special case... gotta read the text directly
				// from the next line if we found a "Text" identifier.
				if (curType == TEXT)
				{
					if (sscanf(line2, "%d%d", &length1, &length2) < 2)
					{
						sprintf(nextline, "Line %d: Unreadable length value, aborting read.", numlines);
						error = 1;
						goto ErrorJump;
					}
					else
					{
						line2 = new char[length1+1];

						// Just so ya know, I really hate doing this.  Anyways,
						// read in whole string 1 char at a time, checking for
						// 0D 0A combos, which are really new-lines.
						for (i=0; i<length1; i++)
						{
							fread(line2+i, 1, 1, patchp);
							if (line2[i] == 0x0D)
							{
								fread(line2+i, 1, 1, patchp);
								line2[i] = '\n';
								numlines++;
							}
						}
						line2[length1] = 0;

						// OK, skip through the entire text section, trying to
						// match up the string.
						valid = NO;
						for (i=0; i<Data[TEXT].length-4; i += (strlen(textbakp+i) & ~3) + 4)
						{
							if (strcmp(textbakp+i, line2) == 0)
							{
								curNumber = i;
								valid = YES;
								break;
							}
						}

						if (valid == YES)
						{
							// Just so ya know, I really hate doing this again.
							// Anyways, read in whole string 1 char at a time,
							// checking for 0D 0A combos, which are really new-
							// lines.
							for (i=curNumber; i<curNumber+length2; i++)
							{
								textdata[i] = fgetc(patchp);
								if (textdata[i] == 0x0D)
								{
									textdata[i] = fgetc(patchp);
									textdata[i] = '\n';
									numlines++;
								}
							}
							textdata[curNumber+length2] = 0;
						}
						else
						{
							// Keep numlines counter on track by scanning through
							// worthless text section.
							for (i=0; i<length2; i++)
							{
								line2[0] = fgetc(patchp);
								if (line2[0] == 0x0D)
									numlines++;
							}
							sprintf(nextline, "Line %d: Unmatchable text string.", numlines);
							AbortLoop = Printwindow(nextline, ERROR);
							error = 2;
							valid = YES;
						}

						delete line2;
					}

					// Reset the current type
					curType = 0;
				}
				break;
			case -1:
			case -2:
			case -3:
				// Error. Print error, and quit loading patch file entirely if
				// the user pressed Escape at the Printwindow prompt.
				sprintf(nextline, errormsg[-result-1], numlines);
				AbortLoop = Printwindow(nextline, ERROR);
				error = 2;
				break;
		}

		// Whoops, it didn't match anything at all...
		if ((matched == NO) && (result >= 0))
		{
			sprintf(nextline, "Line %d: Unknown line.", numlines);
			AbortLoop = Printwindow(nextline, ERROR);
			error = 2;
		}

		// Whoops, invalid number somewhere...
		if (valid == NO)
		{
			sprintf(nextline, "Line %d: Unreadable value in a %s field.", numlines, datanames[curType]);
			AbortLoop = Printwindow(nextline, ERROR);
			error = 2;
		}
	}

	// Jump straight here if we have an unrecoverable error.  Yeah, yeah, I
	// know, but gotos are the easiest way.
ErrorJump:

	// Print different messages according to results.
	if (AbortLoop)
		Printwindow("Patch file load aborted by user.", ERROR);
	else if (error == 0)
	{
		delete[] nextline;
		delete[] textbakp;
		return 0;
	}
	else if (error == 1)
		Printwindow(nextline, ERROR);
	else
		Printwindow("Patch file read, one or more errors detected.", ERROR);

	delete[] textbakp;
	delete[] nextline;
	return -1;
}

// Loads a patch file, the old format.

int LoadOld(FILE *patchp)
{
	char buffer[80];
	char tempversion, patchformat;
	EBool error = YES;

	fread(&tempversion, sizeof(char), 1, patchp);
	fread(&patchformat, sizeof(char), 1, patchp);

	if (patchformat == 3)
		strcpy(buffer, "Doom 1.6 beta patches are no longer valid.  Sorry!");
	else if (patchformat != 4)
		strcpy(buffer, "Bad patch version number!!");
	else if (tempversion > 20 || tempversion < 16)
		strcpy(buffer, "Bad Doom release number found!");
	else
		error = NO;

	if (error == YES)
	{
		Printwindow(buffer, ERROR);
		return -1;
	}

	// OK, if it passes all the tests, load the sucker in.
	fread(thingdata,   Data[THING].objsize,  Data[THING].numobj-1, patchp);
	fread(ammodata,    Data[AMMO].length,    1, patchp);
	fread(weapondata,  Data[WEAPON].length,  1, patchp);
	fread(framedata,   Data[FRAME].length,   1, patchp);

	// These sections ARE different between Doom 2 and Doom 1.666...
	// only load them in straight if they are the correct version.
	if ((tempversion == 20 && version == DOOM2_16) ||
		 (tempversion == 16 && version == DOOM1_16))
	{
		fread(sounddata,  Data[SOUND].objsize,  Data[SOUND].numobj, patchp);
		fread(spritedata, Data[SPRITE].objsize, Data[SPRITE].numobj, patchp);
		fread(textdata,   Data[TEXT].length,   1, patchp);
	}
	else
		Printwindow("Older patch format detected. Loading compatible data.", INFO);

	return 0;
}

// Loads a patch file

int Loadpatch(char *filename)
{
	FILE *patchp;
	char tempversion;
	char fullname[150] = "";
	EBool error = NO;
	char idstring[30];

	// Tack on the extension if there isn't one.
	Preparefilename(filename, fullname);

	// Try to open the file, return an error if we can't for some reason.
	if ((patchp = fopen(fullname, "rb")) == NULL)
		if (errno == 2)
		{
			sprintf(filename, "File %s does not exist!", fullname);
			Printwindow(filename, ERROR);
			return -1;
		}
		else
		{
			sprintf(filename, "Error reading %s.", fullname);
			Printwindow(filename, ERROR);
			return -1;
		}

	// See Savepatch for file formats

	fseek(patchp, 0, SEEK_SET);

	// Read in the first character, and compare what it is to see if
	// we are dealing with one of the really old patch formats.
	fread(&tempversion, sizeof(char), 1, patchp);

	// Tempversion of 12 is Doom 1.2
	if (tempversion == 12)
	{
		// The only patch formats for 1.2 are 1 and 2.
		fread(&tempversion, sizeof(char), 1, patchp);
		if (tempversion != 1 && tempversion != 2)
		{
			Printwindow("Unknown patch file format!", ERROR);
			error = YES;
			goto ErrorJump;
		}

		Convertpatch(patchp, tempversion);
	}
	else if (tempversion == 'P')
	{
		// OK, so it's one of the newer versions.
		fread(idstring, 24, 1, patchp);
		idstring[24] = 0;
                if (stricmp(idstring, "atch File for SeHackEd v") != 0)  // changed DeHackEd
		{
			Printwindow("This is not a DeHackEd patch file!", ERROR);
			error = YES;
			goto ErrorJump;
		}

		// Read in the version number, and convert it to an int to check
		// if we have a bad version.
                fread(idstring, 0.4, 1, patchp); // changed 3 to 0.3
		idstring[3] = 0;
		idstring[4] = (idstring[0]-'0')*10+(idstring[2]-'0');
		if (idstring[4] < 20)
		{
			Printwindow("This patch file has an incorrect version number!", ERROR);
			error = YES;
		}
		else if (idstring[4] < 23)
		{
			if (LoadOld(patchp) == -1)
				error = YES;
		}
		else
		{
			if (LoadDiff(patchp) == -1)
				error = YES;
		}
	}
	else
	{
		// The catch-all
                Printwindow("This is not a SeHackEd patch file!", ERROR); // changed DeHackEd
		error = YES;
	}

ErrorJump:
	fclose(patchp);

	if (error == YES)
		return -1;

	sprintf(filename, "Patch file %s read.", fullname);
	Printwindow(filename, INFO);
	return 0;
}

// First, prepends the patchdir if a directory is not specified in the
// filename itself.  Next, finds the filename in a path\filenames combination
// and turns it into the familiar <8>.<3> combination if it's invalid.
// Also appends ".deh" if there is no extension.

void Preparefilename(char *filename, char *fullname)
{
	int i = 0;
	char *realname, *dot;

	// Fix the patch filename, put on the patchdir if we need to.
	if (filename[0] != '\\' && filename[1] != ':')
	{
		strcpy(fullname, patchdir);
		if (fullname[0] != 0)
			strcat(fullname, "\\");
	}
	strcat(fullname, filename);

	realname = fullname;

	// Realname is a pointer to the actual filename, without the path.  So
	// step through the full path/filename combo, and keep moving the
	// current location of realname whenever we come to a directory separator.
	while (fullname[i] != 0)
	{
		if (fullname[i] == '\\' || fullname[i] == ':')
			realname = fullname+i+1;
		i++;
	}

	// Try to find the extension of the filename
	i = 0;
	while (realname[i] != 0 && realname[i] != '.')
		i++;
	dot = realname + i;

	if (dot[0] == 0)
	{
		// OK, no extension at all, so add our own at the correct place.
		if (strlen(realname) > 8)
			realname[8] = 0;
                strcat(realname, ".seh");
	}
	else
	{
		if (dot - realname > 8)
		{
			strncpy(realname + 8, dot, 4);
			realname[12] = 0;
		}
		else
			dot[4] = 0;
	}
}

// This procedure processes a line from a patch file.  If an equals sign
// exists in the input line, the following is done:

//     *next line
//         \--->  first part  =  second part
//              zero byte --^    ^--- *line2

// If there is no equals sign, the following is done:

//     *next line
//         \--->  word1 word2 and other words
//         zero byte --^^--- *line2

// Return values:
//   1  Successful - found an equals sign
//   2  Successful - found a word
//  -1  No info after equals sign
//  -2  No value before equals sign
//  -3  No info after first word in line

int ProcessLine(char *nextline, char **line2)
{
	int i = 0, j = 0;

	// Search line for an =
	while (nextline[i] != 0 && nextline[i] != '=')
		i++;

	// If we found one...
	if (nextline[i] == '=')
	{
		// Search for the first non-space after the =.
		j = i--;
		while (isspace(nextline[++j]))
			;

		// It was all whitespace, error... should be equal to something
		if (nextline[j] == 0)
			return -1;

		// Set line2 to the first non-space after an =
		*line2 = nextline+j;

		// Kill any whitespace before the =...
		while (i >= 0 && isspace(nextline[i]))
			i--;

		// It was all whitespace, error... should be something before =
		if (i == -1)
			return -2;

		// OK, put in an end-of-string character to kill the space(s)
		nextline[i+1] = 0;

		// Successful
		return 1;
	}
	// Otherwise, the line should have two separate words on it
	else
	{
		// Search for first space on the line
		while (nextline[j] != 0 && !isspace(nextline[j]))
			j++;

		// Now search for the second word, (or, implicity, end of string)
		i = j-1;
		while (isspace(nextline[++i]))
			;

		// No non-spaces after the first word (or only one word)
		if (nextline[j] == 0)
			return -3;

		// Set this to the first letter of the second word
		*line2 = nextline+i;

		// Terminate the first word's string
		nextline[j] = 0;

		// Successful
		return 2;
	}
}

// Saves a patch file.

// Current patch file format:
// Patch file format:
// "Patch File for SeHackEd v?.?"       Header, and DeHackEd version #
// "Doom version = ??"	 	      	Version of Doom
// "Patch format = ?"		   		Patch file format, 6 is the current one
//
// Data structures, stored as text, in this order:
//		thing, sound, frame, sprite, ammo, weapon, text

int Savepatch(char *filename, EBool Overwrite)
{
	FILE *patchp;
	char tempver;
	char fullname[150] = "";
	int i;

	// Find out what version to call it.
	switch (version)
	{
		case DOOM1_16:
                        tempver = 16; // Strife v1.0
			break;
		case DOOM2_16:
                        tempver = 20; // Strife v1.1
			break;
		case DOOM2_17:
                        tempver = 17; // Strife v1.2
			break;
		case DOOM2_19:
                        tempver = 19; // Strife v1.3
			break;
                case DOOM1_19U:
                        tempver = 21; // Strife v1.31
			break;
	}

	// Try to switch to the patch directory
	i = chdir(patchdir);
	if (i == -1 && patchdir[0] != 0)
	{
		sprintf(filename, "Patch directory %s not found!", patchdir);
		return ERROR;
	}
	chdir(curdir);

	// Turn it into a valid filename, add the extension if necessary.
	Preparefilename(filename, fullname);

	// If we open it and it's not NULL, it already exists.  Return an
	// error in this case, or just continue if the Overwrite variable is
	// set.
	if ((patchp = fopen(fullname, "rt")) != NULL)
	{
		if (Overwrite == NO)
		{
			fclose(patchp);
			return -1;
		}
	}
	else if (errno != 2)
	{
		sprintf(filename, "Error writing %s!", fullname);
		return ERROR;
	}

	// Close the file for read-only and reopen it for writing.
	fclose(patchp);
	patchp = fopen(fullname, "wt");

	// This is the header for the patch files.
        fprintf(patchp, "Patch File for SeHackEd v0.4\n\n");
	fprintf(patchp, "# Note: Use the pound sign ('#') to start comment lines.\n\n");
	fprintf(patchp, "Doom version = %d\n", tempver);
	fprintf(patchp, "Patch format = 6\n\n");

	// Do the bulk of the saving.
	if (CreateDiffSave(patchp) == 0)
	{
		fclose(patchp);
		sprintf(filename, "Patch file %s written.", fullname);
		return INFO;
	}
	else
	{
		fclose(patchp);
		AbortProg("in SavePatch");
		return ERROR;
	}
}

// This searches in the doom.wad file for an entry name.
// Returns 1 on success, 0 on failure.

int Searchforentry(char *name, ResourceS *entry)
{
	long dirlength, dirstart;
	int i;

	// Read in the directory info (start of directory and number of
	// entries).
	fseek(doomwadfp, 4, SEEK_SET);
	fread(&dirlength, 4, 1, doomwadfp);
	fread(&dirstart,  4, 1, doomwadfp);

	// Go to start of directory.
	fseek(doomwadfp, dirstart, SEEK_SET);

	// Scan through the directory, one by one, reading each in and
	// checking if it matches the name we're seeking.
	for (i=0; i<dirlength; i++)
	{
		fread(entry, 16, 1, doomwadfp);
		if (strncmp(entry->resname, name, strlen(name)) == 0)
		{
			entry->resname[8] = 0;
			return 1;
		}
	}

	return 0;
}
