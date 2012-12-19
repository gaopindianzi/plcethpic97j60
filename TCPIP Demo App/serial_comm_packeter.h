#ifndef __SERIAL_COMM_PACKER_H__
#define __SERIAL_COMM_PACKER_H__



#define  PACK_MAX_RX_SIZE   32
#define  RX_PACKS_MAX_NUM   2
#define  TX_PACKS_MAX_NUM   2


typedef struct _DATA_RX_PACKET_T
{
  unsigned char buffer[PACK_MAX_RX_SIZE];
  unsigned int  look_up_times;  //别查看次数
  unsigned char index;
  unsigned char state : 4;
  unsigned char finished : 1;
} DATA_RX_PACKET_T;



typedef struct _RX_PACKS_CTL_T
{
	DATA_RX_PACKET_T   rx_packs[RX_PACKS_MAX_NUM];
	DATA_RX_PACKET_T * pcurrent_rx;
} RX_PACKS_CTL_T;





typedef struct _DATA_TX_PACKET_T
{
  unsigned char buffer[PACK_MAX_RX_SIZE + PACK_MAX_RX_SIZE/2];
  unsigned char index;  //发送的大小
  unsigned char finished : 1;
} DATA_TX_PACKET_T;

typedef struct _DATA_TX_CTL_T
{
	DATA_TX_PACKET_T   packet[TX_PACKS_MAX_NUM];
	DATA_TX_PACKET_T * ptx_sending;
} DATA_TX_CTL_T;


extern RX_PACKS_CTL_T rx_ctl;
extern DATA_TX_CTL_T  tx_ctl;


extern void pack_prase_in(unsigned char ch);
extern DATA_RX_PACKET_T * GetFinishedPacket(void);
extern DATA_TX_PACKET_T * find_ready_tx_buffer(void);
extern DATA_TX_PACKET_T * prase_in_buffer(unsigned char * src,unsigned int len);
extern void tx_free_useless_packet(unsigned int net_communication_count);
extern unsigned int tx_pack_and_send(unsigned char * src,unsigned int len);

extern unsigned int modbus_prase_read_multi_coils_ack(unsigned char slave_device,unsigned char * rx_buffer,unsigned int len,unsigned int startbit,unsigned int count);
extern unsigned char modbus_read_multi_coils_request(unsigned int start_coils,unsigned int coils_num,unsigned char slave_device);


#endif
