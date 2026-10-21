#include "w25q32.h"
#include "stdio.h"
#include "systick_delay_freertos.h"

//===========================================================================
/*******************************************************************
********************************************************************
* 名称：u8 w25q32_flash_readSR(void)
* 功能：读状态寄存器
* 参数：null
* 返回值：读到的状态
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
u8 w25q32_flash_readSR(u8 select)
{
	u8 byte=0;   
	if(select == 1)
	{
		W25Q32_CS_PC1 = 1;   // 禁用
		W25Q32_CS_PA4 = 0;   // 启用  //使能器件 
	}
    else
    {
		W25Q32_CS_PC1 = 0;   // 禁用
		W25Q32_CS_PA4 = 1;   // 启用  //使能器件
    }	
	spi1_read_write_byte(W25X_ReadStatusReg1);         //发送读取状态寄存器命令    
	byte=spi1_read_write_byte(DUMMY_DATA);             //读取一个字节  
	if(select == 1)
	{
		W25Q32_CS_PA4 = 1;   // 禁能器件
	}
	else
	{
		W25Q32_CS_PC1 = 1;   // 禁用
	}
	return byte; 
}
/*******************************************************************
********************************************************************
* 名称：void w25q32_spi_write_enable(void)
* 功能：使能w25q32的写
* 参数：null
* 返回值：null
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
static void w25q32_spi_write_enable(u8 select)
{
	if(select == 1)
	{
		W25Q32_CS_PC1 = 1;   // 禁用
		W25Q32_CS_PA4 = 0;   // 启用  //使能器件 
	}
    else
    {
		W25Q32_CS_PC1 = 0;   // 禁用
		W25Q32_CS_PA4 = 1;   // 启用  //使能器件
    } 
	spi1_read_write_byte(W25X_WriteEnable);
	if(select == 1)
	{
		W25Q32_CS_PA4 = 1;   // 禁能器件1
	}
	else
	{
		W25Q32_CS_PC1 = 1;   // 禁用器件2
	} 
}
/*******************************************************************
********************************************************************
* 名称：void w25q32_spi_write_disable(void)
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void w25q32_spi_write_disable(u8 select)
{
	if(select == 1)
	{
		W25Q32_CS_PC1 = 1;   // 禁用
		W25Q32_CS_PA4 = 0;   // 启用  //使能器件 
	}
    else
    {
		W25Q32_CS_PC1 = 0;   // 禁用
		W25Q32_CS_PA4 = 1;   // 启用  //使能器件
    }
	spi1_read_write_byte(W25X_WriteDisable);
	if(select == 1)
	{
		W25Q32_CS_PA4 = 1;   // 禁能器件1
	}
	else
	{
		W25Q32_CS_PC1 = 1;   // 禁用器件2
	}
}

/*******************************************************************
********************************************************************
* 名称：u16 spi_flash_read_device_id(void)
* 功能：读取flash 的device id 
* 参数： select : flash 选择 1： u20  2： u33
* 返回值：读取到的 id  （0x15  w25q32）
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
//读取芯片ID W25X32的ID:0X15
u8 w25q32_flash_read_device_id(u8 select)
{
	u8 Temp = 0;	
    if(select == 1)
	{
		W25Q32_CS_PC1 = 1;   // 禁用
		W25Q32_CS_PA4 = 0;   // 启用  //使能器件 
	}
    else
    {
		W25Q32_CS_PC1 = 0;   // 禁用
		W25Q32_CS_PA4 = 1;   // 启用  //使能器件
    }	
			    
	spi1_read_write_byte(W25X_DeviceID);//发送读取ID命令	    
	spi1_read_write_byte(DUMMY_DATA); 	    
	spi1_read_write_byte(DUMMY_DATA); 	    
	spi1_read_write_byte(DUMMY_DATA); 	 			   
	Temp =spi1_read_write_byte(DUMMY_DATA);  
	if(select == 1)
	{
		W25Q32_CS_PA4 = 1;   // 禁能器件1
	}
	else
	{
		W25Q32_CS_PC1 = 1;   // 禁用器件2
	}				    
	return Temp;
}

/*******************************************************************
********************************************************************
* 名称：u32 spi_flash_read_jedec_id(void)
* 功能： 读取w25q32的edec_id
* 参数：null
* 返回值：读取到的id（0xef4016）
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
u32 w25q32_flash_read_jedec_id(u8 select)
{
	u32 temp = 0;
	if(select == 1)
	{
		W25Q32_CS_PC1 = 1;   // 禁用
		W25Q32_CS_PA4 = 0;   // 启用  //使能器件 
	}
    else
    {
		W25Q32_CS_PC1 = 0;   // 禁用
		W25Q32_CS_PA4 = 1;   // 启用  //使能器件
    }					    
	spi1_read_write_byte(W25X_JedecDeviceID);//发送读取ID命令	
	temp = spi1_read_write_byte(DUMMY_DATA)  << 16;
	temp |= spi1_read_write_byte(DUMMY_DATA) << 8;
	temp |= spi1_read_write_byte(DUMMY_DATA) << 0;
	if(select == 1)
	{
		W25Q32_CS_PA4 = 1;   // 禁能器件1
	}
	else
	{
		W25Q32_CS_PC1 = 1;   // 禁用器件2
	}
	return temp;
}
/*******************************************************************
********************************************************************
* 名称：void  wait_for_read_ready(u8 select)
* 功能：在读取的过程中，我们需要得知状态，看它是否空闲后再写入数据、擦除数据、读取数据
* 参数：select :SPI 选择  1 ：选择 falsh 1  ，2：其他
* 返回值：无
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void  wait_for_read_ready(u8 select)
{
	u8 reg_status = 0x01 ;
	while (reg_status & 0x01)
	{
		if(select == 1)  //使能器件
		{
			W25Q32_CS_PC1 = 1;   // 禁用
			W25Q32_CS_PA4 = 0;   // 启用  
		}
		else
		{
			W25Q32_CS_PA4 = 1;   // 禁用  
			W25Q32_CS_PC1 = 0;   // 启用
		}
		spi1_read_write_byte(W25X_ReadStatusReg1);
		reg_status = spi1_read_write_byte(DUMMY_DATA) ;
		if(select == 1)
		{
			W25Q32_CS_PA4 = 1;   // 禁能器件1
		}
		else
		{
			W25Q32_CS_PC1 = 1;   // 禁用器件2
		}
	}
}

/*******************************************************************
********************************************************************
* 名称：void SPI_Flash_Read(u8* p_buf,u32 read_addr,u16 num_of_read)
* 功能：在指定地址开始读取指定长度的数据
* 参数：
	p_buf：读取的数据存放指针  
	read_addr: 读取的地址   
	num_of_read要读取的字节数(最大 65535)
* 返回值：null
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：由于 W25Q32 支持以任意地址（但是不能超过 W25Q32 的地址范围）开始读取数据
*  W25Q32: 32M-bit / 4M-byte (4,194,304字节)
* 页  ：256-bytes
  扇区：1扇区 = 16页     =4K-bytes   4096 B    1024个扇区
  块  ：1块 =16扇区      =64K_bytes            64块     
时钟速度最高：80MHz 
擦除、写次数：高达100,000次
*********************************************************************
*********************************************************************/
void w25q32_flash_read(u8* p_buf,u32 read_addr,u16 num_of_read,u8 select)
{
	u16 i = 0;
	wait_for_read_ready(select);
	if(select == 1)
	{
		W25Q32_CS_PC1 = 1;   // 禁用
		W25Q32_CS_PA4 = 0;   // 启用  //使能器件 
	}
    else
    {
		W25Q32_CS_PA4 = 1;   // 禁用  //使能器件
		W25Q32_CS_PC1 = 0;   // 启用
    }
	spi1_read_write_byte(W25X_ReadData); //发送读取命令
	
	spi1_read_write_byte((u8)((read_addr)>>16) & 0xff); //发送 24bit 地址
	spi1_read_write_byte((u8)((read_addr)>>8)  & 0xff);
	spi1_read_write_byte((u8)((read_addr)>>0)  & 0xff);
	
	for(i = 0;i < num_of_read; i++) 
	{
		p_buf[i] = spi1_read_write_byte(DUMMY_DATA);//循环读数
	}
	if(select == 1)
	{
		W25Q32_CS_PA4 = 1;   // 禁能器件1
	}
	else
	{
		W25Q32_CS_PC1 = 1;   // 禁用器件2
	}

	
}



