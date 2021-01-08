#include "sept.h"
#include "../types.h"
#include "../config.h"
#include "../gfx/gfx.h"
#include <libs/fatfs/ff.h>
#include <mem/heap.h>
#include <soc/hw_init.h>
#include <soc/pmc.h>
#include <soc/t210.h>
#include "../storage/nx_emmc.h"
#include <storage/nx_sd.h>
#include <storage/sdmmc.h>
#include <utils/btn.h>
#include <utils/types.h>
#include <utils/util.h>
#include <string.h>

extern hekate_config h_cfg;
extern void reloc_patcher(u32 payload_dst, u32 payload_src, u32 payload_size);

#define RELOC_META_OFF   0x7C
#define PATCHED_RELOC_SZ 0x94

#define WB_RST_ADDR 0x40010ED0
#define WB_RST_SIZE 0x30

u8 warmboot_reboot[] = {
	0x14, 0x00, 0x9F, 0xE5, // LDR R0, =0x7000E450
	0x01, 0x10, 0xB0, 0xE3, // MOVS R1, #1
	0x00, 0x10, 0x80, 0xE5, // STR R1, [R0]
	0x0C, 0x00, 0x9F, 0xE5, // LDR R0, =0x7000E400
	0x10, 0x10, 0xB0, 0xE3, // MOVS R1, #0x10
	0x00, 0x10, 0x80, 0xE5, // STR R1, [R0]
	0xFE, 0xFF, 0xFF, 0xEA, // LOOP
	0x50, 0xE4, 0x00, 0x70, // #0x7000E450
	0x00, 0xE4, 0x00, 0x70  // #0x7000E400
};

#define SEPT_PRI_ADDR   0x4003F000

#define SEPT_PK1T_ADDR  0xC0400000
#define SEPT_TCSZ_ADDR  (SEPT_PK1T_ADDR - 0x4)
#define SEPT_STG1_ADDR  (SEPT_PK1T_ADDR + 0x2E100)
#define SEPT_STG2_ADDR  (SEPT_PK1T_ADDR + 0x60E0)
#define SEPT_PKG_SZ     (0x2F100 + WB_RST_SIZE)

int keys_generated = 0;

int has_keygen_ran() {
    if(keys_generated == 1 || h_cfg.t210b01)
        return keys_generated;
    int has_ran = EMC(EMC_SCRATCH0) == 0x80000000;
    EMC(EMC_SCRATCH0) = 0;
    keys_generated = has_ran;
    return has_ran;
}

int reboot_to_sept(const u8 *tsec_fw, u32 hosver)
{
    FIL fp;
    // Copy warmboot reboot code and TSEC fw.
    memcpy((u8 *)(SEPT_PK1T_ADDR - WB_RST_SIZE), (u8 *)warmboot_reboot, sizeof(warmboot_reboot));
    memcpy((void *)SEPT_PK1T_ADDR, tsec_fw, hosver == HOS_FIRMWARE_VERSION_800 ? 0x3000 : 0x3300);
    *(vu32 *)SEPT_TCSZ_ADDR = hosver == HOS_FIRMWARE_VERSION_800 ? 0x3000 : 0x3300;

    // Copy sept-primary.
	if (f_open(&fp, "sept/sept-primary.bin", FA_READ))
		goto error;

	if (f_read(&fp, (u8 *)SEPT_STG1_ADDR, f_size(&fp), NULL))
	{
		f_close(&fp);
		goto error;
	}
	f_close(&fp);


    // Copy sept-secondary.
	if (hosver < KB_FIRMWARE_VERSION_810)
	{
		if (f_open(&fp, "sept/sept-secondary_00.enc", FA_READ))
			goto error;
	}
	else
	{
		if (f_open(&fp, "sept/sept-secondary_01.enc", FA_READ))
			goto error;
	}

	if (f_read(&fp, (u8 *)SEPT_STG2_ADDR, f_size(&fp), NULL))
	{
		f_close(&fp);
		goto error;
	}
	f_close(&fp);

    sd_end();

    u32 pk1t_sept = SEPT_PK1T_ADDR - (ALIGN(PATCHED_RELOC_SZ, 0x10) + WB_RST_SIZE);

    void (*sept)() = (void *)pk1t_sept;

    reloc_patcher(WB_RST_ADDR, pk1t_sept, SEPT_PKG_SZ);

    // Patch SDRAM init to perform an SVC immediately after second write.
    PMC(APBDEV_PMC_SCRATCH45) = 0x2E38DFFF;
    PMC(APBDEV_PMC_SCRATCH46) = 0x6001DC28;
    // Set SVC handler to jump to sept-primary in IRAM.
    PMC(APBDEV_PMC_SCRATCH33) = SEPT_PRI_ADDR;
    PMC(APBDEV_PMC_SCRATCH40) = 0x6000F208;

    display_end();
    
    (*sept)();

    return 1;

error:
    print("oops failed to gen keys\n");

    btn_wait();

    return 0;
}