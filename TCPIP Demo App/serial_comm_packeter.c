#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Include all headers for any enabled TCPIP Stack functions
#include "TCPIP Stack/TCPIP.h"

#include "serial_comm_packeter.h"



#define STREAM_IDLE   0
#define STREAM_NORMAL 1
#define STREAM_IN_ESC 2


#define  STREAM_START        0x0F
#define  STREAM_END          0xF0
#define  STREAM_ESCAPE       0x55
#define  STREAM_ES_S         0x50   //转义字符 'S'
#define  STREAM_ES_E         0x05   //转义字符 'E'
//除了以上特殊字符外，其他都是原始字符




RX_PACKS_CTL_T  rx_ctl;
DATA_TX_CTL_T   tx_ctl;


typedef unsigned short   WORD;

#define GET_OFFSET_MEM_OF_STRUCT(type,member)   (&(((type *)0)->member) - ((type *)0))

#define LSB_BYTES_TO_WORD(bytes)            ((((WORD)((bytes)[1]))<<8)|(bytes)[0])
#define HSB_BYTES_TO_WORD(bytes)            ((((WORD)((bytes)[0]))<<8)|(bytes)[1])

unsigned int CRC16(unsigned char *Array,unsigned int Len)
{
	WORD  IX,IY,CRC;
	CRC=0xFFFF;//set all 1
	if (Len<=0) {
		CRC = 0;
	} else {
		Len--;
		for (IX=0;IX<=Len;IX++)
		{
			CRC=CRC^(WORD)(Array[IX]);
			for(IY=0;IY<=7;IY++) {
				if ((CRC&1)!=0) {
					CRC=(CRC>>1)^0xA001;
				} else {
					CRC=CRC>>1;
				}
			}
		}
	}
	return CRC;
}

unsigned int CRC162(unsigned char *Array,unsigned int Len)
{
	WORD  IX,IY,CRC;
	CRC=0xFFFF;//set all 1
	if (Len<=0) {
		CRC = 0;
	} else {
		Len--;
		for (IX=0;IX<=Len;IX++)
		{
			CRC=CRC^(WORD)(Array[IX]);
			for(IY=0;IY<=7;IY++) {
				if ((CRC&1)!=0) {
					CRC=(CRC>>1)^0xA001;
				} else {
					CRC=CRC>>1;
				}
			}
		}
	}
	return CRC;
}


void rx_find_next_empty_buffer(void)
{
	unsigned int i;
	rx_ctl.pcurrent_rx = NULL;
	for(i=0;i<RX_PACKS_MAX_NUM;i++) {
		if(!(rx_ctl.rx_packs[i].finished)) {
			rx_ctl.pcurrent_rx = &rx_ctl.rx_packs[i];
			break;
		}
	}
}


void pack_prase_in(unsigned char ch)
{
   DATA_RX_PACKET_T * prx;
   if(rx_ctl.pcurrent_rx == NULL) {
	   rx_find_next_empty_buffer();
	   if(rx_ctl.pcurrent_rx == NULL) {
		   return ;
	   }
   }
   prx = rx_ctl.pcurrent_rx;
   if(prx->finished) {
	   rx_find_next_empty_buffer();
	   prx = rx_ctl.pcurrent_rx;
	   if(prx == NULL) {
		   return ;
	   }
   }
   switch(prx->state)
   {
   case STREAM_IDLE:
     if(ch == STREAM_START) {
       prx->state = STREAM_NORMAL;
       prx->index = 0;
     }
     break;
   case STREAM_NORMAL:
     if(ch == STREAM_ESCAPE) {
       prx->state = STREAM_IN_ESC;
     } else if(ch == STREAM_START) {
       prx->state = STREAM_IDLE;
     } else if(ch == STREAM_END) {
       if(prx->index >= 3) {
         unsigned int crc = CRC16(prx->buffer,prx->index-2);
         if(crc == LSB_BYTES_TO_WORD(&prx->buffer[prx->index-2])) {
			 prx->index -= 2;
			 prx->look_up_times = 0;
             prx->finished = 1;
         }
       }
	   prx->state = STREAM_IDLE;
     } else {
       prx->buffer[prx->index++] = ch;
     }
     break;
   case STREAM_IN_ESC:
     if(ch == STREAM_ESCAPE) {
       prx->buffer[prx->index++] = STREAM_ESCAPE;
       prx->state = STREAM_NORMAL;
     } else if(ch == STREAM_ES_S) {
       prx->buffer[prx->index++] = STREAM_START;
       prx->state = STREAM_NORMAL;
     } else if(ch == STREAM_ES_E) {
       prx->buffer[prx->index++] = STREAM_END;
       prx->state = STREAM_NORMAL;
     } else {
       prx->state = STREAM_IDLE;
     }
     break;
   default:
     prx->state = STREAM_IDLE;
     break;
   }
   //溢出判断
   if(prx->index >= sizeof(prx->buffer)) {
     prx->state = STREAM_IDLE;
   }
}






