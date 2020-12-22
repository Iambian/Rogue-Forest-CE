
#include <string.h>

#include <graphx.h>
#include <debug.h>
#include <keypadc.h>
#include <compression.h>
#include <fileioc.h>

#include "defs.h"
#include "vars.h"
#include "gfx/output/gfx_base.h"
#include "gfx/output/gfx_charequtiles.h"


/* Global variables and stuff */
gfx_sprite_t *tiles[256];
gfx_sprite_t *emptysprite;
gfx_sprite_t *curmap;
gfx_tilemap_t tilemap;
stats_t stats;
char stringbuf[100];
char footerbuf[100];
uint8_t tile2color[513];
item_t inventory[40];
item_t equipment[8];
item_t quickbar[10];
sobj_t sobjtable[254];
room_t roomtable[100];
floor_t floortable[254];
moving_t movingtable[254];
uint8_t forestmap_seen[25];
uint8_t forestmap[25];
uint8_t dungeonmap[6];
uint8_t mobjcount,sobjcount,roomcount,floorcount,movingcount;
mobjdef_t pbase,pcalc;
int maxleveltable[] = {
	20,		40,		60,		80,		120,	200,	300,	450,	800,	1200,
	1800,	3000,	4500,	7000,	9200,	13000,	18000,	25000,	34000,	60000
};
//Hacks to get pmobj to merge seamlessly to mobjtable
mobjcomb_t mobjcomb;


/* Function prototypes */
void main_Init(void);
void main_Exit(void);
void main_NewChar(uint8_t class);
void main_Err(uint8_t filenum);
void *util_InitNewSprite(void);




int main(void) {
	uint8_t i,k,state;
	
	main_Init();	//Setup game
	state = GS_NEWGAME;
	
	
	while (1) {
		++stats.timer;
		*footerbuf = 0;
		k = util_GetSK();
		
		if (GS_TITLE == state) {			//Displays full menu. Returns state change.
			state = disp_Title(k);
			
		} else if (GS_NEWGAME == state) {	//TODO: Add class menu with hiscores.
			main_NewChar(0);
			disp_Sidebar(UPD_SIDEBAR);
			state = GS_GAMEMODE;
			
		} else if (GS_LOADGAME == state) {	//TODO: Add file access and loader.
			state = GS_TITLE;
			
		} else if (GS_CREDITS == state) {	//Displays credits until anykey pushed.
			state = disp_Credits(k);
			
		} else if (GS_QUIT == state) {		//Quits the game.
			break;
			
		} else 	if (GS_GAMEMODE == state) {	//Main game
			state = disp_Gamemode(k);
			
		} else if (GS_MENUMODE == state) {	//Menu mode
			state = disp_Menumode(k);
			
		} else state = GS_QUIT;				//Unhandled states immediately quits.
		
		if (state & (GS_GAMEMODE|GS_MENUMODE)) {
			gfx_SetColor(COLOR_BLACK);
			gfx_SetTextFGColor(COLOR_WHITE);
			gfx_SetTextXY(4,LCD_HEIGHT-10);
			gfx_FillRectangle(4,LCD_HEIGHT-10,LCD_WIDTH-4,8);
			gfx_SetTextXY(4,LCD_HEIGHT-10);
			util_PrintUF(footerbuf);
		}
		gfx_SwapDraw();
	}
	main_Exit();	//Perform putaway
	return 0;
}


/*	Returns bitfield corresponding to the bits in...
	group7: dpad [0-3]
	group1: 2nd, mode, del [5-7]
	group2: alpha [remap bit 7 to 4]	*
	Just read kb_Data[] if you need raw data.
*/
uint8_t util_GetSK(void) {
	static uint8_t prevkey;
	uint8_t key,curkey;
	
	kb_Scan();
	key = (kb_Data[1] & 0xE0) | (kb_Data[7] & 0x0F) | ((kb_Data[2] & 0x80) << 3);
	curkey = (key ^ prevkey) & key;
	prevkey = key;
	return curkey;
}

void util_BufStr(char *s) {
	strcat(stringbuf,s);
}
void util_BufChr(char c) {
	char *s;
	for (s = stringbuf; *s; ++s);
	*s = c;
	++s;
	*s = 0;
}

