// SeHackEd version 0.4
// Modified by Mike Fredericks, chohatsu@yahoo.com
// Thanks to Janis Legzdinsh for info, http://www.vavoom-engine.com
// based on DeHackEd version 3.0a
// Written by Greg Lewis, gregl@umich.edu
// If you release any versions of this code, please include
// the author in the credits.  Give credit where credit is due!

// Sizes for the different versions
#define SIZE1_12  753298L
#define SIZE1_16  753291L       // Strife 1.0
#define SIZE2_16  715559L       // Strife 1.1
#define SIZE2_17  752691L       // Strife 1.2
#define SIZE2_17A 752698L
#define SIZE2_18  753218L
#define SIZE2_19  753213L
#define SIZE1_19U 753215L       // The Ultimate Doom v1.9 to Strife 1.31
				// changed to 753215

// Number of versions supported, number of separate editing screen,
// and number of data read in.
#define NUMVERS     5
#define NUMDATA    10
#define NUMSCREENS  6

// Version numbers
typedef enum {DOOM1_16, DOOM2_16, DOOM2_17, DOOM2_19, DOOM1_19U, NO_VER}
	EVersions;

// Editing modes
typedef enum {THING, FRAME, WEAPON, SOUND, SPRITE, TEXT, CODEP, CHEAT, AMMO,
	MISC} EData;
typedef enum {THING_ED, FRAME_ED, MISC_ED, SOUND_ED, SPRITE_ED, TEXT_ED}
	EModes;

// Mouse buttons
typedef enum {buttonDown, buttonUp} EButton;

// Number of fields for some types
#define THING_FIELDS  24
#define FRAME_FIELDS   5
#define SOUND_FIELDS   6  // changed 9 to 6
#define WEAPON_FIELDS  7

#define NUMCODEP 725 // changed 448 to 725

// Extended key codes
#define F1    59
#define F2    60
#define F3    61
#define F4    62
#define F5    63
#define F6    64
#define F7    65
#define F8    66
#define F9    67
#define F10   68

#define HOME  71
#define UP    72
#define PGUP  73
#define LEFT  75
#define RIGHT 77
#define END   79
#define DOWN  80
#define PGDN  81

#define BKSP   8
#define TAB    9
#define RET   13
#define ESC   27

// Window color defines
// 0 black
// 1 blue
// 2 green
// 3 cyan
// 4 red
// 5 purple
// 6 orange
// 7 white
// 8 gray
// 9 light blue
// A bright green
// B bright cyan
// C pink
// D light Purple
// E yellow
// F bright White

// Green Color Scheme
#define INFO     0x3F   // Bright white on cyan
#define INFGRAY  0x37   // White on cyan
#define INFDGRAY 0x38   // Grey on cyan
#define ERROR    0x4F   // Bright white on red
#define INPUT    0x1F   // Bright white on blue
#define INPDGRAY 0x18   // Grey on blue
#define INPHILIT 0x1E   // Yellow on blue
#define NORMAL   0x2F   // Bright White on green
#define NERROR   0x24   // Red on green
#define NGRAY    0x2E   // Yellow on green
#define NHILIT   0x0A   // Bright green on black
#define BARS     0x02   // Green on black

// Matrix Color Scheme
/*#define INFO     0x02   // Green on black
#define INFGRAY  0x0E   // Yellow on black
#define INFDGRAY 0x06   // Orange on black
#define ERROR    0x04   // Red on black
#define INPUT    0x02   // Green on black
#define INPDGRAY 0x06   // Orange on black
#define INPHILIT 0x3E   // Yellow on Cyan
#define NORMAL   0x02   // Green on black
#define NERROR   0x03   // Green on black
#define NGRAY    0x01   // Green on black
#define NHILIT   0x0A   // Bright green on black
#define BARS     0x02   // Green on black*/

// Red Color Scheme
/*#define INFO     0x3F   // Bright white on cyan
#define INFGRAY  0x37   // White on cyan
#define INFDGRAY 0x38   // Grey on cyan
#define ERROR    0x1C   // Pink on blue
#define INPUT    0x2F   // Bright white on green
#define INPDGRAY 0x28   // Grey on green
#define INPHILIT 0x3E   // Yellow on Cyan
#define NORMAL   0x4F   // Bright white on red
#define NERROR   0x42   // Blue on red
#define NGRAY    0x47   // White on red
#define NHILIT   0x06   // Orange on black
#define BARS     0x40   // Black on red*/

// Old Blue Color Scheme
/*#define INFO     0x3F   // Bright white on cyan
#define INFGRAY  0x37   // White on cyan
#define INFDGRAY 0x38   // Grey on cyan
#define ERROR    0x4F   // Bright white on red
#define INPUT    0x2F   // Bright white on green
#define INPDGRAY 0x28   // Grey on green
#define INPHILIT 0x3E   // Yellow on Cyan
#define NORMAL   0x1F   // Bright white on blue
#define NERROR   0x1C   // Pink on blue
#define NGRAY    0x17   // White on blue
#define NHILIT   0x71   // Blue on white
#define BARS     0x1F   // Bright white on blue*/

