// SeHackEd version 0.4
// Modified by Mike Fredericks, chohatsu@yahoo.com
// Thanks to Janis Legzdinsh for info, http://www.vavoom-engine.com
// based on DeHackEd version 3.0a
// Written by Greg Lewis, gregl@umich.edu
// If you release any versions of this code, please include
// the author in the credits.  Give credit where credit is due!

// Function prototypes
void AbortProg(char *func);
int  DetectDoomver(int argc, char *argv[]);
int  Parsecommandline(int argc, char *argv[]);
void Loaddoom(FILE *exefp);
void Parseconfigfile(void);
int  RunExe(void);
int  Updatecodep(void);
int  Updateframe(void);
int  Updatemisc(void);
int  Updatesound(void);
int  Updatesprite(void);
int  Updatetext(void);
int  Updatethings(void);
void Writedoom(void);

// Global pointers to thing, frame, sound and weapon data are declared.
// since Sprite info is simply a bunch of pointers to the Text data, it's
// an array of longs.  Also, the ammo info is a simple array of longs.
// The Text data is simply a large array of strings, declared here as one
// large string.  Same with the cheat data.  The misc data is just that,
// stuff stolen from all over in the exe file.
long (*thingdata )[THING_FIELDS];
long (*framedata )[FRAME_FIELDS];
long (*weapondata)[WEAPON_FIELDS];
long (*sounddata )[SOUND_FIELDS];
long *spritedata;
char *textdata;
long *codepdata;
char *cheatdata;
long *ammodata;
long *miscdata;

void *alldata[NUMDATA];

// Other global variables.
ERedraw redraw = ALL;			// Current redraw state
EModes  mode = THING_ED;		//	The mode we start in
EVersions version = NO_VER;   // The Doom version we're using
FILE *doomexefp = NULL;			// File pointer to doom.exe
FILE *doomwadfp = NULL;       // File pointer to doom.wad
FILE *doombakfp = NULL;			// File pointer to doomhack.exe
char curdir[MAXDIR+1];			// Directory at start of program
EBool changes = NO;				// If changes have been made
EBool batch = YES;				// If we're running in batch mode
ScreenSet *Modeptr;					// Pointer to Data for the current mode

// For the dehacked.ini file
char doompath[80]= "";			// Directory to run doom from
char doomexe[80] = "";			// Doom.exe file name
char doombak[80] = "";			// Heretic.exe backup file
char doomwad[80] = "";			// Doom.wad file name
char doomargs[80]= "";			// Arguments when Doom is run
char patchdir[80]= "";			// Patch file directory
long doomsize = 0;				// Doom.exe size
struct sb_conf dev = {544, 7};// Sound blaster info
EBool askatload = YES;			// Ask about reloading data during load
EBool loadlogo = YES;			// Load the logo at program start

DataSet Data[NUMDATA] = {
	// Global data for Things.  Refer to dehacked.h
        // complete struct explanation.
        { {0xADFDCL, 0xA576CL, 0xADD84L, 0xADF90L, 0xADF90L},
                34560L, 360, 92,
                "Thing" }, // changed 138 to 345 and A5660 to ADF90 and 12696 to 33024or33120 but that being over 32766 gives warning
                // 33120 for 345 things
	// Frame
        { {0xA6944L, 0x9DD28L, 0xA66ECL, 0xA68F8L, 0xA68F8L},
                30360, 1518, 28,
                "Frame" }, // changed 966 to 1344 and 9EC9C to A68F8 and 27048 to 30360

	// Weapon
        { {0x583E4L, 0x54834L, 0x584C4L, 0x589B4L, 0x588F4L},
                308, 11, 25,
                "Weapon" },   // Changed 9 to 11 and 24 to 25 and 9BFB8 to 588F4 and 216 to 308

	// Sound
        { {0xA5860L, 0x9CC58L, 0xA5608L, 0xA5814L, 0xA5814L},
                3240, 135, 36,
                "Sound" },  // changed 9DB24 to A5814 and 107 to 135 and 3852 to 3240 (not 3256)

	// Sprite
        { {0xA6528L, 0x9D920L, 0xA62D0L, 0xA64DCL, 0xA64DCL},
               1052, 263, 4,
                "Sprite" }, // changed 9EA74 to A64DC and 138 to 263 and 552 to 1052

	// Text
        { {0x8CC54L, 0x84654L, 0x8CC54L, 0x8CE54L, 0x8CE54L},
		0, 0, 0,
                "Text" }, // changed A8B78 to 8CE54

	// Code pointers
        { {0x3E1D0L, 0x3E122L, 0x3E1D4L, 0x409EEL, 0x409EEL},
                3864, 1518, 4,
                "Code Pointer" }, //changed 966 to 1344

	// Cheats
	// IDDT is always assumed to be 3624 bytes before the offset given.
	// If this ever changes... ick.
        { {0xA5354L, 0x9C729L, 0xA50FCL, 0xA5308L, 0xA5308L},
                151, 17, 0,
                "Cheat" }, // changed 9CCC4 to A5308

	// Ammo
        { {0xA4070L, 0x9B3D8L, 0xA3E18L, 0xA4024L, 0xA4024L},
                56, 14, 7,
                "Ammo" }, // changed 8 to 14 and 4 to 7or9 and 9BF98 to A4024 and 32 to 56

	// Miscellaneous
	// The real offset material is stored in Miscoff.
        { {0x5C858L, 0x5C848L, 0x5C838L, 0x62BD8L, 0x62BD8L},
		64, 16, 4,
                "Misc" } // changed 62BD8 to

// 67239
// 68B77
// 6914D
// 698D4
// 6A0b4

};

