/* Testing for level generation */
/* First compile size: 1153 bytes */

#include <string.h>

#include <graphx.h>
#include <debug.h>
#include <keypadc.h>
#include <compression.h>
#include <fileioc.h>

#include "defs.h"
#include "main.h"
#include "dgen.h"
#include "sobjs.h"
#include "items.h"
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
floordat_t floordat;

/* Dungeon graphics management */
int totalgens;					//Number of times a room generator was run
gfx_sprite_t* curmap;			//Init to 128,128
gfx_tilemap_t tilemap;
uint8_t tile2color[513];		//For tilemap to minimap conversion. Maybe-collisiondetect?

uint8_t	numsobjs;
sobj_t	sobjs[255];
uint8_t	nummobjs;
mobj_t	mobjs[255];

sobj_t empty_sobj;
mobj_t empty_mobj;
item_t empty_item;
gfx_sprite_t *empty_tile;
gfx_sprite_t **main_tilemap;

item_t inventory[64];
item_t equipment[8];		//rng1,rng2, hat, armor, glov, prim, boots, secnd
item_t quickbar[10];
item_t secondary;



/* Function prototypes goes here */
void disp_ShowSidebar(void);
void disp_ShowInventory(uint8_t state);
uint8_t util_GetNumWidth(int num);	//Returns text width of digits were it displayed
void game_Initialize(void);


int main(void) {
	int sec,subsec;
	uint8_t k,kc;
	uint8_t xpos,ypos,x,y;
	uint8_t *ptr,t,moving,facing,cycle;
	int px,py,cx,cy;
	
	game_Initialize();
	
	/* Testing conditions */
	gen_TestDungeon(20); //Sets positioning
	gfx_SwapDraw();
	pstats.update = UPD_SIDEBAR;
	dbg_sprintf(dbgerr,"Curmap location %X\n",&curmap);
	facing = 0;
	
	while (1) {
		kb_Scan();
		k = kb_Data[7];
		kc = kb_Data[1];
		if (kc & kb_Mode) break;
		
		if (kc & kb_Yequ)			disp_ShowInventory(0);
		else if (kc & kb_Window)	disp_ShowInventory(1);
		else if (kc & kb_Zoom)		disp_ShowInventory(3);
		else if (kc & kb_Trace)		disp_ShowInventory(4);
		else if (kc & kb_Graph)		disp_ShowInventory(5);

		//if (!k) continue;
		
		xpos = pstats.x;
		ypos = pstats.y;
		
		cycle = 0;
		if (( k & kb_Left) && (xpos>0)) 	{ --xpos; facing=8;  }
		if (( k & kb_Right) && (xpos<127))	{ ++xpos; facing=4;  }
		if (( k & kb_Up)   && (ypos>0)) 	{ --ypos; facing=12; }
		if (( k & kb_Down) && (ypos<127))  	{ ++ypos; facing=0;  }
		
		/*	Check for collision. If a collision has occurred, check what you ran
			into to perform an action (or no action if one cannot occur. e.g. wall)
		*/
		t = curmap->data[ypos*128+xpos];
		moving = 1;		//Set to zero if movement is canceled.
		//Check if wall collide. If so, cancel movement.
		if (t<0x40) {
			//moving = 0;
		}
		
		
		

		
		if (moving) {
			/*	You'd insert a loop here to see change in xpos/ypos in pstats.x
				and pstats.y, scrolling pstats.subx and pstats.suby in the process.
				Updating the sidebar need not occur until after the animation
				sequence has concluded. */
			//
		} else {
			//Do not try to optimize away. xpos/ypos still needed below.
			xpos = pstats.x;
			ypos = pstats.y;
		}
		pstats.x = xpos;
		pstats.y = ypos;
		
		x = xpos-7;
		y = ypos-7;
		if (x>230) x = 0;
		else if (x>113) x = 113;
		if (y>230) y = 0;
		else if (y>113) y = 113;
		cx = x*16;
		cy = y*16;
		
		gfx_Tilemap_NoClip(&tilemap,cx,cy);
		
		px = xpos*16+4;
		py = ypos*16+4;
		gfx_SetClipRegion(4,4,4+224,4+224);
		gfx_TransparentSprite(player0_tiles[facing+(cycle&3)],px-cx,py-cy);
		gfx_SetClipRegion(0,0,320,240);
		
		
		
		disp_ShowSidebar();
		asm_LoadMinimap(xpos,ypos);
		/* Testing */
		gfx_SetColor(COLOR_BLACK);
		gfx_SetTextFGColor(COLOR_WHITE);
		gfx_FillRectangle(4,LCD_HEIGHT-10,200,8);
		gfx_SetTextXY(4,LCD_HEIGHT-10);
		gfx_PrintUInt(xpos,3);
		gfx_PrintString(", ");
		gfx_PrintUInt(ypos,3);
		gfx_PrintString(" -- dbg --");
		gfx_PrintUInt(numsobjs,1);
		
		gfx_SwapDraw();
	}
	
	gfx_End();
    return 0;
}



