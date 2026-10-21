#include "max49918.h"
#include "stm32f4xx.h"

/*******************************************************************/
// max49918 的iic地址
// I2C 总线在传送数据过程中共有三种类型信号， 它们分别是：开始信号、结束信号和应答信号
// 1100 10XX 
// bit7 1:读 0：写
// u28        地址： 1100 100X 
// u28 read   地址 ：1100 1001 （0XC9）
// u28 write  地址 ：1100 1000 （0XC8）


// bit7 1:读 0：写
// u29       地址： 1100 101X 
// u29 read  地址 ：1100 1011 （0XCB）
// u29 write 地址 ：1100 1010 （0XCA）
/*******************************************************************
********************************************************************
* 名称：void  max49918_iic1_init(void)
* 功能：max49918 使用的iic1 功能引脚初始化
* 参数：无
* 返回值：无
* 版本：v1
* 作者：matt
* 日期：2024-06-14
* 举例：
* 备注：
* 
* IIC1   : PB6  SCL
           PB7  SDA 
*********************************************************************
*********************************************************************/
void  max49918_iic1_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    I2C_InitTypeDef   I2C_InitStructure;
    
    // 使能GPIOB和I2C1时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    
    // 配置GPIOB的引脚为复用模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // 配置引脚复用映射
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);  // SCL  PB6
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1);  // SDA  PB7
    
    // 配置I2C1
    I2C_DeInit(I2C1);                            // 重置iic1
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;   // 标准iic模式
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2; // 占空比控制
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;  // 设置主设备地址，一般情况下随意设置即可
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;  // 使能应答
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;  // 7bit寻址模式
    I2C_InitStructure.I2C_ClockSpeed = 400000;  // 设置通信速度，这里为400kHz
    I2C_Init(I2C1, &I2C_InitStructure);   // 初始化iic1

    I2C_Cmd(I2C1,ENABLE);

}

/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2024-06-14
* 举例：
* 备注：
* 
* IIC3 PA8 :SCL
       PC9 :SDA
*********************************************************************
*********************************************************************/
void max499118_iic3_init()
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    I2C_InitTypeDef   I2C_InitStructure;
    
    // 使能,GPIOA,GPIOC和I2C3时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C3, ENABLE);
    
    // 配置GPIOA的引脚为复用模式 SCL  PA8
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

     // 配置GPIOC的引脚为复用模式 SDA  PC9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    
    // 配置引脚复用映射
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_I2C3);  // SCL
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_I2C3);  // SDA
    
    // 配置I2C3
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;  // 设置自身设备地址
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = 100000;  // 设置通信速度，这里为100kHz
    
    I2C_Init(I2C3, &I2C_InitStructure);
    I2C_Cmd(I2C3,ENABLE);

}
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2024-06-14
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/

static void I2CX_Start(I2C_TypeDef* I2CX)
{
    while(I2C_GetFlagStatus(I2CX,I2C_FLAG_BUSY)); // 检查iic总线是否繁忙，等待空闲
    I2C_GenerateSTART(I2CX, ENABLE);              // 产生起始信号
    while (!I2C_CheckEvent(I2CX, I2C_EVENT_MASTER_MODE_SELECT)); // 检查EV5事件
}

/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2024-06-14
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/

