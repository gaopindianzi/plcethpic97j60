#include "TCPIP Stack/TCPIP.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


/*
使用宗旨：
满足调试要求，输出简单字符串，输出复杂字符串
输出字符，输出数字
后期实现格式化输出
但首先是输出一个字符和字符串
*/

unsigned char debug_tx[255];
unsigned char debug_tx_count;
#define   DEBUG_PORT        50505


void DebugTaskInit(void)
{
    debug_tx_count = 0;
}

void DebugPrintChar(char ch)
{
    if(debug_tx_count < sizeof(debug_tx)) {
        debug_tx[debug_tx_count++] = ch;
    }
}

void DebugString(const char * ramstr)
{
	while(*ramstr) {
		DebugPrintChar(*ramstr++);
	}
}


void DebugStringNum(const char * str,unsigned int num,const char * tail)
{
	unsigned char buffer[20];
	unsigned char i;
	DebugString(str);
	i = 0;
	while(1) {
		char n = num  % 10;
		buffer[i++] = n + '0';
		num /= 10;
		if(num == 0){
			break;
		}
	}
	if(i) {
		while(1) {
		    i--;
		    DebugPrintChar(buffer[i]);
			if(i == 0) {
				break;
			}
		}
	}
	DebugString(tail);
}


void DebugStringHex(const char * str,unsigned long num,const char * tail)
{
	unsigned char buffer[20];
	unsigned char i;
	DebugString(str);
	i = 0;
	while(1) {
		char n = (char)(num  & 0x0F);
		if(n > 9) {
			buffer[i++] = n + 'A';
		} else {
			buffer[i++] = n + '0';
		}
		num >>= 4;
		if(num == 0){
			break;
		}
	}
	if(i) {
		while(1) {
		    i--;
		    DebugPrintChar(buffer[i]);
			if(i == 0) {
				break;
			}
		}
	}
	DebugString(tail);
}


void DebugTask(void)
{
	static enum {
		DEBUG_HOME = 0,
		DEBUG_DNS_IP,
		DEBUG_DNS_WAIT,
		DEBUG_DNS_CLOSE,
		DEBUG_RUN
	} DebugSM = DEBUG_HOME;

	static UDP_SOCKET	MySocket;
	static TICK         StartTime;
	static IP_ADDR      hostip;

	switch(DebugSM)
	{
		case DEBUG_HOME:
			{
				DebugTaskInit();
				DebugSM = DEBUG_DNS_IP;
				RELAY_OUT_0 = 0;
			}
			break;
		case DEBUG_DNS_IP:
			{
				char buffer[32];
				RELAY_OUT_5 = 1;
				RELAY_OUT_4 = 1;
				RELAY_OUT_3 = 1;
				RELAY_OUT_2 = 1;
				RELAY_OUT_1 = 0;
				if(!DNSBeginUsage()) {
					break;
				}
				StartTime = TickGet();
				//strcpypgm2ram(buffer,(ROM BYTE*)"www.qq.com");
				//DNSResolve(buffer, DNS_TYPE_A);
				DNSResolveROM((ROM BYTE*)"www.qq.com", DNS_TYPE_A);
				DebugSM = DEBUG_DNS_WAIT;
			}
			break;
		case DEBUG_DNS_WAIT:
			{
				RELAY_OUT_2 = 0;
				if(!DNSIsResolved(&hostip)) {
					break;
				}
				DebugStringNum("hostip=",hostip.v[0],".");
				DebugStringNum("",hostip.v[1],".");
				DebugStringNum("",hostip.v[2],".");
				DebugStringNum("",hostip.v[3],",  ");
				DebugSM = DEBUG_DNS_CLOSE;
			}
			break;
		case DEBUG_DNS_CLOSE:
			{
				RELAY_OUT_3 = 0;
				if(!DNSEndUsage()) {
					if((TickGet() - StartTime) > TICK_SECOND*10) {
						DebugSM = DEBUG_DNS_IP;
					}
					break;
				}
				
				DebugSM = DEBUG_RUN;
			}
			break;
		case DEBUG_RUN:
			{
				RELAY_OUT_4 = 0;
				if(TickGet() - StartTime > TICK_SECOND) {
					StartTime = TickGet();
					//DebugPrintString("haha\r\n");
					DebugStringNum("IP地址是",50505,"");
					DebugStringHex("Ip地址是。。。",0x333422,"\r\n");
				} else {
					break;
				}

				
				RELAY_OUT_5 = 0;

				DebugSM = DEBUG_DNS_IP;

				RUN_LED_IO = !RUN_LED_IO;
				


				if(debug_tx_count == 0) {
					break;
				}


				//开始用UDP进行调试信息广播，广播端口号是0xdddd
			    MySocket = UDPOpen(0, NULL, DEBUG_PORT); //随即分配内部端口号，广播模式，对DEBUG_PORT端口进行广播
	            if(MySocket == INVALID_UDP_SOCKET)
		            break;
	
	            // 确定UDP写准备好的。
	            while(!UDPIsPutReady(MySocket));

				//一次性写完
				UDPPutArray((BYTE*)debug_tx,debug_tx_count);
				//UDPPutROMString((ROM BYTE*)"my udp debug test.\r\n");
				debug_tx_count = 0;
	            UDPFlush();
	
	            // 关闭，以便其他地方可以使用
	            UDPClose(MySocket);

				DebugSM = DEBUG_DNS_IP;
			}
			break;
		default:
			DebugSM = DEBUG_HOME;
			break;
	}
}


