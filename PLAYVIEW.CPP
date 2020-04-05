// DeHackEd version 3.0a
// Written by Greg Lewis, gregl@umich.edu
// If you release any versions of this code, please include
// the author in the credits.  Give credit where credit is due!

#include <conio.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "dehacked.h"
#include "playview.h"

// Attempt to show title logo.  It is a pared down PCX format reader.

int Displaylogo(void)
{
    FILE *picfile;
    long i;
    unsigned char data, *palette;
    int numbytes;

    palette = new unsigned char[768];
    if (!palette)
        return -1;

    if ((picfile = fopen("shelogo.pcx", "rb")) == 0)
        return -1;

    toMCGA();

    // Go to palette location in file
    fseek(picfile, -768, SEEK_END);
    fread(palette, 768, 1, picfile);

    // Convert to 6-bit color values and set the palette
    for (i=0; i<768; i++)
        palette[i] /= 4;
    setpalette(palette);

    // Go to the start of the pic data
    fseek(picfile, 128, SEEK_SET);

    i = 0;
    while (i <= 64000L)
    {
        data = getc(picfile);

        // RLE data
        if (data >= 192)
        {
            numbytes = data - 192;
            data = getc(picfile);

            while (numbytes-- > 0)
                pscreen[i++] = data;
        }
        else
            pscreen[i++] = data;
    }

    // Wait for user keypress
    Waitforevent(YES);

    totext();

    fclose(picfile);
    delete palette;

    return 0;
}

// Actually draw a frame on the screen

int Drawlump(ResourceS entry, EBool flip)
{
    int width, height, xoff, yoff;
    long *picoff;
    int i, j;
    unsigned char numpixels;
    unsigned char *pixinfo;
    int curoff = 0;
    unsigned int curx, cury;
    int step;

    // Get the initial info
    fseek(doomwadfp, entry.resstart, SEEK_SET);
    fread(&width, 1, 2, doomwadfp);
    fread(&height, 1, 2, doomwadfp);
    fread(&xoff, 1, 2, doomwadfp);
    fread(&yoff, 1, 2, doomwadfp);

    // Set the x and y offsets if we're dealing with a weapon picture,
    // which has a negative offset.
    if (xoff < 0)
        xoff = width/2;
    if (yoff < 0)
        yoff = height/2;

    // Figure in the y-offset multiply here, rather than later.
    yoff = (90 - (yoff)/2) * 320;

    // If we have to flip the image along the y axis, set the x variable
    // to go left instead of right, and start it on the right side of the
    // image.
    if (flip)
    {
        step = -1;
        curx = 160 + xoff;
    }
    else
    {
        step = 1;
        curx = 160 - xoff;
    }

    // Get memory for image
    picoff = new long[width];
    pixinfo = new unsigned char[entry.reslength];

    if (picoff == NULL || pixinfo == NULL)
        return -1;

    // Get the column info
    fread(picoff, width, 4, doomwadfp);

    // Read in the rest of the image stuff
    fseek(doomwadfp, entry.resstart+picoff[0], SEEK_SET);
    fread(pixinfo, entry.reslength-picoff[0], 1, doomwadfp);

    // Loop through each column
    for (i=0; i<width; i++, curx += step)
    {
        curoff = picoff[i]-picoff[0];

        if (curx >= 320)
            continue;

        while (i < width-1 && (curoff < picoff[i+1]-picoff[0]-1))
        {
            cury = yoff + 320*pixinfo[curoff];
            numpixels = pixinfo[curoff+1];
            curoff += 3;

            for (j=0; j<numpixels; j++, cury+=320, curoff++)
                if (cury < 64000L)
                    pscreen[cury + curx] = pixinfo[curoff];
            curoff++;
        }
    }

    // Free memory
    delete[] picoff;
    delete[] pixinfo;
    return 0;
}

// Plays the actual sound (maybe)

int Play(ResourceS entry)
{
    char *soundp;
    long junk;
    unsigned int samples;

    if (SB_DMA_CHAN > 3)
    {
        Printwindow("DMA channel is not valid!", ERROR);
        return -1;
    }

    if (dsp_open(&dev) == 0)
    {
        Printwindow("Sound Blaster not found!", ERROR);
        return -1;
    }

    fseek(doomwadfp, entry.resstart, SEEK_SET);
    fread(&junk, 4, 1, doomwadfp);
    fread(&samples, 2, 1, doomwadfp);

    soundp = new char[samples];

    if (soundp == NULL)
        return -1;

    fread(soundp, samples, 1, doomwadfp);

    dsp_write(soundp, samples);
    dsp_close();
    sb_unattach();

    delete[] soundp;
    return 0;
}

// Plays a sound.

int Playsound(long soundnum)
{
    ResourceS entry = {0, 0, ""};
    char nametemp[7];
    char soundname[9] = "DS";

    Getsoundname(soundnum, nametemp);

    if (strcmp(nametemp, "none") == 0)
        return -1;
    else if (strcmp(nametemp, "ERROR") == 0)
    {
        Printwindow("Not a valid sound number!", ERROR);
        return -1;
    }

    strcat(soundname, strupr(nametemp));

    // Find the correct sound
    if (Searchforentry(soundname, &entry) == 0)
    {
        Printwindow("Could not find sound in the DOOM.WAD file!", ERROR);
        return -1;
    }

    Play(entry);

    return 0;
}

// General driving forces behind showing a frame.

