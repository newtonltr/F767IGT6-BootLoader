#include "flash_nor.h"
#include "main.h"

uint16_t norflash_type;
volatile uint8_t norflash_addrw = 2;
#if 1
/**
 * @brief       QSPI发送命令
 * @param       cmd : 要发送的指令
 * @param       addr: 发送指令的目的地址
 * @param       mode: 模式,详细位分配如下:
 *  @arg mode[1:0]: 指令模式;00,无指令;01,单线传输指令;10,双线传输指令;11,四线传输指令.
 *  @arg mode[3:2]: 地址模式;00,无地址;01,单线传输地址;10,双线传输地址;11,四线传输地址.
 *  @arg mode[5:4]: 地址长度;00,8位地址;01,16位地址; 10,24位地址;   11,32位地址.
 *  @arg mode[7:6]: 数据模式;00,无数据; 01,单线传输数据;10,双线传输数据;11,四线传输数据.
 * @param       dmcycle: 空指令周期数
 * @retval      无
 */
void qspi_send_cmd(uint8_t cmd, uint32_t addr, uint8_t mode, uint8_t dmcycle)
{
    QSPI_CommandTypeDef qspi_command_handle;
    qspi_command_handle.Instruction = cmd; /* 指令 */
    qspi_command_handle.Address = addr; /* 地址 */
    qspi_command_handle.DummyCycles = dmcycle; /* 空指令周期数 */

    if(((mode >> 0) & 0x03) == 0)
        qspi_command_handle.InstructionMode = QSPI_INSTRUCTION_NONE; /* 指令模式 */
    else if(((mode >> 0) & 0x03) == 1)
        qspi_command_handle.InstructionMode = QSPI_INSTRUCTION_1_LINE; /* 指令模式 */
    else if(((mode >> 0) & 0x03) == 2)
        qspi_command_handle.InstructionMode = QSPI_INSTRUCTION_2_LINES;/* 指令模式 */
    else if(((mode >> 0) & 0x03) == 3)
        qspi_command_handle.InstructionMode = QSPI_INSTRUCTION_4_LINES;/* 指令模式 */

    if(((mode >> 2) & 0x03) == 0)
        qspi_command_handle.AddressMode = QSPI_ADDRESS_NONE; /* 地址模式 */
    else if(((mode >> 2) & 0x03) == 1)
        qspi_command_handle.AddressMode = QSPI_ADDRESS_1_LINE; /* 地址模式 */
    else if(((mode >> 2) & 0x03) == 2)
        qspi_command_handle.AddressMode = QSPI_ADDRESS_2_LINES; /* 地址模式 */
    else if(((mode >> 2) & 0x03) == 3)
        qspi_command_handle.AddressMode = QSPI_ADDRESS_4_LINES; /* 地址模式 */

    if(((mode >> 4)&0x03) == 0)
        qspi_command_handle.AddressSize = QSPI_ADDRESS_8_BITS; /* 地址长度 */
    else if(((mode >> 4) & 0x03) == 1)
        qspi_command_handle.AddressSize = QSPI_ADDRESS_16_BITS; /* 地址长度 */
    else if(((mode >> 4) & 0x03) == 2)
        qspi_command_handle.AddressSize = QSPI_ADDRESS_24_BITS; /* 地址长度 */
    else if(((mode >> 4) & 0x03) == 3)
        qspi_command_handle.AddressSize = QSPI_ADDRESS_32_BITS; /* 地址长度 */

    if(((mode >> 6) & 0x03) == 0)
        qspi_command_handle.DataMode=QSPI_DATA_NONE; /* 数据模式 */
    else if(((mode >> 6) & 0x03) == 1)
        qspi_command_handle.DataMode = QSPI_DATA_1_LINE; /* 数据模式 */
    else if(((mode >> 6) & 0x03) == 2)
        qspi_command_handle.DataMode = QSPI_DATA_2_LINES; /* 数据模式 */
    else if(((mode >> 6) & 0x03) == 3)

    qspi_command_handle.DataMode = QSPI_DATA_4_LINES; /* 数据模式 */
    qspi_command_handle.SIOOMode = QSPI_SIOO_INST_EVERY_CMD; /* 每次发送指令 */
    qspi_command_handle.AlternateByteMode=QSPI_ALTERNATE_BYTES_NONE;/*无附加字节*/
    qspi_command_handle.DdrMode = QSPI_DDR_MODE_DISABLE; /* 关闭DDR模式 */
    qspi_command_handle.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    HAL_QSPI_Command(&hqspi, &qspi_command_handle, 1000);
}

