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
#include "mobjs.h"
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
uint8_t forestmap[25];		//Exitmap for foresting. Trailing entry indicates starting tile
uint8_t forestdungeon[6];	//Locations in forest of dungeons

/* TEST MAP S=START, C=CHERRY
/-+-+-+-+-\
|S    |   |
| +-+ + + +
| |   | | |
| +-+-+ + +
| | |C  | |
| +-+-+-+ +
|   | |   |
|-+ + + +-+
|         |
\-+-+-+-+-/
*/

uint8_t forestmap_test_start = 0x01;
uint8_t forestmap_test[] = {
	FEX_EAST|FEX_SOUTH|FEX_EXIT,
	FEX_EAST|FEX_WEST,
	FEX_WEST|FEX_SOUTH,
	FEX_SOUTH|FEX_EAST,
	FEX_WEST|FEX_SOUTH,
	
	FEX_NORTH|FEX_SOUTH,
	FEX_EAST,
	FEX_WEST|FEX_NORTH,
	FEX_NORTH|FEX_SOUTH,
	FEX_NORTH|FEX_SOUTH,
	
	FEX_NORTH|FEX_SOUTH,
	FEX_SOUTH,
	FEX_EAST,
	FEX_NORTH|FEX_WEST,
	FEX_NORTH|FEX_SOUTH,
	
	FEX_NORTH|FEX_EAST,
	FEX_WEST|FEX_NORTH,
	FEX_SOUTH,
	FEX_SOUTH|FEX_EAST,
	FEX_WEST|FEX_NORTH,
	
	FEX_EAST,
	FEX_WEST|FEX_NORTH|FEX_EAST,
	FEX_WEST|FEX_NORTH|FEX_EAST,
	FEX_WEST|FEX_NORTH|FEX_EAST,
	FEX_WEST
};

uint8_t forestdungeon_test[] = {
	0x01,0x02,0x03,0x04,0x05,0x0D
};

/* Function prototypes goes here */
void disp_ShowSidebar(void);
void disp_ShowInventory(uint8_t state);
uint8_t util_GetNumWidth(int num);	//Returns text width of digits were it displayed
void game_Initialize(void);


