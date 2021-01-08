#include "firmware.h"
#include <mem/minerva.h>

extern hekate_config h_cfg;
extern emummc_cfg_t emu_cfg;

u8 *LoadPackage1(launch_ctxt_t *ctxt)
{
    static const u32 BOOTLOADER_SIZE = 0x40000;
    static const u32 BOOTLOADER_MAIN_OFFSET = 0x100000;
    static const u32 BOOTLOADER_BACKUP_OFFSET = 0x140000;
    static const u32 HOS_KEYBLOBS_OFFSET = 0x180000;

    u32 pk1_offset = h_cfg.t210b01 ? sizeof(Package1_Mariko_header) : 0; // Skip T210B01 OEM header.
    u32 bootloader_offset = BOOTLOADER_MAIN_OFFSET;
    ctxt->pkg1 = (void *)malloc(BOOTLOADER_SIZE);

    // Read package1.
    emummc_storage_set_mmc_partition(&emmc_storage, EMMC_BOOT0);
    emummc_storage_read(&emmc_storage, bootloader_offset / NX_EMMC_BLOCKSIZE, BOOTLOADER_SIZE / NX_EMMC_BLOCKSIZE, ctxt->pkg1);

    ctxt->pkg1_id = pkg1_identify(ctxt->pkg1 + pk1_offset);
    if (!ctxt->pkg1_id)
    {
        print("%kUnknown pkg1 version.", RED);
        EPRINTFARGS("HOS version not supported!%s",
                    (emu_cfg.enabled && !h_cfg.emummc_force_disable) ? "\nOr emuMMC corrupt!" : "");
        return 0;
    }
    // Read the correct keyblob.
    ctxt->keyblob = (u8 *)calloc(NX_EMMC_BLOCKSIZE, 1);
    emummc_storage_read(&emmc_storage, HOS_KEYBLOBS_OFFSET / NX_EMMC_BLOCKSIZE + ctxt->pkg1_id->kb, 1, ctxt->keyblob);

    return ctxt->pkg1_id;
}

int loadFirm()
{
    u8 kb;
    u8 hos;
	u32 secmon_base;
	u32 warmboot_base;
	launch_ctxt_t ctxt;
	bool exo_new = false;
	/*tsec_ctxt_t tsec_ctxt;
	volatile secmon_mailbox_t *secmon_mailbox;*/

	minerva_change_freq(FREQ_1600);
	memset(&ctxt, 0, sizeof(launch_ctxt_t));
	//memset(&tsec_ctxt, 0, sizeof(tsec_ctxt_t));
	list_init(&ctxt.kip1_list);

    print("%k\nSetting up HOS:\n%k", WHITE, ORANGE);

    //Init nand/emummc
    int res = emummc_storage_init_mmc(&emmc_storage, &emmc_sdmmc);
    if (res)
    {
        if (res == 2){
            print("%k Failed init eMMC.\n", RED);
        } else {
            print("%k Failed init emuMMC.\n", RED);
        }

        return 1;
    }

    // Read boot0 and parse Package1
    LoadPackage1(&ctxt);
    print("Firmware kb: %d\n", ctxt.pkg1_id->kb);
    print("Firmware ver: %d\n", ctxt.pkg1_id->hos);

    // Generate 
    if(!keygen())
        print("Failed to keygen...\n");
    return 0;
}

void firmware()
{
    print("%kWelcome to bootloader %d.%d.%d%k\n", WHITE, LP_VER_MJ, LP_VER_MN, LP_VER_BF, ORANGE);
    if (loadFirm() == 0)
    {
        print("%kFirmware loaded.\n", GREEN);
    }
    btn_wait();
}