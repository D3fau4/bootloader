#pragma once

#include <stddef.h>
#include <string.h>
#include <utils/types.h>
#include "libs/fatfs/ff.h"

u32 fopen(const char *path, const char *mode);
u32 fread(void *buf, size_t size, size_t ntimes);
u32 fwrite(void *buf, size_t size, size_t ntimes);
u32 fseek(size_t off);
size_t fsize();
void fclose();