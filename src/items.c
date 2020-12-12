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

char *items_StatDesc(uint8_t offset);



#define fl2fx(x) (x*256)
#define MD(member) offsetof(mobjdef_t,member)
#define pstats_io(offset) *((uint8_t*)pstats+offset)


/* 'name' is the empty entity with which to flag that this stat isn't available.
	Alternatively, value being zero can be used for this in a more portable way */
//Type/id is indexed by position in this array
itemdef_t equipdefs[] = {
	{"Empty\nItem"       ,EFL_MSC, fl2fx(1.0),   0, MD(name) ,   0,MD(name)},
	//  item display name,equtype, enchantmlt,val1,   offset1,val2, offset2
	{"Robe\n"            ,EFL_ARM, fl2fx(1.0),   4, MD(matk) ,   0,MD(name)},
	{"Leather\nArmor"    ,EFL_ARM, fl2fx(1.0),   2, MD(def)  ,   0,MD(name)},
	{"Metal\nArmor"      ,EFL_ARM, fl2fx(1.0),   4, MD(def)  ,  -5,MD(snk)},
	{"Shiny\nArmor"      ,EFL_ARM, fl2fx(1.0),   4, MD(def)  ,   6,MD(refl)},
	{"Bigbrain\nPendant" ,EFL_ARM, fl2fx(1.0),   4, MD(smrt) ,   0,MD(name)},
	{"Stonks\nPendant"   ,EFL_ARM, fl2fx(1.0),   4, MD(str)  ,   0,MD(name)},
	{"Windy\nPendant"    ,EFL_ARM, fl2fx(1.0),   4, MD(spd)  ,   0,MD(name)},
	{"Mystic\nPendant"   ,EFL_ARM, fl2fx(5.0),  20, MD(maxmp),   0,MD(name)},
	{"Beefy\nPendant"    ,EFL_ARM, fl2fx(5.0),  20, MD(maxhp),   0,MD(name)},
	{"Tanky\nPendant"    ,EFL_ARM, fl2fx(1.0),   6, MD(def)  ,   0,MD(name)},
	{"Bouncy\nPendant"   ,EFL_ARM, fl2fx(1.0),   8, MD(refl) ,   0,MD(name)},
	{"BOOSH\nPendant"    ,EFL_ARM, fl2fx(1.0),  10, MD(str)  ,  10,MD(smrt)},
	{"Shell\nRing"       ,EFL_RNG, fl2fx(2.0),   6, MD(mdef) ,   0,MD(name)},
	{"Shock\nRing"       ,EFL_RNG, fl2fx(1.0),   6, MD(edef) ,   3,MD(eatk)},
	{"Blaze\nRing"       ,EFL_RNG, fl2fx(1.0),   6, MD(fdef) ,   3,MD(fatk)},
	{"Toxic\nRing"       ,EFL_RNG, fl2fx(1.0),   6, MD(pdef) ,   3,MD(patk)},
	//  item display name,equtype, enchantmlt,val1,   offset1,val2, offset2
	{"Muscle\nRing"      ,EFL_RNG, fl2fx(1.0),   4, MD(str)  ,   0,MD(name)},	//17
	{"Shield\nRing"      ,EFL_RNG, fl2fx(1.0),   4, MD(def)  ,   0,MD(name)},
	{"Manna\nRing"       ,EFL_RNG, fl2fx(4.0),  15, MD(maxmp),   0,MD(name)},
	{"Sneaky\nRing"      ,EFL_RNG, fl2fx(1.0),   4, MD(snk)  ,   4,MD(blk)},
	{"Punchy\nRing"      ,EFL_RNG, fl2fx(1.0),   8, MD(atk)  ,   0,MD(name)},
	{"Magic\nRing"       ,EFL_RNG, fl2fx(1.0),   4, MD(matk) ,   0,MD(name)},
	{"Healthy\nRing"     ,EFL_RNG, fl2fx(1.0),  15, MD(maxhp),   0,MD(name)},
	{"Rawrs\nRing"       ,EFL_RNG, fl2fx(1.0),   4, MD(rawrs),   0,MD(name)},
	{"Bouncy\nRing"      ,EFL_RNG, fl2fx(1.0),   4, MD(refl) ,   0,MD(name)},
	{"Unused\nRing 1"    ,EFL_RNG, fl2fx(8.0),  20, MD(matk) ,  20,MD(atk)},
	{"Unused\nRing 2"    ,EFL_RNG, fl2fx(8.0),  20, MD(def)  ,  20,MD(mdef)},
	{"Wall\nRing"        ,EFL_RNG, fl2fx(1.0),   4, MD(def)  ,   4,MD(mdef)},
	{"Shoes\n"           ,EFL_BTS, fl2fx(1.0),   4, MD(maxmp),   0,MD(name)},
	{"Leather\nBoots"    ,EFL_BTS, fl2fx(1.0),   4, MD(def)  ,   0,MD(name)},
	{"Metal\nBoots"      ,EFL_BTS, fl2fx(1.0),   6, MD(def)  ,  -5,MD(snk)},
	{"Ninja\nBoots"      ,EFL_BTS, fl2fx(1.0),   6, MD(snk)  ,   6,MD(blk)},
	//  item display name,equtype, enchantmlt,val1,   offset1,val2, offset2
	{"Leather\nCap"      ,EFL_HAT, fl2fx(1.0),   4, MD(def)  ,   0,MD(name)},	//33
	{"Hat\n"             ,EFL_HAT, fl2fx(1.0),   6, MD(maxmp),   0,MD(name)},
	{"Helm\n"            ,EFL_HAT, fl2fx(1.0),   6, MD(def)  ,  -5,MD(snk)},
	{"Shiny\nHelm"       ,EFL_HAT, fl2fx(1.0),   4, MD(def)  ,   4,MD(refl)},
	{"Thief\nGlove"      ,EFL_GLO, fl2fx(1.0),   4, MD(maxmp),   4,MD(snk)},
	{"Leather\nGlove"    ,EFL_GLO, fl2fx(1.0),   4, MD(def)  ,   0,MD(name)},
	{"Battle\nMitts"     ,EFL_GLO, fl2fx(1.0),   4, MD(atk)  ,   2,MD(str)},
	{"The Bad\nTouch"    ,EFL_GLO, fl2fx(2.0),  10, MD(atk)  ,  10,MD(matk)},
	{"Buckler\n"         ,EFL_2ND, fl2fx(1.0),   2, MD(def)  ,   4,MD(blk)},
	{"Small\nShield"     ,EFL_2ND, fl2fx(1.0),   4, MD(def)  ,   4,MD(blk)},
	{"Tower\nShield"     ,EFL_2ND, fl2fx(1.0),   6, MD(def)  ,   6,MD(blk)},
	{"Shiny\nShield"     ,EFL_2ND, fl2fx(1.0),   8, MD(def)  ,  10,MD(refl)},
	{"Magic\nMissile"    ,EFL_2ND, fl2fx(1.0),   5, MD(matk) ,   0,MD(name)},
	{"Boomy\nSparks"     ,EFL_2ND, fl2fx(1.0),   5, MD(eatk) ,   0,MD(name)},
	{"Fireball\n"        ,EFL_2ND, fl2fx(1.0),   5, MD(fatk) ,   0,MD(name)},
	{"Poison\nBolt"      ,EFL_2ND, fl2fx(1.0),   5, MD(patk) ,   0,MD(name)},
	//  item display name,equtype, enchantmlt,val1,   offset1,val2, offset2
	{"Short\nSword"      ,EFL_WPN, fl2fx(1.0),   4, MD(atk)  ,   0,MD(name)},	//49
	{"Long\nSword"       ,EFL_WPN, fl2fx(1.0),   6, MD(atk)  ,   0,MD(name)},
	{"Buster\nSword"     ,EFL_WPN, fl2fx(1.0),   8, MD(atk)  ,   0,MD(name)},
	{"Sneaky\nSword"     ,EFL_WPN, fl2fx(1.0),   8, MD(atk)  ,   4,MD(snk)},
	{"Gold\nDigger"      ,EFL_WPN, fl2fx(1.0),   4, MD(atk)  ,   0,MD(name)},
	{"Poleaxe\n"         ,EFL_WPN, fl2fx(1.0),   8, MD(atk)  ,   0,MD(name)},
	{"Halbeard\n"        ,EFL_WPN, fl2fx(1.0),   8, MD(atk)  ,   0,MD(name)},
	{"Pointy\nStick"     ,EFL_WPN, fl2fx(1.0),   6, MD(atk)  ,   0,MD(name)},
	{"Saftey\nGoggles"   ,EFL_MSC, fl2fx(1.0),   0, MD(name) ,   0,MD(name)},
	{"Prayer\nBeads"     ,EFL_MSC, fl2fx(4.0),  20, MD(maxmp),   2,MD(matk)},
	{"Crystal\nBall"     ,EFL_MSC, fl2fx(1.0),  10, MD(matk) ,  10,MD(mdef)},
	{"Escape\nRope"      ,EFL_MSC, fl2fx(1.0),  10, MD(snk)  ,   0,MD(name)},
	{"Short\nBow"        ,EFL_WPN, fl2fx(1.0),   4, MD(atk)  ,   0,MD(name)},
	{"Long\nBow"         ,EFL_WPN, fl2fx(1.0),   6, MD(atk)  ,   0,MD(name)},
	{"Sling\n"           ,EFL_WPN, fl2fx(1.0),   4, MD(atk)  ,   0,MD(name)},
	{"Hand\nCannon"      ,EFL_WPN, fl2fx(1.0),  10, MD(atk)  ,  -5,MD(snk)},
	
};