/*************************************************
 * 找到空的发送缓冲，意思是找到可以填充数据的缓冲
 */
DATA_TX_PACKET_T * find_next_empty_tx_buffer(void)
{
	DATA_TX_PACKET_T * ptx = NULL;
	unsigned int i;
	for(i=0;i<TX_PACKS_MAX_NUM;i++) {
		if(!tx_ctl.packet[i].finished) {
			ptx = &tx_ctl.packet[i];
		}
	}
	return ptx;
}

/*************************************************
 * 找到已经填充完成的指针，然后用串口等等发送出去
 */
DATA_TX_PACKET_T * find_ready_tx_buffer(void)
{
	DATA_TX_PACKET_T * ptx = NULL;
	unsigned int i;
	for(i=0;i<TX_PACKS_MAX_NUM;i++) {
		if(tx_ctl.packet[i].finished) {
			ptx = &tx_ctl.packet[i];
		}
	}
	return ptx;
}



DATA_TX_PACKET_T * prase_in_buffer(unsigned char * src,unsigned int len)
{
	 DATA_TX_PACKET_T * ptx = NULL;
	 unsigned int i = 0;
	 if(len == 0) {
		 return NULL;
	 }
	 if((ptx = find_next_empty_tx_buffer()) == NULL) {
		 return NULL;
	 }
	 if(1) {
		 unsigned int crc = CRC16(src,len);
		 ptx->index = 0;
		 ptx->buffer[ptx->index++] = STREAM_START;
		 while(len--) {
			 unsigned char reg = src[i++];
			 if(ptx->index >= (sizeof(ptx->buffer) - 4)) {
				 //溢出判断,2字节CRC，一个结束位,一个转义字符
				 ptx->index = 0;
				 break;
			 }
			 if(reg == STREAM_START) {
				 ptx->buffer[ptx->index++] = STREAM_ESCAPE;
				 ptx->buffer[ptx->index++] = STREAM_ES_S;
		     } else if(reg == STREAM_ESCAPE) {
				 ptx->buffer[ptx->index++] = STREAM_ESCAPE;
				 ptx->buffer[ptx->index++] = STREAM_ESCAPE;
			 } else if(reg == STREAM_END) {
				 ptx->buffer[ptx->index++] = STREAM_ESCAPE;
				 ptx->buffer[ptx->index++] = STREAM_ES_E;
			 } else {
				 ptx->buffer[ptx->index++] = reg;
			 }
		 }
		 if(ptx->index > 0) {
		     ptx->buffer[ptx->index++] = crc & 0xFF;
		     ptx->buffer[ptx->index++] = crc >> 8;
		     ptx->buffer[ptx->index++] = STREAM_END;
		     ptx->finished = 1;
		 }
	 }
	 return ptx;
}



