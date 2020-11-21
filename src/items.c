#include <stddef.h>
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
#include "items.h"
#include "gfx/output/gfx_base.h"
#include "gfx/output/gfx_dtiles.h"
#include "gfx/output/gfx_ftiles.h"
#include "gfx/output/gfx_charequtiles.h"

gfx_sprite_t *items_GetItemSprite(uint8_t id);

#define fl2fx(x) (x*256)
#define MD(member) offsetof(pstats_t,member)
#define pstats_io(offset) *((uint8_t*)pstats+offset)


char *item_noname = "Empty\nslot";
char *item_nodesc = "Fill me with stuff!";
char *item_illegal = "Illegal\nitem";
char *item_illdesc = "Memory got corrupted.";
char descbuf[80];


//Type/id is indexed by position in this array
itemdef_t equipdefs[] = {
	//  item display name,enchantmlt,val1,      offset1,val2, offset2
	{"Robe\n"            ,fl2fx(1.0),   4, MD(magic)   ,   0,MD(empty)},
	{"Leather\nArmor"    ,fl2fx(1.0),   2, MD(defense) ,   0,MD(empty)},
	{"Metal\nArmor"      ,fl2fx(1.0),   4, MD(defense) ,  -5,MD(sneaks)},
	{"Shiny\nArmor"      ,fl2fx(1.0),   4, MD(defense) ,   6,MD(reflect)},
	
	{"Bigbrain\nPendant" ,fl2fx(1.0),   4, MD(smarts)  ,   0,MD(empty)},
	{"Stonks\nPendant"   ,fl2fx(1.0),   4, MD(strength),   0,MD(empty)},
	{"Windy\nPendant"    ,fl2fx(1.0),   4, MD(speeds)  ,   0,MD(empty)},
	{"Mystic\nPendant"   ,fl2fx(5.0),  20, MD(maxmp)   ,   0,MD(empty)},
	
	{"Beefy\nPendant"    ,fl2fx(5.0),  20, MD(maxhp)   ,   0,MD(empty)},
	{"Tanky\nPendant"    ,fl2fx(1.0),   6, MD(defense) ,   0,MD(empty)},
	{"Bouncy\nPendant"   ,fl2fx(1.0),   8, MD(reflect) ,   0,MD(empty)},
	{"BOOSH\nPendant"    ,fl2fx(1.0),  10, MD(strength),  10,MD(smarts)},
	
	{"Shell\nRing"       ,fl2fx(2.0),   6, MD(mres)    ,   0,MD(empty)},
	{"Shock\nRing"       ,fl2fx(1.0),   6, MD(eres)    ,   3,MD(eatk)},
	{"Blaze\nRing"       ,fl2fx(1.0),   6, MD(fres)    ,   3,MD(fatk)},
	{"Toxic\nRing"       ,fl2fx(1.0),   6, MD(pres)    ,   3,MD(patk)},
	
	{"Muscle\nRing"      ,fl2fx(1.0),   4, MD(strength),   0,MD(empty)},	//17
	{"Shield\nRing"      ,fl2fx(1.0),   4, MD(defense) ,   0,MD(empty)},
	{"Manna\nRing"       ,fl2fx(4.0),  15, MD(maxmp)   ,   0,MD(empty)},
	{"Sneaky\nRing"      ,fl2fx(1.0),   4, MD(sneaks)  ,   4,MD(evasion)},
	
	{"Punchy\nRing"      ,fl2fx(1.0),   8, MD(attack)  ,   0,MD(empty)},
	{"Magic\nRing"       ,fl2fx(1.0),   4, MD(magic)   ,   0,MD(empty)},
	{"Healthy\nRing"     ,fl2fx(1.0),  15, MD(maxhp)   ,   0,MD(empty)},
	{"Rawrs\nRing"       ,fl2fx(1.0),   4, MD(rawrs)   ,   0,MD(empty)},
	
	{"Bouncy\nRing"      ,fl2fx(1.0),   4, MD(reflect) ,   0,MD(empty)},
	{"Unused\nRing 1"    ,fl2fx(8.0),  20, MD(magic)   ,  20,MD(attack)},
	{"Unused\nRing 2"    ,fl2fx(8.0),  20, MD(defense) ,  20,MD(mres)},
	{"Wall\nRing"        ,fl2fx(1.0),   4, MD(defense) ,   4,MD(mres)},
	
	{"Shoes\n"           ,fl2fx(1.0),   4, MD(maxmp)   ,   0,MD(empty)},
	{"Leather\nBoots"    ,fl2fx(1.0),   4, MD(defense) ,   0,MD(empty)},
	{"Metal\nBoots"      ,fl2fx(1.0),   6, MD(defense) ,  -5,MD(sneaks)},
	{"Ninja\nBoots"      ,fl2fx(1.0),   6, MD(sneaks)  ,   6,MD(evasion)},
	
	{"Leather\nCap"      ,fl2fx(1.0),   4, MD(defense) ,   0,MD(empty)},	//33
	{"Hat\n"             ,fl2fx(1.0),   6, MD(maxmp)   ,   0,MD(empty)},
	{"Helm\n"            ,fl2fx(1.0),   6, MD(defense) ,  -5,MD(sneaks)},
	{"Shiny\nHelm"       ,fl2fx(1.0),   4, MD(defense) ,   4,MD(reflect)},
	
	{"Thief\nGlove"      ,fl2fx(1.0),   4, MD(maxmp)   ,   4,MD(sneaks)},
	{"Leather\nGlove"    ,fl2fx(1.0),   4, MD(defense) ,   0,MD(empty)},
	{"Battle\nMitts"     ,fl2fx(1.0),   4, MD(attack)  ,   2,MD(strength)},
	{"The Bad\nTouch"    ,fl2fx(2.0),  10, MD(attack)  ,  10,MD(magic)},
	
	{"Buckler\n"         ,fl2fx(1.0),   2, MD(defense) ,   4,MD(evasion)},
	{"Small\nShield"     ,fl2fx(1.0),   4, MD(defense) ,   4,MD(evasion)},
	{"Tower\nShield"     ,fl2fx(1.0),   6, MD(defense) ,   6,MD(evasion)},
	{"Shiny\nShield"     ,fl2fx(1.0),   8, MD(defense) ,  10,MD(reflect)},
	
	{"Magic\nMissile"    ,fl2fx(1.0),   5, MD(magic)   ,   0,MD(empty)},
	{"Boomy\nSparks"     ,fl2fx(1.0),   5, MD(eatk)    ,   0,MD(empty)},
	{"Fireball\n"        ,fl2fx(1.0),   5, MD(fatk)    ,   0,MD(empty)},
	{"Poison\nBolt"      ,fl2fx(1.0),   5, MD(patk)    ,   0,MD(empty)},
	
	{"Short\nSword"      ,fl2fx(1.0),   4, MD(attack)  ,   0,MD(empty)},	//49
	{"Long\nSword"       ,fl2fx(1.0),   6, MD(attack)  ,   0,MD(empty)},
	{"Buster\nSword"     ,fl2fx(1.0),   8, MD(attack)  ,   0,MD(empty)},
	{"Sneaky\nSword"     ,fl2fx(1.0),   8, MD(attack)  ,   4,MD(sneaks)},
	
	{"Gold\nDigger"      ,fl2fx(1.0),   4, MD(attack)  ,   0,MD(empty)},
	{"Poleaxe\n"         ,fl2fx(1.0),   8, MD(attack)  ,   0,MD(empty)},
	{"Halbeard\n"        ,fl2fx(1.0),   8, MD(attack)  ,   0,MD(empty)},
	{"Pointy\nStick"     ,fl2fx(1.0),   6, MD(attack)  ,   0,MD(empty)},
	
	{"Saftey\nGoggles"   ,fl2fx(1.0),   0, MD(empty)   ,   0,MD(empty)},
	{"Prayer\nBeads"     ,fl2fx(4.0),  20, MD(maxmp)   ,   2,MD(magic)},
	{"Crystal\nBall"     ,fl2fx(1.0),  10, MD(magic)   ,  10,MD(mres)},
	{"Escape\nRope"      ,fl2fx(1.0),  10, MD(sneaks)  ,   0,MD(empty)},

	{"Short\nBow"        ,fl2fx(1.0),   4, MD(attack) ,   0,MD(empty)},
	{"Long\nBow"         ,fl2fx(1.0),   6, MD(attack) ,   0,MD(empty)},
	{"Sling\n"           ,fl2fx(1.0),   4, MD(attack) ,   0,MD(empty)},
	{"Hand\nCannon"      ,fl2fx(1.0),  10, MD(attack) ,  -5,MD(sneaks)},
	
};

