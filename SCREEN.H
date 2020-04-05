// DeHackEd version 3.0a
// Written by Greg Lewis, gregl@umich.edu
// If you release any versions of this code, please include
// the author in the credits.  Give credit where credit is due!

#define pokeb(w, x, y, z) screenbptr[160*x+2*w+y] = z
#define pokew(x, y, z)    screenwptr[80 *y+x] = z

// Functions in screen.cpp
void Blankwindow(char attr, int left, int top, int right, int bot);
void clearscreen(char attr, char fill);
void clsMCGA(void);
void CPrintfXY(int x, int y, char *string, ...);
void CPutsXY(int x, int y, char *string);
void Drawframe(int type, unsigned char attr, int left, int top, int right,
					int bot);
void Getwindow(int x1, int y1, int x2, int y2);
void MCGAPutsXY (int x, int y, char *string, unsigned char col);
void Putwindow(void);
void setpalette(char palette[768]);
void setpos(int x, int y);
void toMCGA(void);
void totext(void);
void WipeScreen(void);

// Characters defined for text windows
const char *ul = "…⁄€’÷";
const char *ur = "ªø€∏∑";
const char *ll = "»¿€‘”";
const char *lr = "ºŸ€æΩ";
const char *hz = "Õƒ€Õƒ";
const char *vt = "∫≥€≥∫";

// Screen word pointer and byte pointer
int  *screenwptr = (int  *)MK_FP(0xb800, 0);
char *screenbptr = (char *)MK_FP(0xb800, 0);

// Graphics screen pointer
char *pscreen = (char *)MK_FP(0xa000, 0);

// This struct is used for stored background screen data when a window
// is drawn over it.
typedef struct BackgroundS {
	int *image;
	int x1;
	int y1;
	int x2;
	int y2;
	struct BackgroundS *next;
} Background;

// Struct which stores any background data that is stored, and also
// positions necessary to put data back.
Background backhead = {NULL, 0, 0, 0, 0, NULL};

// Function from dehacked.cpp
extern void AbortProg(char *func);