/**
 * @brief       QSPI接收数据
 * @param       buf     : 数据缓冲区起始地址
 * @param       datalen : 要接收的数据长度
 * @retval      0, 成功; 否则, 失败.
 */
int qspi_receive(uint8_t *buf, uint32_t datalen)
{
    hqspi.Instance->DLR = datalen - 1; /* 设置接收数据长度 */
    if (HAL_QSPI_Receive(&hqspi, buf, 5000) != HAL_OK) {
        return -1;
    }

    return 0;
}

/**
 * @brief      QSPI发送数据
 * @param       buf     : 数据缓冲区起始地址
 * @param       datalen : 要发送的数据长度
 * @retval      0, 成功; 否则, 失败.
 */
int qspi_transmit(uint8_t *buf, uint32_t datalen)
{
    hqspi.Instance->DLR = datalen - 1; /* 设置发送数据长度 */
    if (HAL_QSPI_Transmit(&hqspi, buf, 5000) != HAL_OK){
        return -1;
    }

    return 0;
}

/**
 * @brief       等待状态标志
 * @param       flag : 要等待的标志位
 * @param       sta  : 要等待的状态
 * @param       wtime: 等待时间
 * @retval      0, 等待成功; 1, 等待失败.
 */
int qspi_wait_flag(uint32_t flag, uint8_t sta, uint32_t wtime_ms)
{
    uint8_t flagsta = 0;
    while (wtime_ms)
    {
        sleep_ms(1);
        wtime_ms--;
        flagsta = (QUADSPI->SR & flag) ? 1 : 0; /* 获取状态标志 */
        if (flagsta == sta){
            return 0;
        }
    }

    return -1;
}



/**
 * @brief       等待状态
 * @param       无
 * @retval      无
 */
static void norflash_wait_busy(void)
{
    while ((norflash_read_sr(1) & 0x01) == 0x01);   /*  等待BUSY位 */
}

/*
static void norflash_qspi_disable(void)
{
    qspi_send_cmd(FLASH_ExitQPIMode, 0, (0 << 6) | (0 << 4) | (0 << 2) | (3 << 0), 0);
}

static void norflash_qspi_enable(void)
{
    qspi_send_cmd(FLASH_EnterQPIMode, 0, (0 << 6) | (0 << 4) | (0 << 2) | (3 << 0), 0);
}*/

/**
 * @brief       使用FLASH QE位启用IO2/IO3
 * @param       无
 * @retval      无
 */
void norflash_qe_enable(void)
{
    uint8_t stareg2 = 0;
    stareg2 = norflash_read_sr(2);  /* 获取状态寄存器2初始值 */

//    //printf("stareg2:%x\r\n", stareg2);
    if ((stareg2 & 0X02) == 0)      /* QE位未启用 */
    {
        norflash_write_enable();    /* 写使能 */
        stareg2 |= 1 << 1;          /* 启用QE位 */
        norflash_write_sr(2, stareg2);  /* 写状态寄存器2 */
    }
}

/**
 * @brief       25QXX写使能
 *   @note      写状态寄存器S1的WEL位
 * @param       无
 * @retval      无
 */
void norflash_write_enable(void)
{
    /* SPI,写使能指令,地址为0,写_8位地址_无地址_四线传输指令,不保护,0字节数据 */
    qspi_send_cmd(FLASH_WriteEnable, 0, (0 << 6) | (0 << 4) | (0 << 2) | (1 << 0), 0);
}

/**
 * @brief       25QXX写禁止
 *   @note      写状态寄存器WEL位
 * @param       无
 * @retval      无
 */
void norflash_write_disable(void)
{
    /* SPI,写禁止指令,地址为0,写_8位地址_无地址_四线传输指令,不保护,0字节数据 */
    qspi_send_cmd(FLASH_WriteDisable, 0, (0 << 6) | (0 << 4) | (0 << 2) | (1 << 0), 0);
}

