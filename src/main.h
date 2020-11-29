#ifndef __MAIN__INCGUARD__
#define __MAIN__INCGUARD__

#include <stdint.h>
#include "defs.h"

/* ----------------------------------- structs -----------------------------*/
typedef struct pstats_st {
	uint8_t empty;			//MUST BE FIRST ENTRY IN PSTATS_ST OR THINGS BREAK
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
	uint8_t status;
	uint8_t strength,speeds,smarts,rawrs;
	uint8_t attack,defense,evasion,sneaks,magic,reflect;
	uint8_t mres,fres,eres,pres; //magic,fire,electric,poison resistances
	uint8_t matk,fatk,eatk,patk; //magic,fire,electric,poison power
	uint8_t statpoints;
	uint8_t talentpoints;
	uint8_t facing;
	uint8_t walkcycle;	//Used to animate the player sprite when walking
	uint8_t timer;		//like a walk cycle, except updates regardless of mov't
	
} pstats_t; //pstats is the variable being used with this struct

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
	uint8_t hp;
	uint8_t mp;
	uint8_t step;
	uint8_t drop;
} mobj_t;

//You may want to do bitpacking shenanigans if you want to have more than 255 items.
typedef struct item_st {
	uint8_t type;	//Full range. Segment into classifications
	uint8_t data;	//e.g. stacking (cons), enchantment (gear), dungeon ID (keys)
} item_t;

typedef struct floordat_st {
	uint32_t 	seed;
	uint8_t		sobj_count;
	uint8_t		mobj_count;
	
} floordat_t;

/* --------------------------- variable declarations -----------------------*/
extern sobj_t empty_sobj;
extern mobj_t empty_mobj;
extern item_t empty_item;
extern gfx_sprite_t *empty_tile;
extern gfx_sprite_t **main_tilemap;

extern struct pstats_st pstats;
extern int maxlevel_table[];
extern uint8_t numrooms;
extern room_t roomlist[NUMROOMS_MAX];
extern floordat_t floordat;

extern int totalgens;
extern gfx_sprite_t* curmap;
extern gfx_tilemap_t tilemap;
extern uint8_t tile2color[];

extern uint8_t	numsobjs;
extern sobj_t	sobjs[];
extern uint8_t	nummobjs;
extern mobj_t	mobjs[];

extern item_t inventory[];
extern item_t equipment[];
extern item_t quickbar[];
extern item_t secondary;
extern uint8_t forestmap[25];
extern uint8_t forestdungeon[6];


//For debugging purposes
extern uint8_t forestmap_test_start;
extern uint8_t forestmap_test[25];
extern uint8_t forestdungeon_test[6];


/* --------------------- external routines in util.asm ---------------------*/
char *util_BufInt(int num);
uint8_t	*asm_SetTile2ColorStart(void);
void 	asm_LoadMinimap(uint8_t xpos, uint8_t ypos);
void	asm_InterpolateMap(void);
uint8_t asm_GetNumpad(void);




#endif