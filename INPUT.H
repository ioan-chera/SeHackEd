// SeHackEd version 0.23pb
// Modified by Mike Fredericks, chohatsu@yahoo.com
// Thanks to Janis Legzdinsh for info, http://www.vavoom-engine.com
// based on DeHackEd version 3.0a
// Written by Greg Lewis, gregl@umich.edu
// If you release any versions of this code, please include
// the author in the credits.  Give credit where credit is due!

// Function prototypes for input.cpp
int   GetWindow(int curfield);
int   Loadfilelist(char *filename);
int   Printgowindow(void);
int   Printinputwindow(char *buffer, char *prompt, EInptype type, int length, char firstchar = 0);
EBool ProcessKeypress(void);
EBool ProcessMouse(void);
int   Sortfunction(const void *a, const void *b);
EBool Waitforevent(EBool eatevent);

// Which fields belong to which Thing windows...
int thingwindows[][2] = { 1, 10,     // Misc
                                 11, 15,     // Sound
                                 16, 23,     // Frame
                                 24, 39,     // bit1
                                 40, 55,     // bit2
                                 56, 56}; // Info

// Which fields belong to which Misc windows
int miscwindows[][2] = { 1,  8, // Weapon
                                 9, 25, // Cheat
                                26, 41};    // Player

// Variables from dehacked.cpp
extern ERedraw redraw;                    // Current redraw state
extern EModes  mode;                        //    The mode we start in
extern FILE *doomexefp;                    // File pointer to doom.exe
extern FILE *doomwadfp;               // File pointer to doom.wad
extern FILE *doombakfp;                    // File pointer to doom backup exe
extern EVersions version;       // The Doom version we're using
extern char curdir[MAXDIR+1];            // Directory at start of program
extern EBool changes;                    // If changes have been made or not
extern ScreenSet *Modeptr;                // Pointer to Data for the current mode

extern char doompath[80];                // Directory to run doom from
extern char doomexe[80];                // Doom.exe file name
extern char doomargs[80];                // Arguments when Doom is run
extern char patchdir[80];                // Patch file directory

// Data from doom exe file
extern long (*thingdata )[THING_FIELDS];
extern long (*framedata )[FRAME_FIELDS];
extern long (*weapondata)[WEAPON_FIELDS];

// Data on all data and screens
extern DataSet Data[NUMDATA];
extern ScreenSet Screen[NUMSCREENS];

// Other misc data we need
extern char thingorder[THING_FIELDS];
extern char posinfo[NUMSCREENS][3];
extern char Fielddata[][5];

// Functions from dehacked.cpp
extern void AbortProg(char *func);
extern int  RunExe(void);

// Functions from print.cpp
extern void  Printhelp(void);
extern void  Printintro(void);
extern void  Printtextstring(EBool leavewindow);
extern EBool Printwindow(char *message, int type);

// Functions from misc.cpp
extern void  Changemode(EModes newmode);
extern EBool ConfirmQuit(void);
extern int   Copydata(void);
extern int   Gettextnum(int offset);
extern void  GoPageDown(void);
extern void  GoPageUp(void);
extern int   GotoObject(int firstdigit);
extern void  Highlight(unsigned char attribute);
extern void  Jump(void);
extern void  Listkeyinput(char key, int &topnum, int &curnum, int maxnum,
                                  int listsize);
extern void  Listmouseinput(int mousex, int mousey, int arrowx, int arrowy1,
                                     int arrowy2, int &topnum, int &curnum,
                                     int maxnum, int listsize);
extern void  Loadpatchname(void);
extern void  Savepatchname(void);

// Variables from print.cpp
extern char namelist[345][18];  // changed 138 to 345

// Functions from screen.cpp
extern void CPrintfXY(int x, int y, char *string, ...);
extern void CPutsXY(int x, int y, char *string);
extern void Drawframe(int type, unsigned char attr, int left, int top,
                             int right, int bot);
extern void Getwindow(int x1, int y1, int x2, int y2);
extern void Putwindow(void);

// Functions from files.cpp
extern void Loaddoom(FILE *exefp);
extern void Writedoom(void);

// From playview.cpp
extern void Playsound(long soundnum);
extern int  Showframe(long framenum);

// Functions from mouse.cpp
extern void  hideMouseCursor(void);
extern void  showMouseCursor(void);
extern EBool getLastEvent(unsigned &x, unsigned &y, EButton &left_button,
                                  EButton &right_button);

// Variables from mouse.h
extern unsigned eventButtons;
extern unsigned LastEventButtons;
extern EBool eventHappened;    // these vars to use getLastEvent
extern long lclicktime;
