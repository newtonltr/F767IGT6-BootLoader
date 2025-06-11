#include "thread_socket.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "thread_init.h"
#include "tx_api.h"
#include "crc_tool.h"
#include "emb_flash.h"
#include "lite_file_sys.h"

// thread socket parameters
#define THREAD_SOCKET_STACK_SIZE    4096u
#define THREAD_SOCKET_PRIO          25u
TX_THREAD thread_socket_block;
uint64_t thread_socket_stack[THREAD_SOCKET_STACK_SIZE/8];
void thread_socket_entry(ULONG thread_input);


struct file_transfer_protocol_t file_transfer_protocol = {0};

void thread_socket_create(void)
{
	tx_thread_create(&thread_socket_block,
		"tx_socket",
		thread_socket_entry,
		0,
		&thread_socket_stack[0],
		THREAD_SOCKET_STACK_SIZE,
		THREAD_SOCKET_PRIO,
		THREAD_SOCKET_PRIO,
		TX_NO_TIME_SLICE,
		TX_AUTO_START);

}

// TCP socket相关参数定义在这个文件中
NX_TCP_SOCKET tcp_socket;
#define TCP_SERVER_PORT 7000  // 服务器监听端口

UINT nx_send(NX_TCP_SOCKET *socket, uint8_t *data, uint32_t len)
{
    NX_PACKET *packet_ptr;
    UINT status = 0;

    // 分配数据包
    status |= nx_packet_allocate(&pool_0, &packet_ptr, NX_TCP_PACKET, NX_WAIT_FOREVER);
    if (status != NX_SUCCESS)
    {
        return status;
    }

    // 将消息附加到数据包
    status = nx_packet_data_append(packet_ptr, 
                                  (VOID *)data, 
                                  len, 
                                  &pool_0, 
                                  NX_WAIT_FOREVER);
    if (status != NX_SUCCESS)
    {
        nx_packet_release(packet_ptr);
        return status;
    }
    
    // 发送数据包
    status = nx_tcp_socket_send(socket, packet_ptr, NX_WAIT_FOREVER);
    if (status != NX_SUCCESS)
    {
        nx_packet_release(packet_ptr);
    }
    
    return status;

}

UINT nx_receive(NX_TCP_SOCKET *socket, uint8_t *data, ULONG *len)
{
    NX_PACKET *packet_ptr;
    UINT status = 0;
    
    status = nx_tcp_socket_receive(socket, &packet_ptr, NX_WAIT_FOREVER);
    if (status == NX_SUCCESS)
    {
         // 读取数据包内容
        status = nx_packet_data_retrieve(packet_ptr, data, len);
        // 释放数据包
        nx_packet_release(packet_ptr);
    }
    return status;
}

struct lite_file_sys_t lite_sys_upgrade_file = {0};
int litefs_write_status = 0;

uint8_t iap_process_flag = 0;
uint8_t socket_recv_msg[2048] = {0};
ULONG socket_recv_len = 0;
const char connected[] = "client connected \r\n";
const char crc_error[] = "crc error \r\n";
const char recv_incomplete[] = "recv incomplete \r\n";
const char recv_success[] = "recv success \r\n";
const char start_program[] = "start program \r\n";
const char program_complete[] = "program complete \r\n";

// 线程入口函数
void thread_socket_entry(ULONG thread_input)
{
    UINT status;

    // 创建TCP服务器套接字
    status = nx_tcp_socket_create(&ip_0, &tcp_socket, "TCP Server Socket", 
                                 NX_IP_NORMAL, NX_FRAGMENT_OKAY, NX_IP_TIME_TO_LIVE, 
                                 1024, NX_NULL, NX_NULL);
    if (status != NX_SUCCESS)
    {
        return;
    }
    
    // 绑定TCP套接字到服务器端口
    status = nx_tcp_server_socket_listen(&ip_0, TCP_SERVER_PORT, &tcp_socket, 5, NX_NULL);
    if (status != NX_SUCCESS)
    {
        nx_tcp_socket_delete(&tcp_socket);
        return;
    }
        
    while (1) {
        // 等待客户端连接
        status = nx_tcp_server_socket_accept(&tcp_socket, NX_WAIT_FOREVER);
        if (status != NX_SUCCESS)
        {
            nx_tcp_server_socket_unaccept(&tcp_socket);
            nx_tcp_server_socket_unlisten(&ip_0, TCP_SERVER_PORT);
            nx_tcp_socket_delete(&tcp_socket);
            return;
        }

        // 发送连接成功消息
        lite_file_init(&lite_sys_upgrade_file, "sys_upgrade.bin", 0, 400*1024);
        nx_send(&tcp_socket, (uint8_t *)connected, strlen(connected));

        while (1)
        {
            memset(socket_recv_msg, 0, sizeof(socket_recv_msg));
            status = nx_receive(&tcp_socket, (uint8_t *)socket_recv_msg, &socket_recv_len);
            if (status == NX_NOT_CONNECTED)
            {
                nx_tcp_server_socket_unaccept(&tcp_socket);
                nx_tcp_server_socket_relisten(&ip_0, TCP_SERVER_PORT, &tcp_socket);
                break;
            }
            if (socket_recv_len != sizeof(struct file_transfer_protocol_t))
            {
                nx_send(&tcp_socket, (uint8_t *)recv_incomplete, strlen(recv_incomplete));
                continue;
            }
            struct file_transfer_protocol_t *ftp = (struct file_transfer_protocol_t *)socket_recv_msg;
            if (ftp->crc16 != crc_compute_lsb((uint8_t *)ftp, sizeof(struct file_transfer_protocol_t)-2))
            {
                nx_send(&tcp_socket, (uint8_t *)crc_error, strlen(crc_error));
                continue;
            }
            litefs_write_status = lite_file_write(&lite_sys_upgrade_file, ftp->data, ftp->data_size);
            
            if (ftp->pack_index == ftp->total_packs) {
                nx_send(&tcp_socket, (uint8_t *)start_program, strlen(start_program));
                // 循环读取文件数据并写入到内部Flash
                // 总大小
                lite_file_rewind(&lite_sys_upgrade_file);
                int file_size = lite_file_size(&lite_sys_upgrade_file); 
                //单次读取大小, 一次读取1k
                uint8_t read_buf[1024] = {0};
                int read_total_size = 0;
                uint32_t target_addr = AppAddr;
                // 总读取大小=文件大小时跳出
                while (read_total_size < file_size) {
                    int once_read_size = lite_file_read(&lite_sys_upgrade_file, read_buf, sizeof(read_buf));
                    if (once_read_size > 0) {
                        // 计算需要写入的字数量（Flash按32位字写入，不足补齐）
                        uint32_t write_size = (once_read_size%4==0)?(once_read_size/4):(once_read_size/4+1);
                        FLASH_Write(target_addr, (uint32_t *)read_buf, write_size);
                        target_addr += once_read_size;
                        read_total_size += once_read_size;
                        // nx_send(&tcp_socket, (uint8_t *)&read_total_size, sizeof(read_total_size));
                    }
                }
                sleep_ms(500);
                lite_file_rewind(&lite_sys_upgrade_file);
                nx_send(&tcp_socket, (uint8_t *)program_complete, strlen(program_complete));
                iap_process_flag = 1;
                // JumpToApp();
            } else {
                nx_send(&tcp_socket, (uint8_t *)recv_success, strlen(recv_success));
            }
        }  
    }
}


