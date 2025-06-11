#include "lite_file_sys.h"
#include "flash_nor.h"

int lite_file_init(struct lite_file_sys_t *f, char *file_name, uint32_t file_addr, uint32_t file_assigned_size)
{
    strcpy(f->file_name, file_name);
    f->file_addr = file_addr;
    f->file_size = 0;
    f->file_assigned_size = file_assigned_size;
    f->file_index = 0;

    // 擦除f->file_assigned_size空间,单次擦除4k一个扇区
    uint32_t erase_size = (f->file_assigned_size%4096==0)?(f->file_assigned_size/4096):(f->file_assigned_size/4096+1);
    for (uint32_t i = 0; i < erase_size; i++) {
        norflash_erase_sector(i);
    }

    return 0;
}
/*
    写入文件
    参数:
        f: 文件系统结构体
        data: 写入数据的缓冲区
        data_size: 写入的字节数
    返回值: 写入的字节数
    返回-1: 写入失败
*/
int lite_file_write(struct lite_file_sys_t *f, uint8_t *data, uint32_t data_size)
{
    if (f->file_index + data_size > f->file_assigned_size) {
        return -1;
    }
    norflash_write(data, f->file_addr + f->file_index, data_size);
    f->file_index += data_size;
    f->file_size += data_size;
    return data_size;
}

/*
    读取文件
    参数:
        f: 文件系统结构体
        data: 读取数据的缓冲区
        data_size: 读取的字节数
    返回值: 读取的字节数
    返回-1: 读取失败
*/
int lite_file_read(struct lite_file_sys_t *f, uint8_t *data, uint32_t data_size)
{
    int real_read_size = 0;
    if (f->file_index >= f->file_size) {
        return -1;
    }
    if (f->file_index + data_size > f->file_size && f->file_index < f->file_size) {
        real_read_size = f->file_size - f->file_index;
    } else {
        real_read_size = data_size;
    }
    norflash_read(data, f->file_addr + f->file_index, real_read_size);
    f->file_index += real_read_size;
    return real_read_size;
}

/*
    重置文件索引
    参数:
        f: 文件系统结构体
    返回值: 0
*/
int lite_file_rewind(struct lite_file_sys_t *f)
{
    f->file_index = 0;
    return 0;
}

int lite_file_size(struct lite_file_sys_t *f)
{
    return f->file_size;
}




