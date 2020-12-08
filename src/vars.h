#ifndef __INCGUARD_VARS__
#define __INCGUARD_VARS__

#include <graphx.h>


/* Shared functions and variables in disp.c */
uint8_t disp_Title(uint8_t key);	//returns new state of game
uint8_t disp_Credits(uint8_t key);




/* Shared functions and variables in main.c */
extern uint8_t rawkey;
extern gfx_sprite_t *curmap;		//16386
extern gfx_sprite_t *emptysprite;	//256
extern gfx_sprite_t *tiles[256];	//768
extern gfx_tilemap_t tilemap;



uint8_t util_GetSK(void);

#endif