/********************************** (C) COPYRIGHT *******************************
* File Name          : CH395CMD.C
* Author             : WCH
* Version            : V1.1
* Date               : 2014/8/1
* Description        : CH395оƬ����ӿ��ļ�
*                      
*******************************************************************************/

/* ͷ�ļ�����*/
#include "CH395.h"
#include "gpio.h"
#include "string.h"
#include "NRF24L01.h"
#include "oled.h"

/*************************************************************************************************************************/
/* CH395��ض��� */                                                                                                       //
uint8_t CH395MACAddr[6] = {0x0A,0x03,0x04,0x05,0x06,0x07};   /* CH395MAC��ַ */                                           //        
uint8_t CH395IPAddr[4] = {10,12,225,130};                       /* CH395IP��ַ */                                         //          
uint8_t CH395GWIPAddr[4] = {10,12,225,1};                      /* CH395���� */                                            //       
uint8_t CH395IPMask[4] = {255,255,255,0};                      /* CH395�������� */                                        //             

uint8_t IPShow[] = "Local:10.12.225.130";
uint8_t RemoteIpShow[20] = "User:NULL";
uint8_t RemoteIpLen;
                                                                                                                          //              
/* socket ��ض���*/                                                                                                      //
uint8_t  Socket0DesIP[4] = {0xff,0xff,0xff,0xff};              /* Socket 0Ŀ��IP��ַ */                             //                                
uint16_t Socket0SourPort = 1030;                               /* Socket 0Դ�˿� */                                       //                             
uint16_t Socket0DesPort   = 1030;                              /* Socket 0Ŀ�Ķ˿� */                                     //      

uint8_t  Socket1DesIP[4] = {0xff,0xff,0xff,0xff};              /* Socket 1Ŀ��IP��ַ */                             //                                
uint16_t Socket1SourPort = 1040;                               /* Socket 1Դ�˿� */                                       //                             
uint16_t Socket1DesPort   = 1030;                              /* Socket 1Ŀ�Ķ˿� */ 

uint8_t destAddr[4];
uint8_t package_success = 0;
/***************************************************************************************************************************/

extern UART_HandleTypeDef huart3;
extern struct _SOCK_INF  SockInf[4];                                   /* ����Socket��Ϣ */
uint8_t MyBuffer[2][MAX_CACHE];

/* 2401������� */
extern uint8_t TX_frequency;	//24L01Ƶ�ʳ�ʼ��Ϊ5a
extern uint8_t RX_frequency;	//24L01Ƶ�ʳ�ʼ��Ϊ5a
extern uint8_t bandwidth;  //�����ʼ��Ϊ0x06

/********************************************************************************
* Function Name  : CH395CMDReset
* Description    : ��λCH395оƬ
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
* Description    : ʹCH395����˯��״̬
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
* Description    : ��ȡоƬ�Լ��̼��汾�ţ�1�ֽڣ�����λ��ʾоƬ�汾��
                   ����λ��ʾ�̼��汾
* Input          : None
* Output         : None
* Return         : 1�ֽ�оƬ���̼��汾��
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
* Description    : ����������ڲ���Ӳ���Լ��ӿ�ͨѶ
* Input          : testdata 1�ֽڲ�������
* Output         : None
* Return         : Ӳ��OK������ testdata��λȡ��
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
* Description    : ����PHY����Ҫ����CH395 PHYΪ100/10M ����ȫ˫����˫����CH395Ĭ
                    Ϊ�Զ�Э�̡�
* Input          : phystat �ο�PHY �������/״̬
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
* Description    : ��ȡPHY��״̬
* Input          : None
* Output         : None
* Return         : ��ǰCH395PHY״̬���ο�PHY����/״̬����
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
* Description    : ��ȡȫ���ж�״̬���յ�������CH395�Զ�ȡ���жϣ�0x43�����°汾ʹ��
* Input          : None
* Output         : None
* Return         : ���ص�ǰ��ȫ���ж�״̬
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
* Description    : ��ʼ��CH395оƬ��
* Input          : None
* Output         : None
* Return         : ����ִ�н��
*******************************************************************************/
uint8_t CH395CMDInitCH395(void)
{
    uint8_t i = 0;
    uint8_t s = 0;

    xWriteCH395Cmd(CMD0W_INIT_CH395);
    xEndCH395Cmd();
    while(1)
    {
        HAL_Delay(5);                                                 /* ��ʱ��ѯ������2MS����*/
        s = CH395GetCmdStatus();                                     /* ���ܹ���Ƶ����ѯ*/
        if(s !=CH395_ERR_BUSY)break;                                 /* ���CH395оƬ����æ״̬*/
        if(i++ > 200)return CH395_ERR_UNKNOW;                        /* ��ʱ�˳�,��������Ҫ500MS����ִ����� */
    }
    return s;
}