// Prototypes for Print functions
extern void Printframe(void);
extern void Printmisc(void);
extern void Printsound(void);
extern void Printsprite(void);
extern void Printtext(void);
extern void Printthing(void);

// Which fields belong to which windows
extern int thingwindows[][2];
extern int miscwindows[][2];

ScreenSet Screen[NUMSCREENS] = {
	// Screen data for Thing editing screen.  Refer to dehacked.h
	// complete struct explanation.
	{  "Thing",
                0, 1, 0, 1, 360,
                {77, 5, 77, 24}, // changed 138 to 345 and 23 to 24
		6, thingwindows,
		Printthing,
		Updatethings },

	// Frame
	{  "Frame",
                0, 1, 0, 0, 1517,
		{76, 8, 76, 42},
                5, NULL,
		Printframe,
                Updateframe }, // changed 965 to 1343 and 6 to 5

	// Misc
	{  "Weapon",
                0, 1, 0, 1, 11,
                {60, 34, 60, 44},
                2, miscwindows,
		Printmisc,
                Updatemisc },  // Changed 33 to 34 and 43 to 44 and 9 to 11 and 3 to 2

	// Sound
	{  "Sound",
                0, 1, 0, 1, 135,
                {64, 5, 64, 42},
		3, NULL,
		Printsound,
                Updatesound }, // changed 8 to 5 and 107 to 135

	// Sprite
	{  "Sprite",
                0, 1, 0, 0, 262,
		{54, 8, 54, 42},
		1, NULL,
		Printsprite,
                Updatesprite }, // changed 137 to 262

	// Text
	{  "Text",
		0, 1, 0, 0, 0,
		{77, 8, 77, 42},
		1, NULL,
		Printtext,
		Updatetext }
};

int textlength[NUMVERS] = {25068, 23488, 24468, 24476, 24476}; // changed 24072 to 24476 // should be 24988 for 1.0
int textobjs[NUMVERS] = {1421, 1346, 1404, 1405, 1405}; // changed 1094 to 1404

// Order (field-wise) of things
char thingorder[24] = {IDNUM, HP, SPEED, WIDTH, HEIGHT, MISSILEDAMAGE,
                                                          REACTIONTIME, PAINCHANCE, MASS, BITS, ALERTSOUND,
							  ATTACKSOUND, PAINSOUND, DEATHSOUND, ACTSOUND,
							  NORMALFRAME, MOVEFRAME, INJUREFRAME, CLOSEATTACKFRAME,
                                                          FARATTACKFRAME, CRASHFRAME, DEATHFRAME, EXPLDEATHFRAME,
                                                          NAMEP};

// From screen.cpp
extern void clearscreen(char attr, char fill);
extern void CPrintfXY(int x, int y, char *string, ...);
extern void Drawframe(int type, unsigned char attr, int left, int top,
							 int right,	int bot);
extern void Getwindow(int x1, int y1, int x2, int y2);
extern void Putwindow(void);
extern void WipeScreen(void);

// From print.h
extern char *thingfields[24];
extern char *framefields[5];
extern char *fullwepfields[9];
extern char *soundfields[7];
extern char *cheatfields[17];
extern char *miscfields[16];
extern long toff[NUMVERS];
extern char namelist[360][18];  // changed 138 to 345

// From print.cpp
extern int   Printinputwindow(char *buffer, char *prompt, EInptype type, int length, char firstchar = 0);
extern void  Printintro(void);
extern void  Printoptions(void);
extern void  Printtextintro(void);
extern void  Printtextstring(EBool leavewindow);
extern EBool Printwindow(char *message, int type);

// From misc.cpp
extern void Cheatconvert(char *string, char *cheat, int length);
extern int  Gettextnum(int offset);
extern void Highlight(unsigned char attribute);

// From mouse.cpp
extern void  CloseMouse(void);
extern void  InitMouse(void);
extern EBool getLastEvent(unsigned &x, unsigned &y, EButton &left_button,
								  EButton &right_button);

// From mouse.h
extern unsigned LastEventButtons;

// From files.cpp
extern int  GetDoomFiles(char *arg1);
extern int  GetNextLine(char *nextline, int &numlines, FILE *patchp);
extern int  Loadpatch(char *filename);
extern int  ProcessLine(char *nextline, char **line2);
extern int  Savepatch(char *filename, EBool Overwrite);

// From input.cpp
extern int   Inputtext(EBool showonly);
extern EBool ProcessKeypress(void);
extern EBool ProcessMouse(void);
extern EBool Waitforevent(EBool eatevent);

// From playview.cpp
extern int Displaylogo(void);

// From sb_regs.h
extern int SB_DMA_CHAN;
