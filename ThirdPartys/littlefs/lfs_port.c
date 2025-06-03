#include "lfs_port.h"
#include "norflash.h"


/*static uint8_t lfs_read_buffer[LFS_STATIC_READ_BUFF_SIZE];
static uint8_t lfs_prog_buffer[LFS_STATIC_PROG_BUFF_SIZE];
static uint8_t lfs_lookahead_buffer[LFS_STATIC_LOOKAHEAD_BUFF_SIZE];
*/

/**
 * lfs与底层FRAM读
 * @param  c			参数
 * @param  block  块
 * @param  off    块内偏移地址
 * @param  buffer 用于存储读取到的数据
 * @param  size   字节数
 * @return
 */
static int lfs_port_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
	norflash_read((uint8_t *)buffer, c->block_size * block + off, size);
	return LFS_ERR_OK;
}

/**
 * lfs与底层flash写
 * @param  c
 * @param  block  块编号
 * @param  off    块内偏移地址
 * @param  buffer 待写入的数据
 * @param  size   数据大小
 * @return
 */
static int lfs_port_write(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
	norflash_write_nocheck((uint8_t *)buffer, c->block_size * block + off, size);
	return LFS_ERR_OK;
}

/**
 * lfs与底层flash擦除接口
 * @param  c
 * @param  block 块编号
 * @return
 */
static int lfs_port_erase(const struct lfs_config *c, lfs_block_t block)
{
	norflash_erase_sector(block);
	return LFS_ERR_OK;
}

static int lfs_port_sync(const struct lfs_config *c)
{
	return LFS_ERR_OK;
}

const struct lfs_config cfg = 
{
	// block device operations
	.read  = lfs_port_read,
	.prog  = lfs_port_write,
	.erase = lfs_port_erase,
	.sync  = lfs_port_sync,

	// block device configuration
	.read_size = 128,
	.prog_size = 128,
	.block_size = 4096,		//4K
	.block_count = 4096,
	.cache_size = 4096,
	.lookahead_size = 128,
	.block_cycles = 500,		//负载均衡触发条件

	//静态内存分配
	//.read_buffer = lfs_read_buffer,
	//.prog_buffer = lfs_prog_buffer,
	//.lookahead_buffer = lfs_lookahead_buffer,
};


lfs_t lfs_norflash_wq128;
lfs_file_t sys_config_file;
lfs_file_t sys_csv_file;
lfs_file_t sys_log_file;
lfs_file_t boot_info_file;

/*static uint32_t sys_config_file_buf[4096/4];
static uint32_t sys_csv_file_buf[4096/4];
static uint32_t sys_log_file_buf[4096/4];
static uint32_t boot_info_file_buf[4096/4];

struct lfs_file_config sys_config_file_cfg = {
	.buffer = sys_config_file_buf
};

struct lfs_file_config sys_csv_file_cfg = {
	.buffer = sys_csv_file_buf
};

struct lfs_file_config sys_log_file_cfg = {
	.buffer = sys_log_file_buf
};

struct lfs_file_config boot_info_file_cfg = {
	.buffer = boot_info_file_buf
};
*/

TX_SEMAPHORE lfs_file_open_sem;

#if 1
//文件初???化
void file_sys_init()
{
	int err = lfs_mount(&lfs_norflash_wq128, &cfg);

	if(err){
		printf("disk mount failed: %d\n",err);
		lfs_format(&lfs_norflash_wq128, &cfg);
		lfs_mount(&lfs_norflash_wq128, &cfg);	
	}
	else{
		printf("disk mount success\n");
	}
}

//读取VCU配置文件
int file_read(lfs_file_t *file,char *path,uint8_t *data,uint32_t size)
{
	int status;

	//加锁
	tx_semaphore_get(&lfs_file_open_sem,TX_WAIT_FOREVER);

	status = lfs_file_open(&lfs_norflash_wq128, file, path, LFS_O_RDWR | LFS_O_CREAT);
	if(status != LFS_ERR_OK){
		printf("file %s open failed: %d\n",path,status);
		goto FILE_R_ERR;
	}

	status = lfs_file_read(&lfs_norflash_wq128,file,data,size);
	if(status <= 0){
		printf("file %s read failed: %d\n",path,status);
	}

	status = lfs_file_close(&lfs_norflash_wq128,file);
	if(status != LFS_ERR_OK){
		printf("file %s close failed: %d\n",path,status);
		goto FILE_R_ERR;		
	}

FILE_R_ERR:
	//解锁
	tx_semaphore_put(&lfs_file_open_sem);
	return status;
}

//封装-写入文件
int file_write(lfs_file_t *file,char *path,uint8_t *data,uint32_t size)
{	
	int status;

	//加锁
	tx_semaphore_get(&lfs_file_open_sem,TX_WAIT_FOREVER);

	//打开文件
	status = lfs_file_open(&lfs_norflash_wq128, file, path, LFS_O_RDWR | LFS_O_CREAT | LFS_O_TRUNC);
	if(status != LFS_ERR_OK){
		printf("file %s open failed: %d\n",path,status);
		goto FILE_W_ERR;
	}
	//指针移到开头
	status = lfs_file_rewind(&lfs_norflash_wq128, file);
	if(status != LFS_ERR_OK){
		printf("file %s rewind failed: %d\n",path,status);
	}
	//写入
	status = lfs_file_write(&lfs_norflash_wq128,file,data,size);
	if(status <= 0){
		printf("file %s write failed: %d\n",path,status);
	}
	//关闭
	status = lfs_file_close(&lfs_norflash_wq128,file);
	if(status != LFS_ERR_OK){
		printf("file %s close failed: %d\n",path,status);
		goto FILE_W_ERR;	
	}

FILE_W_ERR:
	//解锁
	tx_semaphore_put(&lfs_file_open_sem);
	return status;
}

//删除文件
int file_delete(lfs_file_t *file,char *path)
{
	int status;
	//加锁
	tx_semaphore_get(&lfs_file_open_sem,TX_WAIT_FOREVER);
	status = lfs_remove(&lfs_norflash_wq128,path);
	if(status != LFS_ERR_OK){
		printf("file %s delete failed: %d\n",path,status);
	}
	//解锁
	tx_semaphore_put(&lfs_file_open_sem);
	
	return status;
}

#endif


int file_write_cat(lfs_file_t *file,char *path,uint8_t *data,uint32_t size, struct lfs_file_config *cfg)
{
	int status;
	//加锁
	tx_semaphore_get(&lfs_file_open_sem,TX_WAIT_FOREVER);
	//打开文件
	status = lfs_file_opencfg(&lfs_norflash_wq128, file, path, LFS_O_RDWR | LFS_O_CREAT | LFS_O_APPEND, cfg);
	//写入
	status = lfs_file_write(&lfs_norflash_wq128,file,data,size);
	//关闭
	status = lfs_file_close(&lfs_norflash_wq128,file);
	//解锁
	tx_semaphore_put(&lfs_file_open_sem);

	return status;
}












