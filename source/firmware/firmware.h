#pragma once

#include "Package1.h"
#include "Package2.h"
#include "keygen.h"
#include "../gfx/gfx.h"
#include <string.h>
#include "../storage/nx_emmc.h"
#include "../storage/emummc.h"
#include <utils/btn.h>
#include <utils/types.h>
#include "../config.h"

typedef struct _launch_ctxt_t
{
	void *keyblob;

	void *pkg1;
	const pk11_offs *pkg1_id;
	const pkg2_kernel_id_t *pkg2_kernel_id;

	void *warmboot;
	u32   warmboot_size;
	void *secmon;
	u32   secmon_size;
	void *exofatal;
	u32   exofatal_size;

	void *pkg2;
	u32   pkg2_size;
	bool  new_pkg2;

	void  *kernel;
	u32    kernel_size;

	link_t kip1_list;
	char*  kip1_patches;

	u32  fss0_hosver;
	bool svcperm;
	bool debugmode;
	bool stock;
	bool atmosphere;
	bool fss0_experimental;
	bool emummc_forced;

	//exo_ctxt_t exo_ctx;

	//ini_sec_t *cfg;
} launch_ctxt_t;

void firmware();