//If bit 7 of numzeroes set, then prepend '+' if num is nonnegative
void util_BufInt(int num, uint8_t numzeroes) {
	uint8_t slen,neg,i,j,t;
	char s[20]; 
	
	neg = 0;
	if (num<0) {
		neg = 1;
		num = -num;
	}
	
	for (i=0; num; num/=10,++i) s[i] = (num % 10) + '0';
	for ( ; i < (numzeroes&0x07); ++i) s[i] = '0';
	
	if (neg) {
		s[i] = '-';
		++i;
	} else if (numzeroes & 0x80) {
		s[i] = '+';
		++i;
	}
	s[i] = 0;
	slen = strlen(s)/2;
	//dbg_sprintf(dbgout,"Values: %i,%i,%i,%i\n",num,neg,slen,i);
	for (j=0,--i; j<slen; ++j,--i) {
		t = s[i];
		s[i] = s[j];
		s[j] = t;
	}
	util_BufStr(s);
}

void util_BufTime(void) {
	uint8_t cur_sec,cur_min,cur_hour;
	
	cur_hour = rtc_Hours;
	cur_min = rtc_Minutes;
	cur_sec = rtc_Seconds;
	
	//timedelta: now - start -> delta
	if ((cur_sec-=stats.start_sec)>60) {
		cur_sec += 60;
		cur_min -= 1;
	}
	if ((cur_min-=stats.start_min)>60) {
		cur_min += 60;
		cur_hour -= 1;
	}
	if ((cur_hour-=stats.start_hour)>24) {
		cur_hour += 24;
	}
	util_BufClr();
	util_BufInt(cur_hour,1);
	util_BufChr(':');
	util_BufInt(cur_min,2);
	util_BufChr((cur_sec&1)?(':'):(' '));
	util_BufInt(cur_sec,2);
}

void util_PrintF(char *s) {
	int x;
	char c;
	x = gfx_GetTextX();
	while ((c=(*s++))) {
		if ('\n'==c)	gfx_SetTextXY(x,gfx_GetTextY()+8);
		else			gfx_PrintChar(c);
	}
}

void util_PrintUF(char *s) {
	char c;
	while ((c=(*s++))) {
		if ('\n'==c) c = ' ';
		gfx_PrintChar(c);
	}
}

void util_MemSwap(void *ptr1, void *ptr2, size_t size) {
	uint8_t t;
	for (;size;--size) {
		//dbg_sprintf(dbgout,"Memswap iter %i, adr1 %X, adr2 %X\n",size,ptr1,ptr2);
		t = *(uint8_t*)ptr1;
		*(uint8_t*)ptr1 = *(uint8_t*)ptr2;
		*(uint8_t*)ptr2 = t;
		ptr1 = ((uint8_t*)ptr1) + 1;
		ptr2 = ((uint8_t*)ptr2) + 1;
	}
}



/* --------------------------------------------------------------------------- */

void main_Err(uint8_t filenum) {
	char *s;
	gfx_FillScreen(0);
	gfx_SetTextFGColor(0xE0);	//default reddish color
	gfx_PrintStringXY("Fatal error:",5,5);
	if (!filenum) {
		gfx_PrintString("Unspecified error");
	} else {
		gfx_PrintString("File RoFoDAT");
		gfx_PrintUInt(filenum,1);
		gfx_PrintString(" not found.");
	}
	gfx_PrintStringXY("Press any key to exit",5,15);
	while (!util_GetSK());
	gfx_End();
	exit(1);
}

void *util_InitNewSprite(void) {
	uint8_t i;
	gfx_sprite_t *spr;
	spr = gfx_MallocSprite(16,16);
	i = 0;
	do { spr->data[i] = COLOR_RED; } while ((++i)&0xFF);
	return spr;	
}


