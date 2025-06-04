#include "thread_socket.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "thread_init.h"


// thread socket parameters
#define THREAD_SOCKET_STACK_SIZE    4096u
#define THREAD_SOCKET_PRIO          25u
TX_THREAD thread_socket_block;
uint64_t thread_socket_stack[THREAD_SOCKET_STACK_SIZE/8];
void thread_socket_entry(ULONG thread_input);


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

char recv_msg_debug[128] = {0};
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
        char connected[] = "client connected \r\n";
        nx_send(&tcp_socket, (uint8_t *)connected, strlen(connected));

        while (1)
        {
            char recv_msg[128] = {0};
            ULONG len = 0;
            status = nx_receive(&tcp_socket, (uint8_t *)recv_msg, &len);
            if (status == NX_NOT_CONNECTED) {
                nx_tcp_server_socket_unaccept(&tcp_socket);
                nx_tcp_server_socket_relisten(&ip_0, TCP_SERVER_PORT, &tcp_socket);
                break;
            }
            
            memcpy(recv_msg_debug, recv_msg, strlen(recv_msg));
        }  
    }
}

