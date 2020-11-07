#ifndef __DEFS__INCGUARD__
#define __DEFS__INCGUARD__


#define SBAR_LEFT	(4+16*14+4)
#define SBAR_TOP	2
#define XPFOOD_X	(SBAR_LEFT+2)
#define XPFOOD_Y	(SBAR_TOP+2)
//--
#define MINIMAP_X	(SBAR_LEFT+1)
#define MINIMAP_Y	(XPFOOD_Y+32+2)
#define MAPAREA_X	(MINIMAP_X+67)
#define MAPAREA_Y	(MINIMAP_Y+1)
#define MAPFLOOR_X	(MAPAREA_X)
#define MAPFLOOR_Y	(MAPAREA_Y+28)
//--
#define HPMP_X		(SBAR_LEFT+2)
#define HPMP_Y		(MINIMAP_Y+66+2)
#define CURGEAR_X	(SBAR_LEFT+2)
#define CURGEAR_Y	(HPMP_Y+32+3)
#define QUICKSET_X	(SBAR_LEFT+0)
#define QUICKSET_Y	(CURGEAR_Y+34+3)

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#define UPD_XP			(1<<0)
#define UPD_FOOD		(1<<1)
#define UPD_MINI		(1<<2)
#define UPD_HP			(1<<3)
#define UPD_MP			(1<<4)
#define UPD_CURGEAR		(1<<5)
#define UPD_QUICKSET	(1<<6)
#define UPD_BACKERS		(1<<7)
#define UPD_SIDEBAR		0xFF








/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#define COLOR_BLACK				0
#define COLOR_WHITE				1
#define COLOR_RED				2
#define COLOR_GREEN				3
#define COLOR_BLUE				4
#define COLOR_TRANS				5
#define COLOR_GRAY				6
#define COLOR_LIGHTGRAY			7
#define COLOR_GUNMETALGRAY		8
#define COLOR_GOLD				9
#define COLOR_CYAN				10
#define COLOR_LIME				11
#define COLOR_ORANGE			12
#define COLOR_FORESTGREEN		13
#define COLOR_DARKGRAY			14
#define COLOR_PURPLE			15

#define DCOLOR_BLANK	16
#define DCOLOR_FLOOR	17
	

#define NUMROOMS_MAX 20












#endif