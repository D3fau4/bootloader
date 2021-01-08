#pragma once

#include <stdlib.h>
#include "../storage/nx_emmc.h"
#include "../types.h"

typedef struct _patch_t
{
    u32 off;
    u32 val;
} patch_t;

// Package1 Mariko Header
typedef struct _Package1_Mariko_header
{
    u8 aes_mac[0x10];
    u8 rsa_sig[0x100];
    u8 salt[0x20];
    u8 sha256[0x20];
    u32 version;
    u32 size;
    u32 load_addr;
    u32 entrypoint;
    u8 rsvd[0x10];
} Package1_Mariko_header;

typedef struct
{
    char id[15];
    u32 kb;
    u32 hos;
    u32 tsec_off;
    u32 pkg11_off;
    u32 sec_map[3];
    u32 secmon_base;
    u32 warmboot_base;
} pk11_offs;

typedef struct _pk11_hdr_t
{
    u32 magic;
    u32 wb_size;
    u32 wb_off;
    u32 pad;
    u32 ldr_size;
    u32 ldr_off;
    u32 sm_size;
    u32 sm_off;
} pk11_hdr_t;

// ID (Timestamp),                      KB,                    Fuses,  TSEC,    PK11,   Sec_map,     SECMON,   Warmboot.
static const pk11_offs _pk11_offs[] = {
    {"20161121183008", KB_FIRMWARE_VERSION_100, HOS_FIRMWARE_VERSION_100, 0x1900, 0x3FE0, {2, 1, 0}, 0x40014020, 0x8000D000},   //1.0.0
    {"20170210155124", KB_FIRMWARE_VERSION_200, HOS_FIRMWARE_VERSION_200, 0x1900, 0x3FE0, {0, 1, 2}, 0x4002D000, 0x8000D000},   //2.0.0 - 2.3.0
    {"20170519101410", KB_FIRMWARE_VERSION_300, HOS_FIRMWARE_VERSION_300, 0x1A00, 0x3FE0, {0, 1, 2}, 0x4002D000, 0x8000D000},   //3.0.0
    {"20170710161758", KB_FIRMWARE_VERSION_301, HOS_FIRMWARE_VERSION_300, 0x1A00, 0x3FE0, {0, 1, 2}, 0x4002D000, 0x8000D000},   //3.0.1 - 3.0.2
    {"20170921172629", KB_FIRMWARE_VERSION_400, HOS_FIRMWARE_VERSION_400, 0x1800, 0x3FE0, {1, 2, 0}, 0x4002B000, 0x4003B000},   //4.0.0 - 4.1.0
    {"20180220163747", KB_FIRMWARE_VERSION_500, HOS_FIRMWARE_VERSION_500, 0x1900, 0x3FE0, {1, 2, 0}, 0x4002B000, 0x4003B000},   //5.0.0 - 5.0.2
    {"20180802162753", KB_FIRMWARE_VERSION_600, HOS_FIRMWARE_VERSION_600, 0x1900, 0x3FE0, {1, 2, 0}, 0x4002B000, 0x4003D800},   //6.0.0
    {"20181107105733", KB_FIRMWARE_VERSION_620, HOS_FIRMWARE_VERSION_620, 0x0E00, 0x6FE0, {1, 2, 0}, 0x4002B000, 0x4003D800},   //6.2.0
    {"20181218175730", KB_FIRMWARE_VERSION_700, HOS_FIRMWARE_VERSION_700, 0x0F00, 0x6FE0, {1, 2, 0}, 0x40030000, 0x4003E000},   //7.0.0
    {"20190208150037", KB_FIRMWARE_VERSION_701, HOS_FIRMWARE_VERSION_700, 0x0F00, 0x6FE0, {1, 2, 0}, 0x40030000, 0x4003E000},   //7.0.1
    {"20190314172056", KB_FIRMWARE_VERSION_800, HOS_FIRMWARE_VERSION_800, 0x0E00, 0x6FE0, {1, 2, 0}, 0x40030000, 0x4003E000},   //8.0.0
    {"20190531152432", KB_FIRMWARE_VERSION_810, HOS_FIRMWARE_VERSION_810, 0x0E00, 0x6FE0, {1, 2, 0}, 0x4002B000, 0x4003E000},   //8.1.0
    {"20190809135709", KB_FIRMWARE_VERSION_900, HOS_FIRMWARE_VERSION_900, 0x0E00, 0x6FE0, {1, 2, 0}, 0x40030000, 0x4003E000},   //9.0.0 - 9.0.1
    {"20191021113848", KB_FIRMWARE_VERSION_910, HOS_FIRMWARE_VERSION_910, 0x0E00, 0x6FE0, {1, 2, 0}, 0x40030000, 0x4003E000},   //9.1.0 - 9.2.0
    {"20200303104606", KB_FIRMWARE_VERSION_1000, HOS_FIRMWARE_VERSION_1000, 0x0E00, 0x6FE0, {1, 2, 0}, 0x40030000, 0x4003E000}, //10.0.0 - 10.2.0
    {"20201030110855", KB_FIRMWARE_VERSION_1100, HOS_FIRMWARE_VERSION_1100, 0X0E00, 0X6FE0, {1, 2, 0}, 0x40030000, 0x4003E000}, //11.0.0+
    {NULL}                                                                                                                      // End.
};

const pk11_offs *pkg1_identify(u8 *pkg1);