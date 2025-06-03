#ifndef THREAD_SOCKET_H
#define THREAD_SOCKET_H

#include "main.h"
#include "nx_api.h"

// 外部变量声明 - 这些变量在thread_init.c中定义
extern TX_THREAD thread_socket_block;
extern NX_IP ip_0;
extern ULONG ip0_address;
extern NX_PACKET_POOL pool_0;

void thread_socket_create(void);

#endif // THREAD_SOCKET_H