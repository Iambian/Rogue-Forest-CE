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





gfx_sprite_t *items_GetItemSprite(uint8_t id) {
	uint8_t tmpid;
	void *ptr;
	
	if (!id) return equipicons_tile_0;
	tmpid = id - ITEM_EQUIPPABLE;
	if (tmpid < equips_tiles_num) return equips_tiles[tmpid];
	
	if (tmpid < ITEM_CONSUMABLE) return equipicons_tile_0;
	tmpid = id - ITEM_CONSUMABLE;
	if (tmpid < consumables_tiles_num) return consumables_tiles[tmpid];
	
	return equipicons_tile_0; 
}