/**
 * @brief       获取25QXX状态寄存器25QXX一般有3个状态寄存器
 *   @note      状态寄存器1
 *              BIT7  6   5   4   3   2   1   0
 *              SPR   RV  TB BP2 BP1 BP0 WEL BUSY
 *              SPR:默认0,状态寄存器最高位,写保护
 *              TB,BP2,BP1,BP0:FLASH写入数据
 *              WEL:写使能
 *              BUSY:忙位(1,忙;0,空闲)
 *              默认:0x00
 *
 *              状态寄存器2
 *              BIT7  6   5   4   3   2   1   0
 *              SUS   CMP LB3 LB2 LB1 (R) QE  SRP1
 *
 *              状态寄存器3
 *              BIT7      6    5    4   3   2   1   0
 *              HOLD/RST  DRV1 DRV0 (R) (R) WPS ADP ADS
 *
 * @param       regno: 状态寄存器号,范围:1~3
 * @retval      状态寄存器值
 */
uint8_t norflash_read_sr(uint8_t regno)
{
    uint8_t byte = 0, command = 0;

    switch (regno)
    {
        case 1:
            command = FLASH_ReadStatusReg1;  /* 读状态寄存器1指令 */
            break;

        case 2:
            command = FLASH_ReadStatusReg2;  /* 读状态寄存器2指令 */
            break;

        case 3:
            command = FLASH_ReadStatusReg3;  /* 读状态寄存器3指令 */
            break;

        default:
            command = FLASH_ReadStatusReg1;
            break;
    }

    /* SPI,写command指令,地址为0,四线传输_8位地址_无地址_四线传输指令,不保护,1字节数据 */
    qspi_send_cmd(command, 0, (1 << 6) | (0 << 4) | (0 << 2) | (1 << 0), 0);
    qspi_receive(&byte, 1);
    return byte;
}

/**
 * @brief      写25QXX状态寄存器
 *   @note      状态寄存器说明请参考norflash_read_sr函数说明
 * @param       regno: 状态寄存器号,范围:1~3
 * @param       sr   : 要写入的状态寄存器值
 * @retval      无
 */
void norflash_write_sr(uint8_t regno, uint8_t sr)
{
    uint8_t command = 0;

    switch (regno)
    {
        case 1:
            command = FLASH_WriteStatusReg1;  /* 写状态寄存器1指令 */
            break;

        case 2:
            command = FLASH_WriteStatusReg2;  /* 写状态寄存器2指令 */
            break;

        case 3:
            command = FLASH_WriteStatusReg3;  /* 写状态寄存器3指令 */
            break;

        default:
            command = FLASH_WriteStatusReg1;
            break;
    }

    /* SPI,写command指令,地址为0,四线传输_8位地址_无地址_四线传输指令,不保护,1字节数据 */
    qspi_send_cmd(command, 0, (1 << 6) | (0 << 4) | (0 << 2) | (1 << 0), 0);
    qspi_transmit(&sr, 1);
}

/**
 * @brief       获取芯片ID
 * @param       无
 * @retval      FLASH芯片ID
 *   @note     芯片ID表: norflash.h, 芯片ID表
 */
uint16_t norflash_read_id(void)
{
    uint8_t temp[2];
    uint16_t deviceid;
    //qspi_init();          /* 如果使用前要先初始化 */
    /* SPI,读id,地址为0,四线传输_24位地址_四线传输地址_四线传输指令,不保护,2字节数据 */
    qspi_send_cmd(FLASH_ManufactDeviceID, 0, (1 << 6) | (2 << 4) | (1 << 2) | (1 << 0), 0);
    qspi_receive(temp, 2);
    deviceid = (temp[0] << 8) | temp[1];

    if (deviceid == W25Q256)
    {
        norflash_addrw = 3;   /* 如果是W25Q256, 使用32bit地址模式 */
    }

    return deviceid;
}

/**
 * @brief       读取SPI FLASH,支持QSPI模式
 *   @note      从指定地址开始读取指令数据
 * @param       pbuf    : 数据存储区
 * @param       addr    : 开始读取的地址(最高32bit)
 * @param       datalen : 要读取的字节数(最高65535)
 * @retval      无
 */
void norflash_read(uint8_t *pbuf, uint32_t addr, uint16_t datalen)
{
    /* QSPI,快速读取,地址为addr,4线传输_24/32位地址_4线传输地址_1线传输指令,6字节,datalen字节 */
    qspi_send_cmd(FLASH_FastReadQuad, addr, (3 << 6) | (norflash_addrw << 4) | (3 << 2) | (1 << 0), 6);
    qspi_receive(pbuf, datalen);
}