char *consumenames[][2] = {
	{"Scroll'o\nBlink"   ,"Teleports to cursor"},
	{"Scroll'o\nZappity" ,"Shocks adjacent targets"},
	{"Scroll'o\nHellfire","Full-screen burnination"},
	{"Scroll'o\nGoodies" ,"Get random goodies"},
	{"Potion'o\nHealth"  ,"Refill HP or +4 max HP"},
	{"Potion'o\nMana"    ,"Refill MP or +4 max MP"},
	{"Potion'o\nDeja Vu" ,"Doubles experiences"},
	{"Potion'o\nFleshgro","Regenerates HP faster"},
	{"Potion'o\nFarsight","Regenerates MP faster"},
	{"Potion'o\nAngries" ,"+4 strength and attack"},
	{"Ironhide\nPotion"  ,"+4 def, mdef, and reflect"},
	{"Bottled\nRawrfs"   ,"+10 rawrfs"},
	{"Unused\nPotion 1"  ,"Fancy red thing"},
	{"Unused\nPotion 2"  ,"Splooshy ghostly blue"},
	{"Unused\nPotion 3"  ,"Fancy midnight skies"},
	{"Unused\nPotion 4"  ,"Mercurial Lawsuits"},
	{"Unused\n Wand 1"   ,"Sprite p3n1$."},
	{"Unused\n Wand 2"   ,"Goldenrod showers"},
	{"Unused\n Wand 3"   ,"Was a piece of bark"},
	{"Unused\n Wand 4"   ,"Iron bar with a gold heart"},
	{"Unused\n Wand 5"   ,"Flimsy bar with an iron heart"},
	{"Unused\n Wand 6"   ,"Hellfire tuning fork"},
	{"Unused\n Wand 7"   ,"Mars bars with a side of glitter"},
	{"Unused\n Wand 8"   ,"Sprite m1c0p3n!5."},

};


