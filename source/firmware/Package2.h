#pragma once

#include "../storage/nx_emmc.h"

typedef struct _pkg2_kernel_id_t
{
	u8 hash[8];
	//kernel_patch_t *kernel_patchset;
} pkg2_kernel_id_t;