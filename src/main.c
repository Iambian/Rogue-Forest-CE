/* Testing for level generation */
/* First compile size: 1153 bytes */



#include <graphx.h>
#include <debug.h>
#include <keypadc.h>
#include <compression.h>
#include <fileioc.h>

#include "defs.h"
#include "main.h"
#include "dgen.h"
#include "sobjs.h"
#include "gfx/output/gfx_base.h"
#include "gfx/output/gfx_dtiles.h"
#include "gfx/output/gfx_ftiles.h"
#include "gfx/output/gfx_charequtiles.h"


/* Display data updating and management */
struct pstats_st pstats;
int maxlevel_table[] = {
	20,		40,		60,		80,		120,
	200,	300,	450,	800,	1200,
	1800,	3000,	4500,	7000,	9200,
	13000,	18000,	25000,	34000,	60000
};

/* ROOMS ROOMS ROOMS ROOMS ROOMS ROOMS */
uint8_t numrooms;				//Number of rooms
room_t roomlist[NUMROOMS_MAX];	//Rooms of type 0 are not used/read

/* Dungeon graphics management */
int totalgens;					//Number of times a room generator was run
gfx_sprite_t* curmap;			//Init to 128,128
gfx_tilemap_t tilemap;
uint8_t tile2color[513];		//For tilemap to minimap conversion. Maybe-collisiondetect?

uint8_t	numsobjs;
sobj_t	sobjs[];
uint8_t	nummobjs;
mobj_t	mobjs[];

item_t inventory[60];	//Fix to a value later when we figure out status screen
item_t equipped;
item_t secondary;



/* Function prototypes goes here */
void disp_ShowSidebar(void);
uint8_t util_GetNumWidth(int num);	//Returns text width of digits were it displayed
void game_Initialize(void);


int main(void) {
	int sec,subsec;
	uint8_t k,kc;
	uint8_t xpos,ypos,x,y;
	uint8_t *ptr;
	
	game_Initialize();
	
	/* Testing conditions */
	gen_TestDungeon(20);
	gfx_SwapDraw();
	xpos = ypos = 8;
	pstats.update = UPD_SIDEBAR;
	dbg_sprintf(dbgout,"Curmap location %X\n",&curmap);
	
	while (1) {
		kb_Scan();
		k = kb_Data[7];
		kc = kb_Data[1];
		if (kc & kb_Mode) break;
		if (!k) continue;
		if (( k & kb_Left) && (xpos>0)) 	--xpos;
		if (( k & kb_Right) && (xpos<127)) ++xpos;
		if (( k & kb_Up)   && (ypos>0)) 	--ypos;
		if (( k & kb_Down) && (ypos<127))  ++ypos;
		
		
		/*	You'd insert a loop here to see change in xpos/ypos in pstats.x
			and pstats.y, scrolling pstats.subx and pstats.suby in the process.
			Updating the sidebar need not occur until after the animation
			sequence has concluded. */
		//
		x = xpos-7;
		y = ypos-7;
		if (x>230) x = 0;
		else if (x>113) x = 113;
		if (y>230) y = 0;
		else if (y>113) y = 113;
		gfx_Tilemap_NoClip(&tilemap,x*16,y*16);
		
		disp_ShowSidebar();
		asm_LoadMinimap(xpos,ypos);
		/* Testing */
		gfx_SetColor(COLOR_BLACK);
		gfx_SetTextFGColor(COLOR_WHITE);
		gfx_FillRectangle(4,LCD_HEIGHT-10,88,8);
		gfx_SetTextXY(4,LCD_HEIGHT-10);
		gfx_PrintUInt(xpos,3);
		gfx_PrintString(", ");
		gfx_PrintUInt(ypos,3);
		
		gfx_SwapDraw();
	}
	
	gfx_End();
    return 0;
}