char *items_StatDesc(uint8_t offset) {
	switch (offset) {
		case offsetof(mobjdef_t,maxhp): return "Max HP ";
		case offsetof(mobjdef_t,maxmp): return "Max MP ";
		case offsetof(mobjdef_t,str  ): return "Strength ";
		case offsetof(mobjdef_t,spd  ): return "Speed ";
		case offsetof(mobjdef_t,smrt ): return "Smarts ";
		case offsetof(mobjdef_t,atk  ): return "Attack ";
		case offsetof(mobjdef_t,def  ): return "Defense ";
		case offsetof(mobjdef_t,blk  ): return "Block ";
		case offsetof(mobjdef_t,refl ): return "Reflect ";
		case offsetof(mobjdef_t,snk  ): return "Sneakiness ";
		case offsetof(mobjdef_t,perc ): return "Perception ";
		case offsetof(mobjdef_t,rawrs): return "Rawrs ";
		case offsetof(mobjdef_t,mdef ): return "Magic def ";
		case offsetof(mobjdef_t,matk ): return "Magic atk ";
		case offsetof(mobjdef_t,fdef ): return "Fire def ";
		case offsetof(mobjdef_t,fatk ): return "Fire atk ";
		case offsetof(mobjdef_t,edef ): return "Electric def ";
		case offsetof(mobjdef_t,eatk ): return "Electric atk ";
		case offsetof(mobjdef_t,pdef ): return "Poison def ";
		case offsetof(mobjdef_t,patk ): return "Poison atk ";
		case 0:							return "NULL ";
		default:						return "Unknown stat ";
	}
}

uint8_t items_Filter(item_t *item) {
	uint8_t t;
	t = item->type;
	if ((t >= ITEM_EQUIPPABLE) && (t < EQU_EQUIPPABLE_END)) return t;
	if ((t >= ITEM_CONSUMABLE) && (t < CONS_CONSUMABLE_END)) return t;
	return 0;
}