/**
 * @brief       SPI写一页(0~65535)的写入操作,256字节为1个Page, 4Kbytes为1个Sector, 16个Page为1个Block
 *   @note      从指定地址开始写入256字节的数据
 * @param       pbuf    : 数据存储区
 * @param       addr    : 开始写的地址(最高32bit)
 * @param       datalen : 要写的字节数(最高256),应由上层调用者保证写入的地址范围正确!!!
 * @retval      无
 */
static void norflash_write_page(uint8_t *pbuf, uint32_t addr, uint16_t datalen)
{
    norflash_write_enable();        /* 写使能 */

    /* QSPI,写页指令,地址为addr,4线传输_24/32位地址_1线传输地址_1线传输指令,不保护,datalen字节 */
    qspi_send_cmd(FLASH_PageProgramQuad, addr, (3 << 6) | (norflash_addrw << 4) | (1 << 2) | (1 << 0), 0);

    qspi_transmit(pbuf, datalen);   /* 发送数据 */
    norflash_wait_busy();           /* 等待写操作 */
}

/**
 * @brief       写入SPI FLASH
 *   @note      从指定地址开始写入指令数据 , 必须保证写入的地址范围全为0XFF,否则写入的数据会丢失!!!
 *              自动分页写入
 *              从指定地址开始写入指令数据,必须保证地址范围越界!
 *
 * @param       pbuf    : 数据存储区
 * @param       addr    : 开始写的地址(最高32bit)
 * @param       datalen : 要写的字节数(最高65535)
 * @retval      无
 */
void norflash_write_nocheck(uint8_t *pbuf, uint32_t addr, uint16_t datalen)
{
    uint16_t pageremain;
    pageremain = 256 - addr % 256;  /* 当前页剩余字节数 */

    if (datalen <= pageremain)      /* 如果小于等于256字节 */
    {
        pageremain = datalen;
    }

    while (1)
    {
        /* 写数据时,如果写到页剩余字节数为0时, 一次写入
         * 写数据时,如果写到页剩余字节数为0时, 写入当前页剩余字节, 然后写入页剩余字节, 然后写入剩余长度的不同部分
         */
        norflash_write_page(pbuf, addr, pageremain);

        if (datalen == pageremain)   /* 写入完成 */
        {
            break;
        }
        else     /* datalen > pageremain */
        {
            pbuf += pageremain;         /* pbuf指针偏移,前面已经写入pageremain字节 */
            addr += pageremain;         /* 写地址偏移,前面已经写入pageremain字节 */
            datalen -= pageremain;      /* 写剩余长度减去已经写入的字节数 */

            if (datalen > 256)          /* 剩余数据大于一页,需要写一页 */
            {
                pageremain = 256;       /* 一次写入256字节 */
            }
            else     /* 剩余数据小于一页,需要写一页 */
            {
                pageremain = datalen;   /* 写入256字节 */
            }
        }
    }
}

/**
 * @brief       写入SPI FLASH
 *   @note      从指定地址开始写入指令数据 , 必须保证写入的地址范围全为0XFF,否则写入的数据会丢失!!!
 *              自动分页写入
 *              从指定地址开始写入指令数据,必须保证地址范围越界!
 *
 * @param       pbuf    : 数据存储区
 * @param       addr    : 开始写的地址(最高32bit)
 * @param       datalen : 要写的字节数(最高65535)
 * @retval      无
 */
uint8_t g_norflash_buf[4096];   /* 全局变量 */

