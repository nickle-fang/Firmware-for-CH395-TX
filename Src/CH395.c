/********************************** (C) COPYRIGHT *******************************
* File Name          : CH395CMD.C
* Author             : WCH
* Version            : V1.1
* Date               : 2014/8/1
* Description        : CH395芯片命令接口文件
*                      
*******************************************************************************/

/* 头文件包含*/
#include "CH395.h"
#include "gpio.h"
#include "string.h"
#include "NRF24L01.h"
#include "oled.h"

/*************************************************************************************************************************/
/* CH395相关定义 */                                                                                                       //
uint8_t CH395MACAddr[6] = {0x0A,0x03,0x04,0x05,0x06,0x07};   /* CH395MAC地址 */                                           //        
uint8_t CH395IPAddr[4] = {10,12,225,130};                       /* CH395IP地址 */                                         //          
uint8_t CH395GWIPAddr[4] = {10,12,225,1};                      /* CH395网关 */                                            //       
uint8_t CH395IPMask[4] = {255,255,255,0};                      /* CH395子网掩码 */                                        //             

uint8_t IPShow[] = "Local:10.12.225.130";
uint8_t RemoteIpShow[20] = "User:NULL";
uint8_t RemoteIpLen;
                                                                                                                          //              
/* socket 相关定义*/                                                                                                      //
uint8_t  Socket0DesIP[4] = {0xff,0xff,0xff,0xff};              /* Socket 0目的IP地址 */                             //                                
uint16_t Socket0SourPort = 1030;                               /* Socket 0源端口 */                                       //                             
uint16_t Socket0DesPort   = 1030;                              /* Socket 0目的端口 */                                     //      

uint8_t  Socket1DesIP[4] = {0xff,0xff,0xff,0xff};              /* Socket 1目的IP地址 */                             //                                
uint16_t Socket1SourPort = 1040;                               /* Socket 1源端口 */                                       //                             
uint16_t Socket1DesPort   = 1030;                              /* Socket 1目的端口 */ 

uint8_t destAddr[4];
uint8_t package_success = 0;
/***************************************************************************************************************************/

extern UART_HandleTypeDef huart3;
extern struct _SOCK_INF  SockInf[4];                                   /* 保存Socket信息 */
uint8_t MyBuffer[2][MAX_CACHE];

/* 2401相关数据 */
extern uint8_t TX_frequency;	//24L01频率初始化为5a
extern uint8_t RX_frequency;	//24L01频率初始化为5a
extern uint8_t bandwidth;  //带宽初始化为0x06

/********************************************************************************
* Function Name  : CH395CMDReset
* Description    : 复位CH395芯片
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH395CMDReset(void)
{
    xWriteCH395Cmd(CMD00_RESET_ALL);
    xEndCH395Cmd();
}

/*******************************************************************************
* Function Name  : CH395CMDSleep
* Description    : 使CH395进入睡眠状态
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH395CMDSleep(void)
{
    xWriteCH395Cmd(CMD00_ENTER_SLEEP);
    xEndCH395Cmd();
}

/********************************************************************************
* Function Name  : CH395CMDSleep
* Description    : 获取芯片以及固件版本号，1字节，高四位表示芯片版本，
                   低四位表示固件版本
* Input          : None
* Output         : None
* Return         : 1字节芯片及固件版本号
*******************************************************************************/
uint8_t CH395CMDGetVer(void)
{
    uint8_t i;
    xWriteCH395Cmd(CMD01_GET_IC_VER);
    i = xReadCH395Data();
    xEndCH395Cmd();
    return i;
}

/********************************************************************************
* Function Name  : CH395CMDCheckExist
* Description    : 测试命令，用于测试硬件以及接口通讯
* Input          : testdata 1字节测试数据
* Output         : None
* Return         : 硬件OK，返回 testdata按位取反
*******************************************************************************/
uint8_t CH395CMDCheckExist(uint8_t testdata)
{
    uint8_t i;

    xWriteCH395Cmd(CMD11_CHECK_EXIST);
    xWriteCH395Data(testdata);
    i = xReadCH395Data();
    xEndCH395Cmd();
    return i;
}

/********************************************************************************
* Function Name  : CH395CMDSetPHY
* Description    : 设置PHY，主要设置CH395 PHY为100/10M 或者全双工半双工，CH395默
                    为自动协商。
* Input          : phystat 参考PHY 命令参数/状态
* Output         : None
* Return         : None
*******************************************************************************/
void CH395CMDSetPHY(uint8_t phystat)
{
    xWriteCH395Cmd(CMD10_SET_PHY);
    xWriteCH395Data(phystat);
    xEndCH395Cmd();
}

/*******************************************************************************
* Function Name  : CH395CMDGetPHYStatus
* Description    : 获取PHY的状态
* Input          : None
* Output         : None
* Return         : 当前CH395PHY状态，参考PHY参数/状态定义
*******************************************************************************/
uint8_t CH395CMDGetPHYStatus(void)
{
    uint8_t i;

    xWriteCH395Cmd(CMD01_GET_PHY_STATUS);
    i = xReadCH395Data();
    xEndCH395Cmd();
    return i;
}

/*******************************************************************************
* Function Name  : CH395CMDGetGlobIntStatus
* Description    : 获取全局中断状态，收到此命令CH395自动取消中断，0x43及以下版本使用
* Input          : None
* Output         : None
* Return         : 返回当前的全局中断状态
*******************************************************************************/
uint8_t CH395CMDGetGlobIntStatus(void)
{
    uint8_t init_status;

    xWriteCH395Cmd(CMD01_GET_GLOB_INT_STATUS);
    init_status = xReadCH395Data();
    xEndCH395Cmd();
    return  init_status;
}

/********************************************************************************
* Function Name  : CH395CMDInitCH395
* Description    : 初始化CH395芯片。
* Input          : None
* Output         : None
* Return         : 返回执行结果
*******************************************************************************/
uint8_t CH395CMDInitCH395(void)
{
    uint8_t i = 0;
    uint8_t s = 0;

    xWriteCH395Cmd(CMD0W_INIT_CH395);
    xEndCH395Cmd();
    while(1)
    {
        HAL_Delay(5);                                                 /* 延时查询，建议2MS以上*/
        s = CH395GetCmdStatus();                                     /* 不能过于频繁查询*/
        if(s !=CH395_ERR_BUSY)break;                                 /* 如果CH395芯片返回忙状态*/
        if(i++ > 200)return CH395_ERR_UNKNOW;                        /* 超时退出,本函数需要500MS以上执行完毕 */
    }
    return s;
}