/*******************************************************************
********************************************************************
* 名称：void spi_flash_page_write(u8* p_buf,u32 write_addr,u16 num_to_write)
* 功能：flash的页写函数
* 参数：p_buf：指向要写的内容指针   write_addr：要写入的地址   num_to_write：要写入的数量
* 返回值：null
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 页写入指令的地址并不要求按 256 字节对齐，只要确认目标存储单元是擦除状态即可
* 最多个可以发送 256 字节数据，写入前需要扇区擦除
*********************************************************************
*********************************************************************/
void w25q32_flash_page256_write_no_erase(u8* p_buf,u32 write_addr,u16 num_to_write,u8 select)
{
	u16 i=0;
	w25q32_spi_write_enable(select);
	if(select == 1)
	{
		W25Q32_CS_PC1 = 1;   // 禁用
		W25Q32_CS_PA4 = 0;   // 启用  //使能器件 
	}
    else
    {
		W25Q32_CS_PC1 = 0;   // 禁用
		W25Q32_CS_PA4 = 1;   // 启用  //使能器件
    }
	spi1_read_write_byte(W25X_PageProgram);//发送页写指令
	//发送页地址 高中低地址
	spi1_read_write_byte((u8)((write_addr) >> 16));
	spi1_read_write_byte((u8)((write_addr) >> 8));
	spi1_read_write_byte((u8)((write_addr) >> 0));
//	if(num_to_write >256)
//	{
//		num_to_write = 256;
//		printf("spi flash pagewrite page too large!");
//	}
	//写入数据
	for(i=0;i<num_to_write;i++)
	{
		spi1_read_write_byte(p_buf[i]);
	}
	if(select == 1)
	{
		W25Q32_CS_PA4 = 1;   // 禁能器件1
	}
	else
	{
		W25Q32_CS_PC1 = 1;   // 禁用器件2
	}
	//等待页写完毕
	w25q32_flash_waitfor_write_end(select);
	
}
/*******************************************************************
********************************************************************
* 名称：void spi1_flash_write_no_erase(u8* p_buf,u32 write_addr,u16 num_to_write)
* 功能：在指定地址开始写入指定长度的数据,但是要确保地址不越界!
* 参数：p_buf：指向要写的内容指针   write_addr：要写入的地址   num_to_write：要写入的数量
* 返回值：null
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void w25q32_flash_autopage_write_no_erase(u8* p_buf,u32 write_addr,u32 num_to_write,u8 select)
{
	u16 page_main= 0;              // 一次写入的数据数量
	page_main = 256-write_addr%256;// 根据地址算出此页还有多少空间可以使用
	if(page_main >= num_to_write)  // 如果剩下的空间可以满足写入的字符数量
	{
		page_main = num_to_write;
	}
	while(1)
	{
		w25q32_flash_page256_write_no_erase(p_buf,write_addr,page_main,select); //剩余空间已近用完
		if(page_main == num_to_write)
		{
			break;
		}
		else//要写的数据个数，大于一页内剩下的空间
		{
			p_buf += page_main;         // 要写入的数据移动到正确位置
			write_addr += page_main;    // 要写入的地址移动到正确的位置
			
			num_to_write -=page_main;   //减去已经写入了的字节数,剩下还有多少要写入的
			if(num_to_write >256)  //下次要写入的数据，一页
			{
				page_main =256;
			}
			else
			{
				page_main = num_to_write;
			}
			
		}
	}
	
}

/*********************** ；********************************************
********************************************************************
* 名称：void spi1_flash_write(u8* p_buf,u32 write_addr,u16 num_to_write)
* 功能：在指定地址开始写入指定长度的数据，该函数带擦除操作!
* 参数：p_buf：指向要写的内容指针   write_addr：要写入的地址   num_to_write：要写入的数量 select ： flash 选择
* 返回值：null
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void w25q32_flash_autosector_write_with_erase(u8* p_buf,u32 write_addr,u32 num_to_write,u8 select)
{
	u32 sector_num;         //保存扇区起始地址（4k）
	u16 sector_offset;      //保存扇区偏移地址
	u16 sector_main;        //在此扇区上剩余能使用的空间
	u16 i;                  //循环变量
	sector_num    = write_addr / 4096;  //根据写入地址，计算的扇区起始地址
	sector_offset = write_addr % 4096;  //根据写入地址，计算的扇区偏移地址
	sector_main = 4096 - sector_offset; //扇区剩余空间大小（保存一次要写入的数据量）
	if(num_to_write <= sector_main) //不大于 4096 个字节
	{
		sector_main = num_to_write;
	}
	while(1)
	{
		w25q32_flash_read(SPI1_FLASH_BUF,sector_num*4096,4096,select);//将这个要写入数据的整个扇区原有数据读出
		//判断要不要擦除
		for(i=0;i<sector_main;i++)
		{
			//从读出来的数组中，从偏移地址处开始（要写入数据的地址处），如果不是0xff，就要擦除
			//且只要有一个不是0xff，就要全部擦除
			if(SPI1_FLASH_BUF[i+sector_offset] != 0xFF)
			{
				break;
			}
		}
		//===========================================================================
		if(i < sector_main)//通过i的值来判断是否需要擦除，如果i < sector_main,需要擦除
		{
			w25q32_flash_sector4k_erase(sector_num,select);//擦除整个扇区
			for(i=0;i<sector_main;i++)
			{
				//将本次要写入的数据，从偏移地址处开始依次copy到数组中
				 SPI1_FLASH_BUF[i+sector_offset] = p_buf[i] ;
			}
			w25q32_flash_autopage_write_no_erase(SPI1_FLASH_BUF,sector_num*4096,4096,select);
		}
		else// 不需要擦除
		{
			w25q32_flash_autopage_write_no_erase(p_buf,write_addr,sector_main,select);
		}
		//===========================================================================
		if(sector_main == num_to_write)  //写入结束了
		{
			break;
		}
		else //没有结束
		{
			sector_num++;              		 //扇区地址增1
			sector_offset=0;                 //偏移位置为0 
			
			p_buf += sector_main;            //指针偏移
			write_addr += sector_main;       //写地址偏移
			num_to_write -= sector_main;     //字节数递减
			if(num_to_write > 4096)          //下一个扇区还是写不完
			{
				sector_main = 4096;
			}
			else                             //下一个扇区可以写完了
			{
				sector_main = num_to_write;
			}
		}
	}
}
/*******************************************************************
********************************************************************
* 名称：void spi1_flash_erase_chip(void)
* 功能：全片擦除
* 参数：null
* 返回值：null
* 版本：v1 
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* //W25X16:25s 
//W25X32:40s 
//W25X64:40s 
*********************************************************************
*********************************************************************/
void w25q32_flash_erase_chip(u8 select)
{
	w25q32_spi_write_enable(select);
	w25q32_flash_waitfor_write_end(select);
	if(select == 1)
	{
		W25Q32_CS_PC1 = 1;   // 禁用
		W25Q32_CS_PA4 = 0;   // 启用  //使能器件 
	}
    else
    {
		W25Q32_CS_PC1 = 0;   // 禁用
		W25Q32_CS_PA4 = 1;   // 启用  //使能器件
    }

	spi1_read_write_byte(W25X_ChipErase);//发送chip erase 命令	
	if(select == 1)
	{
		W25Q32_CS_PA4 = 1;   // 禁能器件1
	}
	else
	{
		W25Q32_CS_PC1 = 1;   // 禁用器件2
	}
	w25q32_flash_waitfor_write_end(select);
//	printf("erase is ok\r\n");
}
/*******************************************************************
********************************************************************
* 名称：void w25q32_flash_sector4k_erase(u32 sector_addr)
* 功能：擦除一个扇区
* 参数：sector_addr  扇区地址 （0-----1023） 4M空间  4*1024k / 4k
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 由于 FLASH存储器的特性决定了它只能把原来为“1”的数据位改写成“0”
* 在写入前，必须要对目标存储矩阵进行擦除操作
调用扇区擦除指令时注意输入的地址要对齐到 4KB。
//擦除一个扇区的最少时间:150ms
*********************************************************************
*********************************************************************/
void w25q32_flash_sector4k_erase(u32 sector_addr,u8 select)
{
	sector_addr *= 4096;   // 将扇区号（0-1023），乘以4096后，转换为真实的实际扇区起始地址
	w25q32_spi_write_enable(select);     // 写使能打开，准备写数据
	w25q32_flash_waitfor_write_end(select); // 等待flash空闲
	if(select == 1)
	{
		W25Q32_CS_PC1 = 1;   // 禁用
		W25Q32_CS_PA4 = 0;   // 启用  //使能器件 
	}
    else
    {
		W25Q32_CS_PC1 = 0;   // 禁用
		W25Q32_CS_PA4 = 1;   // 启用  //使能器件
    }
	spi1_read_write_byte(W25X_SectorErase);//发送扇区擦除指令
	//发送扇区地址24bit，发高，中 低 地址
	spi1_read_write_byte((u8)((sector_addr) >> 16));
	spi1_read_write_byte((u8)((sector_addr) >> 8));
	spi1_read_write_byte((u8)((sector_addr) >> 0));
	if(select == 1)
	{
		W25Q32_CS_PA4 = 1;   // 禁能器件1
	}
	else
	{
		W25Q32_CS_PC1 = 1;   // 禁用器件2
	}
	//等待擦除完毕（擦除也是一个写入的过程）
	w25q32_flash_waitfor_write_end(select);
	
}
/*******************************************************************
********************************************************************
* 名称：void spi_flash_waitfor_write_end(void)
* 功能：等待 w25q32 不忙
* 参数：null
* 返回值：null
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void w25q32_flash_waitfor_write_end(u8 select)
{
	while((w25q32_flash_readSR(select) & 0x01) == 0x01); // 等待BUSY位清空
}
/*******************************************************************
********************************************************************
* 名称：void w25q32_flash_powerdown(void)  
* 功能：进入掉电模式
* 参数：null
* 返回值：null
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void w25q32_flash_powerdown(u8 select)   
{ 
	if(select == 1)
	{
		W25Q32_CS_PC1 = 1;   // 禁用
		W25Q32_CS_PA4 = 0;   // 启用  //使能器件 
	}
    else
    {
		W25Q32_CS_PC1 = 0;   // 禁用
		W25Q32_CS_PA4 = 1;   // 启用  //使能器件
    }	//使能器件   
    spi1_read_write_byte(W25X_PowerDown);        //发送掉电命令  
	if(select == 1)
	{
		W25Q32_CS_PA4 = 1;   // 禁能器件1
	}
	else
	{
		W25Q32_CS_PC1 = 1;   // 禁用器件2
	}	      
    systick_delay_us(5);                         //等待TPD  
}
/*******************************************************************
********************************************************************
* 名称：void w25q32_flash_wakeup(void)   
* 功能：唤醒
* 参数：null
* 返回值：null
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void w25q32_flash_wakeup(u8 select)   
{  
	if(select == 1)
	{
		W25Q32_CS_PC1 = 1;   // 禁用
		W25Q32_CS_PA4 = 0;   // 启用  //使能器件 
	}
    else
    {
		W25Q32_CS_PC1 = 0;   // 禁用
		W25Q32_CS_PA4 = 1;   // 启用  //使能器件
    }	//使能器件  
    spi1_read_write_byte(W25X_ReleasePowerDown);   //  send W25X_PowerDown command 0xAB 
	if(select == 1)
	{
		W25Q32_CS_PA4 = 1;   // 禁能器件1
	}
	else
	{
		W25Q32_CS_PC1 = 1;   // 禁用器件2
	}	
    systick_delay_us(5);                           //等待TRES1
}