/********************************************************************************
* Function Name  : CH395CMDSetUartBaudRate
* Description    : ����CH395���ڲ����ʣ����ڴ���ģʽ����Ч
* Input          : baudrate ���ڲ�����
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
* Description    : ��ȡ����ִ��״̬��ĳЩ������Ҫ�ȴ�����ִ�н��
* Input          : None
* Output         : None
* Return         : ������һ������ִ��״̬
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
* Description    : ����CH395��IP��ַ
* Input          : ipaddr ָIP��ַ
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
* Description    : ����CH395������IP��ַ
* Input          : ipaddr ָ������IP��ַ
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
* Description    : ����CH395���������룬Ĭ��Ϊ255.255.255.0
* Input          : maskaddr ָ���������ַ
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
* Description    : ����CH395��MAC��ַ��
* Input          : amcaddr MAC��ַָ��
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
* Description    : ��ȡCH395��MAC��ַ��
* Input          : amcaddr MAC��ַָ��
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
* Description    : ����MAC���ˡ�
* Input          : filtype �ο� MAC����
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
* Description    : ��ȡ���ɴ���Ϣ (IP,Port,Protocol Type)
* Input          : list �����ȡ���Ĳ��ɴ�
                        ��1���ֽ�Ϊ���ɴ���룬��ο� ���ɴ����(CH395INC.H)
                        ��2���ֽ�ΪIP��Э������
                        ��3-4�ֽ�Ϊ�˿ں�
                        ��4-8�ֽ�ΪIP��ַ
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
* Description    : ��ȡԶ�˵�IP�Ͷ˿ڵ�ַ��һ����TCP Serverģʽ��ʹ��
* Input          : sockindex Socket����
                   list ����IP�Ͷ˿�
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
* Description    : ����socket n��Ŀ��IP��ַ
* Input          : sockindex Socket����
                   ipaddr ָ��IP��ַ
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
* Description    : ����socket ��Э������
* Input          : sockindex Socket����
                   prottype Э�����ͣ���ο� socketЭ�����Ͷ���(CH395INC.H)
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
* Description    : ����socket n��Э������
* Input          : sockindex Socket����
                   desprot 2�ֽ�Ŀ�Ķ˿�
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
* Description    : ����socket n��Э������
* Input          : sockindex Socket����
                   desprot 2�ֽ�Դ�˿�
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
* Description    : IPģʽ�£�socket IP��Э���ֶ�
* Input          : sockindex Socket����
                   prototype IPRAWģʽ1�ֽ�Э���ֶ�
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
* Description    : ����/�ر� PING
* Input          : enable : 1  ����PING
                          ��0  �ر�PING
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
* Description    : ���ͻ�����д����
* Input          : sockindex Socket����
                   databuf  ���ݻ�����
                   len   ����
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
* Description    : ��ȡ���ջ���������
* Input          : sockindex Socket����
* Output         : None
* Return         : ���ؽ��ջ�������Ч����
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
* Description    : ������ջ�����
* Input          : sockindex Socket����
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
* Description    : ��ȡ���ջ���������
* Input          : sockindex Socket����
                   len   ����
                   pbuf  ������
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
* Description    : �������Դ���
* Input          : count ����ֵ�����Ϊ20��
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
* Description    : ������������
* Input          : period �������ڵ�λΪ���룬���1000ms
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
* Description    : ��ȡsocket
* Input          : None
* Output         : socket n��״̬��Ϣ����1�ֽ�Ϊsocket �򿪻��߹ر�
                   ��2�ֽ�ΪTCP״̬
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
* Description    : ��socket����������Ҫ�ȴ�ִ�гɹ�
* Input          : sockindex Socket����
* Output         : None
* Return         : ����ִ�н��
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
        HAL_Delay(5);                                                 /* ��ʱ��ѯ������2MS����*/
        s = CH395GetCmdStatus();                                     /* ���ܹ���Ƶ����ѯ*/
        if(s !=CH395_ERR_BUSY)break;                                 /* ���CH395оƬ����æ״̬*/
        if(i++ > 200)return CH395_ERR_UNKNOW;                        /* ��ʱ�˳�*/
    }
    return s;
}

