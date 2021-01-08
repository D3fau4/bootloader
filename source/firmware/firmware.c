#include "firmware.h"

void firmware()
{
    print("%kWelcome to bootloader %d.%d.%d%k", WHITE, LP_VER_MJ, LP_VER_MN, LP_VER_BF, ORANGE);
    btn_wait();
}