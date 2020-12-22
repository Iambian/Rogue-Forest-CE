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

void disp_drawquickbar(int basex, uint8_t basey);

/* =========================================================================== */

char *disp_title_menu[] = {"New Game","Load Game","Credits","Quit"};

uint8_t disp_Title(uint8_t key) {
	uint8_t static curpos;
	uint8_t i,j,k,y,c;
	char *s;
	int x;
	
	if (key&kbit_Up)	--curpos;
	if (key&kbit_Down)	++curpos;
	curpos &= 3;
	if (key&kbit_2nd)	return 1+curpos;	//1=new,2=load,3=cred,4=quit
	if (key&kbit_Mode)	return 4;
	
	gfx_FillScreen(COLOR_FORESTGREEN);
	zx7_Decompress(gfx_vbuffer,rofotitle3_compressed);
	gfx_SetTextScale(2,2);
	for (i=0,y=100; i<4; ++i,y+=24) {
		s = disp_title_menu[i];
		gfx_SetTextFGColor((i == curpos) ? COLOR_GOLD : COLOR_GRAY);
		gfx_PrintStringXY(s,(320-gfx_GetStringWidth(s))/2,y);
	}
	gfx_SetTextFGColor(COLOR_WHITE);
	gfx_SetTextScale(1,1);
	gfx_PrintStringXY(VERSION,LCD_WIDTH-gfx_GetStringWidth(VERSION),LCD_HEIGHT-10);

	
	return 0;	//Always report title
}


/* =========================================================================== */

char *disp_credits_top = "Credits";

void disp_creditentry(void **spr, char *s, uint8_t timer) {
	uint8_t y,color;
	int x;
	y = gfx_GetTextY();
	x = 16;
	gfx_SetTextXY(x+20,y+4);
	gfx_TransparentSprite_NoClip(*(spr+((timer>>4)&1)),x,y);
	gfx_PrintString(s);
	gfx_SetTextXY(x,y+24);

}

uint8_t disp_Credits(uint8_t key) {
	static uint8_t timer;
	uint8_t color;
	
	++timer;
	
	gfx_FillScreen(COLOR_FORESTGREEN);
	gfx_SetTextFGColor(COLOR_GOLD);
	gfx_SetTextScale(3,3);
	gfx_PrintStringXY(disp_credits_top,(320-gfx_GetStringWidth(disp_credits_top))/2,10);
	
	color = gfx_SetTransparentColor(charequtiles_palette_offset);
	
	gfx_SetTextFGColor(COLOR_LIGHTGRAY);
	gfx_SetTextScale(1,1);
	gfx_SetTextXY(0,70);
	disp_creditentry(S_LIZMAN,"Iambian -- CFD/Game programmer",timer);
	disp_creditentry(S_LIGHTS,"Geekboy1011 -- Manager and Sane One",timer);
	disp_creditentry(S_ANGEL,"Jacobly -- CE Toolchain",timer);
	disp_creditentry(S_REAPER,"MateoConLechuga -- Toolchain/ConvImg",timer);
	disp_creditentry(S_HIDDEN,"DragonDePlatino -- DawnLike tileset",timer);
	
	gfx_SetTransparentColor(color);
	
	if (key)	return GS_TITLE;
	else		return GS_CREDITS;
}

/* =========================================================================== */
#define SBAR_LEFT	(4+16*14+4)
#define SBAR_TOP	2
#define XPFOOD_X	(SBAR_LEFT+2)
#define XPFOOD_Y	(SBAR_TOP+2)
//--
#define MINIMAP_X	(SBAR_LEFT+1)
#define MINIMAP_Y	(XPFOOD_Y+32+2)
#define MAPAREA_X	(MINIMAP_X+67)
#define MAPAREA_Y	(MINIMAP_Y+1)
#define MAPFLOOR_X	(MAPAREA_X)
#define MAPFLOOR_Y	(MAPAREA_Y+28)
//--
#define HPMP_X		(SBAR_LEFT+2)
#define HPMP_Y		(MINIMAP_Y+66+2)
#define CURGEAR_X	(SBAR_LEFT+2)
#define CURGEAR_Y	(HPMP_Y+32+3)
#define QUICKSET_X	(SBAR_LEFT+0)
#define QUICKSET_Y	(CURGEAR_Y+34+3)