//*********************
// 发现协议使用的端口号和时间间隔等变量

#define DISCOVERY_PORT  9806
static  DWORD           xid = 0;

extern APP_CONFIG AppConfig;
extern NODE_INFO  remoteNode;

#define DISCOVERY_VERSION   0x10
#define DIST_REQUEST    0
#define DIST_ANNOUNCE   1
#define DIST_APPLY      2
#define DIST_RESET      3

#if 0
typedef struct __attribute__ ((packed)) _DISCOVERY_TELE {
    uint32_t dist_xid;            /*!< \brief Exchange identifier. */
    uint8_t dist_type;           /*!< \brief Message type, 0=request, 1=announce, 2=apply. */
    uint8_t dist_ver;            /*!< \brief Telegram version. */
    uint8_t dist_mac[6];         /*!< \brief Ethernet MAC address. */
    uint32_t dist_ip_addr;        /*!< \brief Last used IP address. */
    uint32_t dist_ip_mask;        /*!< \brief IP netmask. */
    uint32_t dist_gateway;        /*!< \brief Default route. */
    uint32_t dist_cip_addr;       /*!< \brief Configured IP address.  23字节 */ 
#if DISCOVERY_VERSION <= 0x10
    uint8_t dist_hostname[8];    /*!< \brief Host name of the system. */  //31字节
    uint8_t dist_custom[92];     /*!< \brief Bootfile to request. */ //123字节
#else
    uint8_t dist_appendix[100];  /*! \brief Host name with lenght byte in front. */  //123字节
#endif
} DISCOVERY_TELE;
#endif


	struct DISCOVERY_TELE {
        DWORD dist_xid;            /*!< \brief Exchange identifier. */
        BYTE  dist_type;           /*!< \brief Message type, 0=request, 1=announce, 2=apply. */
        BYTE  dist_ver;            /*!< \brief Telegram version. */
        BYTE  dist_mac[6];         /*!< \brief Ethernet MAC address. */
        DWORD dist_ip_addr;        /*!< \brief Last used IP address. */
        DWORD dist_ip_mask;        /*!< \brief IP netmask. */
        DWORD dist_gateway;        /*!< \brief Default route. */
        DWORD dist_cip_addr;       /*!< \brief Configured IP address.  23字节 */ 
#if DISCOVERY_VERSION <= 0x10
        BYTE  dist_hostname[8];    /*!< \brief Host name of the system. */  //31字节
        BYTE  dist_custom[92];     /*!< \brief Bootfile to request. */ //123字节
#else
        BYTE  dist_appendix[100];  /*! \brief Host name with lenght byte in front. */  //123字节
#endif
	};