/********************************************************************************
* Function Name  : CH395CMDSetUartBaudRate
* Description    : 设置CH395串口波特率，仅在串口模式下有效
* Input          : baudrate 串口波特率
* Output         : None
* Return         : None
*******************************************************************************/
void CH395CMDSetUartBaudRate(uint32_t baudrate)
{
    xWriteCH395Cmd(CMD31_SET_BAUDRATE);
    xWriteCH395Data((uint8_t)baudrate);
    xWriteCH395Data((uint8_t)((uint16_t)baudrate >> 8));
    xWriteCH395Data((uint8_t)(baudrate >> 16));
    xEndCH395Cmd();
}

/*******************************************************************************
* Function Name  : CH395GetCmdStatus
* Description    : 获取命令执行状态，某些命令需要等待命令执行结果
* Input          : None
* Output         : None
* Return         : 返回上一条命令执行状态
*******************************************************************************/
uint8_t CH395GetCmdStatus(void)
{
    uint8_t i;

    xWriteCH395Cmd(CMD01_GET_CMD_STATUS);
    i = xReadCH395Data();
    xEndCH395Cmd();
    return i;
}

/********************************************************************************
* Function Name  : CH395CMDSetIPAddr
* Description    : 设置CH395的IP地址
* Input          : ipaddr 指IP地址
* Output         : None
* Return         : None
*******************************************************************************/
void CH395CMDSetIPAddr(uint8_t *ipaddr)
{
    uint8_t i;

    xWriteCH395Cmd(CMD40_SET_IP_ADDR);
    for(i = 0; i < 4;i++)xWriteCH395Data(*ipaddr++);
    xEndCH395Cmd();
}

/********************************************************************************
* Function Name  : CH395CMDSetGWIPAddr
* Description    : 设置CH395的网关IP地址
* Input          : ipaddr 指向网关IP地址
* Output         : None
* Return         : None
*******************************************************************************/
void CH395CMDSetGWIPAddr(uint8_t *gwipaddr)
{
    uint8_t i;

    xWriteCH395Cmd(CMD40_SET_GWIP_ADDR);
    for(i = 0; i < 4;i++)xWriteCH395Data(*gwipaddr++);
    xEndCH395Cmd();
}

/********************************************************************************
* Function Name  : CH395CMDSetMASKAddr
* Description    : 设置CH395的子网掩码，默认为255.255.255.0
* Input          : maskaddr 指子网掩码地址
* Output         : None
* Return         : None
*******************************************************************************/
void CH395CMDSetMASKAddr(uint8_t *maskaddr)
{
    uint8_t i;

    xWriteCH395Cmd(CMD40_SET_MASK_ADDR);
    for(i = 0; i < 4;i++)xWriteCH395Data(*maskaddr++);
    xEndCH395Cmd();
}

/********************************************************************************
* Function Name  : CH395CMDSetMACAddr
* Description    : 设置CH395的MAC地址。
* Input          : amcaddr MAC地址指针
* Output         : None
* Return         : None
*******************************************************************************/
void CH395CMDSetMACAddr(uint8_t *amcaddr)
{
    uint8_t i;

    xWriteCH395Cmd(CMD60_SET_MAC_ADDR);
    for(i = 0; i < 6;i++)xWriteCH395Data(*amcaddr++);
    xEndCH395Cmd();
    HAL_Delay(100); 
}

/********************************************************************************
* Function Name  : CH395CMDGetMACAddr
* Description    : 获取CH395的MAC地址。
* Input          : amcaddr MAC地址指针
* Output         : None
* Return         : None
*******************************************************************************/
void CH395CMDGetMACAddr(uint8_t *amcaddr)
{
    uint8_t i;

    xWriteCH395Cmd(CMD06_GET_MAC_ADDR);
    for(i = 0; i < 6;i++)*amcaddr++ = xReadCH395Data();
    xEndCH395Cmd();
 }

/*******************************************************************************
* Function Name  : CH395CMDSetMACFilt
* Description    : 设置MAC过滤。
* Input          : filtype 参考 MAC过滤
                   table0 Hash0
                   table1 Hash1
* Output         : None
* Return         : None
*******************************************************************************/
void CH395CMDSetMACFilt(uint8_t filtype,uint32_t table0,uint32_t table1)
{
    xWriteCH395Cmd(CMD90_SET_MAC_FILT);
    xWriteCH395Data(filtype);
    xWriteCH395Data((uint8_t)table0);
    xWriteCH395Data((uint8_t)((uint16_t)table0 >> 8));
    xWriteCH395Data((uint8_t)(table0 >> 16));
    xWriteCH395Data((uint8_t)(table0 >> 24));

    xWriteCH395Data((uint8_t)table1);
    xWriteCH395Data((uint8_t)((uint16_t)table1 >> 8));
    xWriteCH395Data((uint8_t)(table1 >> 16));
    xWriteCH395Data((uint8_t)(table1 >> 24));
    xEndCH395Cmd();
}

/********************************************************************************
* Function Name  : CH395CMDGetUnreachIPPT
* Description    : 获取不可达信息 (IP,Port,Protocol Type)
* Input          : list 保存获取到的不可达
                        第1个字节为不可达代码，请参考 不可达代码(CH395INC.H)
                        第2个字节为IP包协议类型
                        第3-4字节为端口号
                        第4-8字节为IP地址
* Output         : None
* Return         : None
*******************************************************************************/
void CH395CMDGetUnreachIPPT(uint8_t *list)
{
    uint8_t i;

    xWriteCH395Cmd(CMD08_GET_UNREACH_IPPORT);
    for(i = 0; i < 8; i++)
    {
        *list++ = xReadCH395Data();
    }   
    xEndCH395Cmd();
}

/********************************************************************************
* Function Name  : CH395CMDGetRemoteIPP
* Description    : 获取远端的IP和端口地址，一般在TCP Server模式下使用
* Input          : sockindex Socket索引
                   list 保存IP和端口
* Output         : None
* Return         : None
*******************************************************************************/
void CH395CMDGetRemoteIPP(uint8_t sockindex,uint8_t *list)
{
    uint8_t i;

    xWriteCH395Cmd(CMD06_GET_REMOT_IPP_SN);
    xWriteCH395Data(sockindex);
    for(i = 0; i < 6; i++)
    {
        *list++ = xReadCH395Data();
    }   
    xEndCH395Cmd();
}

/*******************************************************************************
* Function Name  : CH395SetSocketDesIP
* Description    : 设置socket n的目的IP地址
* Input          : sockindex Socket索引
                   ipaddr 指向IP地址
* Output         : None
* Return         : None
*******************************************************************************/
void CH395SetSocketDesIP(uint8_t sockindex,uint8_t *ipaddr)
{
    xWriteCH395Cmd(CMD50_SET_IP_ADDR_SN);
    xWriteCH395Data(sockindex);
    xWriteCH395Data(*ipaddr++);
    xWriteCH395Data(*ipaddr++);
    xWriteCH395Data(*ipaddr++);
    xWriteCH395Data(*ipaddr++);
    xEndCH395Cmd();
}

