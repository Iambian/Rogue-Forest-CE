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












