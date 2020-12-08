#ifndef __INCGUARD_DEFS__
#define __INCGUARD_DEFS__
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ start ~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#define VERSION "0.02"
#define gfx_SetPaletteEntry(offset,color1555) ((uint16_t*)0xE30200)[offset] = color1555;
/* ------------------------------ Colors -------------------------- */
#define COLOR_BLACK				0
#define COLOR_WHITE				1
#define COLOR_RED				2
#define COLOR_GREEN				3
#define COLOR_BLUE				4
#define COLOR_TRANS				5
#define COLOR_GRAY				6
#define COLOR_LIGHTGRAY			7
#define COLOR_GUNMETALGRAY		8
#define COLOR_GOLD				9
#define COLOR_CYAN				10
#define COLOR_LIME				11
#define COLOR_ORANGE			12
#define COLOR_FORESTGREEN		13
#define COLOR_DARKGRAY			14
#define COLOR_PURPLE			15
/* ------------------------------ Keyboard ------------------------ */
#define kbit_Down		(1<<0)
#define kbit_Left		(1<<1)
#define kbit_Right		(1<<2)
#define kbit_Up			(1<<3)
#define kbit_Alpha		(1<<4)
#define kbit_2nd		(1<<5)
#define kbit_Mode		(1<<6)
#define kbit_Del		(1<<7)
/* ------------------------------- Tiles -------------------------- */
#define wallAbase	( 0+(0*18))
#define wallBbase	( 0+(1*18))
#define wallCbase	( 0+(2*18))
#define floorAbase	(64+(0*21))
#define floorBbase	(64+(1*21))
#define floorCbase	(64+(2*21))
#define floortilebase	(128)
#define kitembase	(128+64)
/* --------------------------- Game State ------------------------- */
#define GS_TITLE	0
#define GS_NEWGAME	(GS_TITLE+1)
#define GS_LOADGAME	(GS_TITLE+2)
#define GS_CREDITS	(GS_TITLE+3)
#define GS_QUIT		(GS_TITLE+4)
#define GS_GAMEMODE	32
/* ----------------------- Enemy Sprite Redefs -------------------- */
#define S_NORMRAT	&(characters_tiles_data[0])
#define S_DIRERAT	&(characters_tiles_data[2])
#define S_SQUIRREL	&(characters_tiles_data[4])
#define S_SNAKE		&(characters_tiles_data[6])
#define S_LIZARD	&(characters_tiles_data[8])
#define S_TURTLE	&(characters_tiles_data[10])
#define S_LIZMAN	&(characters_tiles_data[12])
#define S_HIDDEN	&(characters_tiles_data[14])
#define S_ASP		&(characters_tiles_data[16])
#define S_HYDRA		&(characters_tiles_data[18])
#define S_WISP		&(characters_tiles_data[20])
#define S_BEAN		&(characters_tiles_data[22])
#define S_SKEL		&(characters_tiles_data[24])
#define S_ZOMBIE	&(characters_tiles_data[26])
#define S_ZOMBONE	&(characters_tiles_data[28])
#define S_REAPER	&(characters_tiles_data[30])
#define S_NECRO		&(characters_tiles_data[32])
#define S_SHADE		&(characters_tiles_data[34])
#define S_SCORPION	&(characters_tiles_data[36])
#define S_LADYBUG	&(characters_tiles_data[38])
#define S_SHARK		&(characters_tiles_data[40])
#define S_FISH		&(characters_tiles_data[42])
#define S_BAT		&(characters_tiles_data[44])
#define S_SWAN		&(characters_tiles_data[46])
#define S_PARROT	&(characters_tiles_data[48])
#define S_CHICKEN	&(characters_tiles_data[50])
#define S_DOGGO		&(characters_tiles_data[52])
#define S_DOGMAN	&(characters_tiles_data[54])
#define S_LIGHTS	&(characters_tiles_data[56])
#define S_DARKS		&(characters_tiles_data[58])
#define S_OHDEER	&(characters_tiles_data[60])
#define S_BADOAK	&(characters_tiles_data[62])
#define S_BADPINE	&(characters_tiles_data[64])
#define S_GOBSTER	&(characters_tiles_data[66])
#define S_APE		&(characters_tiles_data[68])
#define S_SMILES	&(characters_tiles_data[70])
#define S_FAE		&(characters_tiles_data[72])
#define S_BADCOP	&(characters_tiles_data[74])
#define S_ANGEL		&(characters_tiles_data[76])
#define S_KING		&(characters_tiles_data[78])











#endif