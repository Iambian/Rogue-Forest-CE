#ifndef __INCGUARD_VARS__
#define __INCGUARD_VARS__

#include <graphx.h>

/* ==================== All the structs used in the game ================== */

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

typedef struct itemdef_st {
	char	*name;
	uint8_t	type;
	int 	enchantmult;	//fixed point 16.8 against modifier1
	int8_t	modifier1;
	uint8_t	offset1;
	int8_t	modifier2;
	uint8_t	offset2;
} itemdef_t;

typedef struct room_st {
	uint8_t type,x,y,w,h;
} room_t;

typedef struct item_st {
	uint8_t type;	//Full range. Segment into classifications
	uint8_t data;	//e.g. stacking (cons), enchantment (gear), dungeon ID (keys)
} item_t;

typedef struct floor_st {
	uint8_t		id;
	uint32_t 	seed;
	uint8_t		sobj_count;
	uint8_t		mobj_count;
} floor_t;

typedef struct moving_st {
	mobj_t *mobj;	//If player, will be null. Probably won't be moving player this way
	int	startx;
	int starty;
	int endx;
	int endy;
} moving_t;
//-=-=-=-=-=-
typedef struct stats_st {
	uint8_t start_sec,start_min,start_hour;
	uint8_t forestarea;		//Current location in top level forest area
	uint8_t dungeonid;		//Will be nonzero if inside a dungeon. 1-5 (or more)
	uint8_t dungeonfloor;	//Current floor, starting at floor 0.
	uint8_t level;
	int		xp,food,hifood;
	uint8_t	spoints,tpoints;
	uint8_t facing;
	uint8_t timer;
	uint8_t mcguffins;
	uint32_t worldseed;
} stats_t;




/* Functions defined in util.asm */
uint8_t	*asm_SetTile2ColorStart(void);
void 	asm_LoadMinimap(uint8_t xpos, uint8_t ypos);
void	asm_InterpolateMap(void);
uint8_t asm_GetNumpad(void);
void 	asm_DrawTitleHead(gfx_sprite_t* sprite);

/* Shared functions defined in objs.c */
#define sobj_Clear() sobjcount = 0;
#define mobj_Clear() mobjcount = 0;
#define moving_Clear() movingcount = 0;
void	sobj_Add(sobj_t *sobj);
void	mobj_Add(mobj_t *mobj);
void	sobj_Del(sobj_t *sobj);
void	mobj_Del(mobj_t *mobj);
sobj_t	*sobj_GetByPos(uint8_t x, uint8_t y);
mobj_t	*mobj_GetByPos(uint8_t x, uint8_t y);
sobj_t	*sobj_GetByDest(uint8_t warpid);
uint8_t	sobj_IsDoorable(uint8_t x, uint8_t y);
void	sobj_WriteToMap(void);
mobjdef_t	*mobj_GetDef(mobj_t *mobj);
void	mobj_RecalcPlayer(void);


/* Shared functions and variables in items.c */
uint8_t		items_Filter(item_t *item);
itemdef_t	*items_FilterDef(item_t *item);
int			items_GetStatValue(item_t *item, uint8_t iter);
uint8_t 	items_GetStatOffset(item_t *item, uint8_t iter);
gfx_sprite_t *items_GetSprite(item_t *item);
char 		*items_GetName(item_t *item);
char 		*items_GetDesc(item_t *item);



/* Shared functions and variables in dgen.c */
void gen_WarpTo(uint8_t id);
void gen_Overworld(void);


/* Shared functions and variables in disp.c */
uint8_t	disp_Title(uint8_t key);	//returns new state of game
uint8_t	disp_Credits(uint8_t key);
void	disp_Sidebar(uint8_t update);
uint8_t	disp_Gamemode(uint8_t key);
uint8_t	disp_Menumode(uint8_t key);



/* Shared functions and variables in main.c */
extern gfx_sprite_t *curmap;		//16386
extern gfx_sprite_t *emptysprite;	//256
extern gfx_sprite_t *tiles[256];	//768
extern gfx_tilemap_t tilemap;
extern stats_t stats;
extern char stringbuf[100];
extern uint8_t tile2color[513];
extern uint8_t forestmap_seen[25];
extern uint8_t forestmap[25];
extern uint8_t dungeonmap[6];
extern int maxleveltable[];
extern item_t inventory[40];
extern item_t equipment[8];
extern item_t quickbar[10];
extern mobj_t mobjtable[254];
extern sobj_t sobjtable[254];
extern room_t roomtable[100];
extern floor_t floortable[254];
extern moving_t movingtable[254];
extern uint8_t mobjcount,sobjcount,roomcount,floorcount,movingcount;
extern mobj_t pmobj;
extern mobjdef_t pbase,pcalc;

uint8_t util_GetSK(void);
#define util_BufClr() stringbuf[0] = 0;
#define util_BufSet(s) util_BufClr(); util_BufStr(%%s%%);
void	util_BufStr(char *s);
void	util_BufChr(char c);
void	util_BufInt(int num, uint8_t numzeroes);
void	util_BufTime(void);
void	util_PrintF(char *s);
void	util_PrintUF(char *s);



#endif