/*******************************************************************************
* Function Name  : CH395SetSocketProtType
* Description    : 设置socket 的协议类型
* Input          : sockindex Socket索引
                   prottype 协议类型，请参考 socket协议类型定义(CH395INC.H)
* Output         : None
* Return         : None
*******************************************************************************/
void CH395SetSocketProtType(uint8_t sockindex,uint8_t prottype)
{
    xWriteCH395Cmd(CMD20_SET_PROTO_TYPE_SN);
    xWriteCH395Data(sockindex);
    xWriteCH395Data(prottype);
    xEndCH395Cmd();
}

/*******************************************************************************

* Function Name  : CH395SetSocketDesPort
* Description    : 设置socket n的协议类型
* Input          : sockindex Socket索引
                   desprot 2字节目的端口
* Output         : None
* Return         : None
*******************************************************************************/
void CH395SetSocketDesPort(uint8_t sockindex,uint16_t desprot)
{
    xWriteCH395Cmd(CMD30_SET_DES_PORT_SN);
    xWriteCH395Data(sockindex);
    xWriteCH395Data((uint8_t)desprot);
    xWriteCH395Data((uint8_t)(desprot >> 8));
    xEndCH395Cmd();
}

/*******************************************************************************
* Function Name  : CH395SetSocketSourPort
* Description    : 设置socket n的协议类型
* Input          : sockindex Socket索引
                   desprot 2字节源端口
* Output         : None
* Return         : None
*******************************************************************************/
void CH395SetSocketSourPort(uint8_t sockindex,uint16_t surprot)
{
    xWriteCH395Cmd(CMD30_SET_SOUR_PORT_SN);
    xWriteCH395Data(sockindex);
    xWriteCH395Data((uint8_t)surprot);
    xWriteCH395Data((uint8_t)(surprot>>8));
    xEndCH395Cmd();
}

/******************************************************************************
* Function Name  : CH395SetSocketIPRAWProto
* Description    : IP模式下，socket IP包协议字段
* Input          : sockindex Socket索引
                   prototype IPRAW模式1字节协议字段
* Output         : None
* Return         : None
*******************************************************************************/
void CH395SetSocketIPRAWProto(uint8_t sockindex,uint8_t prototype)
{
    xWriteCH395Cmd(CMD20_SET_IPRAW_PRO_SN);
    xWriteCH395Data(sockindex);
    xWriteCH395Data(prototype);
    xEndCH395Cmd();
}

/********************************************************************************
* Function Name  : CH395EnablePing
* Description    : 开启/关闭 PING
* Input          : enable : 1  开启PING
                          ：0  关闭PING
* Output         : None
* Return         : None
*******************************************************************************/
void CH395EnablePing(uint8_t enable)
{
    xWriteCH395Cmd(CMD01_PING_ENABLE);
    xWriteCH395Data(enable);
    xEndCH395Cmd();
}


/********************************************************************************
* Function Name  : CH395SendData
* Description    : 向发送缓冲区写数据
* Input          : sockindex Socket索引
                   databuf  数据缓冲区
                   len   长度
* Output         : None
* Return         : None
*******************************************************************************/
void CH395SendData(uint8_t sockindex,uint8_t *databuf,uint16_t len)
{
    uint16_t i;

    xWriteCH395Cmd(CMD30_WRITE_SEND_BUF_SN);
    xWriteCH395Data((uint8_t)sockindex);
    xWriteCH395Data((uint8_t)len);
    xWriteCH395Data((uint8_t)(len>>8));
   
    for(i = 0; i < len; i++)
    {
        xWriteCH395Data(*databuf++);
    }
    xEndCH395Cmd();
}

/*******************************************************************************
* Function Name  : CH395GetRecvLength
* Description    : 获取接收缓冲区长度
* Input          : sockindex Socket索引
* Output         : None
* Return         : 返回接收缓冲区有效长度
*******************************************************************************/
uint16_t CH395GetRecvLength(uint8_t sockindex)
{
    uint16_t i;

    xWriteCH395Cmd(CMD12_GET_RECV_LEN_SN);
    xWriteCH395Data((uint8_t)sockindex);
    i = xReadCH395Data();
    i = (uint16_t)(xReadCH395Data()<<8) + i;
    xEndCH395Cmd();
    return i;
}

/*******************************************************************************
* Function Name  : CH395ClearRecvBuf
* Description    : 清除接收缓冲区
* Input          : sockindex Socket索引
* Output         : None
* Return         : None
*******************************************************************************/
void CH395ClearRecvBuf(uint8_t sockindex)
{
    xWriteCH395Cmd(CMD10_CLEAR_RECV_BUF_SN);
    xWriteCH395Data((uint8_t)sockindex);
    xEndCH395Cmd();
}

/********************************************************************************
* Function Name  : CH395GetRecvLength
* Description    : 读取接收缓冲区数据
* Input          : sockindex Socket索引
                   len   长度
                   pbuf  缓冲区
* Output         : None
* Return         : None
*******************************************************************************/
void CH395GetRecvData(uint8_t sockindex,uint16_t len,uint8_t *pbuf)
{
    uint16_t i;
    if(!len)return;
    xWriteCH395Cmd(CMD30_READ_RECV_BUF_SN);
    xWriteCH395Data(sockindex);
    xWriteCH395Data((uint8_t)len);
    xWriteCH395Data((uint8_t)(len>>8));
    delay_us(1);
    for(i = 0; i < len; i++)
    {
       *pbuf = xReadCH395Data();
       pbuf++;
    }   
    xEndCH395Cmd();
}

/********************************************************************************
* Function Name  : CH395CMDSetRetryCount
* Description    : 设置重试次数
* Input          : count 重试值，最大为20次
* Output         : None
* Return         : None
********************************************************************************/
void CH395CMDSetRetryCount(uint8_t count)
{
    xWriteCH395Cmd(CMD10_SET_RETRAN_COUNT);
    xWriteCH395Data(count);
    xEndCH395Cmd();
}

/********************************************************************************
* Function Name  : CH395CMDSetRetryPeriod
* Description    : 设置重试周期
* Input          : period 重试周期单位为毫秒，最大1000ms
* Output         : None
* Return         : None
*******************************************************************************/
void CH395CMDSetRetryPeriod(uint16_t period)
{
    xWriteCH395Cmd(CMD10_SET_RETRAN_COUNT);
    xWriteCH395Data((uint8_t)period);
    xWriteCH395Data((uint8_t)(period>>8));
    xEndCH395Cmd();
}

/********************************************************************************
* Function Name  : CH395CMDGetSocketStatus
* Description    : 获取socket
* Input          : None
* Output         : socket n的状态信息，第1字节为socket 打开或者关闭
                   第2字节为TCP状态
* Return         : None
*******************************************************************************/
void CH395CMDGetSocketStatus(uint8_t sockindex,uint8_t *status)
{
    xWriteCH395Cmd(CMD12_GET_SOCKET_STATUS_SN);
    xWriteCH395Data(sockindex);
    *status++ = xReadCH395Data();
    *status++ = xReadCH395Data();
    xEndCH395Cmd();
}