static void DiscoveryTellConfig(void)
{
	BYTE tb;
	//逐步发送需要发送的数据
	UDPPutArray((BYTE *)&xid,sizeof(xid));
	tb = DIST_ANNOUNCE;
	UDPPutArray((BYTE *)&tb,sizeof(tb));
	tb = DISCOVERY_VERSION;
	UDPPutArray((BYTE *)&tb,sizeof(tb));
	//发送六个字节的MAC地址
	UDPPutArray((BYTE *)&(AppConfig.MyMACAddr),sizeof(AppConfig.MyMACAddr));
	UDPPutArray((BYTE *)&(AppConfig.DefaultIPAddr.Val),4);
	UDPPutArray((BYTE *)&(AppConfig.MyMask.Val),4);
	UDPPutArray((BYTE *)&(AppConfig.MyGateway.Val),4);
	if(0) { //AppConfig.Flags.bIsDHCPEnabled == TRUE) {
		DWORD w = 0;
		UDPPutArray((BYTE *)&w,4);
	} else {
	    UDPPutArray((BYTE *)&(AppConfig.MyIPAddr.Val),4);
	}
#if DISCOVERY_VERSION <= 0x10
	{
		UDPPutArray((BYTE *)&AppConfig.NetBIOSName,8);
		//25字节
	}
#else
	{
		tb = sizeof(AppConfig.NetBIOSName);
		UDPPutArray((BYTE *)&tb,sizeof(tb));
		UDPPutArray((BYTE *)&AppConfig.NetBIOSName,sizeof(AppConfig.NetBIOSName));
		//26字节
	}
#endif
}



extern void SaveAppConfig(void);


