#ifndef __BOARD_BSP_INFO_H__
#define __BOARD_BSP_INFO_H__

#include "board_definition.h"


#if   BARED_TYPE == BOARD_R7OUT_D9IN_1TMP
#define PHY_RELAY_OUT_NUM     7
#define PHY_DIG_IN_NUM        9
#define PHY_TMP_IN_NUM        1
#endif



#if   BARED_TYPE == BOARD_R8OUT_D8IN_8TMP
#define PHY_RELAY_OUT_NUM     8
#define PHY_DIG_IN_NUM        8
#define PHY_TMP_IN_NUM        8
#endif



#endif