int Showframe(long framenum)
{
    int i, curview = 1;
    ResourceS entry = {0, 0, ""};
    EBool ExitLoop = NO;
    EBool flip, draw = YES;
    char framename[9];
    unsigned char *palette;
    char input;
    unsigned int x, y;        // Mouse x and y
    EButton lbutton, rbutton;// State of left and right buttons
    char order[9] = {0, 5, 8, 7, 6, 1, 4, 3, 2};

    Getframename(framenum, framename);

    if (strcmp(framename, "none") == 0)
        return -1;
    else if (strcmp(framename, "ERROR") == 0)
    {
        Printwindow("Not a valid frame number!", ERROR);
        return -1;
    }

    palette = new unsigned char[768];

    if (palette == NULL)
        AbortProg("in Showframe");

    // Find the Doom palette
    if (Searchforentry("PLAYPAL", &entry) == 0)
    {
        Printwindow("Could not find the default palette!", ERROR);
        delete[] palette;
        return -1;
    }

    // Get the palette setup correctly.
    fseek(doomwadfp, entry.resstart, SEEK_SET);
    fread(palette, 768, 1, doomwadfp);
    for (i=0; i<768; i++)
        palette[i] /= 4;

    // Search for the generic frame ending in '0' first
    framename[5] = '0';
    framename[6] = 0;
    // Check for SHTGA0
    if (Searchforentry(framename, &entry) == 0)
    {
        // Check for BOSSA1
        framename[5] = '1';
        if (Searchforentry(framename, &entry) == 0)
        {
            // Check for special case such as BOSSA5A1
            framename[5] = '5';
            if (Searchforentry(framename, &entry) == 0)
            {
                // Check for special case SPIDA1D1.  Dumb special cases.
                framename[8] = 0;
                framename[7] = '1';
                framename[6] = framename[4];
                framename[5] = '1';
                framename[4] = framename[4] - 3;
                if (Searchforentry(framename, &entry) == 0)
                {
                    Printwindow("Could not find frame in the Doom WAD file!", ERROR);
                    delete[] palette;
                    return -1;
                }
                framename[4] = framename[6];
                framename[6] = 0;
            }
        }
    }

    // Init graphics, my crude way
    toMCGA();
    hideMouseCursor();
    setpalette(palette);

    // While they haven't exited the frame viewer...
    while (ExitLoop == NO)
    {
        // Don't flip the image by default
        flip = NO;

        // Get the frame name of the current frame to draw and find that
        // entry in the WAD file.
        // If the sixth character is '0', we'll ASSUME it's been read into
        // entry already, and is ready to display.
        if (framename[5] != '0')
        {
            framename[5] = '0' + curview;
            if (Searchforentry(framename, &entry) == 0)
            {
                framename[5] = '0' + order[curview];
                if (Searchforentry(framename, &entry) == 0)
                {
                    // Hit the special SPID case again
                    framename[8] = 0;
                    framename[7] = '0' + curview;
                    framename[6] = framename[4];
                    framename[5] = '0' + curview;
                    framename[4] = framename[4] - 3;
                    if (Searchforentry(framename, &entry) == 0)
                    {
                        ExitLoop = YES;
                        break;
                    }
                    framename[4] = framename[6];
                    framename[6] = 0;
                }
                else
                    flip = YES;
            }
        }

        // Only redraw if we have to.
        if (draw == YES)
        {
            clsMCGA();
            MCGAPutsXY(0, 0, "Escape: quit", 92);
            MCGAPutsXY(0, 1, "Space: Next Frame", 92);

            if (entry.resname[5] != '0')
                MCGAPutsXY(0, 2, "Left/right: rotate", 92);

            // If drawing the last frame failed for some reason, quit
            if (Drawlump(entry, flip) == -1)
            {
                ExitLoop = YES;
                break;
            }

            draw = NO;
        }

        if (Waitforevent(NO))
        {
            input = getch();

            // If the first input character is 0, it's an extended key
            // (function or arrow, basically).  We only care about arrow
            // keys if the picture can be rotated, so check that too.
            if (!input)
            {
                input = getch();
                if (input == RIGHT && (entry.resname[5] != '0'))
                {
                    if (curview > 1)
                        curview--;
                    else
                        curview = 8;
                    draw = YES;
                }
                else if (input == LEFT && (entry.resname[5] != '0'))
                {
                    if (curview < 8)
                        curview++;
                    else
                        curview = 1;
                    draw = YES;
                }
                input = 0;
            }
        }
        else
        {
            input = 0;
            getLastEvent(x, y, lbutton, rbutton);

            if (rbutton == buttonUp && LastEventButtons & RIGHTBUTTON)
                input = ESC;
            if (lbutton == buttonUp && LastEventButtons & LEFTBUTTON)
                input = ' ';
        }

        switch (tolower(input))
        {
            case ESC:
                ExitLoop = YES;
                break;
            case ' ':
                if (framedata[framenum][NEXTFRAME] == 0)
                    ExitLoop = YES;
                else
                {
                    framenum = framedata[framenum][NEXTFRAME];
                    Getframename(framenum, framename);
                    if (strcmp(framename, "none") == 0 ||
                         strcmp(framename, "ERROR") == 0)
                    {
                        ExitLoop = YES;
                        break;
                    }

                    // Search for the generic frame ending in '0' first
                    framename[5] = '0';
                    framename[6] = 0;
                    if (Searchforentry(framename, &entry) == 0)
                        framename[5] = '1';
                    draw = YES;
                }
                break;
        }
    }

    delete[] palette;

    // Reinit text mode
    showMouseCursor();
    totext();
    textmode(C4350);
    _setcursortype(_NOCURSOR);
    return 0;
}
