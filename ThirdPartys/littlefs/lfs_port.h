#ifndef	__LFS_PORT_H
#define __LFS_PORT_H

#include "main.h"
#include "lfs.h"

//#define LFS_FRAM_SIZE	32768u

extern lfs_file_t sys_config_file;
extern lfs_file_t sys_csv_file;
extern lfs_file_t sys_log_file;
extern lfs_file_t boot_info_file;

extern lfs_t lfs_norflash_wq128;

extern TX_SEMAPHORE lfs_file_open_sem;

void file_sys_init(void);
int file_read(lfs_file_t *file,char *path,uint8_t *data,uint32_t size);
int file_write(lfs_file_t *file,char *path,uint8_t *data,uint32_t size);
int file_delete(lfs_file_t *file,char *path);

extern const struct lfs_config cfg;

extern struct lfs_file_config sys_config_file_cfg;
extern struct lfs_file_config sys_csv_file_cfg;
extern struct lfs_file_config sys_log_file_cfg;
extern struct lfs_file_config boot_info_file_cfg;


#endif