/*******************************************************************************
* Function Name  : CH395OpenSocket
* Description    : 打开socket，此命令需要等待执行成功
* Input          : sockindex Socket索引
* Output         : None
* Return         : 返回执行结果
*******************************************************************************/
uint8_t  CH395OpenSocket(uint8_t sockindex)
{
    uint8_t i = 0;
    uint8_t s = 0;
    xWriteCH395Cmd(CMD1W_OPEN_SOCKET_SN);
    xWriteCH395Data(sockindex);
    xEndCH395Cmd();
    while(1)
    {
        HAL_Delay(5);                                                 /* 延时查询，建议2MS以上*/
        s = CH395GetCmdStatus();                                     /* 不能过于频繁查询*/
        if(s !=CH395_ERR_BUSY)break;                                 /* 如果CH395芯片返回忙状态*/
        if(i++ > 200)return CH395_ERR_UNKNOW;                        /* 超时退出*/
    }
    return s;
}

/*******************************************************************************
* Function Name  : CH395OpenSocket
* Description    : 关闭socket，
* Input          : sockindex Socket索引
* Output         : None
* Return         : 返回执行结果
*******************************************************************************/
uint8_t  CH395CloseSocket(uint8_t sockindex)
{
    uint8_t i = 0;
    uint8_t s = 0;
    xWriteCH395Cmd(CMD1W_CLOSE_SOCKET_SN);
    xWriteCH395Data(sockindex);
    xEndCH395Cmd();
    while(1)
    {
        HAL_Delay(5);                                                 /* 延时查询，建议2MS以上*/
        s = CH395GetCmdStatus();                                     /* 不能过于频繁查询*/
        if(s !=CH395_ERR_BUSY)break;                                 /* 如果CH395芯片返回忙状态*/
        if(i++ > 200)return CH395_ERR_UNKNOW;                        /* 超时退出*/
    }
    return s;
}

/********************************************************************************
* Function Name  : CH395TCPConnect
* Description    : TCP连接，仅在TCP模式下有效，此命令需要等待执行成功
* Input          : sockindex Socket索引
* Output         : None
* Return         : 返回执行结果
*******************************************************************************/
uint8_t CH395TCPConnect(uint8_t sockindex)
{
    uint8_t i = 0;
    uint8_t s = 0;
    xWriteCH395Cmd(CMD1W_TCP_CONNECT_SN);
    xWriteCH395Data(sockindex);
    xEndCH395Cmd();
    while(1)
    {
        HAL_Delay(5);                                                 /* 延时查询，建议2MS以上*/
        s = CH395GetCmdStatus();                                     /* 不能过于频繁查询*/
        if(s !=CH395_ERR_BUSY)break;                                 /* 如果CH395芯片返回忙状态*/
        if(i++ > 200)return CH395_ERR_UNKNOW;                        /* 超时退出*/
    }
    return s;
}

/******************************************************************************
* Function Name  : CH395TCPListen
* Description    : TCP监听，仅在TCP模式下有效，此命令需要等待执行成功
* Input          : sockindex Socket索引
* Output         : None
* Return         : 返回执行结果
*******************************************************************************/
uint8_t CH395TCPListen(uint8_t sockindex)
{
    uint8_t i = 0;
    uint8_t s = 0;
    xWriteCH395Cmd(CMD1W_TCP_LISTEN_SN);
    xWriteCH395Data(sockindex);
    xEndCH395Cmd();
    while(1)
    {
        HAL_Delay(5);                                                 /* 延时查询，建议2MS以上*/
        s = CH395GetCmdStatus();                                     /* 不能过于频繁查询*/
        if(s !=CH395_ERR_BUSY)break;                                 /* 如果CH395芯片返回忙状态*/
        if(i++ > 200)return CH395_ERR_UNKNOW;                        /* 超时退出*/
    }
    return s;
}

/********************************************************************************
* Function Name  : CH395TCPDisconnect
* Description    : TCP断开，仅在TCP模式下有效，此命令需要等待执行成功
* Input          : sockindex Socket索引
* Output         : None
* Return         : None
*******************************************************************************/
uint8_t CH395TCPDisconnect(uint8_t sockindex)
{
    uint8_t i = 0;
    uint8_t s = 0;
    xWriteCH395Cmd(CMD1W_TCP_DISNCONNECT_SN);
    xWriteCH395Data(sockindex);
    xEndCH395Cmd();
    while(1)
    {
        HAL_Delay(5);                                                 /* 延时查询，建议2MS以上*/
        s = CH395GetCmdStatus();                                     /* 不能过于频繁查询*/
        if(s !=CH395_ERR_BUSY)break;                                 /* 如果CH395芯片返回忙状态*/
        if(i++ > 200)return CH395_ERR_UNKNOW;                        /* 超时退出*/
    }
    return s;
}

/*******************************************************************************
* Function Name  : CH395GetSocketInt
* Description    : 获取socket n的中断状态
* Input          : sockindex   socket索引
* Output         : None
* Return         : 中断状态
*******************************************************************************/
uint8_t CH395GetSocketInt(uint8_t sockindex)
{
    uint8_t intstatus;
    xWriteCH395Cmd(CMD11_GET_INT_STATUS_SN);
    xWriteCH395Data(sockindex);
    delay_us(2);
    intstatus = xReadCH395Data();
    xEndCH395Cmd();
    return intstatus;
}

/*******************************************************************************
* Function Name  : CH395CRCRet6Bit
* Description    : 对多播地址进行CRC运算，并取高6位。
* Input          : mac_addr   MAC地址
* Output         : None
* Return         : 返回CRC32的高6位
*******************************************************************************/
uint8_t CH395CRCRet6Bit(uint8_t *mac_addr)
{
    INT32 perByte;
    INT32 perBit;
    const uint32_t poly = 0x04C11DB7;
    uint32_t crc_value = 0xFFFFFFFF;
    uint8_t c;
    for ( perByte = 0; perByte < 6; perByte ++ ) 
    {
        c = *(mac_addr++);
        for ( perBit = 0; perBit < 8; perBit++ ) 
        {
            crc_value = (crc_value<<1)^((((crc_value>>31)^c)&0x01)?poly:0);
            c >>= 1;
        }
    }
    crc_value=crc_value>>26;                                      
    return ((uint8_t)crc_value);
}

