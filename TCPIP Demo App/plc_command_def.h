#ifndef __PLC_COMMAND_H__
#define __PLC_COMMAND_H__

//指令系统分为基本指令，和数据处理指令
//指令长度不定，但是指令的第一个头字节表示指令编码
//随后的数据代表指令数据
//例如:LD指令
//LD  X001
//LD  X3388
//LD  X7899
//这些指令，它的数据至少16位才能表示输入的索引值，所以数据位有2个字节

#define    PLC_NONE    0     //无效指令，或空指令
#define    PLC_LD      1     //加载指令
#define    PLC_LDI     2
#define    PLC_OUT     3     //输出指令
#define    PLC_AND     4
#define    PLC_ANI     5
#define    PLC_OR      6
#define    PLC_ORI     7
#define    PLC_LDP     8
#define    PLC_LDF     9
#define    PLC_ANDP    10
#define    PLC_ANDF    11
#define    PLC_ORP     12
#define    PLC_ORF     13
#define    PLC_MPS     14
#define    PLC_MRD     15
#define    PLC_MPP     16
#define    PLC_SET     17
#define    PLC_RST     18
#define    PLC_INV     19
//自定义指令
#define    PLC_OUTT    20   //输出到定时器
#define    PLC_OUTC    21   //输出都计数器

#define    PLC_END     0xFF  //结束指令


struct TIME_COUNT_TYPE
{
	WORD   time_count;
};
//定时器指令操作码
typedef struct _TIME_OP
{
	WORD index;
	WORD kval;
} TIME_OP;

#endif


