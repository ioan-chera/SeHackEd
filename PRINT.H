// SeHackEd version 0.4
// Modified by Mike Fredericks, chohatsu@yahoo.com
// Thanks to Janis Legzdinsh for info, http://www.vavoom-engine.com
// based on DeHackEd version 3.0a
// Written by Greg Lewis, gregl@umich.edu
// If you release any versions of this code, please include
// the author in the credits.  Give credit where credit is due!

// Function prototypes for print.cpp
void Printcodep(void);
void Printframe(void);
void Printhelp(void);
int  Printinputwindow(char *buffer, char *prompt, EInptype type, int length,
							 char firstchar = 0);
void Printintro(void);
void Printmisc(void);
void Printmisccheat(void);
void Printmiscmisc(void);
void Printmiscweapon(void);
void Printoptions(void);
void Printsound(void);
void Printsprite(void);
void Printtext(void);
void Printtextintro(void);
void Printtextstring(EBool leavewindow);
void Printthing(void);
void Printthingbits(void);
void Printthingframe(void);
void Printthinginfo(void);
void Printthingmisc(void);
void Printthingsound(void);
EBool Printwindow(char *message, int type);

// This is an offset to be used for computing the text stuff.
long toff[NUMVERS] = {131072L, 131072L, 131072L, 131072L, 131072L};
// Changed 129380 to 131072

// Functions from screen.cpp
extern void CPrintfXY(int x, int y, char *string, ...);
extern void CPutsXY(int x, int y, char *string);
extern void Drawframe(int type, unsigned char attr, int left, int top,
							 int right, int bot);
extern void Getwindow(int x1, int y1, int x2, int y2);
extern void Putwindow(void);

// Variables from dehacked.cpp
extern ERedraw redraw;						//	Current redraw state
extern EModes  mode;							//	The mode we start in
extern EVersions version;   		 // The Doom version we're using
extern char thingorder[THING_FIELDS];
extern char doomexe[80];					// Doom.exe file name
extern char doomwad[80];					// Doom.wad file name
extern char patchdir[80];					// Patch file directory
extern EBool batch;							// If we're running in batch mode
extern ScreenSet *Modeptr;					// Pointer to Data for the current mode

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
extern struct sb_conf dev;
extern int codepoff[725][5]; // changed 448 to 725
extern int cheatinfo[17][2];

// All data on objects
extern DataSet Data[NUMDATA];

// Functions from dehacked.cpp
extern void AbortProg(char *func);

// Functions from misc.cpp
extern void Cheatconvert(char *string, char *cheat, int length);
extern void Getframename(int framenum, char *string);
extern void Getsoundname(int soundnum, char *string);
extern void Getspritename(int spritenum, char *string);
extern void Highlightname(char *name);

// Variables from sb_regs.c
extern int SB_DMA_CHAN;

// Functions from input.h
extern int Getinput(int promptlen, char *string, int maxlen, int type, int x1,
						  char firstchar = 0);
extern EBool Waitforevent(EBool eatevent);

// Names for the different types of data... used in loading patches.
// Must be one word only.
char *datanames[NUMDATA] = {"Thing",   	//	#defined to be... 0
									 "Frame",		// 1
									 "Weapon",		// 2
									 "Sound", 	  	// 3
									 "Sprite",		// 4
									 "Text",   		// 5
									 "Pointer", 	// 6
									 "Cheat", 		// 7
									 "Ammo",			// 8
									 "Misc"};		// 9

// Name for all the fields of all the data types... whew...
char *thingfields[24] = {"ID #",
								 "Initial frame",
								 "Hit points",
								 "First moving frame",
								 "Alert sound",
								 "Reaction time",
								 "Attack sound",
								 "Injury frame",
								 "Pain chance",
								 "Pain sound",
								 "Close attack frame",
								 "Far attack frame",
								 "Crash frame",
								 "Death frame",
								 "Exploding frame",
								 "Death sound",
								 "Speed",
								 "Width",
								 "Height",
								 "Mass",
								 "Missile damage",
								 "Action sound",
								 "Bits",
								 "Name Pointer"};
// changed 9 to 6 and removed stuff
char *soundfields[6] = {"Offset",
								"Zero/One",
								"Value",
								"Zero 1",
								"Zero 2",
								"Zero 3"};
// Changed stuff here!
char *framefields[5] = {"Sprite number",
								"Sprite subnumber",
								"Duration",
								"Action pointer",
								"Next frame"};
