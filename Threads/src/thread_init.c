#include "thread_init.h"
#include "main.h"
#include "nx_stm32_eth_driver.h"
#include "eth.h"
#include "thread_socket.h"
#include "flash_nor.h"
#include "tx_api.h"
#include <stdint.h>



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
#define IP_ADDR3                        232
ULONG  ip0_address = IP_ADDRESS(IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);

//netx thread infomation
#define  THREAD_NETX_IP0_PRIO0                          2u
//#define  THREAD_NETX_IP0_PRIO1                          6u
#define  THREAD_NETX_IP0_STK_SIZE                     	1024*16u
static   uint64_t  thread_netx_ip0_stack[THREAD_NETX_IP0_STK_SIZE/8];

static void led_ctrl(void)
{
	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_11);
}

static uint32_t jump_to_app_cnt = 0;
static uint8_t socket_connect_status = 0;

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

	// tx thread create
	thread_socket_create();
	
	while(1)
	{
		led_ctrl();
		if(tx_semaphore_get(&sem_socket_connected, TX_NO_WAIT) == TX_SUCCESS)
		{
			socket_connect_status = 1;
		}
		if (socket_connect_status == 0)
		{
			jump_to_app_cnt++;
			if(jump_to_app_cnt > 160)
			{
				JumpToApp();
			}
		}
		sleep_ms(50);
	}
}

const volatile uint32_t AppAddr = 0x8080000;

void JumpToApp(void)
{
        uint32_t i=0;
        void (*SysMemAppJump)(void);        /* 声明一个函数指针 */
        //__IO uint32_t AppAddr = 0x08100000; /* STM32H7的系统BootLoader地址 */
        
				//HAL_GPIO_WritePin(PHY_RST_GPIO_Port,PHY_RST_Pin,0);

				//HAL_ETH_DeInit(&heth);
	
		HAL_GPIO_WritePin(PHY_RST_GPIO_Port,PHY_RST_Pin,0);
		HAL_Delay(100);
		HAL_GPIO_WritePin(PHY_RST_GPIO_Port,PHY_RST_Pin,1);
		HAL_Delay(100);
	
	        /* 关闭全局中断 */
        DISABLE_INT(); 
	
        /* 关闭滴答定时器，复位到默认值 */
        SysTick->CTRL = 0;
        SysTick->LOAD = 0;
        SysTick->VAL = 0;
	
        /* 设置所有时钟到默认状态，使用HSI时钟 */
        HAL_RCC_DeInit();

        /* 关闭所有中断，清除所有中断挂起标志 */
        for (i = 0; i < 8; i++)
        {
                NVIC->ICER[i]=0xFFFFFFFF;
                NVIC->ICPR[i]=0xFFFFFFFF;
        }        

        /* 使能全局中断 */
        ENABLE_INT();

        /* 跳转到APP，首地址是MSP，地址+4是复位中断服务程序地址 */
        SysMemAppJump = (void (*)(void)) (*((uint32_t *) (AppAddr + 4)));

        /* 设置主堆栈指针 */
        __set_MSP(*(uint32_t *)AppAddr);
        
        /* 在RTOS工程，这条语句很重要，设置为特权级模式，使用MSP指针 */
        __set_CONTROL(0);

        /* 跳转到APP */
        SysMemAppJump(); 

        /* 跳转成功的话，不会执行到这里，用户可以在这里添加代码 */
        while (1)
        {

        }
}


