// DeHackEd version 3.0a
// Written by Greg Lewis, gregl@umich.edu
// If you release any versions of this code, please include
// the author in the credits.  Give credit where credit is due!

// Function prototypes for playview.cpp
int  Displaylogo(void);
int  Drawlump(ResourceS entry);
int  Play(ResourceS entry);
int  Playsound(long soundnum);
int  Showframe(long framenum);

// From dheinit.h
extern FILE *doomwadfp;
extern long (*framedata )[FRAME_FIELDS];
extern struct sb_conf dev;

// From dehacked.cpp
extern void AbortProg(char *func);

// From screen.h
extern char *pscreen;

// From screen.cpp
extern void clsMCGA(void);
extern void DisplayPic( int x0, int y0, int x1, int y1, char *picname);
extern void MCGAPutsXY (int x, int y, char *string, unsigned char col);
extern void setpalette(unsigned char palette[768]);
extern void toMCGA(void);
extern void totext(void);

// From print.cpp
extern void Printwindow(char *message, int type);

// From misc.cpp
extern void Getframename(int framenum, char *string);
extern void Getsoundname(int soundnum, char *string);

// From files.cpp
extern int Searchforentry(char *name, ResourceS *entry);

// From sb_drive.c
extern int dsp_open (struct sb_conf *dev);
extern int dsp_write (char *ptr, int len);
extern int dsp_close (void);
extern void sb_unattach (void);

// From sb_regs.c
extern int SB_DMA_CHAN;

// From input.cpp
extern EBool Waitforevent(EBool eatevent);

// From mouse.cpp
extern void  hideMouseCursor(void);
extern void  showMouseCursor(void);
extern EBool getLastEvent(unsigned &x, unsigned &y, EButton &left_button,
                                  EButton &right_button);

// Variables from mouse.h
extern unsigned eventButtons;
extern unsigned LastEventButtons;
