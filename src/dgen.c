

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

void gen_ResetAll(void);
void gen_SetMinimapColors(uint8_t *tile2col, uint8_t sizeof_list);
void gen_BufToTilemap(void);






uint8_t trees_minimap[] = {
	COLOR_BLACK,COLOR_BLACK,COLOR_BLACK,COLOR_BLACK,COLOR_BLACK,COLOR_BLACK,
	COLOR_BLACK,COLOR_BLACK,COLOR_BLACK,COLOR_WHITE,COLOR_BLACK,COLOR_BLACK,
	COLOR_BLACK,COLOR_BLACK,COLOR_BLACK,COLOR_WHITE,COLOR_BLACK,COLOR_BLACK,
};
/* ============================================================================
					SIMPLE FOREST DUNGEON GENERATION
============================================================================ */


/* Randomly places rooms around the map, then makes paths between them */
void gen_TestDungeon(uint8_t roomdensity)  {
	uint8_t i,j,k,iscollide,x,y,w,h,x2,y2,t,mx,my,nx,ny;
	uint8_t *ptr;
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
	gfx_SetColor(0xFF);	//paths and path edges
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
	
	/* Place mapping palettes */
	gfx_SetPalette(dtiles_pal,sizeof_dtiles_pal,dtiles_palette_offset);

	/* Use gen_BufToTilemap to do the buffering things */
	gen_BufToTilemap();
	gen_SetMinimapColors(trees_minimap,18);
	/* Place dungeon interpolation logic below */
	asm_InterpolateMap();
	/* Set up tilemap conditions */
	tilemap.tiles = DL_tree3_tiles;
	
	

}




























/* ============================================================================
				   DUNGEON GENERATOR SUBROUTINES AND HELPERS
============================================================================ */

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
	numsobjs = 0;
	nummobjs = 0;
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
			//if (*ptr == 0xFF)	*ptr = 9;
			ptr++;
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

