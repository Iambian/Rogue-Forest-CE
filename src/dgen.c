
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
#include "gfx/output/gfx_base.h"
#include "gfx/output/gfx_dtiles.h"
#include "gfx/output/gfx_ftiles.h"
#include "gfx/output/gfx_charequtiles.h"

void gen_ResetAll(void);
void *gen_GetSavedFloorData(void);
void gen_SetMinimapColors(uint8_t *tile2col, uint8_t sizeof_list);
void gen_SetMinimapColorsDefault(void);
void gen_BufToTilemap(void);




/* ============================================================================
					SIMPLE FOREST DUNGEON GENERATION
============================================================================ */


/* Randomly places rooms around the map, then makes paths between them */
void gen_TestDungeon(uint8_t roomdensity)  {
	uint8_t i,j,k,iscollide,x,y,w,h,x2,y2,t,mx,my,nx,ny;
	uint8_t *ptr;
	floordat_t *f;
	uint32_t temp;
	room_t *room;
	sobj_t sobj;
	
	ptr = gen_GetSavedFloorData();
	f = (floordat_t*) ptr;
	sobj_clear();
	//mobj_clear();
	
	/* Generate floor map */
	if (f)	temp = f->seed;
	else	floordat.seed = temp = rtc_Time();
	srand(temp);
	
	if (roomdensity<3) roomdensity = 3;
	//Ensure at least two rooms are generated.
	do {
		gen_ResetAll();
		
		//Attempt to generate number of rooms equal to roomdensity
		for (i = 0; i < roomdensity; ++i) {
			w = randInt(5,20);
			h = randInt(5,20);
			x = randInt(3,125-w); // A slight distance away from the edges to
			y = randInt(3,125-h); // keep rooms from being adjacent to map bounds
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
	gfx_SetPaletteEntry(dtiles_palette_offset,gfx_RGBTo1555(211,125,44)); //brown
	gfx_SetPalette(ftiles_pal,sizeof_ftiles_pal,ftiles_palette_offset);
	gfx_SetPaletteEntry(ftiles_palette_offset,gfx_RGBTo1555(211,125,44)); //brown

	/* Use gen_BufToTilemap to do the buffering things */
	gen_BufToTilemap();
	gen_SetMinimapColorsDefault();
	//gen_SetMinimapColors(trees_minimap,18);
	
	/* Place dungeon interpolation logic below */
	asm_InterpolateMap();
	/* Set up tilemap conditions */
	memcpy(&main_tilemap[0],DL_tree3_tiles,sizeof(gfx_sprite_t*)*DL_tree3_tiles_num);
	memcpy(&main_tilemap[64],DL_floor4_tiles,sizeof(gfx_sprite_t*)*DL_floor4_tiles_num);
	
	memcpy(&main_tilemap[FLOORBASE],floorstuff_tiles,sizeof(gfx_sprite_t*)*consumables_tiles_num);
	
	/* Write immutable/nonsavable static objects, if any. e.g. decorations */
	
	//
	//
	//
	//
	
	/* Begin writing static objects */
	if (f) {
		//Figure out how to access sobj data from file
	} else {
		//Iterate over rooms and randomly place doors in entryways
		for (i = 0; i < numrooms ; ++i) {
			room = &roomlist[i];
			
			//Horizontal sweep
			//Iterate all X positions in selected room. Keeping Y constant.
			y = room->y - 1;
			y2 = y + room->h + 1;
			for (x = room->x; x < (room->x + room->w); ++x) {
				if (sobj_IsDoorable(x,y)) {
					sobj.x = x;
					sobj.y = y;
					sobj.type = SOBJ_DOOR;
					sobj.data = 0;
					sobj_addentry(&sobj);
				}
				if (sobj_IsDoorable(x,y2)) {
					sobj.x = x;
					sobj.y = y2;
					sobj.type = SOBJ_DOOR;
					sobj.data = 0;
					sobj_addentry(&sobj);
				}
			}
			//Vertical sweep
			//Iterate over all Y's in a room, keeping X constant.
			x  = room->x - 1;
			x2 = x + room->w + 1;
			for (y = room->y; y < (room->y + room->h); ++y) {
				if (sobj_IsDoorable(x,y)) {
					sobj.x = x;
					sobj.y = y;
					sobj.type = SOBJ_DOOR;
					sobj.data = 0;
					sobj_addentry(&sobj);
				}
				x2 = x + room->w+1;
				if (sobj_IsDoorable(x2,y)) {
					sobj.x = x2;
					sobj.y = y;
					sobj.type = SOBJ_DOOR;
					sobj.data = 0;
					sobj_addentry(&sobj);
				}
				
			}
		}
	}
	
	/* Write mobile objects */
	
	//
	//
	//
	//
	
	/* Place static object tiles on the map */
	sobj_WriteToMap();
	
	

	/* DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG */
	/* DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG */
	/* DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG */
	//Pick a spawn location. Any location. We have to test player stuff NOW.
	for (y = 0; y<128; ++y) {
		for (x = 0; x<128; ++x) {
			t = curmap->data[y*128+x];
			if ((t>0x46) && (t<0x80)) {
				pstats.x = x;
				pstats.y = y;
				pstats.subx = pstats.suby = 0;
				goto TESTDUNGEONGEN_LOOPEND;
				//Haha lookit mah goto. My very first Truly Wrong Thing in C.
			}
		}
	}
TESTDUNGEONGEN_LOOPEND: ;
	
	
	
	
	
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
void gen_SetMinimapColorsDefault(void) {
	uint8_t i,*ptr;
	
	ptr = asm_SetTile2ColorStart();
	
	for (i = 0; i<250; ++i,++ptr) {
		if (i<0x40)			*ptr = COLOR_BLACK;
		else if (i>=0x80)	*ptr = COLOR_GRAY;
		else				*ptr = COLOR_WHITE;
	}
}

void *gen_GetSavedFloorData(void) {
	return NULL;
}


