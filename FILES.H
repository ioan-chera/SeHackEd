// SeHackEd version 0.4
// Modified by Mike Fredericks, chohatsu@yahoo.com
// Thanks to Janis Legzdinsh for info, http://www.vavoom-engine.com
// based on DeHackEd version 3.0a
// Written by Greg Lewis, gregl@umich.edu
// If you release any versions of this code, please include
// the author in the credits.  Give credit where credit is due!

// Function prototypes
int  Checkforfile(char *doomfile, char *arg1, char *doomname, char *type, FILE **fp);
void Convertpatch(FILE *patchp, char patchformat);
int  CreateDiffSave(FILE *patchp);
void CreateDoomhack(void);
int  GetDoomFiles(char *arg1);
int  GetNextLine(char *nextline, int &numlines, FILE *patchp);
int  LoadDiff(FILE *patchp);
int  LoadOld(FILE *patchp, EBool commandline);
int  Loadpatch(char *filename, EBool commandline);
void Preparefilename(char *filename, char *fullname);
int  ProcessLine(char *nextline, char **line2);
int  Savepatch(char *filename, EBool Overwrite);
int  Searchforentry(char *name, ResourceS *entry);

// Main variables from dehacked.cpp
extern char doomexe[80];					// Doom.exe file name
extern char doombak[80];
extern char doomwad[80];					// Doom.wad file name
extern char patchdir[80];					// Patch file directory
extern EVersions version;					// The Doom version we're using
extern FILE *doomexefp;						// File pointer to doom.exe
extern FILE *doombakfp;
extern FILE *doomwadfp;						// File pointer to doom.wad
extern char curdir[MAXDIR+1];				// Directory at start of program

// The data arrays
extern long (*thingdata )[THING_FIELDS];
extern long (*framedata )[FRAME_FIELDS];
extern long (*weapondata)[WEAPON_FIELDS];
extern long (*sounddata )[SOUND_FIELDS];
extern long *spritedata;
extern char *textdata;
extern long *codepdata;
extern char *cheatdata;
extern long *ammodata;
extern long *miscdata;
extern void *alldata[NUMDATA];

// Info from data.h for code pointer offsets, cheat offsets, and
// conversions from Doom v1.2 patches.
extern int codepoff[725][5]; // changed 448 to 725
extern int codepconv[725];   // changed 448 to 725
extern int cheatinfo[17][2];
extern unsigned char thingconvar[104];
extern int frameconvar[512];
extern unsigned char soundconvar[63];
extern unsigned char spriteconvar[105];

// All data on objects
extern DataSet Data[NUMDATA];

// Functions from dehacked.cpp
extern void AbortProg(char *func);
extern void Loaddoom(FILE *exefp);

// Functions from print.cpp
extern int   Printinputwindow(char *buffer, char *prompt, EInptype type,
										int length);
extern void  Printoptions(void);
extern EBool Printwindow(char *message, int type);

// Data from print.h
extern char *datanames[NUMDATA];
extern char *thingfields[THING_FIELDS];
extern char *framefields[5];
extern char *weaponfields[7]; // Changed
extern char *soundfields[7];
extern char *weaponlist[11];  // Changed
extern char *cheatfields[17];
extern char *miscfields[16];
extern char *ammolist[9];
extern char namelist[360][18];  // changed 138 to 360 but 345 works

// Functions from misc.cpp
extern void Cheatconvert(char *string, char *cheat, int length);
