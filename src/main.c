/* Testing for level generation */
/* First compile size: 1153 bytes */



#include <graphx.h>
#include <debug.h>
#include <keypadc.h>
#include <compression.h>
#include <fileioc.h>

#include "defs.h"
#include "gfx/output/gfx_ui.h"
#include "gfx/output/gfx_dun0.h"


/* Display data updating and management */
struct pstats_st {
	uint8_t update,updateprev;
	uint8_t x,y;
	uint8_t subx,suby;
	uint8_t forestarea;
	uint8_t dungeonid;
	uint8_t dungeonfloor;
	uint8_t level;
	int		xp,maxxp;
	int		food,maxfood;
	uint8_t hp,maxhp;
	uint8_t mp,maxmp;
	uint8_t	attack,defense,agility,magic,rawrs;
	
	
	
};
struct pstats_st pstats;

int maxlevel_table[] = {
	20,		40,		60,		80,		120,
	200,	300,	450,	800,	1200,
	1800,	3000,	4500,	7000,	9200,
	13000,	18000,	25000,	34000,	60000
};



/* ROOMS ROOMS ROOMS ROOMS ROOMS ROOMS */
typedef struct room_st {
	uint8_t type,x,y,w,h;
} room_t;
uint8_t numrooms;				//Number of rooms
room_t roomlist[NUMROOMS_MAX];	//Rooms of type 0 are not used/read

/* Dungeon graphics management */
int totalgens;					//Number of times a room generator was run
gfx_sprite_t* curmap;			//Init to 128,128
gfx_tilemap_t tilemap;
uint8_t tile2color[513];		//For tilemap to minimap conversion. Maybe-collisiondetect?



/* Function prototypes goes here */


void gen_TestDungeon(uint8_t roomdensity);
void gen_ResetAll(void);
void gen_SetMinimapColors(uint8_t *tile2col, uint8_t sizeof_list);
void gen_BufToTilemap(void);
void disp_ShowSidebar(void);
uint8_t util_GetNumWidth(int num);	//Returns text width of digits were it displayed
uint8_t	*asm_SetTile2ColorStart(void);
void asm_LoadMinimap(uint8_t xpos, uint8_t ypos);
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














