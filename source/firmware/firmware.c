#include "firmware.h"
#include "keygen.h"
#include <mem/minerva.h>

extern hekate_config h_cfg;
extern emummc_cfg_t emu_cfg;

bool customSecmon = false;
bool customWb = false;
bool customKernel = false;

bool hasCustomWb(launch_ctxt_t *ctxt)
{
    if (customWb)
        return customWb;
    if (fopen("/ReiNX/warmboot.bin", "rb") != 0)
    {
        customWb = true;
        ctxt->warmboot = sd_file_read("/ReiNX/warmboot.bin", &ctxt->warmboot_size);
        fclose();
    }
    if (fopen("/ReiNX/lp0fw.bin", "rb") != 0)
    {
        customWb = true;
        ctxt->warmboot = sd_file_read("/ReiNX/lp0fw.bin", &ctxt->warmboot_size);
        fclose();
    }
    return customWb;
}

bool hasCustomSecmon(launch_ctxt_t *ctxt)
{
    if (customSecmon)
        return customSecmon;
    if (fopen("/ReiNX/secmon.bin", "rb") != 0)
    {
        customSecmon = true;
        ctxt->secmon = sd_file_read("/ReiNX/secmon.bin", &ctxt->secmon_size);
        fclose();
    }
    if (fopen("/ReiNX/exosphere.bin", "rb") != 0)
    {
        customSecmon = true;
        ctxt->secmon = sd_file_read("/ReiNX/exosphere.bin", &ctxt->secmon_size);
        fclose();
    }
    return customSecmon;
}

bool hasCustomKern(launch_ctxt_t *ctxt)
{
    if (customKernel)
        return customKernel;
    if (fopen("/ReiNX/kernel.bin", "rb") != 0)
    {
        customKernel = true;
        ctxt->kernel = sd_file_read("/ReiNX/kernel.bin", &ctxt->kernel_size);
        fclose();
    }
    return customKernel;
}

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
    u8 hos;
    u32 secmon_base;
    u32 warmboot_base;
    launch_ctxt_t ctxt;
    bool exo_new = false;
    tsec_ctxt_t tsec_ctxt;
    /*volatile secmon_mailbox_t *secmon_mailbox;*/

    minerva_change_freq(FREQ_1600);
    memset(&ctxt, 0, sizeof(launch_ctxt_t));
    memset(&tsec_ctxt, 0, sizeof(tsec_ctxt_t));
    list_init(&ctxt.kip1_list);

    print("%k\nSetting up HOS:\n%k", WHITE, ORANGE);

    //Init nand/emummc
    int res = emummc_storage_init_mmc(&emmc_storage, &emmc_sdmmc);
    if (res)
    {
        if (res == 2)
        {
            print("%k Failed init eMMC.\n", RED);
        }
        else
        {
            print("%k Failed init emuMMC.\n", RED);
        }

        return 1;
    }

    // Read boot0 and parse Package1
    LoadPackage1(&ctxt);
    if (!hasCustomSecmon(&ctxt) || !hasCustomWb(&ctxt)){
        print("%kMissing warmboot.bin or secmon.bin. These are required!\n", RED);
        return 1;
    }
    print("Firmware kb: %d\n", ctxt.pkg1_id->kb);
    print("Firmware ver: %d\n", ctxt.pkg1_id->hos);
    print("Decrypting Package1...\n");
    bool emummc_enabled = emu_cfg.enabled;

    // Enable emummc patching.
    if (emummc_enabled)
    {
        print("Enable emummc patching.");
        //config_kip1patch(&ctxt, "emummc");
    }

    // Check if secmon is new exosphere.
    if (ctxt.secmon)
        exo_new = !memcmp((void *)((u8 *)ctxt.secmon + ctxt.secmon_size - 4), "LENY", 4);
    const pk11_offs *pk1_latest = pkg1_get_latest();
    secmon_base = exo_new ? pk1_latest->secmon_base : ctxt.pkg1_id->secmon_base;
    warmboot_base = exo_new ? pk1_latest->warmboot_base : ctxt.pkg1_id->warmboot_base;
    h_cfg.aes_slots_new = exo_new;

    // Generate keys
    if (ctxt.pkg1_id->kb < KB_FIRMWARE_VERSION_700)
    {
        if (!keygen(ctxt.keyblob, ctxt.pkg1_id->kb, &tsec_ctxt, &ctxt))
            print("%kFailed to keygen...\n", RED);
    }
    else
    {
        if (!has_keygen_ran())
        {
            reboot_to_sept(ctxt.pkg1 + ctxt.pkg1_id->tsec_off, ctxt.pkg1_id->hos);
        }
        else
        {
            // Generate keys.
            if (!h_cfg.se_keygen_done)
            {
                tsec_ctxt.fw = (u8 *)ctxt.pkg1 + ctxt.pkg1_id->tsec_off;
                tsec_ctxt.pkg1 = ctxt.pkg1;
                tsec_ctxt.pkg11_off = ctxt.pkg1_id->pkg11_off;
                tsec_ctxt.secmon_base = secmon_base;

                if (ctxt.pkg1_id->kb >= KB_FIRMWARE_VERSION_700 && !h_cfg.sept_run)
                {
                    print("%kFailed to run sept", RED);
                    return 1;
                }

                if (!keygen(ctxt.keyblob, ctxt.pkg1_id->kb, &tsec_ctxt, &ctxt))
                {
                    print("%kFailed to keygen...\n", RED);
                }
                if (ctxt.pkg1_id->kb <= KB_FIRMWARE_VERSION_600)
                    h_cfg.se_keygen_done = 1;
            }
        }
    }

    print("Unpacking pkg1\n");

    return 0;
}

void firmware()
{
    print("%kWelcome to bootloader %d.%d.%d%k\n", WHITE, LP_VER_MJ, LP_VER_MN, LP_VER_BF, ORANGE);
    if (loadFirm() == 0)
    {
        print("%kFirmware loaded.\n", GREEN);
    }
    else
    {
        print("%kFailed to load the firmware...\n", RED);
    }
    btn_wait();
}