char *weaponfields[7] = {"Ammo type",
								 "Deselect frame",
								 "Select frame",
								 "Bobbing frame",
								 "Shooting frame",
								 "Firing frame",
								 "? Unknown"};
char *fullwepfields[9] = {"Ammo number",
								  "Init. max ammo",
								  "Ammo per item",
								  "Deselect frame",
								  "Select frame",
								  "Bobbing frame",
								  "Shooting frame",
								  "Firing frame",
								  "? Unknown"};
char *cheatfields[17] = {"Change Music",
								 "Level Warp",
								 "Stealth Boots",
								 "Sigil piece",
								 "FPS",
								 "TeleportMapSpot",
								 "Gold&StatTokens",
								 "God mode",
								 "Keys",
								 "Weapons & Ammo",
								 "Massacre",
								 "No Clipping",
								 "Berserk",
								 "Invisibility",
								 "Enviro Suit",
								 "Health",
								 "Backpack"};
char *miscfields[16] = {"Initial Health",
								"Initial Bullets",
								"Max Health",
								"Max Armor",
								"LeatherArmorClass",
								"Metal Armor Class",
								"Max Soulsphere",
								"Soulsphere Health",
								"Megasphere Health",
								"God Mode Health",
								"IDFA Armor",
								"IDFA Armor Class",
								"IDKFA Armor",
								"IDKFA Armor Class",
								"Mauler Cells/Shot",
								"Monsters Infight"};

// Here are names for all of the types of Ammo, types of Weapons, and
// types of Things.
// Changed stuff here!
char *ammolist[9] = {"Bullets",
							"E Bolts",
							"P Bolts",
							"Cells",
							"Missiles",
							"H-E Gren",
							"PhosphorG",
							"NotUsed",
							"Unlimited"};

// Changed stuff here!
char *weaponlist[11]={"Fists",
							"ElectricCrossbow",
							"Assault Gun",
							"Missile Launcher",
							"H-E Grenade Laun",
							"Flame Thrower",
							"Mauler Blaster",
							"Sigil",
							"Poison Crossbow",
							"Phos-Gren Launch",
							"Mauler Torpedo"};

