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

void mobj_basicmove(mobj_t *mobj);
void mobj_zoomove(mobj_t *mobj);



/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

mobjdef_t playerdef[] = {
//	name field ,spriteobj ,scriptname 
//	mhp,mmp,str,spd,smr,atk,def,blk,ref,snk,per,rwr,mdf,mat,fdf,fat,edf,eat,pdf,pat
	{"Rawrs"   ,S_PLAYER0 ,NULL,
	 20,  6, 11, 12, 13,  1,  2,  3,  4,  5,  6,  7,  8,  9,  1,  2,  3,  4,  5,  6},
	{"Rawrs"   ,S_PLAYER0 ,NULL,
	 20,  6, 11, 12, 13,  1,  2,  3,  4,  5,  6,  7,  8,  9,  1,  2,  3,  4,  5,  6},
};

mobjdef_t enemydef[] = {
//	name field ,spriteobj ,scriptname 
//	mhp,mmp,str,spd,smr,atk,def,blk,ref,snk,per,rwr,mdf,mat,fdf,fat,edf,eat,pdf,pat
	{"NULL"    ,S_NULL    ,mobj_zoomove,
	 1,   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1},
	{"Rat"     ,S_NORMRAT ,mobj_zoomove,
	 10,  0,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{"Dire Rat",S_DIRERAT ,mobj_zoomove,
	 20,  0,  2,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{"Squirrel",S_SQUIRREL,mobj_zoomove,
	 10,  0,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{"Snake"   ,S_SNAKE,   mobj_zoomove,
	 10,  0,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	
};

uint8_t mobj_tilepassable[] = {
//	0	1	2	3	4	5	6	7	8	9	A	B	C	D	E	F	
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	//00
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	//10
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	//20
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	//30
	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	//40
	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	//50
	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	//60
	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	//70
	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	//80 traps and open doors
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	//90
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	//A0
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	//B0
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	//C0
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	//D0
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	//E0
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	//F0
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */


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
		ty  = s->type;								//Type in temp
		tyh = ty & SOBJTMSKHI;						//Get only type bits
		tyf = s->type & ~(SOBJTMSKLO|SOBJTMSKHI);	//Get only activate bit
		ty  = ty & 0x7F;							//Strip activate from rest
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

mobjdef_t *mobj_GetDef(mobj_t *mobj) {
	uint8_t i,t;
	
	t = mobj->type;
	if (t < MOB_PLAYER0) {
		if (t >= (sizeof(enemydef)/sizeof(mobjdef_t))) t = 0;
		return &enemydef[t];
	} else {
		t -= MOB_PLAYER0;
		if (t >= (sizeof(playerdef)/sizeof(mobjdef_t))) t = 0;
		return &playerdef[t];
	}
}


void mobj_RecalcPlayer(void) {
	uint8_t i,j,offset;
	int value;
	itemdef_t *item;
	item_t *gear;
	
	pcalc = pbase;
	
	for (i=0; i<(sizeof(equipment)/sizeof(item_t)); ++i) {
		gear = &equipment[i];
		if (gear->type) {
			for (j=0; j<2; ++j) {
				value  = items_GetStatValue(gear,j);
				offset = items_GetStatOffset(gear,j);
				if (value) {
					*(((uint8_t*)&pcalc)+offset) += value;
				}
			}
		}
	}
}

/* ~~~~~~~~~~ Combined Object Recognition and Interaction Routines ~~~~~~~~~ */
//action: kbit
//returns: 0 = nothing is blocking, 1 = sobj blocking, 2 = mobj blocking
//also returns: 0x80 + sobj->type for when object interact needs return
uint8_t obj_Collide(mobj_t *imobj, uint8_t x, uint8_t y, uint8_t action) {
	uint8_t i,t,ty,tyh,tyf;
	void *ptr;
	sobj_t *sobj;
	mobj_t *mobj;
	
	mobj = mobj_GetByPos(x,y);
	if (mobj) return 2;
	
	//atm using action key to ident between player and monsters.
	//Monsters will never push Alpha.
	ptr = &curmap[128*y+x];
	t = *(uint8_t*)ptr;
	if (t >= 0x80) {
		sobj = sobj_GetByPos(x,y);
		if (!sobj) return 0;	//well, then.
		ty = sobj->type;
		tyh= ty & SOBJTMSKHI;
		tyf= ty & ~(SOBJTMSKLO|SOBJTMSKHI);
		ty = ty ^ tyf;	//Always zero top bit so it doesn't interfere in typecheck
		switch (tyh) {
			case SOBJ_DOORBASE:
			if (tyf) return 0;
			if (ty == SOBJ_DOOR) { sobj->type |= 0x80; return 0; }
			break;
			
			case SOBJ_WARPBASE:
			if ((action & kbit_Alpha) && tyf) return 0x80|ty;
			return 0;
			break;
			
			case SOBJ_TRAPBASE:
			return 0;
			break;
			
			case SOBJ_CHESTBASE:
			if ((action & kbit_Alpha) && tyf) return 0x80|ty;
			return 0;
			break;
			
			case SOBJ_ITEMBASE:
			if ((action & kbit_Alpha) && tyf) return 0x80|ty;
			return 0;
			break;
			
			default:
			break;
		}
		return 1;
	}
	return 0;
}
uint8_t obj_Interact(mobj_t mobj, uint8_t x, uint8_t y) {
	
	return 0;
}


/* ~~~~~~~~~~~~~~~~~~~~~~ Mobile Object Movement Routines ~~~~~~~~~~~~~~~~~~ */

void mobj_PushMove(mobj_t* mobj, uint8_t newx, uint8_t newy) {
	int x,y;
	moving_t *m;
	
	m = &movingtable[movingcount];
	++movingcount;
	m->mobj = mobj;
	m->startx = mobj->x*16;
	m->starty = mobj->y*16;
	m->endx = newx*16;
	m->endy = newy*16;
	mobj->x = newx;
	mobj->y = newy;
	mobj->flags |= MSTAT_ISMOVING;

}

uint8_t mobj_trymove(mobj_t *mobj, int8_t dx, int8_t dy) {
	uint8_t cx,cy,t;
	cx = mobj->x + dx;
	cy = mobj->y + dy;
	t = curmap->data[cy*128+cx];
	return mobj_tilepassable[t];
}
//Only move if player is within 10 squares of enemy
void mobj_basicmove(mobj_t *mobj) {
	int8_t dx,dy;
	uint8_t nx,ny;
	
	nx = mobj->x;
	ny = mobj->y;
	dx = pmobj.x-nx;
	dy = pmobj.y-ny;
	mobj->flags &= ~MSTAT_ISMOVING;
	if ((abs(dx) < 10) && (abs(dy) < 10)) {
		dx = ((dx>0)?1:-1);
		dy = ((dy>0)?1:-1);
		if (mobj_trymove(mobj,dx,0)) nx = nx+dx;
		if (mobj_trymove(mobj,0,dy)) ny = ny+dy;
		if ((nx != mobj->x) && (ny != mobj->y)) {
			mobj_PushMove(mobj,nx,ny);
			mobj->flags |= MSTAT_ISMOVING;
		}
	}
}

void mobj_zoomove(mobj_t *mobj) {
	int8_t dx,dy;
	uint8_t nx,ny;
	
	mobj->flags &= ~MSTAT_ISMOVING;
	//if (randInt(0,10)>2) return;	//A sedate pace
	nx = mobj->x;
	ny = mobj->y;
	dx = randInt(-1,1);
	dy = randInt(-1,1);
	if (mobj_trymove(mobj,dx,0)) nx = nx+dx;
	if (mobj_trymove(mobj,0,dy)) ny = ny+dy;
	if ((nx != mobj->x) && (ny != mobj->y)) {
		mobj_PushMove(mobj,nx,ny);
		mobj->flags |= MSTAT_ISMOVING;
	}
}



