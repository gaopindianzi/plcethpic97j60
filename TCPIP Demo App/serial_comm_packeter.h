#ifndef __SERIAL_COMM_PACKER_H__
#define __SERIAL_COMM_PACKER_H__


#define  PACK_MAX_RX_SIZE   32


typedef struct _DATA_RX_PACKET_T
{
  unsigned char buffer[PACK_MAX_RX_SIZE];
  unsigned char index;
  unsigned char state : 4;
  unsigned char finished : 1;
} DATA_RX_PACKET_T;


typedef struct _DATA_TX_PACKET_T
{
  unsigned char buffer[PACK_MAX_RX_SIZE*2+2];
  unsigned char index;
  unsigned char finished : 1;
} DATA_TX_PACKET_T;


//extern DATA_RX_PACKET_T rx_pack;
//extern DATA_TX_PACKET_T tx_pack;

//extern unsigned int prase_in_buffer(unsigned char * src,unsigned int len);
//extern void pack_prase_in(unsigned char ch);

#endif
