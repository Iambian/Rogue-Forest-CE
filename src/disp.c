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
	zx7_Decompress(gfx_vbuffer,rofotitle_compressed);
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


void disp_Sidebar(uint8_t update) {
	static uint8_t prevupdate;
	uint8_t u,i,t;
	int xphi,xplo,temp;
	
	u = update | prevupdate;
	
	//Always update the minimap
	asm_LoadMinimap(pmobj.x,pmobj.y);
	
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
		xphi = maxleveltable[stats.level];
		xplo = (stats.level) ? (maxleveltable[stats.level-1]) : 0;
		temp = (stats.xp - xplo) / (xphi - xplo) * 100;
		util_BufClr();
		if (stats.level>18) {
			util_BufInt(temp,2);
			util_BufStr("%  ");
		}
		util_BufStr("LV ");
		util_BufInt(stats.level+1,1);
		gfx_SetTextFGColor(COLOR_BLACK);
		disp_sidebarbar(stats.xp-xplo,xphi-xplo,COLOR_GOLD,XPFOOD_Y+2,stringbuf);
	}
	if (u & UPD_FOOD) {
		util_BufClr();
		util_BufInt(stats.food,1);
		gfx_SetTextFGColor(COLOR_WHITE);
		disp_sidebarbar(stats.food, stats.hifood, COLOR_RED, XPFOOD_Y+21, stringbuf);
	}
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

uint8_t disp_Gamemode(uint8_t key) {
	uint8_t u,i,j,k,x,y,tx,ty;
	int cx,cy,px,py,ex,ey;
	
	u = 0;
	if (key & kbit_Mode) return GS_TITLE;
	
	//
	//insert method of handling top row keypresses for menuing.
	//or have that handled out in main.
	//or handle just one here then make the user switch modes
	//once you get to the menumode. idk. figure it out and
	//make it not suck.
	//
	key |= kb_Data[7];	//merge momentaries with persistent arrows
	
	tx = x = pmobj.x;
	ty = y = pmobj.y;
	if (key & kbit_Left)	--tx;
	if (key & kbit_Right)	++tx;
	if (key & kbit_Up)		--ty;
	if (key & kbit_Down)	++ty;
	
	//
	// direction handly stuff
	//
	
	
	cx = disp_clampcoord(pmobj.x) * 16;
	cy = disp_clampcoord(pmobj.y) * 16;
	
	//BEGIN TESTING
	x = tx;
	y = ty;
	//END TESTING
	
	gfx_Tilemap(&tilemap,cx,cy);
	
	pmobj.x = x;
	pmobj.y = y;
	
	gfx_SetColor(COLOR_BLACK);
	gfx_SetTextFGColor(COLOR_WHITE);
	gfx_SetTextXY(4,LCD_HEIGHT-10);
	gfx_FillRectangle(4,LCD_HEIGHT-10,LCD_WIDTH-4,8);
	util_BufClr();
	util_BufStr("Positions: (");
	util_BufInt(pmobj.x,2);
	util_BufChr(',');
	util_BufInt(pmobj.y,1);
	util_BufChr(')');
	gfx_PrintStringXY(stringbuf,4,LCD_HEIGHT-10);

	
	disp_Sidebar(u);
	u = 0;
	return GS_GAMEMODE;
}











/* =========================================================================== */
uint8_t disp_Menumode(uint8_t key) {
	
	if (key & kbit_Mode) return GS_GAMEMODE;
	
	
	
	return GS_MENUMODE;
}












/* =========================================================================== */


