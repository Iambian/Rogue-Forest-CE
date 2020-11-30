/* Static objects access and behavior file */
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


sobj_t scratchsobj;






void sobj_clear(void) {
	uint8_t i;
	numsobjs = 0;
	for (i=0;i<250;++i) {
		sobjs[i] = empty_sobj;
	}
}

void sobj_addentry(sobj_t* sobj) {
	if (numsobjs<250) {
		memcpy(&sobjs[numsobjs],sobj,sizeof empty_sobj);
		++numsobjs;
	}
}
//TIL: sobj-sobjs results in an index rather than the result of
//subtracting two addresses.
uint8_t sobj_getindex(sobj_t *sobj) {
	return sobj-sobjs;
}
void sobj_rementry(uint8_t index) {
	uint8_t i;
	for (i=index;i<250;++i) {
		sobjs[i] = sobjs[i+1];
		//memcpy(&sobjs[i],&sobjs[i+1],sizeof(empty_sobj));
	}
	sobjs[i] = empty_sobj;
	--numsobjs;
}

sobj_t *sobj_getentrybypos(uint8_t x, uint8_t y) {
	uint8_t i;
	if (!numsobjs) return NULL;
	for (i=0; i<numsobjs; ++i) {
		if ((sobjs[i].x == x) && (sobjs[i].y == y)) {
				return &sobjs[i];
		}
	}
	return NULL;
}

//Searches for a warp point by its data/destination
sobj_t *sobj_getwarpbydest(uint8_t dest) {
	uint8_t i,t;
	sobj_t *s;
	if (!numsobjs) return NULL;
	for (i=0; i<numsobjs; ++i) {
		s = &sobjs[i];
		if ((s->type & SOBJTMSKHI) == SOBJ_WARPBASE) {
			if (s->data == dest) return s;
		}
	}
	return NULL;
}

void sobj_WriteToMap(void) {
	uint8_t i,tyh,tyl,tyf,t;
	sobj_t *s;
	
	for (i=0; i< numsobjs; ++i) {
		s = &sobjs[i];
		tyh = s->type & SOBJTMSKHI;
		tyl = s->type & SOBJTMSKLO;
		tyf = s->type & ~(SOBJTMSKLO|SOBJTMSKHI);
		t = 0;
		/* DOORS */
		if (tyh == SOBJ_DOORBASE) {
			if (tyf) {
				t = TILE_DOOROPEN;
			} else {
				if (tyl == (SOBJ_LOCKDOOR&SOBJTMSKLO))
					t = TILE_DOORLOCKED;
				else if (tyl == (SOBJ_LOCKDOOR&SOBJTMSKLO))
					t = TILE_DOORSEALED;
				else
					t = TILE_DOORCLOSED;
			}
			if (sobj_IsDoorable(s->x|0x80,s->y) == 2) ++t;
		/* WARPS */
		} else if (tyh == SOBJ_WARPBASE) {
			if (tyl == (SOBJ_WARPGATE&SOBJTMSKLO)) {
				t = TILE_PORTAL1;
			} else if (tyl == (SOBJ_WARPGATE2&SOBJTMSKLO)) {
				t = TILE_PORTAL2;
			} else if (tyl == (SOBJ_STAIRSUP&SOBJTMSKLO)) {
				t = TILE_STAIRSUP;
			} else if (tyl == (SOBJ_STAIRSDOWN&SOBJTMSKLO)) {
				t = TILE_STAIRSDOWN;
			} else {
				t = TILE_CANDLES;  //The thing you know because is wrong
			}
			
			if ((tyl == (SOBJ_WARPGATE&SOBJTMSKLO)) || (tyl == (SOBJ_WARPGATE2&SOBJTMSKLO))) {
				if (tyf) {
					++t;
					if (pstats.timer & 1)
						++t;
				}
			} else if ((tyl == (SOBJ_STAIRSUP&SOBJTMSKLO)) || (tyl == (SOBJ_STAIRSDOWN&SOBJTMSKLO))) {
				if (pstats.timer & 1)
					t+=2;
			}
		/* CHESTS */
		} else if (tyh == SOBJ_CHESTBASE) {
			if (tyl == (SOBJ_NORMCHEST&SOBJTMSKLO)) {
				t = TILE_TREASURECHEST;
			} else if (tyl == (SOBJ_LOCKCHEST&SOBJTMSKLO)) {
				t = TILE_TREASURELOCKED;
			} else if (tyl == (SOBJ_MLOKCHEST&SOBJTMSKLO)) {
				t = TILE_TREASURESEALED;
			} else if (tyl == (SOBJ_CRYSCHEST&SOBJTMSKLO)) {
				t = TILE_CRYSTALTREASURE;
			} else if (tyl == (SOBJ_FAKECHEST&SOBJTMSKLO)) {
				t = TILE_TREASURECHEST;
			} else ;
			
			if (tyf) ++t;
			
		/* TRAPS */
		} else if (tyh == SOBJ_TRAPBASE) {
		/* ITEMS */
		} else if (tyh == SOBJ_ITEMBASE) {
			if (tyl == (SOBJ_MCGUFFIN&SOBJTMSKLO)) {
				//data in sobj indicates which mcguffin. animates by +[0,1].
				t = TILE_MCGUFFIN1+s->data*2+(pstats.timer&1);
			} else if (tyl == (SOBJ_CHERRY&SOBJTMSKLO)) {
				t = TILE_LCHERRY+(pstats.timer&1);
			}
		/* unhandled messes */
		} else {
			t = 0;
		}
		//Write tile if tile was assigned
		if (t)	curmap->data[s->y*128+s->x] = t;
	}
}


/*	Warp point behavior. load new map and spawn player at receiving end.
	Generate dungeon and spawn player at recieving warp point. Maybe
	static object and dgen are separate? Figure out separation of
	responsibilities, along with how to load already-seen maps
*/

/*	When you go through a warp, the destination map changes. The spawn point
	is the warp whose destination is the map you came from. There can only
	be one such pairing, so always pick the first one that appears */







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
