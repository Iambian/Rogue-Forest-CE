
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
#include "mobjs.h"
#include "gfx/output/gfx_base.h"
#include "gfx/output/gfx_dtiles.h"
#include "gfx/output/gfx_ftiles.h"
#include "gfx/output/gfx_charequtiles.h"

void gen_ResetAll(void);
void *gen_GetSavedFloorData(void);
void gen_SetMinimapColors(uint8_t *tile2col, uint8_t sizeof_list);
void gen_SetMinimapColorsDefault(void);
void gen_BufToTilemap(void);


//0bDDDFFFFF, D=dungeon (0-8), F=floor (1-31)
//Forest area: D = 0
void gen_WarpTo(uint8_t warpdest) {
	uint8_t dungeonid;
	uint8_t floorid;
	
	if (warpdest == 0xFF) return;	//Should've caught this to end the game.
	
	dungeonid	= (warpdest & 0xE0)>>5;
	floorid		= (warpdest & 0x1F);
	if (!dungeonid) {
		//Forest.
		gen_TestDungeon(40,floorid); //Uses previous value of forestarea
		pstats.forestarea = floorid;
		pstats.dungeonid = 0;
		pstats.dungeonfloor = 0;
	} else {
		pstats.dungeonid = dungeonid;
		pstats.dungeonfloor = floorid;
		//You want to do other things. Like actually generate dungeon floors.
	}
	return;
}

/* ============================================================================
					SIMPLE FOREST DUNGEON GENERATION
============================================================================ */
uint8_t rofotreetypemap[] = {
	0,1,0,1,0,
	1,2,1,2,1,
	0,1,3,1,0,
	1,2,1,2,1,
	0,1,1,1,0,
};

