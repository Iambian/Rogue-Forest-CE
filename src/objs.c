//Combined objects handlers
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

mobj_t emptymobj;
sobj_t emptysobj;

void sobj_Add(sobj_t *sobj) {
	if (sobjcount>250) return;
	memcpy(&sobjtable[sobjcount++],sobj,sizeof(sobj_t));
}

void mobj_Add(mobj_t *mobj) {
	if (mobjcount>250)
	memcpy(&mobjtable[mobjcount++],mobj,sizeof(mobj_t));
}


void sobj_Del(sobj_t *sobj) {
	unsigned int idx;
	uint8_t i;
	idx = sobj - sobjtable;
	if (idx >= sobjcount) return;	//Outside range. Do not attempt to modify table.
	for (i=idx; i<251; ++i) sobjtable[i] = sobjtable[i+1];
	sobjtable[i] = emptysobj;
}

void mobj_Del(mobj_t *mobj) {
	unsigned int idx;
	uint8_t i;
	idx = mobj - mobjtable;
	if (idx >= mobjcount) return;	//Outside range. Do not attempt to modify table.
	for (i=idx; i<251; ++i) mobjtable[i] = mobjtable[i+1];
	mobjtable[i] = emptymobj;
}

sobj_t *sobj_GetByPos(uint8_t x, uint8_t y) {
	uint8_t i;
	for (i=0; i<sobjcount; ++i) {
		if ((sobjtable[i].x == x ) && (sobjtable[i].y == y)) return &sobjtable[i];
	}
	return NULL;
}

mobj_t *mobj_GetByPos(uint8_t x, uint8_t y) {
	uint8_t i;
	for (i=0; i<mobjcount; ++i) {
		if ((mobjtable[i].x == x ) && (mobjtable[i].y == y)) return &mobjtable[i];
	}
	return NULL;
}

sobj_t *sobj_GetByDest(uint8_t warpid) {
	uint8_t i,t;
	sobj_t *s;
	for (i=0,s=sobjtable; i<sobjcount; ++i,++s) {
		if (((s->type & SOBJTMSKHI) == SOBJ_WARPBASE) && (s->data == warpid)) return s;
	}
	return NULL;
	
}

int sobj_cardinals[] = {-128,-1,1,128};
//Returns: 0 = door does not go here, 1=top-down door, 2=side view door
uint8_t sobj_IsDoorable(uint8_t x, uint8_t y) {
	uint8_t i,c,msk;
	unsigned int cidx;
	
	cidx = (y&0x7F)*128+(x&0x7F);
	c = curmap->data[cidx];
	if (~((x|y)&0x80)) {
		if ((c<0x40)) return 0;  //No door if x,y is not a path.
	}
	msk = 0;
	for (i=0; i<4; ++i) {
		c = curmap->data[cidx+sobj_cardinals[i]];
		msk <<= 1;
		msk |= ((c<0x40) || (c>0x80));	//Bit set if is nonpath, zero if path
	}
	//dbg_sprintf(dbgout,"Candidate location (%i,%i), maskout: %X\n",x,y,msk);
	//If bitpattern in msk is 0b00000110, is top-down door (walls on side)
	if (msk == 0x06)	return 1;
	//Otherwise if mask pattern 0b00001001, is side view door.
	if (msk == 0x09)	return 2;
	//Or else we can't place jack.
	return 0;
}


void sobj_WriteToMap(void) {
	uint8_t i,ty,tyh,tyf,t;
	sobj_t *s;
	
	for (i=0,s=sobjtable; i< sobjcount; ++i,++s) {
		ty  = s->type;
		tyh = ty & SOBJTMSKHI;
		tyf = s->type & ~(SOBJTMSKLO|SOBJTMSKHI);
		t = 0;
		/* DOORS */
		if (tyh == SOBJ_DOORBASE) {
			if (tyf) {
				t = TILE_DOOROPEN;
			} else {
				if (ty == SOBJ_LOCKDOOR)
					t = TILE_DOORLOCKED;
				else if (ty == SOBJ_LOCKDOOR)
					t = TILE_DOORSEALED;
				else
					t = TILE_DOORCLOSED;
			}
			if (sobj_IsDoorable(s->x|0x80,s->y) == 2) ++t;
		/* WARPS */
		} else if (tyh == SOBJ_WARPBASE) {
			if (ty == SOBJ_WARPGATE) {
				t = TILE_PORTAL1;
			} else if (ty == SOBJ_WARPGATE2) {
				t = TILE_PORTAL2;
			} else if (ty == SOBJ_STAIRSUP) {
				t = TILE_STAIRSUP;
			} else if (ty == SOBJ_STAIRSDOWN) {
				t = TILE_STAIRSDOWN;
			} else {
				t = TILE_CANDLES;  //The thing you know because is wrong
			}
			
			if ((ty == SOBJ_WARPGATE) || (ty == SOBJ_WARPGATE2)) {
				if (tyf) {
					++t;
					if (stats.timer & 1)
						++t;
				}
			} else if ((ty == SOBJ_STAIRSUP) || (ty == SOBJ_STAIRSDOWN)) {
				if (stats.timer & 1)
					t+=2;
			}
		/* CHESTS */
		} else if (tyh == SOBJ_CHESTBASE) {
			if (ty == SOBJ_NORMCHEST) {
				t = TILE_TREASURECHEST;
			} else if (ty == SOBJ_LOCKCHEST) {
				t = TILE_TREASURELOCKED;
			} else if (ty == SOBJ_MLOKCHEST) {
				t = TILE_TREASURESEALED;
			} else if (ty == SOBJ_CRYSCHEST) {
				t = TILE_CRYSTALTREASURE;
			} else if (ty == SOBJ_FAKECHEST) {
				t = TILE_TREASURECHEST;
			} else ;
			
			if (tyf) ++t;
			
		/* TRAPS */
		} else if (tyh == SOBJ_TRAPBASE) {
		/* ITEMS */
		} else if (tyh == SOBJ_ITEMBASE) {
			if (ty == SOBJ_MCGUFFIN) {
				//data in sobj indicates which mcguffin. animates by +[0,1].
				t = TILE_MCGUFFIN1+s->data*2+(stats.timer&1);
			} else if (ty == SOBJ_CHERRY) {
				t = TILE_LCHERRY+(stats.timer&1);
			}
		/* unhandled messes */
		} else {
			t = 0;
		}
		//Write tile if tile was assigned
		if (t)	curmap->data[s->y*128+s->x] = t;
	}
}