void disp_sidebarbar(int small, int large, uint8_t color, uint8_t y, char *s) {
	int w1,w2;
	if (large)	w1 = (small * 80) / large;
	else		w1 = 0;		//div by zero? show no bar.
	if (w1>80)	w1 = 80;	//clamp to full bar if bar should be overfull
	w2 = 80 - w1;
	gfx_SetColor(color);
	gfx_FillRectangle_NoClip(HPMP_X+2,y,w1,9);
	gfx_SetColor(COLOR_DARKGRAY);
	gfx_FillRectangle_NoClip(HPMP_X+2+w1,y,w2,9);
	gfx_PrintStringXY(s,(HPMP_X+2)+(80-gfx_GetStringWidth(s))/2,y+1);
}

void disp_sidebarbarhpmp(int lo, int hi, uint8_t color, uint8_t y) {
	util_BufClr();
	util_BufInt(lo,1);
	util_BufChr('/');
	util_BufInt(hi,1);
	disp_sidebarbar(lo,hi,color,y,stringbuf);
}

void disp_sidebargear(uint8_t equslot, uint8_t y) {
	void *itm,*spr;
	
	itm = &equipment[equslot];
	spr = items_GetSprite(itm);
	if (spr == equipicons_tiles[0]) spr = equipicons_tiles[equslot+1];
	gfx_Sprite_NoClip(spr,CURGEAR_X,y);
	gfx_SetTextXY(CURGEAR_X+18,y+1);
	util_PrintF(items_GetName(itm));
}

void disp_Sidebar(uint8_t update) {
	static uint8_t prevupdate;
	uint8_t u,i,t;
	int xphi,xplo,temp;
	
	u = update | prevupdate;
	
	if (u & UPD_BACKERS) {	//The whole background parts of the sidebar
		// the overall background
		gfx_SetColor(COLOR_DARKGRAY);
		gfx_FillRectangle_NoClip(SBAR_LEFT,0,LCD_WIDTH-SBAR_LEFT,LCD_HEIGHT-12);
		// the two bar pair backgrounds
		gfx_Sprite_NoClip(xpfood,XPFOOD_X,XPFOOD_Y);
		gfx_Sprite_NoClip(hpmp,HPMP_X,HPMP_Y);
		// the background for the equipment stuffs
		gfx_SetColor(COLOR_GUNMETALGRAY);
		gfx_FillRectangle_NoClip(CURGEAR_X,CURGEAR_Y,84,34);
		gfx_FillRectangle_NoClip(QUICKSET_X,QUICKSET_Y,88,50);
		gfx_Sprite_NoClip(digitbar_top,QUICKSET_X+6,QUICKSET_Y+1);
		gfx_Sprite_NoClip(digitbar_btm,QUICKSET_X+5,QUICKSET_Y+43);
		// the minimap area
		gfx_SetColor(COLOR_PURPLE);
		gfx_Rectangle_NoClip(MINIMAP_X,MINIMAP_Y,66,66);
		gfx_Sprite_NoClip(areagfx,MAPAREA_X,MAPAREA_Y);
		gfx_Sprite_NoClip(floorgfx,MAPFLOOR_X,MAPFLOOR_Y);
	}
	
	if (u & UPD_XP) {
		gfx_SetTextFGColor(COLOR_BLACK);
		xphi = maxleveltable[stats.level];
		xplo = (stats.level) ? (maxleveltable[stats.level-1]) : 0;
		temp = (stats.xp - xplo) / (xphi - xplo) * 100;
		util_BufClr();
		if (stats.level<18) {
			util_BufInt(temp,1);
			util_BufStr("%  ");
		}
		util_BufStr("LV ");
		util_BufInt(stats.level+1,1);
		disp_sidebarbar(stats.xp-xplo,xphi-xplo,COLOR_GOLD,XPFOOD_Y+2,stringbuf);
	}
	if (u & UPD_FOOD) {
		gfx_SetTextFGColor(COLOR_WHITE);
		util_BufClr();
		util_BufInt(stats.food,1);
		disp_sidebarbar(stats.food, stats.hifood, COLOR_RED, XPFOOD_Y+21, stringbuf);
	}
	
	if (u & UPD_MINI) {
		gfx_SetTextFGColor(COLOR_WHITE);
		gfx_SetTextXY(MAPAREA_X+1,MAPAREA_Y+11);
		if (stats.dungeonid)	gfx_PrintUInt(stats.dungeonid,2);
		else {
			t = stats.forestarea-1;
			gfx_PrintChar(t%5+'A');
			gfx_PrintChar(t/5+'1');
		}
		gfx_SetTextXY(MAPFLOOR_X+1,MAPFLOOR_Y+11);
		if (stats.dungeonid)	gfx_PrintUInt(stats.dungeonfloor,2);
	}
	if (u & (UPD_HP|UPD_MP)) {
		gfx_SetTextFGColor(COLOR_WHITE);
		mobj_RecalcPlayer();
		disp_sidebarbarhpmp(pmobj.hp,pcalc.maxhp,COLOR_FORESTGREEN,HPMP_Y+2);
		disp_sidebarbarhpmp(pmobj.mp,pcalc.maxmp,COLOR_BLUE,HPMP_Y+21);
	}
	
	if (u & UPD_CURGEAR) {
		disp_sidebargear(5,CURGEAR_Y);
		disp_sidebargear(7,CURGEAR_Y+18);
	}

	if (u & UPD_QUICKSET) {
		disp_drawquickbar(QUICKSET_X,(QUICKSET_Y + 8));
	}

	//Always update the minimap
	asm_LoadMinimap(pmobj.x,pmobj.y);
	//Maintain the chain
	prevupdate = update;
}