/******************************************************************************
* Function Name  : CH395DHCPEnable
* Description    : 启动/停止DHCP
* Input          : flag   1:启动DHCP;0：停止DHCP
* Output         : None
* Return         : 执行状态
*******************************************************************************/
uint8_t  CH395DHCPEnable(uint8_t flag)
{
    uint8_t i = 0;
    uint8_t s;
    xWriteCH395Cmd(CMD10_DHCP_ENABLE);
    xWriteCH395Data(flag);
    xEndCH395Cmd();
    while(1)
    {
        HAL_Delay(20);
        s = CH395GetCmdStatus();                                     /* 不能过于频繁查询*/
        if(s !=CH395_ERR_BUSY)break;                                 /* 如果CH395芯片返回忙状态*/
        if(i++ > 200)return CH395_ERR_UNKNOW;                        /* 超时退出*/
    }
    return s;
}

/******************************************************************************
* Function Name  : CH395GetDHCPStatus
* Description    : 获取DHCP状态
* Input          : None
* Output         : None
* Return         : DHCP状态，0为成功，其他值表示错误
*******************************************************************************/
uint8_t CH395GetDHCPStatus(void)
{
    uint8_t status;
    xWriteCH395Cmd(CMD01_GET_DHCP_STATUS);
    status = xReadCH395Data();
    xEndCH395Cmd();
    return status;
}

/*******************************************************************************
* Function Name  : CH395GetIPInf
* Description    : 获取IP，子网掩码和网关地址
* Input          : None
* Output         : 12个字节的IP,子网掩码和网关地址
* Return         : None
*******************************************************************************/
void CH395GetIPInf(uint8_t *addr)
{
    uint8_t i;
    xWriteCH395Cmd(CMD014_GET_IP_INF);
    for(i = 0; i < 20; i++)
    {
     *addr++ = xReadCH395Data();
    }
    xEndCH395Cmd();
}

/*******************************************************************************
* Function Name  : CH395WriteGPIOAddr
* Description    : 写GPIO寄存器
* Input          : regadd   寄存器地址
*                ：regval   寄存器值
* Output         : None
* Return         : None
*******************************************************************************/
void CH395WriteGPIOAddr(uint8_t regadd,uint8_t regval)
{
    xWriteCH395Cmd(CMD20_WRITE_GPIO_REG);
    xWriteCH395Data(regadd);
    xWriteCH395Data(regval);
}

/*******************************************************************************
* Function Name  : CH395ReadGPIOAddr
* Description    : 读GPIO寄存器
* Input          : regadd   寄存器地址
* Output         : None
* Return         : 寄存器的值
*******************************************************************************/
uint8_t CH395ReadGPIOAddr(uint8_t regadd)
{
    uint8_t i;
    xWriteCH395Cmd(CMD10_READ_GPIO_REG);
    xWriteCH395Data(regadd);
    HAL_Delay(1);
    i = xReadCH395Data();
    return i;
}

/*******************************************************************************
* Function Name  : CH395EEPROMErase
* Description    : 擦除EEPROM
* Input          : None
* Output         : None
* Return         : 执行状态
*******************************************************************************/
uint8_t CH395EEPROMErase(void)
{
    uint8_t i;    
    xWriteCH395Cmd(CMD00_EEPROM_ERASE);
    while(1)
    {
        HAL_Delay(20);
       i = CH395GetCmdStatus();
       if(i == CH395_ERR_BUSY)continue;
       break;
    }
    return i;
}

/*******************************************************************************
* Function Name  : CH395EEPROMWrite
* Description    : 写EEPROM
* Input          : eepaddr  EEPROM地址
*                ：buf      缓冲区地址
*                ：len      长度
* Output         : None
* Return         : 执行状态
*******************************************************************************/
uint8_t CH395EEPROMWrite(uint16_t eepaddr,uint8_t *buf,uint8_t len)
{
    uint8_t i;
    xWriteCH395Cmd(CMD30_EEPROM_WRITE);
    xWriteCH395Data((uint8_t)(eepaddr));
    xWriteCH395Data((uint8_t)(eepaddr >> 8));
    xWriteCH395Data(len);  
    while(len--)xWriteCH395Data(*buf++);
    while(1)
    {
        HAL_Delay(20);
       i = CH395GetCmdStatus();
       if(i == CH395_ERR_BUSY)continue;
       break;
    }
    return i;
}
  
/*******************************************************************************
* Function Name  : CH395EEPROMRead
* Description    : 写EEPROM
* Input          : eepaddr  EEPROM地址
*                ：buf      缓冲区地址
*                ：len      长度
* Output         : None
* Return         : None
*******************************************************************************/
void CH395EEPROMRead(uint16_t eepaddr,uint8_t *buf,uint8_t len)
{
    xWriteCH395Cmd(CMD30_EEPROM_READ);
    xWriteCH395Data((uint8_t)(eepaddr));
    xWriteCH395Data((uint8_t)(eepaddr >> 8));
    xWriteCH395Data(len);  
    HAL_Delay(1);
    while(len--)*buf++ = xReadCH395Data();
}

/*******************************************************************************
* Function Name  : CH395SetTCPMss
* Description    : 设置TCP MSS值
* Input          : tcpmss 
* Output         : None
* Return         : None
*******************************************************************************/
void CH395SetTCPMss(uint16_t tcpmss)
{
    xWriteCH395Cmd(CMD20_SET_TCP_MSS);
    xWriteCH395Data((uint8_t)(tcpmss));
    xWriteCH395Data((uint8_t)(tcpmss >> 8));
}

/*******************************************************************************
* Function Name  : CH395SetSocketRecvBuf
* Description    : 设置Socket接收缓冲区
* Input          : sockindex  socket索引
                 ：startblk   起始地址
                 ：blknum     单位缓冲区个数 ，单位为512字节
* Output         : None
* Return         : None
*******************************************************************************/
void CH395SetSocketRecvBuf(uint8_t sockindex,uint8_t startblk,uint8_t blknum)
{
    xWriteCH395Cmd(CMD30_SET_RECV_BUF);
    xWriteCH395Data(sockindex);
    xWriteCH395Data(startblk);
    xWriteCH395Data(blknum);
}

/*******************************************************************************
* Function Name  : CH395SetSocketSendBuf
* Description    : 设置Socket发送缓冲区
* Input          : sockindex  socket索引
                 ：startblk   起始地址
                 ：blknum     单位缓冲区个数
* Output         : None
* Return         : None
*******************************************************************************/
void CH395SetSocketSendBuf(uint8_t sockindex,uint8_t startblk,uint8_t blknum)
{
    xWriteCH395Cmd(CMD30_SET_SEND_BUF);
    xWriteCH395Data(sockindex);
    xWriteCH395Data(startblk);
    xWriteCH395Data(blknum);
}