#define sidebar_center(strwidth) ((((LCD_WIDTH-SBAR_LEFT)-strwidth)/2)+SBAR_LEFT)
void disp_ShowSidebar(void) {
	uint8_t u;
	uint8_t y;
	uint8_t w,sw,t,i;
	int x;
	int temp;
	void *ptr;
	
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
		t = equipment[5].type;
		if (!t)	ptr = equipicons_tiles[6];
		else	ptr = items_GetItemSprite(t);
		gfx_Sprite_NoClip(ptr,CURGEAR_X,CURGEAR_Y);
		gfx_SetTextXY(CURGEAR_X+18,CURGEAR_Y+1);
		items_PrintItemname_Left(t);
		
		t = equipment[7].type;
		if (!t)	ptr = equipicons_tiles[8];
		else	ptr = items_GetItemSprite(t);
		gfx_Sprite_NoClip(ptr,CURGEAR_X,CURGEAR_Y+18);
		gfx_SetTextXY(CURGEAR_X+18,CURGEAR_Y+19);
		items_PrintItemname_Left(t);
	}
	if (u|UPD_QUICKSET) {
		//Quickbar
		for (i=0; i<10; ++i) {
			x = QUICKSET_X + 18*(i%5);
			y = QUICKSET_Y + 8 + 18*(i/5);
			t = quickbar[i].type;
			if (!t)	ptr = equipicons_tiles[i+10];	//skip over top row
			else	ptr = items_GetItemSprite(t);
			gfx_Sprite_NoClip(ptr,x,y);
		}		
		//Row 1 starting at QUICKSET_X,QUICKSET_Y+8
		//Row 2 starting at QUICKSET_X,QUICKSET_X+8+16+2
		//Equipment X-spacing at 18px intervals
	}
	
	
	pstats.updateprev = pstats.update;
	pstats.update = 0;
	return;
}

void disp_printstatplus(char *s, uint8_t base, uint8_t plus) {
	gfx_PrintString(s);
	if (base) {
		gfx_PrintUInt(base,2);
		if (plus) {
			gfx_PrintChar('+');
			gfx_PrintUInt(plus,2);
		}
	}
	gfx_SetTextXY(6,gfx_GetTextY() + 12);
}

