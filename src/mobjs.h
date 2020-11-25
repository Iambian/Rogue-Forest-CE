#ifndef __MOBJS__INCGUARD__
#define __MOBJS__INCGUARD__

#include <stdint.h>
#include "main.h"
#include "gfx/output/gfx_charequtiles.h"



typedef void (*mobj_script)(mobj_t *mobj);

typedef struct mobjdef_st {
	char	*name;
	void 	**sprobj;
	mobj_script script;
	uint8_t maxhp,maxmp;
	int8_t str,spd,smrt;		//strength,speed,intelligence/smarts
	int8_t atk,def,blk,refl;	//dma, dmg absorpb, dmg block, dmg reflect
	int8_t snk,perc,rawrs;		//evade notice, power of noticing, luck/antisneak
	int8_t mdef,matk;			//magic (general purpose, stacking)
	int8_t fdef,fatk;			//fire
	int8_t edef,eatk;			//electrical
	int8_t pdef,patk;			//poison
} mobjdef_t;

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


extern mobjdef_t emptyplayer,playerbase,playercalc,enemycalc;
extern mobjdef_t playerdef,enemydef[];

void mobj_newchar(void);
void mobj_clear(void);
void mobj_addentry(mobj_t* mobj);
void mobj_rementry(uint8_t index);
void mobj_recalcplayer(void);







#endif