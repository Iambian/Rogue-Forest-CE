#ifndef __ITEMS__INCGUARD__
#define __ITEMS__INCGUARD__

#include <stdint.h>
#include "main.h"

#define EFL_RNG (1<<0)
#define EFL_HAT (1<<1)
#define EFL_ARM (1<<2)
#define EFL_GLO (1<<3)
#define EFL_WPN (1<<4)
#define EFL_BTS (1<<5)
#define EFL_2ND (1<<6)
#define EFL_MSC (1<<7)


// ========================
#define ITEM_EQUIPPABLE 1

#define EQU_ROBE 1
#define EQU_LEATHERARMOR 2
#define EQU_METALARMOR 3
#define EQU_REFLECTARMOR 4

#define EQU_INTPENDANT 5
#define EQU_STRPENDANT 6
#define EQU_AGIPENDANT 7
#define EQU_MPPENDANT 8
#define EQU_HPPENDANT 9
#define EQU_DEFPENDANT 10
#define EQU_REFLECTPENDANT 11
#define EQU_ALLMIGHTPENDANT 12

#define EQU_MDEFRING 13
#define EQU_ELECDEFRING 14
#define EQU_FIREDEFRING 15
#define EQU_POISONDEFRING 16

#define EQU_STRRING 17
#define EQU_DEFRING 18
#define EQU_MPRING 19
#define EQU_SNEAKRING 20

#define EQU_ATKRING 21
#define EQU_MAGRING 22
#define EQU_HPRING 23
#define EQU_RAWRSRING 24

#define EQU_REFLECTRING 25
#define EQU_UNUSEDRING1 26
#define EQU_UNUSEDRING2 27
#define EQU_WALLRING 28

#define EQU_SHOES 29
#define EQU_BOOTS 30
#define EQU_METALBOOTS 31
#define EQU_NINJABOOTS 32

#define EQU_LEATHERCAP 33
#define EQU_HAT 34
#define EQU_METALHAT 35
#define EQU_REFLECTHAT 36

#define EQU_THIEFGLOVE 37
#define EQU_LEATHERGLOVE 38
#define EQU_BATTLEMITTS 39
#define EQU_BADTOUCHER 40

#define EQU_BUCKLER 41
#define EQU_SMALLSHIELD 42
#define EQU_TOWERSHIELD 43
#define EQU_REFLECTSHIELD 44

#define EQU_MAGICMISSILE 45
#define EQU_LIGHTNING 46
#define EQU_FIREBALL 47
#define EQU_POISONBOLT 48

#define EQU_SHORTSWORD 49
#define EQU_LONGSWORD 50
#define EQU_BUSTERSWORD 51
#define EQU_SHADOWSWORD 52

#define EQU_MININGPICK 53
#define EQU_LONGAXE 54
#define EQU_HALBERD 55
#define EQU_SPEAR 56

#define EQU_SAFTEYGOGGLES 57
#define EQU_PRAYERBEADS 58
#define EQU_CRYSTALBALL 59
#define EQU_ESCAPEROPE 60

#define EQU_SHORTBOW 61
#define EQU_LONGBOW 62
#define EQU_SLING 63
#define EQU_HANDRIFLE 64

// ========================
#define ITEM_CONSUMABLE 192

#define CONS_BLINK 192
#define CONS_CHAINLIGHTNING 193
#define CONS_HELLFIRE 194
#define CONS_ACQUIREMENT 195

#define CONS_HEALTH 196
#define CONS_MANA 197
#define CONS_EXPERIENCE 198
#define CONS_REGENERATION 199

#define CONS_FARSIGHT 200
#define CONS_BLOODTHIRST 201
#define CONS_IRONHIDE 202
#define CONS_LOUDERRAWRS 203

#define CONS_UNUSEDPOTION1 204
#define CONS_UNUSEDPOTION2 205
#define CONS_UNUSEDPOTION3 206
#define CONS_UNUSEDPOTION4 207

#define CONS_UNUSEDWAND1 208
#define CONS_UNUSEDWAND2 209
#define CONS_UNUSEDWAND3 210
#define CONS_UNUSEDWAND4 211

#define CONS_UNUSEDWAND5 212
#define CONS_UNUSEDWAND6 213
#define CONS_UNUSEDWAND7 214
#define CONS_UNUSEDWAND8 215



typedef struct itemdef_st {
	char	*name;
	uint8_t	type;
	int 	enchantmult;	//fixed point 16.8 against modifier1
	int8_t	modifier1;
	uint8_t	offset1;
	int8_t	modifier2;
	uint8_t	offset2;
} itemdef_t;


extern itemdef_t equipdefs[];



gfx_sprite_t *items_GetItemSprite(uint8_t id);
char *items_GetItemName(uint8_t id);
char *items_GetItemDesc(uint8_t id);
void items_PrintItemname_Left(uint8_t id);
void items_PrintItemname_Bottom(uint8_t id);
uint8_t items_GetItemType(uint8_t id);
itemdef_t *items_GetItemDef(item_t *item);




#endif