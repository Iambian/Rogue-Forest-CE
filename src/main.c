
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
uint8_t rawkey;
gfx_sprite_t *tiles[256];
gfx_sprite_t *emptysprite;
gfx_sprite_t *curmap;
gfx_tilemap_t tilemap;



/* Function prototypes */
void main_Init(void);
void main_Exit(void);
void main_Err(uint8_t filenum);
void *util_InitNewSprite(void);




int main(void) {
	uint8_t i,k,state;
	
	main_Init();	//Setup game
	state = 0;
	while (1) {
		k = util_GetSK();
		if (GS_TITLE == state) {
			state = disp_Title(k);
		} else if (GS_NEWGAME == state) {
			state = GS_GAMEMODE;
		} else if (GS_LOADGAME == state) {
			state = GS_TITLE;
		} else if (GS_CREDITS == state) {
			state = disp_Credits(k);
		} else if (GS_QUIT == state) {
			break;
		} else if (GS_GAMEMODE == state) {
			break;
		} else state = GS_QUIT;
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
	gfx_SetPaletteEntry(charequtiles_palette_offset,gfx_RGBTo1555(128,160,160)); //gunmetal gray
	gfx_SetPalette(charequtiles_pal,sizeof_charequtiles_pal,charequtiles_palette_offset);
	
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
}


void main_Exit(void) {
	gfx_End();
}