/*******************************************************************************
* Function Name  : CH395OpenSocket
* Description    : �ر�socket��
* Input          : sockindex Socket����
* Output         : None
* Return         : ����ִ�н��
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
        HAL_Delay(5);                                                 /* ��ʱ��ѯ������2MS����*/
        s = CH395GetCmdStatus();                                     /* ���ܹ���Ƶ����ѯ*/
        if(s !=CH395_ERR_BUSY)break;                                 /* ���CH395оƬ����æ״̬*/
        if(i++ > 200)return CH395_ERR_UNKNOW;                        /* ��ʱ�˳�*/
    }
    return s;
}

/********************************************************************************
* Function Name  : CH395TCPConnect
* Description    : TCP���ӣ�����TCPģʽ����Ч����������Ҫ�ȴ�ִ�гɹ�
* Input          : sockindex Socket����
* Output         : None
* Return         : ����ִ�н��
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
        HAL_Delay(5);                                                 /* ��ʱ��ѯ������2MS����*/
        s = CH395GetCmdStatus();                                     /* ���ܹ���Ƶ����ѯ*/
        if(s !=CH395_ERR_BUSY)break;                                 /* ���CH395оƬ����æ״̬*/
        if(i++ > 200)return CH395_ERR_UNKNOW;                        /* ��ʱ�˳�*/
    }
    return s;
}

/******************************************************************************
* Function Name  : CH395TCPListen
* Description    : TCP����������TCPģʽ����Ч����������Ҫ�ȴ�ִ�гɹ�
* Input          : sockindex Socket����
* Output         : None
* Return         : ����ִ�н��
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
        HAL_Delay(5);                                                 /* ��ʱ��ѯ������2MS����*/
        s = CH395GetCmdStatus();                                     /* ���ܹ���Ƶ����ѯ*/
        if(s !=CH395_ERR_BUSY)break;                                 /* ���CH395оƬ����æ״̬*/
        if(i++ > 200)return CH395_ERR_UNKNOW;                        /* ��ʱ�˳�*/
    }
    return s;
}

/********************************************************************************
* Function Name  : CH395TCPDisconnect
* Description    : TCP�Ͽ�������TCPģʽ����Ч����������Ҫ�ȴ�ִ�гɹ�
* Input          : sockindex Socket����
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
        HAL_Delay(5);                                                 /* ��ʱ��ѯ������2MS����*/
        s = CH395GetCmdStatus();                                     /* ���ܹ���Ƶ����ѯ*/
        if(s !=CH395_ERR_BUSY)break;                                 /* ���CH395оƬ����æ״̬*/
        if(i++ > 200)return CH395_ERR_UNKNOW;                        /* ��ʱ�˳�*/
    }
    return s;
}

/*******************************************************************************
* Function Name  : CH395GetSocketInt
* Description    : ��ȡsocket n���ж�״̬
* Input          : sockindex   socket����
* Output         : None
* Return         : �ж�״̬
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
* Description    : �Զಥ��ַ����CRC���㣬��ȡ��6λ��
* Input          : mac_addr   MAC��ַ
* Output         : None
* Return         : ����CRC32�ĸ�6λ
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
* Description    : ����/ֹͣDHCP
* Input          : flag   1:����DHCP;0��ֹͣDHCP
* Output         : None
* Return         : ִ��״̬
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
        s = CH395GetCmdStatus();                                     /* ���ܹ���Ƶ����ѯ*/
        if(s !=CH395_ERR_BUSY)break;                                 /* ���CH395оƬ����æ״̬*/
        if(i++ > 200)return CH395_ERR_UNKNOW;                        /* ��ʱ�˳�*/
    }
    return s;
}