static void DiscoverySaveConfig(struct DISCOVERY_TELE * dist)
{

	memcpy((void*)&AppConfig.MyMACAddr.v[0], (void*)&dist->dist_mac[0], sizeof(AppConfig.MyMACAddr));
	AppConfig.MyIPAddr.Val = dist->dist_cip_addr;
	AppConfig.DefaultIPAddr.Val = AppConfig.MyIPAddr.Val;
	if(AppConfig.MyIPAddr.Val == 0) {
		//DHCP分配
		AppConfig.Flags.bIsDHCPEnabled = TRUE;
		AppConfig.Flags.bInConfigMode = TRUE;
	} else {
		AppConfig.Flags.bIsDHCPEnabled = FALSE;
		AppConfig.Flags.bInConfigMode = FALSE;
	}
	AppConfig.MyMask.Val = dist->dist_ip_mask;
	AppConfig.MyGateway.Val = dist->dist_gateway;

#if DISCOVERY_VERSION <= 0x10
	memcpy(AppConfig.NetBIOSName,dist->dist_hostname,sizeof(dist->dist_hostname));
#else
	if(dist->dist_appendix[0] > sizeof(AppConfig.NetBIOSName)) {
	    memcpy(AppConfig.NetBIOSName,&dist->dist_appendix[1],sizeof(AppConfig.NetBIOSName);
	} else {
		memcpy(AppConfig.NetBIOSName,&dist->dist_appendix[1],sizeof(dist->dist_appendix[0]);
	}
#endif
	SaveAppConfig();

	Reset();

}


void DiscoverTask(void)
{


	static enum {
		DISCOVERY_HOME = 0,
		WAIT,
		DISCOVER,
		DISCOVERY_LISTEN,
		DISCOVERY_REQUEST,
		DISCOVERY_APPLY,
		DISCOVERY_RESET,
	} TaskSM = DISCOVERY_HOME;

	static UDP_SOCKET	MySocket;
	static TICK         StartTime;

	switch(TaskSM)
	{

		case DISCOVERY_HOME:
			{
				StartTime = TickGet();
				TaskSM++;
			}
			break;
		case WAIT:
			{
				if(TickGet() - StartTime > TICK_SECOND*5) {
					TaskSM++;
				}
			}
			break;
		case DISCOVER:
			{
			    MySocket = UDPOpen(DISCOVERY_PORT,NULL, DISCOVERY_PORT);
	            if(MySocket == INVALID_UDP_SOCKET)
		            break;

				RELAY_OUT_0 = 0;

				TaskSM = DISCOVERY_REQUEST;
			}
			break;
		case DISCOVERY_LISTEN:
			{
				WORD   len,minlen;
				struct DISCOVERY_TELE dist;

				len = UDPIsGetReady(MySocket);
				//if(!len) {
				//	break;
				//}

#if DISCOVERY_VERSION <= 0x10
				minlen = sizeof(dist) - sizeof(dist.dist_custom);
#else
				minlen = sizeof(dist) - sizeof(dist.dist_appendix) + 1;
#endif
				if (len >= minlen) {// Check to see if a packet has arrived
				    UDPGetArray((BYTE *)&dist,minlen);
					UDPDiscard();
					if (dist.dist_type == DIST_REQUEST) {
						
						// Change the destination to the unicast address of the last received packet
						memcpy((void*)&UDPSocketInfo[MySocket].remoteNode, (const void*)&remoteNode, sizeof(remoteNode));

						TaskSM = DISCOVERY_REQUEST;

						break;
					}
					/* 
					* Apply telegram. 
					*/
					else if (dist.dist_type == DIST_APPLY
						/* Check exchange ID. */
						&& dist.dist_xid == xid
						/* Required protocol version. */
						&& dist.dist_ver == DISCOVERY_VERSION) {    
							xid += TickGet();
							/* Store configuration. */
							DiscoverySaveConfig(&dist);

							RELAY_OUT_2 = !RELAY_OUT_2;
					} else if(dist.dist_type == DIST_RESET) {
						//重启
						RELAY_OUT_3 = !RELAY_OUT_3;
					}
				} else {
					UDPDiscard();
					RELAY_OUT_4 = !RELAY_OUT_4;
				}
			}
			break;
		case DISCOVERY_REQUEST:
			{
				/* Respond to requests. */
				WORD txlen = UDPIsPutReady(MySocket);

				if(txlen < sizeof(struct DISCOVERY_TELE)) {
					break;
				}
				//写数据
				DiscoveryTellConfig();
				//
				UDPFlush();

				RELAY_OUT_1 = !RELAY_OUT_1;

				TaskSM = DISCOVERY_LISTEN;
			}
			break;
		default:
			TaskSM = DISCOVERY_HOME;
			break;
	}
}

extern void ResetToDefaultConfig(void);

void ResetTask(void)
{
	static enum {
		HOME = 0,
		WAIT,
		IDLE,
		RESET_KEY_PUSH_DELAY
	} TaskSM = HOME;

	static TICK         StartTime;

	switch(TaskSM)
	{
	case HOME:
		{
			StartTime = TickGet();
			TaskSM++;
		}
		break;
	case WAIT:
		{
	    	if(TickGet() - StartTime < TICK_SECOND*5) {
				break;
			}
			TaskSM++;
		}
		break;
	case IDLE:
		{
			if(!IP_CONFIG_IO) {
				RELAY_OUT_5 = 0;
				StartTime = TickGet();
				TaskSM++;
			}
		}
		break;
	case RESET_KEY_PUSH_DELAY:
		{
			if(TickGet() - StartTime < TICK_SECOND*5) {
				if(IP_CONFIG_IO) {
					RELAY_OUT_5 = 1;
					TaskSM--;
					break;
				}
			} else {
				//复位
				ResetToDefaultConfig();
				Reset();
			}
		}
		break;
	default:
		TaskSM = HOME;
		break;
	}
}





//#define  DEBUG_TCP_CLIENT

void DebugTcpTask(void)
{
	static enum _BridgeState
	{
		SM_HOME = 0,
		SM_SOCKET_OBTAINED
	} BridgeState = SM_HOME;
	static TCP_SOCKET MySocket = INVALID_SOCKET;
	static BYTE RX_Buffer[200];
	WORD wMaxPut, wMaxGet;

	switch(BridgeState)
	{
		case SM_HOME:

			putrsUART((ROM char*)"\r\n tcp sm home.");

			#if defined(DEBUG_TCP_CLIENT) //客户端
				// Connect a socket to the remote TCP server
				MySocket = TCPOpen((DWORD)"192.168.1.36", TCP_OPEN_ROM_HOST, 2000, TCP_PURPOSE_CMD_SERVER);
			#else //服务器
				MySocket = TCPOpen(0, TCP_OPEN_SERVER, 2000, TCP_PURPOSE_CMD_SERVER);
			#endif			
			// Abort operation if no TCP socket of type TCP_PURPOSE_UART_2_TCP_BRIDGE is available
			// If this ever happens, you need to go add one to TCPIPConfig.h
			if(MySocket == INVALID_SOCKET)
				break;

			// Eat the first TCPWasReset() response so we don't 
			// infinitely create and reset/destroy client mode sockets
			TCPWasReset(MySocket);

			// We have a socket now, advance to the next state
			BridgeState = SM_SOCKET_OBTAINED;
			break;

		case SM_SOCKET_OBTAINED:
			if(TCPWasReset(MySocket))
			{
				// Optionally discard anything in the UART FIFOs
				//RXHeadPtr = vUARTRXFIFO;
				//RXTailPtr = vUARTRXFIFO;
				//TXHeadPtr = vUARTTXFIFO;
				//TXTailPtr = vUARTTXFIFO;
				
				// If we were a client socket, close the socket and attempt to reconnect
				#if defined(DEBUG_TCP_CLIENT)
					TCPDisconnect(MySocket);
					MySocket = INVALID_SOCKET;
					BridgeState = SM_HOME;
					break;
				#endif
			}
			// Don't do anything if nobody is connected to us
			if(!TCPIsConnected(MySocket))
			{
				break;
			}
			while(1)
			{

				WORD w = sizeof(RX_Buffer);
				wMaxGet = TCPIsGetReady(MySocket);	// Get TCP RX FIFO byte count

				if(w >= wMaxGet) {

				    TCPGetArray(MySocket, &RX_Buffer[0], wMaxGet);

					wMaxPut = TCPIsPutReady(MySocket);	// Get TCP TX FIFO space

					if(wMaxPut >= wMaxGet) {
						TCPPutArray(MySocket, RX_Buffer, wMaxGet);
					} else {
					}
					TCPFlush(MySocket);
					break;

				} else {
					break;
				}

			}
			break;
		default:
			BridgeState = SM_HOME;
			break;
	}
}


/**********************************************
 *
 * LED 闪烁接口和任务函数
 *
 */

static unsigned char led_work_mode = 0;
static BOOL          led_state = 0;
static unsigned int  led_flash_on_time,led_flash_off_time;
static unsigned int  led_flash_times;
#define LED_WORK_ONOFF  0
#define LED_FLASH_MODE  1

#define SET_LED_ON(on)    do{ if(on){ RUN_LED_IO = 0; }else{ RUN_LED_IO = 1; }} while(0)


/***************
 * 参数说明
 * on : 设置LED开或者关
 */
void set_led(BOOL on)
{
	led_work_mode = LED_WORK_ONOFF;
	led_state = on;
}

/***************
 * 参数说明
 * on_time : 开启时间，单位毫秒
 * off_time : 关闭时间，单位毫秒
 * times : 0不限次数,大雨0，闪烁times次
 */
void set_led_flash(unsigned int on_time,unsigned int off_time,unsigned int times)
{
	led_work_mode = LED_FLASH_MODE;
	led_flash_on_time = on_time;
	led_flash_off_time = off_time;
	led_flash_times = times;
}


void TaskLedFlash(void)
{
    static enum LedState
    {
		LED_HOME = 0,
		LED_WAIT_NORMAL,
		LED_ON_OFF,
        LED_FLASH_START,
        LED_FLASH_WAIT_OFF,
        LED_FLASH_WAIT_ON
    } LedSm = LED_HOME;

    static TICK           flash_start_time;
    switch(LedSm)
	{
		case LED_HOME:
		{
			flash_start_time = TickGet();
			LedSm = LED_WAIT_NORMAL;
		}
		break;
		case LED_WAIT_NORMAL:
		{
			if(TickGet() - flash_start_time >= TICK_SECOND*5) {
				LedSm = LED_ON_OFF;
			}
		}
		break;	
	    case LED_ON_OFF:
	    {
		    if(led_work_mode == LED_FLASH_MODE) {
			    LedSm = LED_FLASH_START;
			}
			SET_LED_ON(led_state);
		}
		break;
		case LED_FLASH_START:
		{
			SET_LED_ON(1);
			LedSm = LED_FLASH_WAIT_OFF;
			flash_start_time = TickGet();
		}
		break;
		case LED_FLASH_WAIT_OFF:
		{
			if(led_work_mode != LED_FLASH_MODE) {
				LedSm = LED_ON_OFF;
			} else if(TickGet() - flash_start_time >= (TICK_SECOND*led_flash_on_time/1000)) {
				SET_LED_ON(0);
				if(led_flash_times == 0) {
					flash_start_time = TickGet();
					LedSm = LED_FLASH_WAIT_ON;
				} else if(led_flash_times >= 1) {
					led_flash_times--;
					if(led_flash_times == 0) {
						led_work_mode = LED_WORK_ONOFF;
						LedSm = LED_ON_OFF;
						led_state = FALSE;
					}
					flash_start_time = TickGet();
					LedSm = LED_FLASH_WAIT_ON;
				}
			}
         }
		break;
		case LED_FLASH_WAIT_ON:
    		{
				if(led_work_mode != LED_FLASH_MODE) {
					LedSm = LED_ON_OFF;
				} else if(TickGet() - flash_start_time >= (TICK_SECOND*led_flash_off_time/1000)) {
        		    SET_LED_ON(1);
        		    flash_start_time = TickGet();
        		    LedSm = LED_FLASH_WAIT_OFF;
				}
			}
		break;
		default:
		    LedSm = LED_HOME;
		    break;
    }

}