//index corresponds to tile ID, value is color to display.
//Set main with gen_SetMinimapColors
uint8_t dun0_tile2color[] = {COLOR_WHITE,COLOR_BLACK};
/* Randomly places rooms around the map, then makes paths between them */
void gen_TestDungeon(uint8_t roomdensity)  {
	uint8_t i,j,k,iscollide,x,y,w,h,x2,y2,t,mx,my,nx,ny;
	room_t *room;
	
	if (roomdensity<3) roomdensity = 3;
	//Ensure at least two rooms are generated.
	do {
		gen_ResetAll();
		
		//Attempt to generate number of rooms equal to roomdensity
		for (i = 0; i < roomdensity; ++i) {
			w = randInt(5,20);
			h = randInt(5,20);
			x = randInt(0,127-w);
			y = randInt(0,127-h);
			iscollide = 0;
			for (j = 0; j < NUMROOMS_MAX; ++j) {
				room = &roomlist[j];
				if (room->type) {
					iscollide |=
					gfx_CheckRectangleHotspot(x,y,w,h,room->x,room->y,room->w,room->h);
				} else break;
			}
			if (!iscollide) {
				//dbg_sprintf(dbgout,"Roomgen %i - (%i,%i,%i,%i)\n",numrooms,x,y,w,h);
				room = &roomlist[numrooms];
				room->type = 1;
				room->x = x;
				room->y = y;
				room->w = w;
				room->h = h;
				++numrooms;
				if ((numrooms+1) == NUMROOMS_MAX) break;	//stop generating.
			}
		}
	} while (numrooms < 3);
	gfx_SetColor(1);	//walls
	//Write rooms to the grid
	for (i = 0; i < numrooms ; ++i) {
		//dbg_sprintf(dbgout,"Room grid ID %i\n",i);
		room = &roomlist[i];
		if (!room->type) continue; //guard
		gfx_FillRectangle_NoClip(room->x,room->y,room->w,room->h);
		//Provide one or two paths out of the current room
		for (j = randInt(1,2); j; --j) {
			//dbg_sprintf(dbgout,"Path return ID %i\n",j);
			//Ensure the random room to path to is not itself.
			while ((k=randInt(0,(numrooms-1))) == i);
			//Select random point inside of room
			room = &roomlist[i];
			//dbg_sprintf(dbgout,"Path item A-(%i) dims: (%i,%i,%i,%i)\n",i,room->x,room->y,room->w,room->h);
			x = room->x + randInt(0,(room->w - 2));
			y = room->y + randInt(0,(room->h - 2));
			//Select random point inside other room
			room = &roomlist[k];
			//dbg_sprintf(dbgout,"Path item B-(%i) dims: (%i,%i,%i,%i)\n",i,room->x,room->y,room->w,room->h);
			x2 = room->x + randInt(0,(room->w - 2));
			y2 = room->y + randInt(0,(room->h - 2));
			//dbg_sprintf(dbgout,"Path return coords: (%i,%i) - (%i,%i)\n",x2,y2,x,y);
			
			if (randInt(0,1)) {
				//Vertical-first
				gfx_Line_NoClip(x, y, x,y2);
				gfx_Line_NoClip(x2,y2,x,y2);
			} else {
				//Horizontal-first
				gfx_Line_NoClip(x, y, x2,y );
				gfx_Line_NoClip(x2,y2,x2,y );
			}
		}
	}
	
	/* Use gen_BufToTilemap to do the buffering things */
	gen_BufToTilemap();
	gen_SetMinimapColors(dun0_tile2color,2);
	/* Place dungeon interpolation logic below */
	
	
	
	
	
}



void gen_ResetAll(void) {
	uint8_t i;
	
	for (i = 0; i < NUMROOMS_MAX; ++i) {
		roomlist[i].type = 0;
	}
	for (i=0; i<255; ++i) {
		tile2color[i] = 0;
	}
	numrooms = 0;
	gfx_FillScreen(0x00);
	++totalgens;
}


/* 	I'm not entirely sure what kind of transform I'd be doing here, but the generator
	really ought to correspond to the tilemap itself. So far, we've gotten lucky due
	to alignment and probable clamping on the side of the tilemapper */
void gen_BufToTilemap(void) {
	uint8_t x,y,*ptr;
	gfx_GetSprite(curmap,0,0);
	ptr = (uint8_t*)curmap;
	ptr += 2;
	for(y=0; y<128; ++y) {
		for(x=0; x<128; ++x) {
			//*ptr = *ptr + 16;
		}
	}
}
void gen_SetMinimapColors(uint8_t *tile2col, uint8_t sizeof_list) {
	uint8_t i,*ptr1,*ptr2;
	
	ptr1 = asm_SetTile2ColorStart();
	ptr2 = tile2col;
	
	for (i = 0; i<sizeof_list; ++i,++ptr1,++ptr2) {
		*ptr1 = *ptr2;
	}
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
	curmap = gfx_MallocSprite(128,128);
	//minimap = gfx_MallocSprite(64,64);
	gfx_SetPalette(base_palette,sizeof_base_palette,ui_palette_offset);
	gfx_SetPalette(dungeon0_pal,sizeof_dungeon0_pal,dungeon0_palette_offset);
	
	tilemap.map = (((uint8_t*)curmap)+2);
	tilemap.tiles = dungeon0_tiles;
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

uint8_t util_GetNumWidth(int num) {
	if (num<10) 	return (8*1);
	if (num<100) 	return (8*2);
	if (num<1000) 	return (8*3);
	if (num<10000) 	return (8*4);
	if (num<100000) return (8*5);
	return 48;	//We will not be displaying wider numbers.
}