void main_Init(void)  {
	uint8_t i,j,k,*iptr;
	void **v1,**v2;
	//Initialize libraries and graphical stuffs.
	gfx_Begin();
	gfx_SetTransparentColor(COLOR_TRANS);
	gfx_SetTextTransparentColor(COLOR_TRANS);
	gfx_SetTextBGColor(COLOR_TRANS);
	gfx_SetTextFGColor(COLOR_WHITE);
	gfx_SetDrawBuffer();
	
	// Check for game external data. Immediately exit with error if any missing.
	if (!RoFoDAT1_init()) main_Err(1);
	if (!RoFoDAT2_init()) main_Err(2);
	//Initialize persistent sprite/map data
	curmap = gfx_MallocSprite(128,128);
	emptysprite = util_InitNewSprite();
	//Initialize palettes
	gfx_SetPalette(base_pal,sizeof_base_pal,ui_palette_offset);
	gfx_SetPalette(dtiles_pal,sizeof_dtiles_pal,dtiles_palette_offset);
	gfx_SetPalette(ftiles_pal,sizeof_ftiles_pal,ftiles_palette_offset);
	gfx_SetPalette(charequtiles_pal,sizeof_charequtiles_pal,charequtiles_palette_offset);
	gfx_SetPaletteEntry(charequtiles_palette_offset,gfx_RGBTo1555(128,160,160)); //gunmetal gray
	// Initialize tiles and tilemap/struct
	i = 0; do { tiles[i] = emptysprite; } while ((++i)&0xFF);
	for (i=wallAbase; i<(wallAbase+6*3); ++i)	tiles[i] = util_InitNewSprite();
	for (i=floorAbase; i<(floorAbase+7*3); ++i) tiles[i] = util_InitNewSprite();
	for (i=0; i<floorstuff_tiles_num; ++i)
		tiles[floortilebase+i] = floorstuff_tiles[i];
	for (i=0; i<food_kitems_tiles_num; ++i)
		tiles[kitembase+i] = food_kitems_tiles[i];
	
	tilemap.map 		= curmap->data;
    tilemap.type_width  = gfx_tile_16_pixel;
    tilemap.type_height = gfx_tile_16_pixel;
	tilemap.tile_height = 16;
    tilemap.tile_width  = 16;
    tilemap.draw_height = 14;
    tilemap.draw_width  = 14;
    tilemap.height      = 128;
    tilemap.width       = 128;
    tilemap.y_loc       = 4;
    tilemap.x_loc       = 4;
	tilemap.tiles		= tiles;
	
	//Initialize minimap colors
	iptr = asm_SetTile2ColorStart();
	for(i=0;i< 64;++i) iptr[i+  0] = COLOR_BLACK;		//wall tiles
	for(i=0;i< 64;++i) iptr[i+ 64] = COLOR_WHITE;		//floor tiles
	for(i=0;i<128;++i) iptr[i+128] = COLOR_LIGHTGRAY;	//temp assign for all sobjs
}


void main_Exit(void) {
	gfx_End();
}

//Class may be values 245-255, matching with MOB definitions
void main_NewChar(uint8_t class) {
	uint8_t i,j;
	
	//Init player inventories
	memset(inventory,0,sizeof(inventory));
	memset(equipment,0,sizeof(equipment));
	memset(quickbar,0,sizeof(quickbar));
	//Init player stats
	memset(&pmobj,0,sizeof(pmobj));
	pmobj.type = 245;	//class goes here
	pmobj.hp = pcalc.maxhp;
	pmobj.mp = pcalc.maxmp;
	memcpy(&pbase,mobj_GetDef(&pmobj),sizeof(pbase));
	mobj_RecalcPlayer();
	//Init persistent stats
	memset(&stats,0,sizeof(stats));
	stats.start_sec = rtc_Seconds;
	stats.start_min = rtc_Minutes;
	stats.start_hour= rtc_Hours;
	stats.worldseed = rtc_Time();
	stats.forestarea= 0xFF;
	stats.food = stats.hifood = 20;
	//Init player mobj
	memset(&pmobj,0,sizeof(pmobj));
	//Init overworld (this also loads initial map state)
	gen_Overworld();
	
	//
	// DEBUGGING STUFFS.
	//
	
	/* DEBUGGING -- SET PLAYER INVENTORY WITH STUFFS */
	
	for (j=0,i=1;i<64;i+=4,++j) {
		inventory[j].type = i;
		inventory[j].data = 0;
	}
	inventory[4].data = 2;	//test enchantment display for top-right item
	for (i=ITEM_CONSUMABLE; i < CONS_UNUSEDPOTION1 ; ++i,++j) {
		inventory[j].type = i;
		inventory[j].data = 1;
	}
	
}