/* =========================================================================== */



uint8_t disp_clampcoord(uint8_t coord) {
	uint8_t c;
	c = coord-7;
	if (c>230) return 0;
	if (c>113) return 113;
	return c;
}

#define disp_mapdelta(dx,dy) (curmap->data[(ty+(dy*CLIPPING))*128+(tx+(dx*CLIPPING))])

uint8_t disp_Gamemode(uint8_t key) {
	uint8_t u,i,j,k,x,y,tx,ty,pass;
	int cx,cy,px,py,ex,ey,dx,dy;
	void *ptr,*spr;
	mobj_t *mobj;
	
	pass = u = 0;
	if (key & kbit_Mode) return GS_TITLE;
	if (kb_Data[1] & (kb_Yequ|kb_Window|kb_Zoom)) return GS_MENUMODE;
	key |= kb_Data[7];	//merge momentaries with persistent arrows
	
	tx = x = pmobj.x;
	ty = y = pmobj.y;
	
	//Move and slide against walls if possible.
	if ((key & kbit_Left ) && (tx > 0x00)) {
		if (disp_mapdelta(-1, 0) > 0x3F) --tx;
		stats.facing = 8;
	}
	if ((key & kbit_Right) && (tx < 0x7F)) {
		if (disp_mapdelta( 1, 0) > 0x3F) ++tx;
		stats.facing = 4;
	}
	if ((key & kbit_Up   ) && (ty > 0x00)) {
		if (disp_mapdelta( 0,-1) > 0x3F) --ty;
		stats.facing = 12;
	}
	if ((key & kbit_Down ) && (ty < 0x7F)) {
		if (disp_mapdelta( 0, 1) > 0x3F) ++ty;
		stats.facing = 0;
	}
	
	//
	//Add collision detection and reset tx,ty if movement is disallowed. The
	//resetting thing is important because of mobj_PushMove pmobj.
	//
	
	stats.dx = (int8_t) tx - x;	//passthrough for actioning. Basic collision
	stats.dy = (int8_t) ty - y;	//is done already.
	
	
	
	if (!*footerbuf) {
		util_BufClr();
		util_BufStr("Positions: (");
		util_BufInt(pmobj.x,2);
		util_BufChr(',');
		util_BufInt(pmobj.y,1);
		util_BufChr(')');
		util_BufToFooter();
	}
	
	disp_Sidebar(u);
	return GS_GAMEMODE;
}











/* =========================================================================== */



char* string_forestmap = "Forest Map";
char* string_dungeonmap = "Dungeon Map";

void disp_inventorycursor(uint8_t cursorpos) {
	uint8_t t,y;
	int x;
	
	t = cursorpos & 0x3F;
	if (cursorpos & 0x80) {
		x = 4+80+2+2+ 18*(t&1);
		y = 4+2+2+    18*(t/2);
	} else {
		x = 4+80+2+36+14 + 18*(t%5);
		y = 4+2+36+14+ 18*(t/5);
	}
	//Color is set outside this routine
	gfx_HorizLine_NoClip(x,y-1,16);		//top line
	gfx_HorizLine_NoClip(x,y+16,16);	//bottom line
	gfx_VertLine_NoClip(x-1,y,16);		//left line
	gfx_VertLine_NoClip(x+16,y,16);		//Right line
}