void disp_ShowInventory(uint8_t state) {
	uint8_t icursor,scursor,update;
	kb_key_t sk,k,pk;
	uint8_t temp,gx,gy,i,t;
	int x,y;
	void *ptr;
	
	gfx_BlitScreen(gfx_buffer);
	icursor = scursor = pk = k = 0;
	update = 1;
	while (1) {
		kb_Scan();
		k = kb_Data[1];
		switch (k) {
			case kb_Yequ:	state = 0; update = 1; break;
			case kb_Window:	state = 1; update = 1; break;
			case kb_Zoom:	state = 2; update = 1; break;
			case kb_Trace:	state = 3; update = 1; break;
			case kb_Graph:	state = 4; update = 1; break;
			default: break;
		}
		k = (kb_Data[1] & 0xE0) | kb_Data[7];	//Combine dpad and del/mode/2nd
		sk = (k ^ pk) & k;	//read changes to key state, but only on press.
		pk = k;				//After that, save actual key state for next run
		if (k&kb_Mode) break;
		if (k|update) {
			/* Backer */
			if ((k&kb_Down) && (icursor < 30)) icursor += 5;
			if ((k&kb_Up)   && (icursor > 4 )) icursor -= 5;
			if ((k&kb_Right)&& (icursor < 34)) icursor += 1;
			if ((k&kb_Left) && (icursor > 0))  icursor -= 1;
			
			
			
			gfx_SetColor(COLOR_GUNMETALGRAY);
			gfx_FillRectangle_NoClip(4,4,224,224);
			gfx_SetTextFGColor(COLOR_WHITE);
			gfx_SetTextXY(6,10);
			if (!state) {
				disp_printstatplus("STR: ",pstats.strength,0);
				disp_printstatplus("AGI: ",pstats.speeds,0);
				disp_printstatplus("INT: ",pstats.smarts,0);
				disp_printstatplus("---------",0,0);
				disp_printstatplus("ATK: ",pstats.attack,0);
				disp_printstatplus("DEF: ",pstats.defense,0);
				disp_printstatplus("EVA: ",pstats.evasion,0);
				disp_printstatplus("SNK: ",pstats.sneaks,0);
				disp_printstatplus("MAG: ",pstats.magic,0);
				disp_printstatplus("---------",0,0);
				disp_printstatplus("M res: ",pstats.mres,0);
				disp_printstatplus("F res: ",pstats.fres,0);
				disp_printstatplus("L res: ",pstats.eres,0);
				disp_printstatplus("P res: ",pstats.pres,0);
				disp_printstatplus("Rflct: ",pstats.reflect,0);
				disp_printstatplus("---------",0,0);
				disp_printstatplus("S Pts: ",pstats.statpoints,0);
				disp_printstatplus("T Pts: ",pstats.talentpoints,0);
				gfx_SetColor(COLOR_DARKGRAY);
				gfx_FillRectangle(123+4,6,4,77);	//vertical divider
				gfx_FillRectangle(127+4,47,95,4);	//quick/inv divider
				gfx_FillRectangle(82+4,79,45,4);	//equip/packs divider
				
				//Equipment
				for (i=0; i<8; ++i) {
					x = 4+80+4 + 18*(i&1);
					y = (i>>1) * 18 + 4+4;
					t = equipment[i].type;
					if (!t)	ptr = equipicons_tiles[i+1];	//skip over blank default
					else	ptr = items_GetItemSprite(t);
					gfx_Sprite_NoClip(ptr,x,y);
				}
				//Quickbar
				for (i=0; i<10; ++i) {
					x = 4+80+2+36+14 + 18*(i%5);
					y = 4+2+ 18*(i/5);
					t = quickbar[i].type;
					if (!t)	ptr = equipicons_tiles[i+10];	//skip over top row
					else	ptr = items_GetItemSprite(t);
					gfx_Sprite_NoClip(ptr,x,y);
				}
				//Inventory
				for (i=0; i<35; ++i) {
					x = 4+80+2+36+14 + 18*(i%5);
					y = 4+2+36+14+ 18*(i/5);
					t = inventory[i].type;
					if (!t)	ptr = equipicons_tiles[0];
					else	ptr = items_GetItemSprite(t);
					if (i==34) ptr = equipicons_tiles[9]; //trashcan
					gfx_Sprite_NoClip(ptr,x,y);
				}
				//Should put inventory box selector here. Or something else.
				x = 4+80+2+36+14 + 18*(icursor%5);
				y = 4+2+36+14+ 18*(icursor/5);
				gfx_HorizLine_NoClip(x,y-1,16);		//top line
				gfx_HorizLine_NoClip(x,y+16,16);	//bottom line
				gfx_VertLine_NoClip(x-1,y,16);		//left line
				gfx_VertLine_NoClip(x+16,y,16);		//Right line
				
				gfx_SetColor(COLOR_BLACK);
				gfx_FillRectangle_NoClip(0,228,320,12);
				gfx_SetTextFGColor(COLOR_WHITE);
				gfx_SetTextXY(4,230);
				items_PrintItemname_Bottom(inventory[icursor].type);
				gfx_PrintString(" :: ");
				gfx_PrintString(items_GetItemDesc(inventory[icursor].type));
				
				
			} else if (state == 1) {
				disp_printstatplus("Pure stats page.",0,0);
				disp_printstatplus("Nothing here yet.",0,0);
			} else {
				disp_printstatplus("idk what will go here.",0,0);
				disp_printstatplus("No plans yet.",0,0);
			}
			gfx_SwapDraw();
			update = 0;
		}
	}
	while (kb_AnyKey());
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
	uint8_t *ptr,j;
	int i;
	
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
	empty_tile = gfx_MallocSprite(16,16);
	for (i=0,ptr=empty_tile->data; i<256; ++i,++ptr) *ptr = COLOR_RED;
	
	main_tilemap = malloc(sizeof(gfx_sprite_t*)*256);
	for (i=0; i<256; ++i) main_tilemap[i] = empty_tile;
	
	gfx_SetPalette(charequtiles_pal,sizeof_charequtiles_pal,charequtiles_palette_offset);
	gfx_SetPaletteEntry(charequtiles_palette_offset,gfx_RGBTo1555(128,160,160)); //gunmetal gray
	
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
	tilemap.tiles		= main_tilemap;
	
	for (j=0,i=1;i<64;i+=4,++j) {
		inventory[j].type = i;
		inventory[j].data = 0;
	}
	for (i=ITEM_CONSUMABLE; i < CONS_UNUSEDPOTION1 ; ++i,++j) {
		inventory[j].type = i;
		inventory[j].data = 1;
	}
	
	equipment[0].type = EQU_SNEAKRING;
	equipment[0].data = 0;
	equipment[1].type = EQU_RAWRSRING;
	equipment[1].data = 0;
	equipment[2].type = EQU_HAT;
	equipment[2].data = 0;
	equipment[3].type = EQU_LEATHERARMOR;
	equipment[3].data = 0;
	equipment[4].type = EQU_BADTOUCHER;
	equipment[4].data = 0;
	equipment[5].type = EQU_SHADOWSWORD;
	equipment[5].data = 0;
	equipment[6].type = EQU_NINJABOOTS;
	equipment[6].data = 0;
	equipment[7].type = EQU_REFLECTSHIELD;
	equipment[7].data = 0;
	
	
	
}

