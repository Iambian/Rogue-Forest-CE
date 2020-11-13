#ifndef __MAIN__INCGUARD__
#define __MAIN__INCGUARD__

#include <stdint.h>
#include "defs.h"

/* ----------------------------------- structs -----------------------------*/
struct pstats_st {
	uint8_t update,updateprev;
	uint8_t x,y;
	uint8_t subx,suby;
	uint8_t forestarea;		//Current location in top level forest area
	uint8_t dungeonid;		//Will be nonzero if inside a dungeon. 1-5 (or more)
	uint8_t dungeonfloor;	//Current floor, starting at floor 0.
	uint8_t level;
	int		xp,maxxp;
	int		food,maxfood;
	uint8_t hp,maxhp;
	uint8_t mp,maxmp;
	uint8_t	attack,defense,agility,magic,rawrs;
};

typedef struct room_st {
	uint8_t type,x,y,w,h;
} room_t;
typedef struct sobj_st {
	uint8_t type;	//Stuff trigger/open state in bit 7 of type. Not many items.
	uint8_t x;
	uint8_t y;
	uint8_t data;	//Such things as treasure id or warpto location
} sobj_t;

typedef struct mobj_st {
	uint8_t type;	//May be many mobs. Have full range available.
	uint8_t x;
	uint8_t y;
	uint8_t flags;
	uint8_t data[4];
} mobj_t;

//You may want to do bitpacking shenanigans if you want to have more than 255 items.
typedef struct item_st {
	uint8_t type;	//Full range. Segment into classifications
	uint8_t data;	//e.g. stacking (cons), enchantment (gear), dungeon ID (keys)
} item_t;





/* --------------------------- variable declarations -----------------------*/

extern struct pstats_st pstats;
extern int maxlevel_table[];
extern uint8_t numrooms;
extern room_t roomlist[NUMROOMS_MAX];

extern int totalgens;
extern gfx_sprite_t* curmap;
extern gfx_tilemap_t tilemap;
extern uint8_t tile2color[];

extern uint8_t	numsobjs;
extern sobj_t	sobjs[];
extern uint8_t	nummobjs;
extern mobj_t	mobjs[];

extern item_t inventory[];
extern item_t equipped;
extern item_t secondary;



/* --------------------- external routines in util.asm ---------------------*/
uint8_t	*asm_SetTile2ColorStart(void);
void 	asm_LoadMinimap(uint8_t xpos, uint8_t ypos);
void	asm_InterpolateMap(void);





#endif