char namelist[360][18] = {"Force Field Guard",
							  "Player",

							  "Weapon Smith 4Tan",
							  "Bar Keep 1 Gold",
							  "Armorer 2 B Green",
							  "Medic 3 Blue",
							  "Peasant Tan 1",
							  "Peasant Tan 2",
							  "Peasant Tan 3",
							  "Peasant Red 1",
							  "Peasant Red 2",
							  "Peasant Red 3",
							  "Peasant Gray 1",
							  "Peasant Gray 2",
							  "Peasant Gray 3",
							  "Peasant Rust 1",
							  "Peasant Rust 2",
							  "Peasant Rust 3",
							  "Peasant D Green 1",
							  "Peasant D Green 2",
							  "Peasant D Green 3",
							  "Peasant B Green 1",
							  "Peasant B Green 2",
							  "Peasant B Green 3",
							  "Peasant Gold 1",
							  "Peasant Gold 2",
							  "Peasant Gold 3",
							  "Peasant Blue",
							  "Zombie",
							  "Becoming Acolyte",
							  "Zombie Spawner",
							  "Huge Tank 1",
							  "Huge Tank 2",
							  "Huge Tank 3",
							  "Tank 4",
							  "Tank 5",
							  "Tank 6",
							  "Kneeling Guy",
							  "Beggar 1",
							  "Beggar 2",
							  "Beggar 3",
							  "Beggar 4",
							  "Beggar 5",
							  "Rebel 1",

							  "Rebel 2",
							  "Rebel 3",
							  "Rebel 4",
							  "Rebel 5",
							  "Rebel 6",
							  "Macil 1Invincible",
							  "Macil 2Spectral",
							  "Missile trail",
							  "Reaver",
							  "Acolyte 1 Tan",
							  "Acolyte 2 Red",
							  "Acolyte 3 Rust",
							  "Acolyte 4 Gray",
							  "Acolyte 5 D Green",
							  "Acolyte 6 Gold",
							  "Acolyte 7 B Green",
							  "Acolyte 8 Blue",
							  "Shadow Acolyte",
							  "Templar",
							  "Crusader",
							  "Bishop",
							  "Oracle",
							  "Priest/Loremaster",
							  "Specter A",
							  "DyingSpecterNode",
							  "DyingSpecterChunk",
							  "Specter B",
							  "Specter C",
							  "Specter D",
							  "Specter E",
							  "Entity",
							  "Sub Entity",
							  "Entity Nest",
							  "Entity Pod",
							  "Sigil B shot",
							  "Specter Sigil B",
							  "Sigil C shot",
							  "Specter Sigil C",
							  "Sigil E offshoot",
							  "Sigil trail",
							  "Sigil E shot",
							  "Specter Sigil E",
							  "Sigil A left zap",

							  "Sigil A right zap",
							  "Sigil A ground",
							  "Sigil D Shot",
							  "Specter Sigil D",
							  "Sentinel",
							  "Stalker",
							  "Inquisitor",
							  "Inquisitor Arm",
							  "Programmer",
							  "Programmer Base",
							  "Grappling Hook",
							  "Grappling Chain",
							  "Player Missile",
							  "Crusader Missile",
							  "Bishop Missile",
							  "Electric Bolt",
							  "Poison Bolt",
							  "Sentinel R Laser",
							  "Sentinel L Laser",
							  "H-E Grenade",
							  "Phosphorous Gren",
							  "InquisitorGrenade",
							  "Phosphorous Flame",
							  "Mauler Torpedo",
							  "Mauler T Spread",
							  "Player Flame",
							  "Crusader Flame",
							  "Mauler Blast",
							  "Bullet Puff",
							  "Robot Spark",
							  "Blood from Death",
							  "Teleport Fog",
							  "Item Respawn Fog",
							  "TeleportDestinatn",
							  "Claxon Light",
							  "Ceiling Turret",
							  "Gates (Piston)",
							  "Computer",
							  "Med Patch",
							  "Medical Kit",
							  "Surgery Kit",
							  "Degnin Ore",
							  "Metal Armor",

							  "Leather Armor",
							  "Water Bottle",
							  "Mug",
							  "Base Key (Front)",
							  "Govs Key",
							  "Travel Passcard",
							  "Blue ID Card",
							  "Prison Key",
							  "Severed Hand",
							  "Power1 Key",
							  "Power2 Key",
							  "Power3 Key",
							  "Gold Key",
							  "Gold ID Badge",
							  "Silver Key",
							  "Oracle Key",
							  "Military ID",
							  "Order Key",
							  "Warehouse Key",
							  "Brass Key",
							  "Red Crystal Key",
							  "Blue Crystal Key",
							  "Chapel Key",
							  "Catacomb Key",
							  "Security Key",
							  "Core Key",
							  "Mauler Key",
							  "Factory Key",
							  "Mine Key",
							  "New Key5",
							  "Shadow Armor",
							  "EnvironmentalSuit",
							  "Guard Uniform",
							  "Officer's Uniform",
							  "Map",
							  "Scanner",
							  "Teleporter Beacon",
							  "Targeter",
							  "Coin",
							  "10 Gold",
							  "25 Gold",
							  "50 Gold",

							  "300 Gold",
							  "Ring",
							  "Offering Chalice",
							  "Ear",
							  "Communicator",
							  "HE Grenade Rounds",
							  "Phosphor Grenades",
							  "Clip of Bullets",
							  "Box of Bullets",
							  "Mini Missiles",
							  "Crate of Missiles",
							  "Energy Pod",
							  "Energy Pack",
							  "Poison Bolts",
							  "Electric Bolts",
							  "Ammo Satchel",
							  "Assault Gun",
							  "Assault Gun S",
							  "Flame Thrower",
							  "FlameThrowerParts",
							  "Missile Launcher",
							  "Mauler",
							  "Crossbow",
							  "Grenade Launcher",
							  "Sigil A",
							  "Sigil B",
							  "Sigil C",
							  "Sigil D",
							  "Sigil E",
							  "(Power) Crystal",
							  "Rat buddy",
							  "Wooden Barrel",
							  "Explosive Barrel",
							  "Practice Target",
							  "SilverFlourescent",
							  "Brass Flourescent",
							  "Gold Flourescent",
							  "Globe Light",
							  "Techno Pillar",
							  "Aztec Pillar",
							  "DamagedAztecPillr",
							  "RuinedAztecPillar",
							  "Huge Tech Pillar",

							  "Huge Alien Pillar",
							  "Stalactite",
							  "Small Stalactite",
							  "Stalacmite",
							  "Cave Pillar Top",
							  "Cave PillarBottom",
							  "Stalacmite Small",
							  "Candle",
							  "Candelabra",
							  "Floor Water Drip",
							  "Waterfall Splash",
							  "Ceiling WaterDrip",
							  "Fountain",
							  "Hearts in Tank",
							  "Teleport Swirl",
							  "Dead Crusader",
							  "Dead Player",
							  "Dead Peasant",
							  "Dead Acolyte",
							  "Dead Reaver",
							  "Dead Rebel",
							  "Sacrificed Guy",
							  "Guts and Bones",
							  "Burning Barrel",
							  "Burning Bowl",
							  "Burning Brazier",
							  "SmallBurningTorch",
							  "Small Unlit Torch",
							  "Ceiling Chain",
							  "Cage Light",
							  "Statue",
							  "Ruined Statue",
							  "Medium Torch",
							  "Outside Lamp",
							  "Pole Lantern",
							  "Rock 1",
							  "Rock 2",
							  "Rock 3",
							  "Rock 4",
							  "Stick in Water",
							  "Rubble 1",
							  "Rubble 2",
							  "Rubble 3",

							  "Rubble 4",
							  "Rubble 5",
							  "Rubble 6",
							  "Rubble 7",
							  "Rubble 8",
							  "Surgery Crab",
							  "Large Torch",
							  "Huge Torch",
							  "Palm Tree",
							  "Big Tree",
							  "Potted Tree",
							  "Tree Stub",
							  "Short Bush",
							  "Tall Bush",
							  "Chimneystack",
							  "Barricade Column",
							  "Pot",
							  "Pitcher",
							  "Stool",
							  "Metal Pot",
							  "Tub",
							  "Anvil",
							  "Silver Tech Lamp",
							  "Brass Tech Lamp",
							  "Tray",
							  "Ammo Filler",
							  "Sigil Banner",
							  "Rebel Boots",
							  "Rebel Helmet",
							  "Rebel Shirt",
							  "Power Coupling",
							  "Broken Coupling",
							  "AlienBubbleColumn",
							  "Alien FloorBubble",
							  "Alien Ceil Bubble",
							  "Alien Asp Climber",
							  "Alien SpiderLight",
							  "Giblets",
							  "Shrapnel",
							  "Little flame",
							  "Ammo (50 bullets)",
							  "Health(ToHalfMax)",

							  "Info",
							  "Alarm",
							  "B50D0 Unknown 1",
							  "B5130 Unknown 2",
							  "Prison Pass",
							  "B51F0 Unknown 3",
							  "B5250 Unknown 4",
							  "B52B0 Unknown 5",
							  "Report",
							  "Toughness",
							  "Accuracy",
							  "Oracle Pass",
							  "B5490 Unknown 6",
							  "B54F0 Unknown 7",
							  "B5550 Unknown 8",
							  "Quest4",
							  "Quest5",
							  "Quest6",
							  "B56D0 Unknown 9",
							  "B5730 Unknown 10",
							  "B5790 Unknown 11",
							  "B57F0 Unknown 12",
							  "B5850 Unknown 13",
							  "B58B0 Unknown 14",
							  "B5910 Unknown 15",
							  "Blown the Crystal",
							  "B59D0 Unknown 16",
							  "Blown the Gates",
							  "B5A90 Unknown 17",
							  "B5AF0 Unknown 18",
							  "B5B50 Unknown 19",
							  "B5BB0 Unknown 20",
							  "Killed the Bishop",
							  "B5C70 Unknown 21",
							  "Killed the Oracle",
							  "Killed Macil",
							  "B5D90 Unknown 22",
							  "Killed Loremaster",
							  "Blown Up Computer",
							  "B5EB0 Unknown 23",
							  "B5F10 Unknown 24",
							  "B5F70 Unknown 25",
							  "B5FD0 Unknown 26",
							  "B6030 Unknown 27",
							  "Do not edit 345",
							  "Clipboard 346",
							  "Clipboard 347",
							  "Clipboard 348",
							  "Clipboard 349",
							  "Clipboard 350",
							  "Clipboard 351",
							  "Clipboard 352",
							  "Clipboard 353",
							  "Clipboard 354",
							  "Clipboard 355",
							  "Clipboard 356",
							  "Clipboard 357",
							  "Clipboard 358",
							  "Clipboard 359",
							  "Clipboard 360"};
