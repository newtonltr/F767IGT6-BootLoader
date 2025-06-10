#ifndef __LITE_FILE_SYS_H__
#define __LITE_FILE_SYS_H__

#include "main.h"
#include "stdint.h"

struct lite_file_sys_t
{
    char file_name[16]; // 文件名
    uint32_t file_addr; // 文件起始地址
    uint32_t file_size; // 文件当前大小
    uint32_t file_assigned_size; // 文件分配大小
    uint32_t file_index; // 文件当前指向地址
};

int lite_file_init(struct lite_file_sys_t *f, char *file_name, uint32_t file_addr, uint32_t file_assigned_size);
int lite_file_write(struct lite_file_sys_t *f, uint8_t *data, uint32_t data_size);
int lite_file_read(struct lite_file_sys_t *f, uint8_t *data, uint32_t data_size);
int lite_file_rewind(struct lite_file_sys_t *f);
int lite_file_size(struct lite_file_sys_t *f);

#endif
