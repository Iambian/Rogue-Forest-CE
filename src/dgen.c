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

void gen_TestDungeon(uint8_t density, uint8_t floorid);






void gen_WarpTo(uint8_t id) {
	uint8_t dungeonid,floorid,oldid;
	void *ptr;
	
	gfx_FillScreen(0);	//Clear display, allowing for direct-to-screen load indic.
	gfx_SwapDraw();		//Show the blank screen.
	gfx_FillScreen(0);	//This one is for the dungeon generator. To start clean.
	gfx_SetColor(0xFF);	//Set our paintbrush to white.
	
	if (stats.dungeonid)	oldid = AREAHICONV(stats.dungeonid)+stats.dungeonfloor;
	else					oldid = AREA_FOREST | stats.forestarea;
	
	if (0xFF == id) return;
	dungeonid	= AREAHIUNCONV(id);
	floorid		= (AREALOMASK &id);
	
	if (!dungeonid) {
		//Load forest
		gen_TestDungeon(80,id);
		gen_AddDoors(id);
		forestmap_seen[floorid-1] = 1;
		stats.forestarea = floorid;
		stats.dungeonid = 0;
		stats.dungeonfloor = 0;
	} else {
		//Load dungeon
		//---
		stats.dungeonid = dungeonid;
		stats.dungeonfloor = floorid;
	}
	sobj_WriteToMap();
	ptr = sobj_GetByDest(oldid);
	if (ptr) {
		pmobj.x = ((sobj_t*)ptr)->x;
		pmobj.y = ((sobj_t*)ptr)->y;
	}
	gfx_FillScreen(0);	//Clean up after dungeon generate.
	return;
}


uint8_t genoverworld_testforest[] = {
	FEX_EAST|FEX_SOUTH|FEX_EXIT,	FEX_EAST|FEX_WEST,	FEX_WEST|FEX_SOUTH,	FEX_SOUTH|FEX_EAST,	FEX_WEST|FEX_SOUTH,
	FEX_NORTH|FEX_SOUTH,	FEX_EAST,	FEX_WEST|FEX_NORTH,	FEX_NORTH|FEX_SOUTH,	FEX_NORTH|FEX_SOUTH,	
	FEX_NORTH|FEX_SOUTH,	FEX_SOUTH,	FEX_EAST,	FEX_NORTH|FEX_WEST,	FEX_NORTH|FEX_SOUTH,	
	FEX_NORTH|FEX_EAST,	FEX_WEST|FEX_NORTH|FEX_SOUTH,	FEX_SOUTH,	FEX_SOUTH|FEX_EAST,	FEX_WEST|FEX_NORTH,
	FEX_EAST,	FEX_WEST|FEX_NORTH|FEX_EAST,	FEX_WEST|FEX_NORTH|FEX_EAST,	FEX_WEST|FEX_NORTH|FEX_EAST,	FEX_WEST
};
uint8_t genoverworld_testdungeon[] = {0x01,0x02,0x03,0x04,0x05,0x0D};


void gen_Overworld(void) {
	uint8_t startpoint;
	srand(stats.worldseed);
	startpoint = 0x01;
	memset(forestmap_seen,0,sizeof forestmap_seen);
	memcpy(forestmap,genoverworld_testforest,sizeof forestmap);
	memcpy(dungeonmap,genoverworld_testdungeon,sizeof dungeonmap);

	//
	// Generate 5x5 forestgrid and grid-to-dungeon mappings
	//
	//
	
	
	
	
	gen_WarpTo(startpoint);
}



/* ---------------------------------------------------------------------- */
void **gen_testdungeon_forests[] = {
	DL_tree1_tiles_compressed,
	DL_tree2_tiles_compressed,
	DL_tree3_tiles_compressed,
	DL_tree4_tiles_compressed
};


