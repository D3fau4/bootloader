#include "fs.h"

FIL fp;

u32 fopen(const char *path, const char *mode) {
    u32 m = (mode[0] == 0x77 ? (FA_WRITE|FA_CREATE_NEW) : FA_READ);
    if (f_open(&fp, path, m) != FR_OK) 
        return 0;
    return 1;
}

u32 fread(void *buf, size_t size, size_t ntimes) {
    u8 *ptr = buf;
    while (size > 0) {
        u32 rsize = MIN(ntimes * size, size);
        if (f_read(&fp, ptr, rsize, NULL) != FR_OK) {
            error("Failed read!\n");
            return 0;
        }

        ptr += rsize;
        size -= rsize;
    }
    return 1;
}

u32 fseek(size_t off) {
    if(f_lseek(&fp, off) != FR_OK) {
        error("Failed read!\n");
        return 0;
    }
    return 1;
}

u32 fwrite(void *buf, size_t size, size_t ntimes) {
    u8 *ptr = buf;
    while (size > 0) {
        u32 rsize = MIN(ntimes * size, size);
        if (f_write(&fp, ptr, rsize, NULL) != FR_OK) {
            error("Failed write!\n");
            return 0;
        }

        ptr += rsize;
        size -= rsize;
    }
    return 1;
}

size_t fsize() {
    return f_size(&fp);
}

void fclose() {
    f_close(&fp);
}