/*******************************************************************************
* Function Name  : CH395UDPSendTo
* Description    : UDP向指定的IP和端口发送数据
* Input          : buf     : 发送数据缓冲区
                   len     : 发送数据长度
				   ip      : 目标IP
				   port    : 目标端口
				   sockeid : socket索引值
* Output         : None
* Return         : None
*******************************************************************************/
void CH395UDPSendTo(uint8_t *buf,uint32_t len,uint8_t *ip,uint16_t port,uint8_t sockindex)
{
    CH395SetSocketDesIP(sockindex,ip);                            /* 设置socket 0目标IP地址 */         
    CH395SetSocketDesPort(sockindex,port);
    CH395SendData(sockindex,buf,len);    
}

/*******************************************************************************
* Function Name  : CH395SetStartPara
* Description    : 设置CH395启动参数
* Input          : mdata
* Output         : None
* Return         : None
*******************************************************************************/
void CH395SetStartPara(uint32_t mdata)
{
    xWriteCH395Cmd(CMD40_SET_FUN_PARA);
    xWriteCH395Data((uint8_t)mdata);
    xWriteCH395Data((uint8_t)((uint16_t)mdata>>8));
    xWriteCH395Data((uint8_t)(mdata >> 16));
    xWriteCH395Data((uint8_t)(mdata >> 24));
}

/*******************************************************************************
* Function Name  : CH395CMDGetGlobIntStatus
* Description    : 获取全局中断状态，收到此命令CH395自动取消中断,0x44及以上版本使用
* Input          : None
* Output         : None
* Return         : 返回当前的全局中断状态
*******************************************************************************/
uint16_t CH395CMDGetGlobIntStatus_ALL(void)
{
  	uint16_t init_status;
  	xWriteCH395Cmd(CMD02_GET_GLOB_INT_STATUS_ALL);
	delay_us(2);
	init_status = xReadCH395Data();
	init_status = (uint16_t)(xReadCH395Data()<<8) + init_status;
	xEndCH395Cmd();
	return 	init_status;
}
/**************************** endfile *************************************/

/*==============================================================================
* Function Name  : xWriteCH395Cmd
* Description    : 向CH395写命令
* Input          : cmd 8位命令码
* Output         : None
* Return         : None
==============================================================================*/
void xWriteCH395Cmd(uint8_t cmd)									    
{
//	* ( __IO uint8_t * ) ( FSMC_Addr_CH395_CMD ) = cmd;                   /* 向CH395写命令 */
//			delay_us(2);                                                    
 	Set_Output();											 /* 设置输出 */
	ParaOut(cmd);		 /* 向CH395的并口输出数据 */	
	HAL_GPIO_WritePin(A0_GPIO_Port, A0_Pin, GPIO_PIN_SET);		 											 /* 将A0置为高 */
	//delay_us(15);

	HAL_GPIO_WritePin(WR_GPIO_Port, WR_Pin, GPIO_PIN_RESET);		     									 /* 将WR拉低 */
	HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_SET);                              /* 将RD拉高 */
	HAL_GPIO_WritePin(PCS_GPIO_Port, PCS_Pin, GPIO_PIN_RESET);		     									 /* 将CS拉低 */
	delay_us(1);                                                                        /* 等待命令执行完成 */
	
	HAL_GPIO_WritePin(WR_GPIO_Port, WR_Pin, GPIO_PIN_SET);		     										 /* 将WR置为高 */
		HAL_GPIO_WritePin(PCS_GPIO_Port, PCS_Pin, GPIO_PIN_SET);		     										 /* 将CS置为高 */
	//delay_us(5);
		HAL_GPIO_WritePin(A0_GPIO_Port, A0_Pin, GPIO_PIN_RESET);		 										 /* 将A0拉低 */
	//Set_Input();
	//delay_us(1);
}

/*==============================================================================
* Function Name  : xWriteCH395Data
* Description    : 向CH395写8位数据
* Input          : mdata 8位数据
* Output         : None
* Return         : None
==============================================================================*/
void  xWriteCH395Data(uint8_t mdata)
{			
//	* ( __IO uint8_t * ) ( FSMC_Addr_CH395_DATA ) = mdata;														 /* 向CH395写数据 */	
//		delay_us(10);
	Set_Output();										 /* 设置输出 */
	ParaOut(mdata);		 /* 向CH395的并口输出数据 */
	HAL_GPIO_WritePin(A0_GPIO_Port, A0_Pin, GPIO_PIN_RESET);                           /* Reset A0 to data mode*/
	//delay_us(15);
	
    HAL_GPIO_WritePin(WR_GPIO_Port, WR_Pin, GPIO_PIN_RESET);		     									 /* 将WR拉低 */
	  HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_SET);                             /* 将RD拉高 */
	HAL_GPIO_WritePin(PCS_GPIO_Port, PCS_Pin, GPIO_PIN_RESET);	     									 /* 将CS拉低 */
	delay_us(1);
                                                                          /* 等待命令执行完成 */
	HAL_GPIO_WritePin(WR_GPIO_Port, WR_Pin, GPIO_PIN_SET);		     									 /* 将WR拉高 */
	 HAL_GPIO_WritePin(PCS_GPIO_Port, PCS_Pin, GPIO_PIN_SET);	     										 /* 将CS置为高 */
	//Set_Input();
	//delay_us(1);

}

uint8_t	xReadCH395Data( void )                                       /* 从CH395读数据 */
{
//	delay_us(2);
//	return ( * ( __IO uint8_t * ) ( FSMC_Addr_CH395_DATA ) );
    uint8_t i = 0;
    Set_Input(); 											 /* 数据端口设置为输入 */
	delay_us(1);
		HAL_GPIO_WritePin(A0_GPIO_Port, A0_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_RESET);		     								     /* 将RD拉低 */
	HAL_GPIO_WritePin(WR_GPIO_Port, WR_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PCS_GPIO_Port, PCS_Pin, GPIO_PIN_RESET);		     									 /* 将CS拉低 */
	delay_us(1);
		i = ParaIn();											 /* 从端口读数据 */
    HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_SET);	     										 /* 将RD置为高 */
	HAL_GPIO_WritePin(PCS_GPIO_Port, PCS_Pin, GPIO_PIN_SET);		     										 /* 将CS置为高 */
	//Set_Input(); 
	return i;
}

void ParaOut(uint8_t cmd){
//	HAL_GPIO_WritePin(D7_GPIO_Port, D7_Pin, (cmd & 0x80) >> 7);
//	HAL_GPIO_WritePin(D6_GPIO_Port, D6_Pin, (cmd & 0x40) >> 6);
//	HAL_GPIO_WritePin(D5_GPIO_Port, D5_Pin, (cmd & 0x20) >> 5);
//	HAL_GPIO_WritePin(D4_GPIO_Port, D4_Pin, (cmd & 0x10) >> 4);
//	HAL_GPIO_WritePin(D3_GPIO_Port, D3_Pin, (cmd & 0x08) >> 3);
//	HAL_GPIO_WritePin(D2_GPIO_Port, D2_Pin, (cmd & 0x04) >> 2);
//	HAL_GPIO_WritePin(D1_GPIO_Port, D1_Pin, (cmd & 0x02) >> 1);
//	HAL_GPIO_WritePin(D0_GPIO_Port, D0_Pin, (cmd & 0x01) );
	
	GPIOD->ODR = cmd;
}