char *consumenames[][2] = {
	{"Scroll'o\nBlink"   ,"Teleports to cursor"},
	{"Scroll'o\nZappity" ,"Shocks adjacent targets too"},
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











gfx_sprite_t *items_GetItemSprite(uint8_t id) {
	uint8_t tmpid;
	void *ptr;
	
	if (!id) return equipicons_tile_0;
	tmpid = id - ITEM_EQUIPPABLE;
	if (tmpid < equips_tiles_num) return equips_tiles[tmpid];
	
	if (id < ITEM_CONSUMABLE) return equipicons_tile_0;
	tmpid = id - ITEM_CONSUMABLE;
	if (tmpid < consumables_tiles_num) return consumables_tiles[tmpid];
	
	return equipicons_tile_0; 
}




char *items_GetItemName(uint8_t id) {
	uint8_t tmpid;
	
	if (!id) return item_noname;
	tmpid = id - ITEM_EQUIPPABLE;
	if (tmpid < (sizeof(equipdefs)/sizeof(itemdef_t))) return equipdefs[tmpid].name;
	if (id < ITEM_CONSUMABLE) return item_illegal;
	tmpid = id - ITEM_CONSUMABLE;
	if (tmpid < consumables_tiles_num) return consumenames[tmpid][0];
	return item_illegal; 
}

char *items_GetItemDesc(uint8_t id) {
	uint8_t tmpid;
	
	if (!id) return item_nodesc;
	tmpid = id - ITEM_EQUIPPABLE;
	if (tmpid < (sizeof(equipdefs)/sizeof(itemdef_t))) {
		//OH BOY. YOU GOTTA CONSTRUCT THE DESCRIPTION INTO descbuf AND
		//PASS THAT THROUGH FOR IMMEDIATE RENDER
		return ""; 
	}
	if (id < ITEM_CONSUMABLE) return item_illdesc;
	tmpid = id - ITEM_CONSUMABLE;
	if (tmpid < consumables_tiles_num) return consumenames[tmpid][1];
	return item_illdesc;
}

void items_PrintItemname_Left(uint8_t id) {
	char *s;
	int x,y;
	x = gfx_GetTextX();
	y = gfx_GetTextY();
	s = items_GetItemName(id);
	while (*s) {
		if (*s == '\n')	gfx_SetTextXY(x,y+8);
		else			gfx_PrintChar(*s);
		++s;
	}
}

void items_PrintItemname_Bottom(uint8_t id) {
	char *s;
	int x,y;
	s = items_GetItemName(id);
	while (*s) {
		if (*s == '\n')	gfx_PrintChar(' ');
		else			gfx_PrintChar(*s);
		++s;
	}
}