#define sidebar_center(strwidth) ((((LCD_WIDTH-SBAR_LEFT)-strwidth)/2)+SBAR_LEFT)
void disp_ShowSidebar(void) {
	uint8_t u;
	uint8_t y;
	uint8_t w,sw;
	int temp;
	
	u = pstats.update | pstats.updateprev;
	
	if (u|UPD_BACKERS) {
		/* the overall background */
		gfx_SetColor(COLOR_DARKGRAY);
		gfx_FillRectangle_NoClip(SBAR_LEFT,0,LCD_WIDTH-SBAR_LEFT,LCD_HEIGHT-12);
		/* the two bar pair backgrounds */
		gfx_Sprite_NoClip(xpfood,XPFOOD_X,XPFOOD_Y);
		gfx_Sprite_NoClip(hpmp,HPMP_X,HPMP_Y);
		/* the background for the equipment stuffs*/
		gfx_SetColor(COLOR_GUNMETALGRAY);
		gfx_FillRectangle_NoClip(CURGEAR_X,CURGEAR_Y,84,34);
		gfx_FillRectangle_NoClip(QUICKSET_X,QUICKSET_Y,88,50);
		gfx_Sprite_NoClip(digitbar_top,QUICKSET_X+6,QUICKSET_Y+1);
		gfx_Sprite_NoClip(digitbar_btm,QUICKSET_X+5,QUICKSET_Y+43);
		/* the minimap area */
		gfx_SetColor(COLOR_PURPLE);
		gfx_Rectangle_NoClip(MINIMAP_X,MINIMAP_Y,66,66);
		gfx_Sprite_NoClip(areagfx,MAPAREA_X,MAPAREA_Y);
		gfx_Sprite_NoClip(floorgfx,MAPFLOOR_X,MAPFLOOR_Y);
	}
	if (u|UPD_XP) {
		//Draw bar at XPFOOD_X+2,XPFOOD_Y+2, w80,h9. COLOR_YELLOW
		gfx_SetTextFGColor(COLOR_BLACK);
		temp = pstats.xp*100/maxlevel_table[pstats.level];
		w  = 36 + util_GetNumWidth(pstats.level+1);
		if (pstats.level<18) w += util_GetNumWidth(temp);
		gfx_SetTextXY(sidebar_center(w),XPFOOD_Y+3);
		if (pstats.level<18) {
			gfx_PrintUInt(temp,1);
			gfx_PrintChar('%');
		}
		gfx_PrintString("  LV ");
		gfx_PrintUInt(pstats.level+1,1);
	}
	if (u|UPD_FOOD) {
		//Draw bar at XPFOOD_X+2,XPFOOD_Y+21, w80,h9. COLOR_RED
		gfx_SetTextFGColor(COLOR_WHITE);
		gfx_SetTextXY(sidebar_center(util_GetNumWidth(pstats.food)),XPFOOD_Y+22);
		gfx_PrintUInt(pstats.food,1);
	}
	if (u|UPD_MINI) {
		gfx_SetTextFGColor(COLOR_WHITE);
		gfx_SetTextXY(MAPAREA_X+1,MAPAREA_Y+11);
		//Insert area text
		gfx_SetTextFGColor(COLOR_WHITE);
		gfx_SetTextXY(MAPFLOOR_X+1,MAPFLOOR_Y+11);
		//Insert floor text

	}
	if (u|UPD_HP) {
		//Draw bar at HPMP_X+2,HPMP_Y+2, w80,h9. COLOR_GREEN
		gfx_SetTextFGColor(COLOR_WHITE);
		w = 8+util_GetNumWidth(pstats.hp)+util_GetNumWidth(pstats.maxhp);
		gfx_SetTextXY(sidebar_center(w),HPMP_Y+3);
		gfx_PrintUInt(pstats.hp,1);
		gfx_PrintChar('/');
		gfx_PrintUInt(pstats.maxhp,1);
	}
	if (u|UPD_MP) {
		//Draw bar at HPMP_X+2,HPMP_Y+21, w80,h9. COLOR_BLUE
		gfx_SetTextFGColor(COLOR_WHITE);
		w = 8+util_GetNumWidth(pstats.mp)+util_GetNumWidth(pstats.maxmp);
		gfx_SetTextXY(sidebar_center(w),HPMP_Y+22);
		gfx_PrintUInt(pstats.mp,1);
		gfx_PrintChar('/');
		gfx_PrintUInt(pstats.maxmp,1);
	}
	if (u|UPD_CURGEAR) {
		//Top icon (weapon) at CURGEAR_X,CURGEAR_Y
		gfx_SetTextXY(CURGEAR_X+18,CURGEAR_Y+1);
		//Top gear text line 1 here.
		gfx_SetTextXY(CURGEAR_X+18,CURGEAR_Y+1+8);
		//Top gear text line 2 here.
		
		//Bottom icon (auxillary) at CURGEAR_X,CURGEAR_Y+18
		gfx_SetTextXY(CURGEAR_X+18,CURGEAR_Y+19);
		//Bottom gear text line 1 here.
		gfx_SetTextXY(CURGEAR_X+18,CURGEAR_Y+19+8);
		//Bottom gear text line 2 here.
	}
	if (u|UPD_QUICKSET) {
		//Row 1 starting at QUICKSET_X,QUICKSET_Y+8
		//Row 2 starting at QUICKSET_X,QUICKSET_X+8+16+2
		//Equipment X-spacing at 18px intervals
	}
	
	
	pstats.updateprev = pstats.update;
	pstats.update = 0;
	return;
}

















uint8_t util_GetNumWidth(int num) {
	if (num<10) 	return (8*1);
	if (num<100) 	return (8*2);
	if (num<1000) 	return (8*3);
	if (num<10000) 	return (8*4);
	if (num<100000) return (8*5);
	return 48;	//We will not be displaying wider numbers.
}


void sys_filenotfound(uint8_t filenum) {
	gfx_FillScreen(0);
	gfx_SetTextFGColor(0xE0);
	gfx_PrintStringXY("Fatal error: File RoFoDAT",8,8);
	gfx_PrintUInt(filenum,1);
	gfx_PrintString(" not found.");
	gfx_PrintStringXY("Press any key to exit.",8,20);
	gfx_SwapDraw();
	while (!kb_AnyKey());
	gfx_End();
	exit(0);
}

void game_Initialize(void) {
	gfx_Begin();
	srand(rtc_Time());
	
	gfx_SetDrawScreen();	//Drawing on screen buffer our room things.
	gfx_SetTransparentColor(COLOR_TRANS);
	gfx_SetTextTransparentColor(COLOR_TRANS);
	gfx_SetTextBGColor(COLOR_TRANS);
	gfx_SetTextFGColor(COLOR_WHITE);
	gfx_SetDrawBuffer();
	gfx_FillScreen(0);
	gfx_SwapDraw();
	gfx_FillScreen(0);
	//Game data init
	
	if (!RoFoDAT0_init()) sys_filenotfound(0);
	if (!RoFoDAT1_init()) sys_filenotfound(1);
	if (!RoFoDAT2_init()) sys_filenotfound(2);
	if (!RoFoDAT3_init()) sys_filenotfound(3);
	
	gfx_SetPalette(base_pal,sizeof_base_pal,ui_palette_offset);
	curmap = gfx_MallocSprite(128,128);
	
	tilemap.map = (((uint8_t*)curmap)+2);
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
}