void norflash_write(uint8_t *pbuf, uint32_t addr, uint16_t datalen)
{
    uint32_t secpos;
    uint16_t secoff;
    uint16_t secremain;
    uint16_t i;
    uint8_t *norflash_buf;

    norflash_buf = g_norflash_buf;
    secpos = addr / 4096;       /* 扇区地址 */
    secoff = addr % 4096;       /* 扇区偏移 */
    secremain = 4096 - secoff;  /* 扇区剩余空间 */

//    printf("ad:%X,nb:%X\r\n", addr, datalen); /* 调试信息 */
    if (datalen <= secremain)
    {
        secremain = datalen;    /* 数据长度不超过当前扇区剩余空间，只需处理datalen字节 */
    }

    while (1)
    {
        norflash_read(norflash_buf, secpos * 4096, 4096);   /* 读取整个扇区数据到缓冲区 */

        for (i = 0; i < secremain; i++)   /* 检查目标区域是否全为0xFF */
        {
            if (norflash_buf[secoff + i] != 0XFF)
            {
                break;      /* 发现非0xFF数据，需要擦除扇区，退出检查 */
            }
        }

        if (i < secremain)   /* 目标区域有数据，需要擦除和重写 */
        {
            norflash_erase_sector(secpos);  /* 擦除扇区 */

            for (i = 0; i < secremain; i++)   /* 将新数据合并到缓冲区 */
            {
                norflash_buf[i + secoff] = pbuf[i];
            }

            norflash_write_nocheck(norflash_buf, secpos * 4096, 4096);  /* 写入整个扇区 */
        }
        else        /* 目标区域全为0xFF，可直接写入 */
        {
            norflash_write_nocheck(pbuf, addr, secremain);  /* 直接写入新数据 */
        }

        if (datalen == secremain)
        {
            break;  /* 所有数据写入完成 */
        }
        else        /* 还有数据未写入，准备处理下一个扇区 */
        {
            secpos++;               /* 扇区地址+1 */
            secoff = 0;             /* 下一个扇区从偏移0开始 */

            pbuf += secremain;      /* 源数据指针偏移 */
            addr += secremain;      /* 目标地址偏移 */
            datalen -= secremain;   /* 剩余数据长度减少 */

            if (datalen > 4096)
            {
                secremain = 4096;   /* 下一轮需要写入整个扇区 */
            }
            else
            {
                secremain = datalen;/* 下一轮写入剩余的所有数据 */
            }
        }
    }
}

/**
 * @brief       擦除整个芯片
 *   @note      等待时间超长...
 * @param       无
 * @retval      无
 */
void norflash_erase_chip(void)
{
    norflash_write_enable();    /* 写使能 */
    norflash_wait_busy();       /* 等待空闲 */
    /* QPI,写整个芯片指令,地址为0,写_8位地址_无地址_1线传输指令,不保护,0字节数据 */
    qspi_send_cmd(FLASH_ChipErase, 0, (0 << 6) | (0 << 4) | (0 << 2) | (1 << 0), 0);
    norflash_wait_busy();       /* 等待芯片擦除完成 */
}

/**
 * @brief       擦除一个扇区
 *   @note      注意,擦除一个扇区时,不能擦除扇区内的数据,否则会擦除失败!
 *              擦除一个扇区时:150ms
 *
 * @param       saddr : 扇区地址 实际应用中应为扇区起始地址
 * @retval      无
 */
void norflash_erase_sector(uint32_t saddr)
{
    //printf("fe:%x\r\n", saddr);   /* 如果falsh地址错误,会擦除整个flash */
    saddr *= 4096;
    norflash_write_enable();        /* 写使能 */
    norflash_wait_busy();           /* 等待空闲 */

    /* QPI,写扇区擦除指令,地址为0,写_24/32位地址_1线传输地址_1线传输指令,不保护,0字节数据 */
    qspi_send_cmd(FLASH_SectorErase, saddr, (0 << 6) | (norflash_addrw << 4) | (1 << 2) | (1 << 0), 0);

    norflash_wait_busy();           /* 等待擦除完成 */
}

/**
 * @brief       初始化SPI NOR FLASH
 * @param       无
 * @retval      无
 */
void norflash_init(void)
{
    uint8_t temp;
    //qspi_init(); /* 初始化QSPI */
    //norflash_qspi_disable(); /* 关闭QPI模式(在初始化芯片之前关闭QPI模式,否则会初始化失败) */
    norflash_qe_enable(); /* 启用QE位 */
    norflash_type = norflash_read_id();/* 获取FLASH ID. */
    if (norflash_type == W25Q256)/* SPI FLASH为W25Q256, 应该启用4字节地址模式 */
    {
        temp = norflash_read_sr(3); /* 获取状态寄存器3判断地址模式 */
        if ((temp & 0X01) == 0) /* 不是4字节地址模式,启用4字节地址模式 */
        {
            norflash_write_enable();/* 写使能 */
            temp |= 1 << 1; /* ADP=1, 启用4字节地址模式 */
            norflash_write_sr(3, temp);/* 写SR3 */
            norflash_write_enable(); /* 写使能 */
            /* QPI,启用4字节地址指令,地址为0,写_8位地址_无地址_四线传输指令,
            不保护,0字节数据 */
            qspi_send_cmd(FLASH_Enable4ByteAddr, 0, (0 << 6) | (0 << 4)
                                                | (0 << 2) | (1 << 0), 0);
        }
    }
}


#endif









