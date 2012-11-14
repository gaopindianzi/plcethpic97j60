#ifndef __DEVICE_INFO_H__
#define __DEVICE_INFO_H__

typedef struct _DeviceInfo;
{
	unsigned char id[4];
	unsigned char mac[6];
	unsigned char type;
	unsigned char health_status;
	unsigned char report_time[8]; //uint64
	unsigned char last_event;
	unsigned char dev_ipaddr[4];
	unsigned char dev_netmask[4];
	unsigned char dev_gateway[4];
	unsigned char info_host_addr[64];
	unsigned char info_host_port[2];
	unsigned char info_status_addr[64];
	unsigned char infO_status_port[2];
	unsigned char version[6];
	unsigned char 
} DeviceInfo;

typedef struct _DeviceStatus;
{
	unsigned char type;
} DeviceStatus;



#endif