uint8_t ParaIn(void){
//	uint8_t data_in = 0;
//	data_in = HAL_GPIO_ReadPin(D7_GPIO_Port, D7_Pin);
//	data_in = data_in << 1;
//	data_in += HAL_GPIO_ReadPin(D6_GPIO_Port, D6_Pin);
//	data_in = data_in << 1;
//	data_in += HAL_GPIO_ReadPin(D5_GPIO_Port, D5_Pin);
//	data_in = data_in << 1;
//	data_in += HAL_GPIO_ReadPin(D4_GPIO_Port, D4_Pin);
//	data_in = data_in << 1;
//	data_in += HAL_GPIO_ReadPin(D3_GPIO_Port, D3_Pin);
//	data_in = data_in << 1;
//	data_in += HAL_GPIO_ReadPin(D2_GPIO_Port, D2_Pin);
//	data_in = data_in << 1;
//	data_in += HAL_GPIO_ReadPin(D1_GPIO_Port, D1_Pin);
//	data_in = data_in << 1;
//	data_in += HAL_GPIO_ReadPin(D0_GPIO_Port, D0_Pin);
	
	return GPIOD->IDR;
}


void CH395_Port_Init(){
	HAL_GPIO_WritePin(RSTI_GPIO_Port, RSTI_Pin, GPIO_PIN_RESET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(RSTI_GPIO_Port, RSTI_Pin, GPIO_PIN_SET);
	HAL_Delay(100);
	
	HAL_GPIO_WritePin(WR_GPIO_Port, WR_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(PCS_GPIO_Port, PCS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(A0_GPIO_Port, A0_Pin, GPIO_PIN_RESET);
	Set_Input();
}


uint8_t CH395Init(){
	uint8_t i ;
  i = CH395CMDCheckExist(0x65);
  if(i != 0x9a)return CH395_ERR_UNKNOW;                            /* 测试命令，如果无法通过返回0XFA */
	
	CH395CMDSetMACAddr(CH395MACAddr);                             /* 设置CH395的MAC地址 */
	CH395CMDSetIPAddr(CH395IPAddr);                               /* 设置CH395的IP地址 */
  CH395CMDSetGWIPAddr(CH395GWIPAddr);                           /* 设置网关地址 */
  CH395CMDSetMASKAddr(CH395IPMask);                             /* 设置子网掩码，默认为255.255.255.0*/ 
  HAL_Delay(10);  
  i = CH395CMDInitCH395();                                /* 初始化CH395芯片 */
  
	return i;
}

/**********************************************************************************
* Function Name  : mStopIfError
* Description    : 调试使用，显示错误代码，并停机
* Input          : iError
* Output         : None
* Return         : None
**********************************************************************************/
void mStopIfError(uint8_t iError)
{
    if (iError == CMD_ERR_SUCCESS) return;                           /* 操作成功 */
    while ( 1 )
    {
      HAL_Delay(200);
			HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);  /* Error Flag LED */
    }
}


void InitSocketParam(void)
{
    memset(&SockInf[0],0,sizeof(SockInf[0]));                        /* ?SockInf[0]????*/
    memcpy(SockInf[0].IPAddr,Socket0DesIP,sizeof(Socket0DesIP));     /* ???IP???? */
    SockInf[0].DesPort = Socket0DesPort;                             /* ???? */
    SockInf[0].SourPort = Socket0SourPort;                           /* ??? */
    SockInf[0].ProtoType = PROTO_TYPE_UDP;                           /* UDP?? */
	
	memset(&SockInf[1],0,sizeof(SockInf[1]));                        /* ?SockInf[0]????*/
    memcpy(SockInf[1].IPAddr,Socket1DesIP,sizeof(Socket0DesIP));     /* ???IP???? */
    SockInf[1].DesPort = Socket1DesPort;                             /* ???? */
    SockInf[1].SourPort = Socket1SourPort;                           /* ??? */
    SockInf[1].ProtoType = PROTO_TYPE_UDP;                           /* UDP?? */

}


void CH395SocketInitOpen(void)
{
    uint8_t i;

    CH395SetSocketDesIP(0,SockInf[0].IPAddr);                        /* ??socket 0??IP?? */         
    CH395SetSocketProtType(0,PROTO_TYPE_UDP);                        /* ??socket 0???? */
    CH395SetSocketDesPort(0,SockInf[0].DesPort);                     /* ??socket 0???? */
    CH395SetSocketSourPort(0,SockInf[0].SourPort);                   /* ??socket 0??? */
    i = CH395OpenSocket(0);                                          /* ??socket 0 */
    mStopIfError(i);
	
	
	  CH395SetSocketDesIP(1,SockInf[1].IPAddr);
    CH395SetSocketProtType(1,PROTO_TYPE_UDP);                        /* ??socket 1???? */
    CH395SetSocketSourPort(1,SockInf[1].SourPort);                   /* ??socket 1??? */
	  CH395SetSocketDesPort(1,SockInf[1].DesPort);                     /* ??socket 0???? */
    i = CH395OpenSocket(1);                                          /* ??socket 1 */
    mStopIfError(i);                                                 /* ?????? */                                        /* ???? */ 
	
}


void CH395GlobalInterrupt(void)
{
   uint16_t  init_status;
   //uint8_t  buf[10]; 
 
    init_status = CH395CMDGetGlobIntStatus_ALL();
//    if(init_status & GINT_STAT_UNREACH)                              /* ?????,??????? */
//    {
//        CH395CMDGetUnreachIPPT(buf);           
//    }
//    if(init_status & GINT_STAT_IP_CONFLI)                            /* ??IP????,??????CH395? IP,????CH395*/
//    {
//			
//    }
//    if(init_status & GINT_STAT_PHY_CHANGE)                           /* ??PHY????*/
//    {
//			
//	  }
    if(init_status & GINT_STAT_SOCK0)
    {
      //HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);                  /* 测试是否收到中断-->success */
			CH395SocketInterrupt(0);                                     /* ??socket 0??*/
    }
    if(init_status & GINT_STAT_SOCK1)                               
    {
        //CH395SocketInterrupt(1);                                     /* ??socket 1??*/
    }
//    if(init_status & GINT_STAT_SOCK2)                                
//    {
//        CH395SocketInterrupt(2);                                     /* ??socket 2??*/
//    }
//    if(init_status & GINT_STAT_SOCK3)                                
//    {
//        CH395SocketInterrupt(3);                                     /* ??socket 3??*/
//    }
//    if(init_status & GINT_STAT_SOCK4)
//    {
//        CH395SocketInterrupt(4);                                     /* ??socket 4??*/
//    }
//    if(init_status & GINT_STAT_SOCK5)                                
//    {
//        CH395SocketInterrupt(5);                                     /* ??socket 5??*/
//    }
//    if(init_status & GINT_STAT_SOCK6)                                
//    {
//        CH395SocketInterrupt(6);                                     /* ??socket 6??*/
//    }
//    if(init_status & GINT_STAT_SOCK7)                                
//    {
//        CH395SocketInterrupt(7);                                     /* ??socket 7??*/
//    }
}


void CH395SocketInterrupt(uint8_t sockindex)
{
    uint8_t  sock_int_socket;
    uint8_t  i;
    uint16_t len;
    //uint16_t tmp;
    //uint8_t  buf[10];
		//uint8_t length;
	  uint8_t check;

    sock_int_socket = CH395GetSocketInt(sockindex);                  /* 获取中断类型 */
//	HAL_UART_Transmit(&huart3, &sock_int_socket, 1, 0xFF);           /* Debug */
    if(sock_int_socket & SINT_STAT_SENBUF_FREE)                      /* ???????,???????????? */
    {

    }
    else if(sock_int_socket & SINT_STAT_SEND_OK)                          /* ?????? */
    {
			//HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);////////////////////////////////////////////////////////////////////////////////////debug
			return;
    }
    else if(sock_int_socket & SINT_STAT_RECV)                             /* ???? */
    {
        len = CH395GetRecvLength(sockindex);                         /* ???????????? */
        if(len == 0)return;
        if(len > MAX_CACHE)len = MAX_CACHE;                                      /* ????????2048 */
      CH395GetRecvData(sockindex, len, MyBuffer[sockindex]);         /* ???? */
			
			//HAL_UART_Transmit(&huart3, MyBuffer[sockindex], len, 0xFF);///////////////////////////////////////////////////////////////////////////debug
			//CH395SendData( sockindex, MyBuffer[sockindex], len);  /*向发送缓冲区写数据*/
			
			
		if ((len == 14) || (len == 33))
		{
			if((MyBuffer[sockindex][8] == 0xF0)	&& (len == 14))//配置包解包
			{
				check = 0;
				for(i = 8; i < 13; i ++)
					check =  check + MyBuffer[sockindex][i];
				check = check & 0xFF;
				if(check == MyBuffer[sockindex][13])
				{
					memcpy(destAddr, MyBuffer[sockindex]+4, 4);
					TX_frequency = MyBuffer[sockindex][9];
					RX_frequency = MyBuffer[sockindex][10];
					if(MyBuffer[sockindex][12] == 0x01)
						bandwidth = 0x26;
					else if(MyBuffer[sockindex][12] == 0x02)
						bandwidth = 0x06;
					else if(MyBuffer[sockindex][12] == 0x03)
						bandwidth = 0x0F;
				  HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
			  	HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
			  	HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
				}
				TX_Mode();
				RX_Mode();
				IpTrans();
				OLED_CLS();
				OLED_ShowStr(0, 0, IPShow, 1);
				OLED_ShowStr(0, 2, RemoteIpShow, 1);
				
				/*初始化socket1*/
//				CH395SetSocketDesIP(1,destAddr);                        /* ??socket 0??IP?? */         
//				CH395SetSocketProtType(1,PROTO_TYPE_UDP);                        /* ??socket 0???? */
//				CH395SetSocketDesPort(1,Socket1DesPort);                     /* ??socket 0???? */
//				CH395SetSocketSourPort(1,Socket1SourPort);                   /* ??socket 0??? */
//				i = CH395OpenSocket(1);                                          /* ??socket 0 */
//				mStopIfError(i);
				/*初始化socket1*/
				
			}
			else if (len == 33)   //数据包发送
			{
				if(NRF24L01_TxPacket(MyBuffer[sockindex]+8) == TX_OK)
				{
					package_success++;
					if (package_success > 32)
					{
						HAL_GPIO_TogglePin(LED_TX_GPIO_Port, LED_TX_Pin);
						package_success=0;
					}
				}
			}
		}
			


   }
   else if(sock_int_socket & SINT_STAT_CONNECT)                            /* ????,??TCP?????*/
   {
//       if(SockInf[sockindex].TcpMode == TCP_SERVER_MODE)              /* ??socket ??????,?????????IP???*/
//       {
//           CH395CMDGetRemoteIPP(sockindex,buf);
//           tmp = (uint16_t)(buf[5]<<8) + buf[4];

//       }
   }
   else if(sock_int_socket & SINT_STAT_DISCONNECT)                        /* ????,??TCP????? */
   {
		//i = CH395CloseSocket(sockindex);                             
		//mStopIfError(i);
   }
   else if(sock_int_socket & SINT_STAT_TIM_OUT)                           /* ????,??TCP????? */
   {
		//i = CH395CloseSocket(sockindex);
		//mStopIfError(i);
//       if(SockInf[sockindex].TcpMode == TCP_CLIENT_MODE)             
//       {
//           HAL_Delay(200);                                            /* ??200MS?????,?????????? */
//           i = CH395OpenSocket(sockindex);
//           mStopIfError(i);
//           CH395TCPConnect(sockindex);                               /* ???? */
//           mStopIfError(i);
//       }
    }
}


/* 将十六进制的远程IP转换成字符串输出到屏幕上 */
void IpTrans()
{
	uint8_t i;
	RemoteIpLen = 5;
	
	for (i=0; i<4; i++){
		if (destAddr[i] >= 10 && destAddr[i] < 100){
			RemoteIpShow[RemoteIpLen] = destAddr[i] / 10 + 48;
			RemoteIpLen ++;
			RemoteIpShow[RemoteIpLen] = destAddr[i] - (destAddr[i] / 10) * 10 + 48;
			RemoteIpLen ++;
			RemoteIpShow[RemoteIpLen] = '.';
			RemoteIpLen ++;
		}
		
		else if (destAddr[i] >= 100){
			RemoteIpShow[RemoteIpLen] = destAddr[i] / 100 + 48;
			RemoteIpLen ++;
			RemoteIpShow[RemoteIpLen] = (destAddr[i] - (destAddr[i] / 100) * 100) / 10 + 48;
			RemoteIpLen ++;
			RemoteIpShow[RemoteIpLen] = destAddr[i] - (destAddr[i] / 10) * 10 + 48;
			RemoteIpLen ++;
			RemoteIpShow[RemoteIpLen] = '.';
			RemoteIpLen ++;
		}
			
		else{
			RemoteIpShow[RemoteIpLen] = destAddr[i] + 48;
			RemoteIpLen ++;
			RemoteIpShow[RemoteIpLen] = '.';
			RemoteIpLen ++;
		}
	}
	RemoteIpShow[RemoteIpLen - 1] = '\0';
}