void disp_statdata(char *s, int base, int plus) {
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

void disp_statline(char *s, uint8_t offset) {
	int base,plus;
	if (offset) {
		base = *(((int8_t*)&pbase)+offset);
		plus = *(((int8_t*)&pcalc)+offset) - base;
	} else {
		base = plus = 0;
	}
	disp_statdata(s,base,plus);
}



uint8_t disp_Menumode(uint8_t key) {
	static uint8_t state,cursor,prevcursor,isselected;
	uint8_t i,j,k,u,a,c,t,x,y;
	int cx,cy;
	void *ptr,*ptrcmp;
	
	u = 0;
	mobj_RecalcPlayer();
	if (key & kbit_Mode) {
		if (isselected) {
			cursor = prevcursor;
			prevcursor = isselected = 0;
		} else {
			cursor = prevcursor = isselected = 0;
			return GS_GAMEMODE;
		}
	}
	k = kb_Data[1];	//toprow should last long enough from gamemode to twig this.
	if (k & kb_Yequ) state = 0;		//because most ppl can't do 60fps keyboarding
	if (k & kb_Window) state = 1;	//but nbd if they actually can. it's just one
	if (k & kb_Zoom) state = 2;		//lost frame. not like ppl are gonna speedrun this.
	
	gfx_SetColor(COLOR_GUNMETALGRAY);
	gfx_FillRectangle_NoClip(4,4,224,224);
	gfx_SetTextFGColor(COLOR_WHITE);
	gfx_SetTextXY(6,10);
	// -------------------- Render inventory and stats ------------------------
	if (!state) {
		ptrcmp = equipicons_tiles[0];	//If item lookup OOR, result equ this.
		//Stat summary
		disp_statline("STR: ",offsetof(mobjdef_t,str));
		disp_statline("AGI: ",offsetof(mobjdef_t,spd));
		disp_statline("INT: ",offsetof(mobjdef_t,smrt));
		gfx_SetTextXY(6,gfx_GetTextY() + 4);
		disp_statline("ATK: ",offsetof(mobjdef_t,atk));
		disp_statline("SNK: ",offsetof(mobjdef_t,snk));
		disp_statline("RWR: ",offsetof(mobjdef_t,rawrs));
		disp_statline("DEF: ",offsetof(mobjdef_t,def));
		disp_statline("BLK: ",offsetof(mobjdef_t,blk));
		disp_statline("REF: ",offsetof(mobjdef_t,refl));
		gfx_SetTextXY(6,gfx_GetTextY() + 4);
		disp_statline("MATK: ",offsetof(mobjdef_t,matk));
		disp_statline("FATK: ",offsetof(mobjdef_t,fatk));
		disp_statline("EATK: ",offsetof(mobjdef_t,eatk));
		disp_statline("PATK: ",offsetof(mobjdef_t,patk));
		gfx_SetTextXY(6,gfx_GetTextY() + 4);
		disp_statline("MDEF: ",offsetof(mobjdef_t,mdef));
		disp_statline("FDEF: ",offsetof(mobjdef_t,fdef));
		disp_statline("EDEF: ",offsetof(mobjdef_t,edef));
		disp_statline("PDEF: ",offsetof(mobjdef_t,pdef));
		gfx_SetTextXY(6,gfx_GetTextY() + 4);
		disp_statdata("S Pts: ",stats.spoints,0);
		disp_statdata("T Pts: ",stats.tpoints,0);
		//Keyboard interactions
		if (key & kbit_2nd) {
			if (isselected) {
				if (items_SwapSlots(cursor,prevcursor)) {
					prevcursor = isselected = 0;
					mobj_RecalcPlayer();
					u = UPD_SIDEBAR;
				}
			} else {
				isselected = 1;
				prevcursor = cursor;
			}
		}
		if (key & kbit_Del) {
			ptr = items_FindEmptySlot();
			if (ptr && (cursor&0x80)) {
				t = cursor & 0x3F;
				*(item_t*)ptr = equipment[t];
				equipment[t].type = 0;
				equipment[t].data = 0;
			}
			mobj_RecalcPlayer();
			u = UPD_SIDEBAR;
		}
		if (key & (kbit_Up|kbit_Down|kbit_Left|kbit_Right)) {
			t  = cursor & 0x3F;
			if (cursor & 0x80) {
				if ((key & kbit_Down ) && (t <  6))	cursor += 2;
				if ((key & kbit_Up   ) && (t >  1))	cursor -= 2;
				if ((key & kbit_Left ) && (t &  1))	cursor -= 1;
				if  (key & kbit_Right) {
					if (t & 1)	cursor = 0;
					else		cursor+= 1;
				}
			} else {
				i = t % 5;
				if ((key & kbit_Down ) && (t < 30))	cursor += 5;
				if ((key & kbit_Up   ) && (t >  4))	cursor -= 5;
				if ((key & kbit_Right) && (i <  4))	cursor += 1;
				if  (key & kbit_Left ) {
					if (i)	cursor -= 1;
					else	cursor = (0x80+7);
				}
			}
		}
		k = asm_GetNumpad();
		if (k && k<=10) {
			items_SwapSlots((k-1)+0x40,cursor);
			u |= UPD_QUICKSET;
		}
		//Equipment
		for (i=0; i<8; ++i) {
			x = 4+80+4 + 18*(i&1);
			y = (i>>1) * 18 + 4+4;
			if (ptrcmp == (ptr=items_GetSprite(&equipment[i])))
				ptr = equipicons_tiles[i+1];	//skip over blank default
			gfx_Sprite_NoClip(ptr,x,y);
		}
		//Quickbar
		disp_drawquickbar(4+80+2+36+14, 4+4);
		
		//Inventory
		for (i=0; i<35; ++i) {
			x = 4+80+2+36+14 + 18*(i%5);
			y = 4+2+36+14+ 18*(i/5);
			ptr = items_GetSprite(&inventory[i]);
			if (i==34) ptr = equipicons_tiles[9]; //trashcan
			gfx_Sprite_NoClip(ptr,x,y);
		}
		//UI background elements
		gfx_SetColor(COLOR_DARKGRAY);
		gfx_FillRectangle(123+4,6,4,77);	//vertical divider
		gfx_FillRectangle(127+4,47,95,4);	//quick/inv divider
		gfx_FillRectangle(82+4,79,45,4);	//equip/packs divider
		//Render cursors
		gfx_SetColor(COLOR_PURPLE);
		if (isselected) {
			disp_inventorycursor(prevcursor);
			gfx_SetColor(COLOR_ORANGE);
		}
		disp_inventorycursor(cursor);
		//Draw bottom bar description
		if (cursor == 34) {
			strcat(footerbuf,"Trash can ~ Discard unwanted items here");
		} else {
			t = cursor & 0x3F;
			ptr = ((cursor&0x80) ? (&equipment[t]) : (&inventory[t]));
			strcpy(footerbuf,items_GetName(ptr));
			strcat(footerbuf," ~ ");
			strcat(footerbuf,items_GetDesc(ptr));
		}
		
	// ---------------- Render dedicated stats and allocations ----------------
	} else if (1==state) {
		gfx_SetTextXY(4,230);
		strcpy(footerbuf,"TODO: Stats, skills, and allocs");
	// ------------------- Render forest and dungeon maps ---------------------
	} else if (2==state) {
		if (stats.dungeonid) {
			//Dungeon map
			
			ptr = string_dungeonmap;
		} else {
			//Forest map
			t = 0;
			for (i=0,y=(4+24); i<5; ++i,y+=40) {
				for (j=0,cx=(4+16); j<5; ++j,cx+=40) {
					if (forestmap_seen[t]) {
						gfx_SetColor(COLOR_FORESTGREEN);
						a = forestmap[t];
						if (a&FEX_NORTH)	gfx_FillRectangle_NoClip(cx+14,y-8 ,4,8);
						if (a&FEX_SOUTH)	gfx_FillRectangle_NoClip(cx+14,y+32,4,8);
						if (a&FEX_EAST)		gfx_FillRectangle_NoClip(cx+32,y+14,8,4);
						if (a&FEX_WEST)		gfx_FillRectangle_NoClip(cx-8 ,y+14,8,4);
						c = COLOR_FORESTGREEN;
						if (((stats.timer>>3)&1)&&(t==(stats.forestarea-1)))
							c = COLOR_GOLD;
					} else {
						c = COLOR_GRAY;
						if (t==0x0C)	c = COLOR_RED;
					}
					gfx_SetColor(c);
					gfx_FillRectangle_NoClip(cx,y,32,32);
					gfx_SetTextFGColor(COLOR_WHITE);
					gfx_SetTextXY(cx+4,y+4);
					gfx_PrintChar(t%5+'A');
					gfx_PrintChar(t/5+'1');
					++t;
				}
			}
			ptr = string_forestmap;
		}
		gfx_SetTextFGColor(COLOR_GOLD);
		gfx_PrintStringXY(ptr,(224-gfx_GetStringWidth(ptr))/2+4,(4+8));
		strcpy(footerbuf,"TODO: Forest maps AND dungeons ");
	}
	
	disp_Sidebar(u);
	return GS_MENUMODE;
}












/* =========================================================================== */
//Utilities or sections that are called more than once even if they ought
//to belong inline with the routine it is found in


void disp_drawquickbar(int basex, uint8_t basey) {
	uint8_t i,y;
	int x;
	void *ptr;
	for (i=0; i<10; ++i) {
		x = basex + 18*(i%5);
		y = basey + 18*(i/5);
		if (equipicons_tiles[0] == (ptr = items_GetSprite(&quickbar[i])))
			ptr = equipicons_tiles[i+10];	//skip over top row
		gfx_Sprite_NoClip(ptr,x,y);
	}
}












