#ifndef __SOBJS__INCGUARD__
#define __SOBJS__INCGUARD__
//Defines for static object behaviors that should be externally accessible
//Up to eight types per classification. Up to 16 (0-15) classifications.
//Uppermost bit is reserved for activation status (isactive/isopened/etc)
#define SOBJTMSKLO 0x07
#define SOBJTMSKHI 0x78
#define SOBJTYPECAT(x) (x<<3)

#define SOBJ_DOORBASE SOBJTYPECAT(0)
#define SOBJ_DOOR		SOBJ_DOORBASE+0
#define SOBJ_LOCKDOOR	SOBJ_DOORBASE+1
#define SOBJ_MLOKDOOR	SOBJ_DOORBASE+2
#define SOBJ_TIMEDOOR	SOBJ_DOORBASE+3
#define SOBJ_BOSSDOOR	SOBJ_DOORBASE+4
#define SOBJ_SW1DOOR	SOBJ_DOORBASE+5
#define SOBJ_SW2DOOR	SOBJ_DOORBASE+6
#define SOBJ_SW3DOOR	SOBJ_DOORBASE+7

#define SOBJ_WARPBASE SOBJTYPECAT(1)
#define SOBJ_WARPHIDDEN	SOBJ_WARPBASE+0
#define SOBJ_WARPGATE	SOBJ_WARPBASE+1
#define SOBJ_WARPPAD	SOBJ_WARPBASE+2
#define SOBJ_WARPGATE2	SOBJ_WARPBASE+3
#define SOBJ_STAIRSUP	SOBJ_WARPBASE+4
#define SOBJ_STAIRSDOWN	SOBJ_WARPBASE+4

#define SOBJ_TRAPBASE SOBJTYPECAT(2)
#define SOBJ_TRAPSPIKES	SOBJ_TRAPBASE+0
#define SOBJ_TRAPSTICKS	SOBJ_TRAPBASE+1
#define SOBJ_TELEPORTER	SOBJ_TRAPBASE+2
#define SOBJ_TRAPPIT	SOBJ_TRAPBASE+3

#define SOBJ_CHESTBASE SOBJTYPECAT(3)
#define SOBJ_NORMCHEST	SOBJ_CHESTBASE+0
#define SOBJ_LOCKCHEST	SOBJ_CHESTBASE+1
#define SOBJ_MLOKCHEST	SOBJ_CHESTBASE+2
#define SOBJ_CRYSCHEST	SOBJ_CHESTBASE+3
#define SOBJ_FAKECHEST	SOBJ_CHESTBASE+4

#define SOBJ_ITEMBASE SOBJTYPECAT(4)
#define SOBJ_MONEY		SOBJ_ITEMBASE+0
#define SOBJ_OPTIONALS	SOBJ_ITEMBASE+1
#define SOBJ_MCGUFFIN	SOBJ_ITEMBASE+2
#define SOBJ_HIDDENKITM	SOBJ_ITEMBASE+3
#define SOBJ_FOOD		SOBJ_ITEMBASE+4
#define SOBJ_MOARFOOD	SOBJ_ITEMBASE+5

extern sobj_t scratchsobj;


void sobj_clear(void);
void sobj_addentry(sobj_t* sobj);
void sobj_rementry(uint8_t index);
sobj_t *sobj_getentrybypos(uint8_t x, uint8_t y);
sobj_t *sobj_getwarpbydest(uint8_t dest);
void sobj_WriteToMap(void);
uint8_t sobj_IsDoorable(uint8_t x, uint8_t y);





#endif