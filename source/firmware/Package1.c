#include "Package1.h"
#include "Package2.h"
#include "firmware.h"
#include "../config.h"

const pk11_offs *pkg1_identify(u8 *pkg1)
{
    char build_date[15];
    memcpy(build_date, (char *)(pkg1 + 0x10), 14);
    build_date[14] = 0;
    print("Package1 build date: %s\n", build_date);
    for (u32 i = 0; _pk11_offs[i].id; i++)
        if (!memcmp(pkg1 + 0x10, _pk11_offs[i].id, 8))
            return &_pk11_offs[i];
    return NULL;
}

const pk11_offs *pkg1_get_latest()
{
	return &_pk11_offs[ARRAY_SIZE(_pk11_offs) - 2];
}

void pkg1_unpack(){
    //return 0;
}