/* Randomly places rooms around the map, then makes paths between them */
void gen_TestDungeon(uint8_t roomdensity, uint8_t floorid)  {
	uint8_t i,j,k,iscollide,x,y,w,h,x2,y2,t,mx,my,nx,ny;
	uint8_t curenemies,maxenemies;
	uint8_t *ptr;
	floordat_t *f;
	uint32_t temp;
	room_t *room;
	sobj_t sobj;
	uint8_t warpmap,warpdest,warptype;
	void *vptr;
	
	ptr = gen_GetSavedFloorData();
	f = (floordat_t*) ptr;
	sobj_clear();
	mobj_clear();
	
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
			/*
			x = randInt(3,125-w); // A slight distance away from the edges to
			y = randInt(3,125-h); // keep rooms from being adjacent to map bounds
			*/
			/* A much more restricted thing because maps are too large for testing */
			x = randInt(32,96-w);
			y = randInt(32,96-h);
			
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
	i = rofotreetypemap[floorid-1];
	switch (i) {
		case 0:		vptr = DL_tree1_tiles; break; 
		case 1:		vptr = DL_tree2_tiles; break; 
		case 2:		vptr = DL_tree3_tiles; break; 
		case 3:		vptr = DL_tree4_tiles; break; 
		default:	vptr = DL_tree1_tiles; break;
	}
	memcpy(&main_tilemap[0],vptr,sizeof(gfx_sprite_t*)*DL_tree3_tiles_num);
	memcpy(&main_tilemap[64],DL_floor4_tiles,sizeof(gfx_sprite_t*)*DL_floor4_tiles_num);
	
	memcpy(&main_tilemap[FLOORBASE],floorstuff_tiles,sizeof(gfx_sprite_t*)*floorstuff_tiles_num);
	memcpy(&main_tilemap[KITEMEBASE],food_kitems_tiles,sizeof(gfx_sprite_t*)*food_kitems_tiles_num);
	
	
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
		//Iterate over rooms and randomly place warps
		warpmap = 0x01;
		dbg_sprintf(dbgout,"*** Warps on floor ID %X\n",floorid);
		while (warpmap & 0x0FF) {
			warptype = warpdest = 0;
			if (forestmap[floorid-1] & warpmap) {
				dbg_sprintf(dbgout,"warpmap %X\n",warpmap);
				warptype = SOBJ_WARPGATE|0x80; //Preactivated warpgates
				switch (warpmap) {
					case FEX_WEST:
						warpdest = floorid-1;
						break;
					case FEX_EAST:
						warpdest = floorid+1;
						break;
					case FEX_SOUTH:
						warpdest = floorid+5;
						break;
					case FEX_NORTH:
						warpdest = floorid-5;
						break;
					case FEX_EXIT:
						warpdest = 0xFF;
						warptype = SOBJ_WARPGATE2;
						if (pstats.hascherry) warptype |= 0x80;
						break;
					case FEX_DUNGEON:
						for (i=0;(i<6)||(forestdungeon[i]==floorid);++i);
						warpdest = i<<5+1;
						warptype = SOBJ_STAIRSDOWN;
						break;
					case FEX_FLOORUP:
						warptype = SOBJ_STAIRSUP;
						warpdest = 0;	//only valid wrt in-dungeon. This is
						break;			//not to be done in this generator.
					case FEX_FLOORDOWN:
						warptype = SOBJ_STAIRSDOWN;
						warpdest = 0;	//only valid wrt in-dungeon. This is
						break;			//not to be done in this generator.
					default:
						warptype = SOBJ_WARPHIDDEN;
						warpdest = 0;	//Impossible for this to reach.
						break;			//Still... set invalid floor.
				} 
				dbg_sprintf(dbgout,"Warpgate loading type %X\n",warptype);
			}
			if (warpdest) {
				dbg_sprintf(dbgout,"Warpgate destination %X\n",warpdest);
				while (1) {
					room = &roomlist[randInt(0,numrooms-1)];
					x = randInt(room->x+1,room->x+room->w-2);
					y = randInt(room->y+1,room->y+room->h-2);
					//Rooms should never be so full as to lock this up.
					//If it is, you should generate fewer things.
					if (sobj_getentrybypos(x,y)) continue;
					//Disable warp to the cherry if you do not own
					//all four mcguffins... erh. gems... of power.
					//if ((warpdest == 0x0D) && !(pstats.gems^0x0F)) warptype &= 0x7F;
					scratchsobj.type = warptype;
					scratchsobj.x = x;
					scratchsobj.y = y;
					scratchsobj.data = warpdest;
					sobj_addentry(&scratchsobj);
					break;
				}
				dbg_sprintf(dbgout,"Warpgate generated\n");
			}
			warpmap <<= 1;
		}
		//If you're making the central floor of the game, place the cherry somewhere.
		if ((floorid == 0x0D) && (!pstats.hascherry)) {	//There can only be one
			while (1) {
				room = &roomlist[randInt(0,numrooms-1)];
				x = randInt(room->x+1,room->x+room->w-2);
				y = randInt(room->y+1,room->y+room->h-2);
				if (sobj_getentrybypos(x,y)) continue;
				if (mobj_getentrybypos(x,y)) continue;
				scratchsobj.type = SOBJ_CHERRY;
				scratchsobj.x = x;
				scratchsobj.y = y;
				scratchsobj.data = 0xFF;	//maximum cherry flavoring
				sobj_addentry(&scratchsobj);
				dbg_sprintf(dbgout,"Cherry generated at (%i,%i)\n",x,y);
				break;
			}
		}
	}
	
	/* Write mobile objects */
	maxenemies = randInt(numrooms,numrooms*2);
	curenemies = 0;

	while (curenemies<maxenemies) {
		//This algorithm is a crapshoot. Also, will always have more enemies
		//than maxenemies. This is not a bug. maxenemies is intended to be
		//more of a soft limit. Be sure that there aren't too many rooms, though.
		for(i=0;i<numrooms;++i) {
			if (randInt(0,5) != 3) continue;	//Only generate in a room on chance
			room = &roomlist[i];
			x = randInt(room->x+1,room->x+room->w-2);
			y = randInt(room->y+1,room->y+room->h-2);
			if (sobj_getentrybypos(x,y)) continue;	//Don't generate if sobj in way
			if (!mobj_getentrybypos(x,y)) {
				//Only generate if another mobj isn't in way
				scratchmobj.x = x;
				scratchmobj.y = y;
				scratchmobj.type = randInt(1,3);
				scratchmobj.flags = 0;
				scratchmobj.hp = mobj_getmobjdef(&scratchmobj)->maxhp;
				scratchmobj.mp = mobj_getmobjdef(&scratchmobj)->maxmp;
				scratchmobj.step = 0;
				scratchmobj.drop = 0;
				mobj_addentry(&scratchmobj);
				++curenemies;
			}
		}
	}
	
	/* Place static object tiles on the map */
	sobj_WriteToMap();
	
	
	
	//Coming back out of a dungeon
	if (pstats.dungeonid) {
		vptr = sobj_getwarpbydest(AREAHICONV(pstats.dungeonid)+pstats.dungeonfloor);
	} else {
		vptr = sobj_getwarpbydest(AREA_FOREST|pstats.forestarea);
	}
	dbg_sprintf(dbgout,"Location of warpto: %X\n",vptr);
	dbg_sprintf(dbgout,"Location of warpto: %X\n",vptr);
	memcpy(&sobj,vptr,sizeof sobj);
	pstats.x = sobj.x;
	pstats.y = sobj.y;
	
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


