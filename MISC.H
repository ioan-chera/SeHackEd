// SeHackEd version 0.22pb
// Modified by Mike Fredericks, chohatsu@yahoo.com
// Thanks to Janis Legzdinsh for info, http://www.vavoom-engine.com
// based on DeHackEd version 3.0a
// Written by Greg Lewis, gregl@umich.edu
// If you release any versions of this code, please include
// the author in the credits.  Give credit where credit is due!

// Function prototypes for misc.cpp
void  Changemode(EModes newmode);
void  Cheatconvert(char *string, char *cheat, int length);
EBool ConfirmQuit(void);
int   Copydata(void);
void  Getframename(int framenum, char *string);
int   Getinput(int x, int y, char *string, int maxlen, int type);
void  Getsoundname(int soundnum, char *string);
void  Getspritename(int spritenum, char *string);
int   Gettextnum(int offset);
void  GoPageDown(void);
void  GoPageUp(void);
int   GotoObject(int firstdigit);
void  Highlight(unsigned char attribute);
void  Highlightname(char *name);
void  Jump(void);
void  Listkeyinput(char key, int &topnum, int &curnum, int maxnum, int listsize);
void  Listmouseinput(int mousex, int mousey, int arrowx, int arrowy1,
                            int arrowy2, int &topnum, int &curnum, int maxnum,
                            int listsize);
void  Loadpatchname(void);
void  Savepatchname(void);
int   Textsearch(void);

// Data from dheinit.h
extern EVersions version;          // The Doom version we're using
extern ERedraw redraw;                // Current redraw state
extern EModes  mode;                    //    The mode we start in
extern EBool   changes;                // If changes have been made or not
extern ScreenSet *Modeptr;            // Pointer to Data for the current mode
extern EBool   askatload;            // Ask about reloading data during load
extern FILE *doombakfp;                // File pointer to doom backup exe

extern long (*thingdata )[THING_FIELDS];
extern long (*framedata )[FRAME_FIELDS];
extern long (*weapondata)[WEAPON_FIELDS];
extern long (*sounddata )[SOUND_FIELDS];
extern long *spritedata;
extern char *textdata;
extern long *codepdata;

// All data on objects
extern DataSet Data[NUMDATA];
extern ScreenSet Screen[NUMSCREENS];

// Other misc info needed
extern char thingorder[THING_FIELDS];

// From dehacked.cpp
extern void Loaddoom(FILE *exefp);

// Data from print.h
extern long toff[NUMVERS];
extern char namelist[345][18];  // changed 138 to 345

// Functions from print.cpp
extern EBool Printwindow(char *message, int type);
extern int   Printinputwindow(char *buffer, char *prompt, EInptype type,
                                        int length, char firstchar = 0);

// From files.cpp
extern int Loadpatch(char *filename);
extern int Savepatch(char *filename, EBool Overwrite);

// From screen.h
extern char *screenbptr;

// From input.h
extern char posinfo[NUMSCREENS][3];
extern char Fielddata[][5];

// From input.cpp
extern int Loadfilelist(char *filename);

// From mouse.h
extern unsigned eventButtons;
extern unsigned LastEventButtons;
extern EBool eventHappened;    // these vars to use getLastEvent
extern long lclicktime;