/******************************************************************************
* Function Name  : CH395GetDHCPStatus
* Description    : ��ȡDHCP״̬
* Input          : None
* Output         : None
* Return         : DHCP״̬��0Ϊ�ɹ�������ֵ��ʾ����
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
* Description    : ��ȡIP��������������ص�ַ
* Input          : None
* Output         : 12���ֽڵ�IP,������������ص�ַ
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
* Description    : дGPIO�Ĵ���
* Input          : regadd   �Ĵ�����ַ
*                ��regval   �Ĵ���ֵ
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
* Description    : ��GPIO�Ĵ���
* Input          : regadd   �Ĵ�����ַ
* Output         : None
* Return         : �Ĵ�����ֵ
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
* Description    : ����EEPROM
* Input          : None
* Output         : None
* Return         : ִ��״̬
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
* Description    : дEEPROM
* Input          : eepaddr  EEPROM��ַ
*                ��buf      ��������ַ
*                ��len      ����
* Output         : None
* Return         : ִ��״̬
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
* Description    : дEEPROM
* Input          : eepaddr  EEPROM��ַ
*                ��buf      ��������ַ
*                ��len      ����
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
* Description    : ����TCP MSSֵ
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
* Description    : ����Socket���ջ�����
* Input          : sockindex  socket����
                 ��startblk   ��ʼ��ַ
                 ��blknum     ��λ���������� ����λΪ512�ֽ�
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
* Description    : ����Socket���ͻ�����
* Input          : sockindex  socket����
                 ��startblk   ��ʼ��ַ
                 ��blknum     ��λ����������
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
* Description    : UDP��ָ����IP�Ͷ˿ڷ�������
* Input          : buf     : �������ݻ�����
                   len     : �������ݳ���
				   ip      : Ŀ��IP
				   port    : Ŀ��˿�
				   sockeid : socket����ֵ
* Output         : None
* Return         : None
*******************************************************************************/
void CH395UDPSendTo(uint8_t *buf,uint32_t len,uint8_t *ip,uint16_t port,uint8_t sockindex)
{
    CH395SetSocketDesIP(sockindex,ip);                            /* ����socket 0Ŀ��IP��ַ */         
    CH395SetSocketDesPort(sockindex,port);
    CH395SendData(sockindex,buf,len);    
}

/*******************************************************************************
* Function Name  : CH395SetStartPara
* Description    : ����CH395��������
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
* Description    : ��ȡȫ���ж�״̬���յ�������CH395�Զ�ȡ���ж�,0x44�����ϰ汾ʹ��
* Input          : None
* Output         : None
* Return         : ���ص�ǰ��ȫ���ж�״̬
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
* Description    : ��CH395д����
* Input          : cmd 8λ������
* Output         : None
* Return         : None
==============================================================================*/
void xWriteCH395Cmd(uint8_t cmd)									    
{
//	* ( __IO uint8_t * ) ( FSMC_Addr_CH395_CMD ) = cmd;                   /* ��CH395д���� */
//			delay_us(2);                                                    
 	Set_Output();											 /* ������� */
	ParaOut(cmd);		 /* ��CH395�Ĳ���������� */	
	HAL_GPIO_WritePin(A0_GPIO_Port, A0_Pin, GPIO_PIN_SET);		 											 /* ��A0��Ϊ�� */
	//delay_us(15);

	HAL_GPIO_WritePin(WR_GPIO_Port, WR_Pin, GPIO_PIN_RESET);		     									 /* ��WR���� */
	HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_SET);                              /* ��RD���� */
	HAL_GPIO_WritePin(PCS_GPIO_Port, PCS_Pin, GPIO_PIN_RESET);		     									 /* ��CS���� */
	delay_us(1);                                                                        /* �ȴ�����ִ����� */
	
	HAL_GPIO_WritePin(WR_GPIO_Port, WR_Pin, GPIO_PIN_SET);		     										 /* ��WR��Ϊ�� */
		HAL_GPIO_WritePin(PCS_GPIO_Port, PCS_Pin, GPIO_PIN_SET);		     										 /* ��CS��Ϊ�� */
	//delay_us(5);
		HAL_GPIO_WritePin(A0_GPIO_Port, A0_Pin, GPIO_PIN_RESET);		 										 /* ��A0���� */
	//Set_Input();
	//delay_us(1);
}

