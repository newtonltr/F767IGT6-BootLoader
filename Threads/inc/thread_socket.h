#ifndef THREAD_SOCKET_H
#define THREAD_SOCKET_H

#include "main.h"
#include "nx_api.h"
#include <stdint.h>

// 外部变量声明 - 这些变量在thread_init.c中定义
extern TX_THREAD thread_socket_block;
extern NX_IP ip_0;
extern ULONG ip0_address;
extern NX_PACKET_POOL pool_0;

__packed struct file_transfer_protocol_t
{
	uint32_t pack_index;	// 这一包数据的序号
	uint32_t total_bytes;	// 总数据大小
	uint32_t total_packs;	// 总包数
	uint32_t data_size; 	// 这一包数据的大小
	uint8_t data[512];	// 数据
	uint16_t crc16;	// 校验码
};

struct iap_process_t
{
	uint8_t iap_mode;
	uint32_t receive_bytes;
	uint32_t receive_packs;

};

void thread_socket_create(void);
void thread_iap_create(void);

#endif // THREAD_SOCKET_H