// Redraw defines
typedef enum {NOT, DATA, ALL} ERedraw;

// My own defines
typedef enum {NO, YES} EBool;

// Mouse defines
#define LEFTBUTTON	       1   /* bit 0 */
#define RIGHTBUTTON	      2   /* bit 1 */
#define MIDDLEBUTTON	     4   /* bit 2 */
#define CURSOR_LOCATION_CHANGED  1   /* event mask bits */
#define LEFT_BUTTON_PRESSED      2
#define LEFT_BUTTON_RELEASED     4
#define RIGHT_BUTTON_PRESSED     8
#define RIGHT_BUTTON_RELEASED    16
#define MIDDLE_BUTTON_PRESSED    32
#define MIDDLE_BUTTON_RELEASED   64

// String input constants
typedef enum {STRING, LONGINT, YESNO, CHARINT} EInptype;

// Labelled indices of Thing array info.  The fields appear in exactly
// the order that they appear in the .exe, so they can be loaded
//	very easily.
//	Frame # fields end in 'frame', and Sound # fields end in 'sound'.
//	(Go figure!)

#define IDNUM	     0
#define NORMALFRAME       1
#define HP				2
#define MOVEFRAME	 3
#define ALERTSOUND	4
#define REACTIONTIME      5
#define ATTACKSOUND       6
#define INJUREFRAME       7
#define PAINCHANCE	8
#define PAINSOUND	 9
#define CLOSEATTACKFRAME 10
#define FARATTACKFRAME   11
#define CRASHFRAME       12
#define DEATHFRAME       13
#define EXPLDEATHFRAME   14
#define DEATHSOUND       15
#define SPEED			    16
#define WIDTH			    17
#define HEIGHT			   18
#define MASS			     19
#define MISSILEDAMAGE    20
#define ACTSOUND			 21
#define BITS			     22
#define NAMEP	    23

// Labelled defines for Frame arrays

#define SPRITENUM 0
#define SPRITESUB 1
#define DURATION  2
#define ACTIONPTR 3
#define NEXTFRAME 4

// Constants for fields of Sound info.

#define TEXTP    0
#define ZERO_ONE 1
#define VALUE    2
#define SZERO1   3
#define NEGONE1  4
#define NEGONE2  5
// deleted some

// Fields for Weapon info.

#define AMMOTYPE   0
#define BOB1FRAME  1
#define BOB2FRAME  2
#define BOB3FRAME  3
#define SHOOTFRAME 4
#define FIREFRAME 5
#define UNKNOWNW  6

// Fields for Misc info.

#define INITHEALTH  0
#define INITAMMO    1
#define MAXHEALTH   2
#define MAXARMOR    3
#define GREENCLASS  4
#define BLUECLASS   5
#define MAXSOUL     6
#define SOULHEALTH  7
#define MEGAHEALTH  8
#define GODHEALTH   9
#define IDFAARMOR  10
#define IDFACLASS  11
#define IDKFAARMOR 12
#define IDKFACLASS 13
#define BFGAMMO    14
#define INFIGHTING 15

// Structure definition for the dev sb variable
struct sb_conf
{
	unsigned int   addr;	 /* Address of sb (0x220 or 0x240) */
	unsigned int   irq;	  /* IRQ # (factory default=7) */
};

// Format for directory entries in the heretic.wad file
typedef struct ResourceT
{
	long resstart;
	long reslength;
	char resname[9];
} ResourceS;

// This is all of the information needed for one particular type of
// data from the exe.
typedef struct DataSetS {
	long offset[NUMVERS];	// Offsets for this data for each Doom version
	long length;			    // length of the entire data block
	long numobj;				// Number of objects in this data block
	long objsize;				// Size of each object
	char *name;					// The name of the data
} DataSet;

// Required (?) for the following struct.
typedef int twodim[2];

// This is the information needed for each editing screen.
typedef struct ScreenSetS {
	char *name;					// The name of the screen
	int current;				// The current piece of data
	int field;					// The current field in the editors
	int toprow;					// The top row of the list-style editors.
									// Ignored for non-list editors.
	int min;						// The minimum data value (0/1)
	int max;						// The maximum data value
	int arrows[4];				// The location of the arrows on the screen
	int numfields;				// The number of windows (for Thing and Misc)
									// or fields on this screen.
	twodim *windowdata;		// Pointer to an array of data on windows
	void (*printfunc)();		// The function to call to print this data
	int  (*updatefunc)();	// The function to call to input new data
} ScreenSet;
