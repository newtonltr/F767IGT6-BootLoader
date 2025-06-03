#include "thread_init.h"
#include "nx_stm32_eth_driver.h"
#include "eth.h"

/******************************netxduo define**************************************/
//ip0 private packet pool
NX_PACKET_POOL    pool_0;
//ip instance
NX_IP             ip_0;
//pool0 size
#define NX_PACKET_POOL_SIZE ((1536 + sizeof(NX_PACKET)) * 8)
//pool0 area
ULONG  packet_pool_area[NX_PACKET_POOL_SIZE/4 + 4] __attribute__((section(".NxPoolSection"))) = {0};
//ARP server buff
ULONG    arp_space_area[52*20 / sizeof(ULONG)] __attribute__((section(".NxPoolSection"))) = {0};

//default buff
#define IP_ADDR0                        192
#define IP_ADDR1                        168
#define IP_ADDR2                        0
#define IP_ADDR3                        200
ULONG  ip0_address = IP_ADDRESS(IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);

//netx thread infomation
#define  THREAD_NETX_IP0_PRIO0                          2u
//#define  THREAD_NETX_IP0_PRIO1                          6u
#define  THREAD_NETX_IP0_STK_SIZE                     	1024*16u
static   uint64_t  thread_netx_ip0_stack[THREAD_NETX_IP0_STK_SIZE/8];

void thread_init(ULONG input)
{
	UINT nx_init_status = 0;

	HAL_ETH_DeInit(&heth);
	nx_system_initialize();
	nx_init_status |= nx_packet_pool_create(&pool_0,
									"NetX Main Packet Pool",
									1536,  (ULONG*)(((int)packet_pool_area + 15) & ~15) ,
									NX_PACKET_POOL_SIZE);
	nx_init_status |= nx_ip_create(&ip_0,
						"NetX IP0",
						ip0_address,
						0xFFFFFF00UL,
						&pool_0, nx_stm32_eth_driver,
						(UCHAR*)thread_netx_ip0_stack,
						sizeof(thread_netx_ip0_stack),
						THREAD_NETX_IP0_PRIO0);
	nx_init_status |= nx_arp_enable(&ip_0, (void *)arp_space_area, sizeof(arp_space_area));
	nx_init_status |= nx_ip_fragment_enable(&ip_0);
	nx_init_status |= nx_tcp_enable(&ip_0);
	nx_init_status |= nx_udp_enable(&ip_0);
	nx_init_status |= nx_icmp_enable(&ip_0);

	ULONG gateway_ip = ip0_address;
	gateway_ip = (gateway_ip & 0xFFFFFF00) | 0x01;
	nx_ip_gateway_address_set(&ip_0, gateway_ip);
	sleep_ms(300);

	if (nx_init_status != NX_SUCCESS){

	}

	while(1)
	{
		sleep_ms(10);
	}
}