void gen_TestDungeon(uint8_t density, uint8_t floorid) {
	uint8_t x,y,x2,y2,dx,dy,w,h,i,j,k;
	int temp;
	floor_t *floor;
	room_t *room,*room2;
	
	//Generate roomtable
	if (density<3) density = 3;
	floor = gen_MakeFloor(floorid);
	do {
		//Clear floor plan, then attempt to generate floors
		roomcount = 0;
		for (i=0; i<density; ++i) {
			w = randInt(5,20);
			h = randInt(5,20);
			x = randInt(32,96-w);
			y = randInt(32,96-h);
			//Iterate over preexisting rooms to ensure no overlap
			for (j=0; j<roomcount; ++j) {
				room = &roomtable[j];
				if (gfx_CheckRectangleHotspot(x,y,w,h,room->x,room->y,room->w,room->h))
					break;
			}
			//If loop above passes through, then no collision. Can place room.
			if (j>=roomcount) {
				room = &roomtable[roomcount];
				room->type = 1;
				room->x = x;
				room->y = y;
				room->w = w;
				room->h = h;
				++roomcount;
				if (roomcount == (NUMROOMS_MAX-1)) break;	//No more rooms if at max.
			}
		}
	} while (roomcount<3);	//Repeat until there's at least 3 rooms on the floor.
	
	//Create outline map on screen buffer, then move to curmap
	for (i=0; i<roomcount; ++i) {
		room = &roomtable[i];
		x = room->x + randInt(0,(room->w - 2));
		y = room->y + randInt(0,(room->h - 2));
		gfx_FillRectangle_NoClip(room->x, room->y, room->w, room->h);	//Draw room
		for (j=randInt(1,2); j; --j) {	//Generate at least 1, up to 2 paths.
			do { k = randInt(0,(roomcount-1)); } while (k==i);	//another non-self room
			room2 = &roomtable[k];
			x2 = room2->x + randInt(0,(room2->w - 2));
			y2 = room2->y + randInt(0,(room2->h - 2));
			if (randInt(0,1)) {
				//Vertical-first
				dx = x;
				dy = y2;
			} else {
				//Horizontal-first
				dx = x2;
				dy = y;
			}
			gfx_Line_NoClip(x, y, dx,dy);
			gfx_Line_NoClip(x2,y2,dx,dy);
		}
	}
	//Save drawing results to curmap, then prettify it.
	gfx_GetSprite(curmap,0,0);
	asm_InterpolateMap();
	//Load forest and grass tiles with accompanying palette entries
	gen_LoadTiles(wallAbase,gen_testdungeon_forests[floorid%5],DL_tree1_tiles_num);
	gfx_SetPaletteEntry(dtiles_palette_offset,gfx_RGBTo1555(211,125,44)); //brown
	gen_LoadTiles(floorAbase,DL_floor4_tiles_compressed,DL_floor1_tiles_num);
	gfx_SetPaletteEntry(ftiles_palette_offset,gfx_RGBTo1555(211,125,44)); //brown
}


void gen_AddDoors(uint8_t floorid) {
	uint8_t i,j,k,x,y,x2,y2;
	room_t *room;
	sobj_t sobj;
	
	srand(gen_FindFloor(floorid)->seed);
	sobj.data = 0;
	sobj.type = SOBJ_DOOR;
	for (i=0; i<roomcount; ++i) {
		room = &roomtable[i];
		//Horizontal sweep. Iterate all X, keep Y const.
		y = room->y - 1;
		y2= room->h + y + 1;
		for (x=room->x; x<(room->x+room->w); ++x) {
			sobj.x = x;
			if (sobj_IsDoorable(x,y)) {
				sobj.y = y;
				sobj_Add(&sobj);
			}
			if (sobj_IsDoorable(x,y2)) {
				sobj.y = y2;
				sobj_Add(&sobj);
			}
		}
		//Vertical sweep. Iterate all Y, keep X const.
		x = room->x - 1;
		x2= room->w + x + 1;
		for (y=room->y; y<(room->y+room->h); ++y) {
			sobj.y = y;
			if (sobj_IsDoorable(x,y)) {
				sobj.x = x;
				sobj_Add(&sobj);
			}
			if (sobj_IsDoorable(x2,y)) {
				sobj.x = x2;
				sobj_Add(&sobj);
			}
		}
	}
}



void gen_AddWarps(uint8_t floorid);




void gen_LoadTiles(uint8_t tilestart, void **compressed, uint8_t numtile) {
	uint8_t i,j,k;
	
	for (i=0,j=tilestart; i<numtile; ++i,++j,++compressed) {
		zx7_Decompress(tiles[j],*compressed);
	}
}

floor_t *gen_FindFloor(uint8_t floorid) {
	uint8_t i;
	for (i=0; i<floorcount; ++i) {
		if (floortable[i].id == floorid) return &floortable[i];
	}
	return NULL;
}

floor_t *gen_MakeFloor(uint8_t floorid) {
	floor_t *floor;
	
	if (floor = gen_FindFloor(floorid)) return floor;
	floor = &floortable[floorcount++];
	floor->id = floorid;
	floor->seed = rtc_Time();
	floor->mobj_count = 0;
	floor->sobj_count = 0;
	
	return floor;
}