/*==============================================================================
* Function Name  : xWriteCH395Data
* Description    : ��CH395д8λ����
* Input          : mdata 8λ����
* Output         : None
* Return         : None
==============================================================================*/
void  xWriteCH395Data(uint8_t mdata)
{			
//	* ( __IO uint8_t * ) ( FSMC_Addr_CH395_DATA ) = mdata;														 /* ��CH395д���� */	
//		delay_us(10);
	Set_Output();										 /* ������� */
	ParaOut(mdata);		 /* ��CH395�Ĳ���������� */
	HAL_GPIO_WritePin(A0_GPIO_Port, A0_Pin, GPIO_PIN_RESET);                           /* Reset A0 to data mode*/
	//delay_us(15);
	
    HAL_GPIO_WritePin(WR_GPIO_Port, WR_Pin, GPIO_PIN_RESET);		     									 /* ��WR���� */
	  HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_SET);                             /* ��RD���� */
	HAL_GPIO_WritePin(PCS_GPIO_Port, PCS_Pin, GPIO_PIN_RESET);	     									 /* ��CS���� */
	delay_us(1);
                                                                          /* �ȴ�����ִ����� */
	HAL_GPIO_WritePin(WR_GPIO_Port, WR_Pin, GPIO_PIN_SET);		     									 /* ��WR���� */
	 HAL_GPIO_WritePin(PCS_GPIO_Port, PCS_Pin, GPIO_PIN_SET);	     										 /* ��CS��Ϊ�� */
	//Set_Input();
	//delay_us(1);

}

uint8_t	xReadCH395Data( void )                                       /* ��CH395������ */
{
//	delay_us(2);
//	return ( * ( __IO uint8_t * ) ( FSMC_Addr_CH395_DATA ) );
    uint8_t i = 0;
    Set_Input(); 											 /* ���ݶ˿�����Ϊ���� */
	delay_us(1);
		HAL_GPIO_WritePin(A0_GPIO_Port, A0_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_RESET);		     								     /* ��RD���� */
	HAL_GPIO_WritePin(WR_GPIO_Port, WR_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PCS_GPIO_Port, PCS_Pin, GPIO_PIN_RESET);		     									 /* ��CS���� */
	delay_us(1);
		i = ParaIn();											 /* �Ӷ˿ڶ����� */
    HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_SET);	     										 /* ��RD��Ϊ�� */
	HAL_GPIO_WritePin(PCS_GPIO_Port, PCS_Pin, GPIO_PIN_SET);		     										 /* ��CS��Ϊ�� */
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
  if(i != 0x9a)return CH395_ERR_UNKNOW;                            /* �����������޷�ͨ������0XFA */
	
	CH395CMDSetMACAddr(CH395MACAddr);                             /* ����CH395��MAC��ַ */
	CH395CMDSetIPAddr(CH395IPAddr);                               /* ����CH395��IP��ַ */
  CH395CMDSetGWIPAddr(CH395GWIPAddr);                           /* �������ص�ַ */
  CH395CMDSetMASKAddr(CH395IPMask);                             /* �����������룬Ĭ��Ϊ255.255.255.0*/ 
  HAL_Delay(10);  
  i = CH395CMDInitCH395();                                /* ��ʼ��CH395оƬ */
  
	return i;
}

/**********************************************************************************
* Function Name  : mStopIfError
* Description    : ����ʹ�ã���ʾ������룬��ͣ��
* Input          : iError
* Output         : None
* Return         : None
**********************************************************************************/
void mStopIfError(uint8_t iError)
{
    if (iError == CMD_ERR_SUCCESS) return;                           /* �����ɹ� */
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
      //HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);                  /* �����Ƿ��յ��ж�-->success */
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

    sock_int_socket = CH395GetSocketInt(sockindex);                  /* ��ȡ�ж����� */
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
			//CH395SendData( sockindex, MyBuffer[sockindex], len);  /*���ͻ�����д����*/
			
			
		if ((len == 14) || (len == 33))
		{
			if((MyBuffer[sockindex][8] == 0xF0)	&& (len == 14))//���ð����
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
				
				/*��ʼ��socket1*/
//				CH395SetSocketDesIP(1,destAddr);                        /* ??socket 0??IP?? */         
//				CH395SetSocketProtType(1,PROTO_TYPE_UDP);                        /* ??socket 0???? */
//				CH395SetSocketDesPort(1,Socket1DesPort);                     /* ??socket 0???? */
//				CH395SetSocketSourPort(1,Socket1SourPort);                   /* ??socket 0??? */
//				i = CH395OpenSocket(1);                                          /* ??socket 0 */
//				mStopIfError(i);
				/*��ʼ��socket1*/
				
			}
			else if (len == 33)   //���ݰ�����
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


/* ��ʮ�����Ƶ�Զ��IPת�����ַ����������Ļ�� */
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


