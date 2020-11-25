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
#include "items.h"
#include "gfx/output/gfx_base.h"
#include "gfx/output/gfx_dtiles.h"
#include "gfx/output/gfx_ftiles.h"
#include "gfx/output/gfx_charequtiles.h"



//playercalc is generated on every floor load and menu exit
//enemycalc is generated on each battle transaction
mobjdef_t emptyplayer,playerbase,playercalc,enemycalc;

mobjdef_t playerdef = {
//	name field ,spriteobj ,scriptname 
//	mhp,mmp,str,spd,smr,atk,def,blk,ref,snk,per,rwr,mdf,mat,fdf,fat,edf,eat,pdf,pat
	{"Rawrs"   ,NULL      ,NULL,
	 20,  6, 11, 12, 13,  1,  2,  3,  4,  5,  6,  7,  8,  9,  1,  2,  3,  4,  5,  6},
};

mobjdef_t enemydef[] = {
//	name field ,spriteobj ,scriptname 
//	mhp,mmp,str,spd,smr,atk,def,blk,ref,snk,per,rwr,mdf,mat,fdf,fat,edf,eat,pdf,pat
	{"Rat"     ,S_NORMRAT ,NULL,
	 10,  0,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{"Dire Rat",S_DIRERAT ,NULL,
	 20,  0,  2,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{"Squirrel",S_SQUIRREL,NULL,
	 10,  0,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	
};

void mobj_newchar(void) {
	uint8_t i,j;
	
	//Reset player stats
	playerbase = playerdef;
	//Reset inventories
	for (i=0;i<35;++i) {
		inventory[i].type = 0;
		inventory[i].data = 0;
	}
	for (i=0;i<8;++i) {
		equipment[i].type = 0;
		equipment[i].data = 0;
	}
	for (i=0;i<10;++i) {
		quickbar[i].type = 0;
		quickbar[i].data = 0;
	}
	/* DEBUGGING -- SET PLAYER INVENTORY WITH STUFFS */
	
	for (j=0,i=1;i<64;i+=4,++j) {
		inventory[j].type = i;
		inventory[j].data = 0;
	}
	for (i=ITEM_CONSUMABLE; i < CONS_UNUSEDPOTION1 ; ++i,++j) {
		inventory[j].type = i;
		inventory[j].data = 1;
	}
	
	equipment[0].type = EQU_SNEAKRING;
	equipment[0].data = 0;
	equipment[1].type = EQU_RAWRSRING;
	equipment[1].data = 0;
	equipment[2].type = EQU_HAT;
	equipment[2].data = 0;
	equipment[3].type = EQU_LEATHERARMOR;
	equipment[3].data = 0;
	equipment[4].type = EQU_BADTOUCHER;
	equipment[4].data = 0;
	equipment[5].type = EQU_SHADOWSWORD;
	equipment[5].data = 0;
	equipment[6].type = EQU_NINJABOOTS;
	equipment[6].data = 0;
	equipment[7].type = EQU_REFLECTSHIELD;
	equipment[7].data = 0;
}

void mobj_clear(void) {
	uint8_t i;
	nummobjs = 0;
	for (i=0;i<250;++i) {
		mobjs[i] = empty_mobj;
	}
}

void mobj_addentry(mobj_t* mobj) {
	if (nummobjs<250) {
		memcpy(&mobjs[nummobjs],mobj,sizeof empty_mobj);
		++nummobjs;
	}
}
void mobj_rementry(uint8_t index) {
	uint8_t i;
	for (i=index;i<250;++i) {
		mobjs[i] = mobjs[i+1];
	}
	mobjs[i] = empty_mobj;
}

void mobj_recalcplayer(void) {
	uint8_t i,j,offset;
	int t,enchantval;
	itemdef_t *item;
	playercalc = playerbase;
	for (i = 0; i<8; ++i) {
		if (equipment[i].type) {
			item = &equipdefs[equipment[i].type];
			offset = item->offset1;
			t = item->modifier1;
			if (t) {
				*(((uint8_t*)&playercalc)+offset) += t + (item->enchantmult * t)>>8;
			}
			offset = item->offset2;
			t = item->modifier2;
			if (t) {
				*(((uint8_t*)&playercalc)+offset) += t + (item->enchantmult * t)>>8;
			}
		}
	}
}





