int main(void) {
	int sec,subsec;
	uint8_t k,kc;
	uint8_t xpos,ypos,x,y,i,color;
	uint8_t *ptr,t,moving,facing,cycle,ecycle;
	uint8_t enemycollide,sobjcollide;
	int px,py,ex,ey,cx,cy;
	mobj_t *mobj;
	sobj_t *sobj;
	uint8_t sobj_maintype;
	
	
	game_Initialize();
	
	/* Testing conditions */
	mobj_newchar();		//Generates overworld map and warps into first floor
	gfx_FillScreen(0);
	gfx_SwapDraw();
	gfx_FillScreen(0);
	pstats.update = UPD_SIDEBAR;
	dbg_sprintf(dbgerr,"Curmap location %X\n",&curmap);
	ecycle = facing = 0;
	
	while (1) {
		kb_Scan();
		k = kb_Data[7];
		kc = kb_Data[1];
		if (kc & kb_Mode) break;
		++ecycle;
		
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
		sobjcollide = enemycollide = 0;
		//Check if wall collide. If so, cancel movement.
		if (t<0x40) {
			moving = 0;
		}
		//Check if sobj collide. If so, check object and see if action needs cancel
		if (t>=0x80) {
			sobj = sobj_getentrybypos(xpos,ypos);
			if (!sobj) {
				//We hit on something that's not in the sobj table. wat.
				moving = 0;
			} else {
				sobj_maintype = sobj->type & SOBJTMSKHI;
				if (sobj_maintype == SOBJ_DOORBASE) {
					if (sobj->type & 0x80) {
						moving = 1;  //Door was already opened. Don't impede path.
					} else {
						if (sobj->type == SOBJ_DOOR) {
							sobj->type |= 0x80;	//Open the door.
							moving = 1;
						} else {
							//Door was locked or cannot be opened at this time.
							//Test later on for other door types, such as if
							//you have keys that can open this door.
							moving = 0;
						}
					}
					
				} else if (sobj_maintype == SOBJ_WARPBASE) {
					//Add warp handling. Mostly allow walkover with a traytip
					//indicating where the warp goes to, and to tell the user
					//to push ALPHA in order to take the warp out.
					
				} else {
					
					//Unhandled type. fill in later for other object types.
					//For now though, cancel movement.
					moving = 0;
				}
				sobj_WriteToMap();	//In case things changed. This is cheap anyway.
			}
		}
		
		//Check if enemy collide. If so, cancel movement.
		for (i=0;i<nummobjs;++i) {
			mobj = &mobjs[i];
			if ((xpos == mobj->x) && (ypos == mobj->y)) {
				enemycollide = 1;
				moving = 0;
				break;
			}
		}
		//On exit of this loop, mobj will be the enemy that you collided with
		//only if enemycollide was set to 1.
		
		

		
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
		
		//Draw final state of the enemies
		gfx_SetClipRegion(4,4,4+224,4+224);
		color = gfx_SetTransparentColor(charequtiles_palette_offset);
		for (i=0;i<nummobjs;++i) {
			mobj = &mobjs[i];
			ex = mobj->x*16 + 4;
			ey = mobj->y*16 + 4;
			gfx_TransparentSprite(*(mobj_getmobjdef(mobj)->sprobj+((ecycle&0x08)>>3)),ex-cx,ey-cy);
		}
		gfx_SetTransparentColor(color);
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

//Used only in disp_ShowInventory. Not prototyped. It probably ought to be...
item_t *disp_GetCursorAddress(uint8_t cur) {
	if ((cur&0xC0) == 0xC0)	return &quickbar[cur&0x3F];
	if ((cur&0x80) == 0x80)	return &equipment[cur&0x7F];
	else					return &inventory[cur];
}
//Used only in disp_ShowInventory. Not prototyped.
void disp_SwapInvSlots(uint8_t cur, uint8_t prv) {
	item_t t;
	
	t = *disp_GetCursorAddress(cur);
	*disp_GetCursorAddress(cur) = *disp_GetCursorAddress(prv);
	*disp_GetCursorAddress(prv) = t;
	
}
//Used only in disp_ShowInventory. Not prototyped.
uint8_t dispisslotgoodtable[] = {EFL_RNG,EFL_RNG,EFL_HAT,EFL_ARM,EFL_GLO,EFL_WPN,EFL_BTS,EFL_2ND};
uint8_t disp_IsSlotGood(uint8_t cursor) {
	uint8_t t,i;
	if (cursor&0x80) {
		cursor &= 0x7F;
		if (!(t = equipment[cursor].type)) return 1;	//Always allow emptiness.
		t = items_GetItemType(t);
		if (t == 0xFF) return 0;	//consumables not allowed in equipment slots
		if (dispisslotgoodtable[cursor] & t) return 1;
		return 0;
	} else return 1;	//Items in inventory slots are always valid
}


//Used only in disp_ShowInventory. Not prototyped.
void disp_InvCursor(uint8_t cursor) {
	uint8_t t;
	int x,y;
	
	t = cursor & 0x7F;
	if (cursor&0x80) {
		x = 4+80+2+2+ 18*(t&1);
		y = 4+2+2+    18*(t/2);
	} else {
		x = 4+80+2+36+14 + 18*(t%5);
		y = 4+2+36+14+ 18*(t/5);
	}
	gfx_HorizLine_NoClip(x,y-1,16);		//top line
	gfx_HorizLine_NoClip(x,y+16,16);	//bottom line
	gfx_VertLine_NoClip(x-1,y,16);		//left line
	gfx_VertLine_NoClip(x+16,y,16);		//Right line
}
//Used only in disp_ShowInventory. Not prototyped.
void disp_stats(char *s, uint8_t base, int plus) {
	gfx_PrintString(s);
	if (base|plus) {
		gfx_PrintUInt(base,2);
		if (plus) {
			if (plus<0) {
				gfx_PrintChar('-');
				plus = -plus;
			} else 	gfx_PrintChar('+');
			gfx_PrintUInt(plus,2);
		}
	}
	gfx_SetTextXY(6,gfx_GetTextY() + 10);
}
#define disp_diff(member) playerbase.##member##,(playercalc.##member##-playerbase.##member##)
void disp_ShowInventory(uint8_t state) {
	uint8_t cursor,prevcursor,update,cursormode;
	kb_key_t sk,k,pk,nk;
	uint8_t temp,gx,gy,i,t;
	int x,y;
	void *ptr;
	
	mobj_recalcplayer();	//MOVE THIS TO START, CLOSE, AND ON GEARCHANGE
	gfx_BlitScreen(gfx_buffer);
	cursor = prevcursor = pk = k = cursormode = 0;
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
		nk = asm_GetNumpad();
		
		//Activate only if pressed numpad in inventory while not already moving stuff
		if (nk && !state && !cursormode) {
			t = disp_GetCursorAddress(cursor)->type;
			if ((0xFF == items_GetItemType(t)) || !t) {
				//dbg_sprintf(dbgout,"Keyorg: %i\n",nk);
				disp_SwapInvSlots((nk-1)|0xC0,cursor);
				pstats.update |= UPD_CURGEAR|UPD_QUICKSET;
				update = 1;
			}
		}
		
		
		if (sk&kb_Mode) {
			if (cursormode) {
				cursor = prevcursor;
				prevcursor = cursormode = 0;
			} else break;
		}
		if (sk&kb_Del) {
			if (!state) {
				//Inventory management
				if (cursor&0x80) {
					//If equipment, try to move gear to inventory
					for (i=0; i<34; ++i) if (!inventory[i].type) break;
					if (i<34) {
						t = cursor & 0x7F;
						//An empty slot was found. Shove gear into it.
						inventory[i] = equipment[t];
						equipment[t].type = 0;
						equipment[t].data = 0;
					}
				}
			}
		}
		
		if (sk&kb_2nd) {
			if (!state) {
				//inventory mode. select/swap items in inventory
				if (!cursormode) {
					prevcursor = cursor;
					cursormode = 1;
				} else {
					//Inventory mode: Complete the transaction (if possible)
					disp_SwapInvSlots(cursor,prevcursor);
					if (disp_IsSlotGood(cursor) & disp_IsSlotGood(prevcursor)) {
						prevcursor = cursormode = 0;
						mobj_recalcplayer();	//MOVE THIS TO START, CLOSE, AND ON GEARCHANGE
						inventory[34].type = 0;
						inventory[34].data = 0; //Items moved to trashcan are deleted.
						pstats.update |= UPD_CURGEAR|UPD_QUICKSET;
					} else disp_SwapInvSlots(cursor,prevcursor); //undo if failed
				}
			} else;
		}
		if (sk|update) {
			t = cursor & 0x7F;
			if (cursor&0x80) {
				//Cursor in equipment
				if ((sk&kb_Down) && (t<6)) cursor += 2;
				if ((sk&kb_Up)   && (t>1)) cursor -= 2;
				if ((sk&kb_Left) && (t&1)) --cursor;
				if (sk&kb_Right) {
					//If maybe leaving equipment
					if (t&1)		cursor = 0;	//leaving: set to first inv slot
					else			++cursor;	//staying
				}
			} else {
				//cursor in inventory
				t = t%5;
				if ((sk&kb_Down) && (cursor < 30))	cursor += 5;
				if ((sk&kb_Up)   && (cursor > 4 ))	cursor -= 5;
				if ((sk&kb_Right) && (t<4))			++cursor;
				if (sk&kb_Left) {
					//If maybe leaving inventory
					if (t)	--cursor;
					else	cursor = 0x80+7;
				}
			}
			
			/* Backer */
			gfx_SetColor(COLOR_GUNMETALGRAY);
			gfx_FillRectangle_NoClip(4,4,224,224);
			gfx_SetTextFGColor(COLOR_WHITE);
			gfx_SetTextXY(6,10);
			if (!state) {
				disp_stats("STR: ",disp_diff(str));
				disp_stats("AGI: ",disp_diff(spd));
				disp_stats("INT: ",disp_diff(smrt));
				gfx_SetTextXY(6,gfx_GetTextY() + 4);
				disp_stats("ATK: ",disp_diff(atk));
				disp_stats("SNK: ",disp_diff(snk));
				disp_stats("RWR: ",disp_diff(rawrs));
				disp_stats("DEF: ",disp_diff(def));
				disp_stats("BLK: ",disp_diff(blk));
				disp_stats("REF: ",disp_diff(refl));
				gfx_SetTextXY(6,gfx_GetTextY() + 4);
				disp_stats("MATK: ",disp_diff(matk));
				disp_stats("FATK: ",disp_diff(fatk));
				disp_stats("EATK: ",disp_diff(eatk));
				disp_stats("PATK: ",disp_diff(patk));
				gfx_SetTextXY(6,gfx_GetTextY() + 4);
				disp_stats("MDEF: ",disp_diff(mdef));
				disp_stats("FDEF: ",disp_diff(fdef));
				disp_stats("EDEF: ",disp_diff(edef));
				disp_stats("PDEF: ",disp_diff(pdef));				
				gfx_SetTextXY(6,gfx_GetTextY() + 4);
				disp_stats("S Pts: ",pstats.statpoints,0);
				disp_stats("T Pts: ",pstats.talentpoints,0);
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
					y = 4+4+ 18*(i/5);
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
				gfx_SetColor(COLOR_PURPLE);
				if (cursormode) {
					disp_InvCursor(prevcursor);
					gfx_SetColor(COLOR_ORANGE);
				}
				disp_InvCursor(cursor);
				
				gfx_SetColor(COLOR_BLACK);
				gfx_FillRectangle_NoClip(0,228,320,12);
				gfx_SetTextFGColor(COLOR_WHITE);
				gfx_SetTextXY(4,230);
				
				if (cursor&0x80)	t = equipment[cursor&0x7F].type;
				else				t = inventory[cursor].type;
				items_PrintItemname_Bottom(t);
				gfx_PrintString(" :: ");
				gfx_PrintString(items_GetItemDesc(t));
				
				
			} else if (state == 1) {
				disp_stats("Pure stats page.",0,0);
				disp_stats("Nothing here yet.",0,0);
			} else {
				disp_stats("idk what will go here.",0,0);
				disp_stats("No plans yet.",0,0);
			}
			disp_ShowSidebar();
			gfx_SwapDraw();
			update = 0;
		}
	}
	while (kb_AnyKey());
	mobj_recalcplayer();	//MOVE THIS TO START, CLOSE, AND ON GEARCHANGE
	return;
}








char numbuf[9];

char *util_BufInt(int num) {
	uint8_t i,j,slen,lim,sign;
	char *ptr,c;
	
	//Record sign of input num
	if (num<0) {
		sign = 1;
		num = -num;
	} else sign = 0;
	
	//Get digits in reverse order, guaranteeing at least '0' being printed.
	ptr = numbuf;
	do {
		*ptr = (num % 10) + '0';
		num  = num / 10;
		++ptr;
	} while (num);
	//Append sign either '+' or '-'
	c = (sign) ? '-' : '+';
	*ptr = c;
	++ptr;
	
	//Cap off the number buffer and then reverse the characters in it
	*ptr = 0;
	slen = strlen(numbuf);
	lim = slen/2+slen%2;
	for (i = 0, j = slen-1; i<lim; ++i, --j) {
		c = numbuf[i];
		numbuf[i] = numbuf[j];
		numbuf[j] = c;
	}
	return numbuf;
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
	
}