static void I2CX_Stop(I2C_TypeDef* I2CX)
{
    I2C_GenerateSTOP(I2CX, ENABLE);  //发送停止信号
    
}
/*******************************************************************
********************************************************************
* 名称：void IIC_WriteByte(I2C_TypeDef* i2cx,uint16_t slave_addr,uint8_t write_addr,uint8_t pbuffer)
* 功能：向指定从机的指定地址写入8bit数据
* 参数：
        i2cx       ：iic号
        slave_addr ：从机地址
        write_addr ：从机的内存单元地址
        pbuffer    ：要写入的数据内容
* 返回值：无
* 版本：v1
* 作者：matt
* 日期：2024-06-14
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void IIC_WriteByte(I2C_TypeDef* i2cx,uint16_t slave_addr,uint8_t write_addr,uint8_t pbuffer)
{
    I2CX_Start(i2cx); // 开始信号

    I2C_Send7bitAddress(i2cx,slave_addr,I2C_Direction_Transmitter);// 发送设备地址 写模式
    while(I2C_CheckEvent(i2cx,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) == ERROR); // 检测ev6事件

    I2C_SendData(i2cx,write_addr); // 发送要写入的内存单元地址
    while(I2C_CheckEvent(i2cx,I2C_EVENT_MASTER_BYTE_TRANSMITTING) == ERROR); // 检测ev8事件

    I2C_SendData(i2cx,pbuffer); // 发送要写入的内容
    while(I2C_CheckEvent(i2cx,I2C_EVENT_MASTER_BYTE_TRANSMITTING) == ERROR); // 检测ev8.2事件

    I2CX_Stop(i2cx);  // 结束信号

} 

/*******************************************************************
********************************************************************
* 名称：uint8_t IIC_ReadByte(I2C_TypeDef* i2cx,uint16_t slave_addr,uint8_t read_addr)
* 功能：从指定地址读取8bit数据
* 参数：
        i2cx ： iic 号
        slave_addr ：从机地址
        read_addr ：从机的寄存器地址
* 返回值：读取到的8bit数据
* 版本：v1
* 作者：matt
* 日期：2024-06-14
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
uint8_t IIC_ReadByte(I2C_TypeDef* i2cx,uint16_t slave_addr,uint8_t read_addr)
{
    uint8_t read_data ; // 用于保存读取出来的8bit数据
    I2CX_Start(i2cx); // 开始信号

    I2C_Send7bitAddress(i2cx,slave_addr,I2C_Direction_Transmitter);// 发送设备地址 写模式
    while(I2C_CheckEvent(i2cx,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) == ERROR); // 检测ev6事件

    I2C_SendData(i2cx,read_addr); // 发送要读取的内存单元地址
    while(I2C_CheckEvent(i2cx,I2C_EVENT_MASTER_BYTE_TRANSMITTING) == ERROR); // 检测ev8事件

    I2C_GenerateSTART(i2cx,ENABLE);
    while(I2C_CheckEvent(i2cx,I2C_EVENT_MASTER_MODE_SELECT) == ERROR);

    I2C_Send7bitAddress(i2cx,slave_addr+1,I2C_Direction_Receiver);
    while(I2C_CheckEvent(i2cx,I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) == ERROR);

    I2C_AcknowledgeConfig(i2cx,DISABLE); // 发送非应答信号
    I2CX_Stop(i2cx);  // 结束信号
    while(I2C_CheckEvent(i2cx,I2C_EVENT_MASTER_BYTE_RECEIVED) == ERROR); // 检测ev7事件
    read_data = I2C_ReceiveData(i2cx);    // 读数据
    I2C_AcknowledgeConfig(i2cx,ENABLE);   // 开启应答信号
    return read_data ;
}
/*******************************************************************
********************************************************************
* 名称：void max49918_set_gain(I2C_TypeDef* i2cx,uint16_t ic_select,u8 gain_value)
* 功能：设置max49918的增益倍数
* 参数：
        i2cx ：iic 号
        ic_select_addr ：需要要设置的max49918的写地址
        gain_value :要设置的增益值（通过宏定义设置）

* 返回值：
* 版本：v1
* 作者：matt
* 日期：2024-06-14
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void max49918_set_gain(I2C_TypeDef* i2cx,uint16_t ic_select_addr,u8 gain_value)
{

    IIC_WriteByte(i2cx,ic_select_addr,0x00,gain_value); 
}

/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2024-06-14
* 举例：
* 备注：
* Vsensor = vout/gain
* I = Vsensor/0.1R
I = (vout/gain) / 0.1R
*********************************************************************
*********************************************************************/