unsigned int tx_pack_and_send(unsigned char * src,unsigned int len)
{
	//立即发送，只有发完之后才能返回
	DATA_TX_PACKET_T * ptx = prase_in_buffer(src,len);
	if(ptx == NULL) {
		return 0;
	}
	if(ptx->finished) {
		//立即发送
        unsigned int i;
        for(i=0;i<ptx->index;i++) {
            //send_uart1(ptx->buffer[i]);
        }
        ptx->finished = 0;
	}
	return ptx->index;
}



unsigned int tx_pack_and_post(unsigned char * src,unsigned int len)
{
	//不发送
	DATA_TX_PACKET_T * ptx = prase_in_buffer(src,len);
    //启动发送中断
    if(ptx) {
        return (ptx->finished)?len:0;
    } else {
        return 0;
    }
}




DATA_RX_PACKET_T * GetFinishedPacket(void)
{
	unsigned int  i;
	DATA_RX_PACKET_T * prx;
	for(i=0;i<RX_PACKS_MAX_NUM;i++) {
		prx = &rx_ctl.rx_packs[i]; 
		if(prx->finished) {
			break;
		}
	}
	if(i == RX_PACKS_MAX_NUM) {
		return NULL;
	} else {
		return prx;
	}
}


void tx_free_useless_packet(unsigned int net_communication_count)
{
	unsigned int  i;
	DATA_RX_PACKET_T * prx;
	for(i=0;i<RX_PACKS_MAX_NUM;i++) {
		prx = &rx_ctl.rx_packs[i]; 
		if(prx->finished) {
			if(prx->look_up_times == net_communication_count) {
				prx->finished = 0; //所有人都看过了，结果没有人需要，则丢弃它。
			}
		}
	}
}









//-----------------------------------------------------------
// MODBUS读指令
//-----------------------------------------------------------


unsigned char modbus_read_multi_coils_request(unsigned int start_coils,unsigned int coils_num,unsigned char slave_device)
{
    struct modbus_read_coil_status_req_type
    {
		unsigned char slave_addr;
		unsigned char function;
		unsigned char start_addr_hi;
		unsigned char start_addr_lo;
		unsigned char reg_munber_hi;
		unsigned char reg_number_lo;
		unsigned char crc_hi;
		unsigned char crc_lo;
	};
	struct modbus_read_coil_status_req_type tx;
	unsigned int crc;
	tx.slave_addr = slave_device;
	tx.function = 0x01;  //READ_COIL_STATUS
	tx.start_addr_hi = start_coils >> 8;
	tx.start_addr_lo = start_coils & 0xFF;
	tx.reg_munber_hi = coils_num >> 8;
	tx.reg_number_lo = coils_num & 0xFF;
	crc = CRC16((unsigned char *)&tx,sizeof(tx)-2);
	tx.crc_hi = crc >> 8;
	tx.crc_lo = crc & 0xFF;
	return tx_pack_and_send((unsigned char *)&tx,sizeof(tx));
}



unsigned int modbus_prase_read_multi_coils_ack(unsigned char slave_device,unsigned char * rx_buffer,unsigned int len,unsigned int startbit,unsigned int count)
{
	typedef struct __modbus_force_multiple_coils_req_type
	{
		unsigned char slave_addr;
		unsigned char function;
		unsigned char start_addr_hi;
		unsigned char start_addr_lo;
		unsigned char quantiry_coils_hi;
		unsigned char quantiry_coils_lo;
		unsigned char byte_count;
		unsigned char database;
	} modbus_force_multiple_coils_req_type;
	modbus_force_multiple_coils_req_type * pack = (modbus_force_multiple_coils_req_type *)rx_buffer;
	if(len > sizeof(modbus_force_multiple_coils_req_type) && pack->slave_addr == slave_device) {
		if(pack->function == 0x01) {  //READ_COIL_STATUS
			if(startbit == HSB_BYTES_TO_WORD(&pack->start_addr_hi)) {
				unsigned int count = HSB_BYTES_TO_WORD(&pack->start_addr_hi);
				if(count > 0) {
					//根据读到的数据，修改某些寄存器的位
					return count;
				}
			}
		}
	}
	return 0;
}

