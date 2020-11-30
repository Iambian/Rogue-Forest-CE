#ifndef __DGEN__INCGUARD__
#define __DGEN__INCGUARD__

#define AREAHIMASK 0xE0
#define AREALOMASK 0x1F
#define AREAHICONV(x) ((x&0x07)<<5)
#define AREAHIUNCONV(x) ((x>>5)&0x07)

#define AREA_FOREST AREAHICONV(0)
#define AREA_DUNGEON1 AREAHICONV(1)
#define AREA_DUNGEON2 AREAHICONV(2)
#define AREA_DUNGEON3 AREAHICONV(3)
#define AREA_DUNGEON4 AREAHICONV(4)
#define AREA_DUNGEON5 AREAHICONV(5)




void gen_TestDungeon(uint8_t roomdensity, uint8_t floorid);
void gen_WarpTo(uint8_t warpdest);



#endif