uint8_t items_itemdefmodifiers[] =  {offsetof(itemdef_t,modifier1),offsetof(itemdef_t,modifier2)};
uint8_t items_itemdefoffsets[] = {offsetof(itemdef_t,offset1),offsetof(itemdef_t,offset2)};
itemdef_t *items_FilterDef(item_t *item) { 
	uint8_t t; 
	t = items_Filter(item); 
	if (!t || t >= EQU_EQUIPPABLE_END)	return &equipdefs[0]; //emptyitem
	else 								return &equipdefs[t]; 
}

int items_GetStatValue(item_t *item, uint8_t iter) {	//Iter will either be 0 or 1.
	uint8_t t;
	int temp;
	itemdef_t *idef;
	
	idef = items_FilterDef(item);
	temp = *(((uint8_t*)idef)+items_itemdefmodifiers[iter]);
	return temp + ((idef->enchantmult * temp * item->data) / 256);
}

uint8_t items_GetStatOffset(item_t *item, uint8_t iter) {
	return *(((uint8_t*)items_FilterDef(item))+items_itemdefoffsets[iter]);
}

gfx_sprite_t *items_GetSprite(item_t *item) {
	uint8_t t;
	t = items_Filter(item);
	if (!t)	return equipicons_tile_0;
	if (t < EQU_EQUIPPABLE_END)	return equips_tiles[t-ITEM_EQUIPPABLE];
	else						return consumables_tiles[t-ITEM_CONSUMABLE];
}

char *items_GetName(item_t *item) {
	uint8_t t;
	t = items_Filter(item);
	if (!t) return "Empty\nslot";
	if (t < EQU_EQUIPPABLE_END) {
		util_BufClr();
		if (item->data) {
			util_BufInt(item->data,0x81);
			util_BufChr(' ');
		}
		util_BufStr(equipdefs[t].name);
		return stringbuf;
	}
	else {
		return consumenames[t-ITEM_CONSUMABLE][0];
	}
}

char *items_GetDesc(item_t *item) {
	uint8_t t,i;
	t = items_Filter(item);
	if (!t) return "Nothing to see here";
	if (t < EQU_EQUIPPABLE_END) {
		util_BufClr();
		for (i=0; i<2; ++i) {
			t = items_GetStatValue(item,i);
			if (t) {
				if (i==1) util_BufStr(", ");
				util_BufStr(items_StatDesc(items_GetStatOffset(item,i)));
				util_BufInt(t,0x81);
			}
		}
		return stringbuf;
	} else {
		return consumenames[t-ITEM_CONSUMABLE][1];
	}
}
/* bit 7 = in equipment, bit 6 = in quickbar. undefined if both set */
item_t *items_GetPtrFromCursor(uint8_t cursor) {
	uint8_t t;
	t = cursor & 0x3F;
	if (cursor & 0x80) return &equipment[t];
	if (cursor & 0x40) return &quickbar[t];
	return &inventory[t];
}


uint8_t items_verifygeartable[] = {EFL_RNG,EFL_RNG,EFL_HAT,EFL_ARM,EFL_GLO,EFL_WPN,EFL_BTS,EFL_2ND};
uint8_t items_verifygear(uint8_t cursor) {
	uint8_t t;
	item_t *item;
	
	item = items_GetPtrFromCursor(cursor);
	t = cursor & 0x3F;
	if (cursor & 0x80) {
		if (!item->type) return 1;
		if (items_FilterDef(item)->type & items_verifygeartable[t]) return 1;
		return 0;
	}
	if (cursor & 0x40) {
		if ((item->type >= ITEM_CONSUMABLE) && (item->type < CONS_CONSUMABLE_END))
			return 1;
		return 0;
	}
	return 1;
}

uint8_t items_SwapSlots(uint8_t cursor1, uint8_t cursor2) {
	item_t *item1,*item2;
	
	item1 = items_GetPtrFromCursor(cursor1);
	item2 = items_GetPtrFromCursor(cursor2);
	util_MemSwap(item1,item2,sizeof(item_t));
	if (items_verifygear(cursor1) && items_verifygear(cursor2)) return 1;
	util_MemSwap(item1,item2,sizeof(item_t));
	return 0;
}

item_t *items_FindEmptySlot(void) {
	uint8_t i;
	item_t *item;
	
	for (i=0; i<34; ++i) {	//34, not 35 to avoid trashcan slot
		if (!((item=&inventory[i])->type)) return item;
	}
	return NULL;
}





















