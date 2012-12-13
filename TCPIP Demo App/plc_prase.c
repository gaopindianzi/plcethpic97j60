/************************************************************
 * 这个文件处理脚本，实现一些标准的PLC协议，当然也会实现自己的一些特殊协议
 * 目前这是处于尝试阶段
 */

#include "TCPIP Stack/TCPIP.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "tcp_cmd_prase_handle.h"
#include "relay_cmd_definition.h"
#include "hal_io_interface.h"
#include "plc_command_def.h"
#include "plc_prase.h"


#define  TIMING100MS_BASE   0
#define  TIMING100MS_MAX         20
#define  TIMING1S_BASE      20
#define  TIMING1S_MAX            20
#define  COUNT_BASE         40
#define  COUNTER_MAX             60


#define  IO_INPUT_BASE          0
#define  IO_INPUT_COUNT             16
#define  IO_OUTPUT_BASE         16
#define  IO_OUTPUT_COUNT            16
#define  AUXI_RELAY_BASE        32
#define  AUXI_RELAY_COUNT           200
#define  TIMING100MS_EVENT_BASE 232
#define  TIMING100MS_EVENT_COUNT    20
#define  TIMING1S_EVENT_BASE    252
#define  TIMING1S_EVENT_COUNT       20
//

//输入口
unsigned int  input_num;
unsigned char inputs_new[BITS_TO_BS(IO_INPUT_COUNT)];
unsigned char inputs_last[BITS_TO_BS(IO_INPUT_COUNT)];
//输出继电器
unsigned int  output_num;
unsigned char output_last[BITS_TO_BS(IO_OUTPUT_COUNT)];
unsigned char output_new[BITS_TO_BS(IO_OUTPUT_COUNT)];
//辅助继电器
unsigned char auxi_relays[BITS_TO_BS(AUXI_RELAY_COUNT)];
unsigned char auxi_relays_last[BITS_TO_BS(AUXI_RELAY_COUNT)];
//定时器定义，自动对内部的时钟脉冲进行计数
unsigned int  time100ms_come_flag;
unsigned int  time1s_come_flag;
struct COUNT_TYPE  timing100ms[TIMING100MS_MAX];
unsigned char timeing100ms_event_last[BITS_TO_BS(TIMING100MS_MAX)];
unsigned char timeing100ms_event[BITS_TO_BS(TIMING100MS_MAX)];
struct COUNT_TYPE  timing1s[TIMING1S_MAX];
unsigned char timing1s_event_last[BITS_TO_BS(TIMING1S_MAX)];
unsigned char timing1s_event[BITS_TO_BS(TIMING1S_MAX)];
//计数器定义
struct COUNT_TYPE       counter[COUNTER_MAX];
unsigned char           counter_event[BITS_TO_BS(COUNTER_MAX)];
unsigned char           counter_event_last[BITS_TO_BS(COUNTER_MAX)];
//运算器的寄存器
#define  BIT_STACK_LEVEL     32
unsigned char  bit_acc;
unsigned char  bit_stack[BITS_TO_BS(BIT_STACK_LEVEL)];   //比特堆栈，PLC的位运算结果压栈在这里，总共有32层栈
unsigned char  bit_stack_sp;   //比特堆栈的指针

//指令编码
unsigned long plc_command_index;     //当前指令索引，
unsigned char plc_command_array[16]; //当前指令字节编码
#define       PLC_CODE     (plc_command_array[0])


/*************************************************
 * 以下是私有的实现
 */
//内部用系统计数器
static TICK  last_tick;
static TICK  last_tick1s;
static void sys_time_tick_init(void)
{
	last_tick = TickGet();
	last_tick1s = TickGet();
}
static unsigned long sys_time_tick_process(void)
{
	TICK curr = TickGet();
	if((curr - last_tick) >= TICK_SECOND / 10) {
		time100ms_come_flag++;
		last_tick = curr;
	}
	if((curr - last_tick1s) >= TICK_SECOND) {
		time1s_come_flag++;
		last_tick1s = curr;
	}
}




/*********************************************
 * 系统初始化
 */

void PlcInit(void)
{
	bit_acc = 0;
	memset(bit_stack,0,sizeof(bit_stack));
	bit_stack_sp = 0;
	input_num = io_in_get_bits(0,inputs_new,IO_INPUT_COUNT);
	output_num = io_out_get_bits(0,output_last,IO_OUTPUT_COUNT);
	memcpy(inputs_last,inputs_new,sizeof(inputs_new));
	plc_command_index = 0;
    //memset(timing100ms,0,sizeof(timing100ms));
	//memset(counter,0,sizeof(counter));
	memset(output_last,0,sizeof(output_last));
	sys_time_tick_init();
	time100ms_come_flag = 0;
	time1s_come_flag = 0;
	//memset(timeing100ms_event,0,sizeof(timeing100ms_event));
	//memset(timeing1s_event,0,sizeof(timeing1s_event));
}



/**********************************************
 *  获取下一条指令的指令码
 *  也许是从EEPROM中读取的程序脚本
 *  这里一次性读取下一个指令，长度为最长指令长度
 */

void read_next_plc_code(void)
{
	//read_plc_programer(plc_command_index,plc_command_array,sizeof(plc_command_array));
}

void handle_plc_command_error(void)
{
	//提示第几条指令出错
	//然后复位，或停止运行
	plc_command_index  = 0;
}

static unsigned char get_bitval(unsigned int index)
{
	unsigned char bitval = 0;
	unsigned char B,b;
	if(index >= IO_INPUT_BASE && index < (IO_INPUT_BASE+IO_INPUT_COUNT)) {
		index -= IO_INPUT_BASE;
	    B = index / 8;
	    b = index % 8;
		bitval = (inputs_new[B]&code_msk[b])?1:0;
	} else if(index >= IO_OUTPUT_BASE && index < (IO_OUTPUT_BASE+IO_OUTPUT_COUNT)) {
		index -= IO_OUTPUT_BASE;
		B = index / 8;
	    b = index % 8;
		bitval = (output_new[B]&code_msk[b])?1:0;
	} else if(index >= AUXI_RELAY_BASE && index < (AUXI_RELAY_BASE + AUXI_RELAY_COUNT)) {
		index -= AUXI_RELAY_BASE;
	    B = index / 8;
	    b = index % 8;
		bitval = (auxi_relays[B]&code_msk[b])?1:0;
	} else if(index >= TIMING100MS_EVENT_BASE && index < (TIMING100MS_EVENT_BASE+TIMING100MS_EVENT_COUNT)) {
		index -= TIMING100MS_EVENT_BASE;
	    B = index / 8;
	    b = index % 8;
		bitval = (timeing100ms_event[B]&code_msk[b])?1:0;
	} else if(index >= TIMING1S_EVENT_BASE && index < (TIMING1S_EVENT_BASE + TIMING1S_EVENT_COUNT)) {
		index -= TIMING1S_EVENT_BASE;
	    B = index / 8;
	    b = index % 8;
		bitval = (timing1s_event[B]&code_msk[b])?1:0;
	}
	return bitval;
}
static unsigned char get_last_bitval(unsigned int index)
{
	unsigned char bitval = 0;
	unsigned char B,b;
	if(index >= IO_INPUT_BASE && index < (IO_INPUT_BASE+IO_INPUT_COUNT)) {
		index -= IO_INPUT_BASE;
	    B = index / 8;
	    b = index % 8;
		bitval = (inputs_last[B]&code_msk[b])?1:0;
	} else if(index >= IO_OUTPUT_BASE && index < (IO_OUTPUT_BASE+IO_OUTPUT_COUNT)) {
		index -= IO_OUTPUT_BASE;
		B = index / 8;
	    b = index % 8;
		bitval = (output_last[B]&code_msk[b])?1:0;
	} else if(index >= AUXI_RELAY_BASE && index < (AUXI_RELAY_BASE + AUXI_RELAY_COUNT)) {
		index -= AUXI_RELAY_BASE;
	    B = index / 8;
	    b = index % 8;
		bitval = (auxi_relays_last[B]&code_msk[b])?1:0;
	} else if(index >= TIMING100MS_EVENT_BASE && index < (TIMING100MS_EVENT_BASE+TIMING100MS_EVENT_COUNT)) {
		index -= TIMING100MS_EVENT_BASE;
	    B = index / 8;
	    b = index % 8;
		bitval = (timeing100ms_event_last[B]&code_msk[b])?1:0;
	} else if(index >= TIMING1S_EVENT_BASE && index < (TIMING1S_EVENT_BASE + TIMING1S_EVENT_COUNT)) {
		index -= TIMING1S_EVENT_BASE;
	    B = index / 8;
	    b = index % 8;
		bitval = (timing1s_event_last[B]&code_msk[b])?1:0;
	}
	return bitval;
}
static void set_bitval(unsigned int index,unsigned char bitval)
{
	unsigned char B,b;
	if(index >= IO_INPUT_BASE && index < (IO_INPUT_BASE+IO_INPUT_COUNT)) {
		//输入值不能修改
	} else if(index >= IO_OUTPUT_BASE && index < (IO_OUTPUT_BASE+IO_OUTPUT_COUNT)) {
		index -= IO_OUTPUT_BASE;
	    B = index / 8;
	    b = index % 8;
		if(bitval) {
			output_new[B] |=  code_msk[b];
		} else {
			output_new[B] &= ~code_msk[b];
		}
	} else if(index >= AUXI_RELAY_BASE && index < (AUXI_RELAY_BASE + AUXI_RELAY_COUNT)) {
		index -= IO_OUTPUT_BASE;
	    B = index / 8;
	    b = index % 8;
		if(bitval) {
			auxi_relays[B] |=  code_msk[b];
		} else {
			auxi_relays[B] &= ~code_msk[b];
		}
	}
}
/**********************************************
 * 根据条件对计时器进行增加
 */
static unsigned char timing_cell_prcess(void)
{
#if 0
	unsigned int i;
	unsigned char B,b;
	if(time100ms_come_flag) {
	    for(i=0;i<TIMING100MS_MAX;i++) {
		    WORD time = timing100ms[i].count;
			if(time > 0) {
		        if(time > time100ms_come_flag) { //最高位为允许计时位
				    time -= time100ms_come_flag;
			    } else {
				    time = 0; //时间到触发事件
			    }
				if(time == 0) {
				    B = i/8;
				    b = i%8;
					timeing100ms_event[B] |= code_msk[b];
				}
				timing100ms[i].count = time;
			}
	    }
		time100ms_come_flag = 0;
	}
	if(time1s_come_flag) {
		for(i=0;i<TIMING1S_MAX;i++) {
			WORD time = timing1s[i].count;
			if(time > 0) {
				if(time > time1s_come_flag) {
					time -= time1s_come_flag;
				} else {
					time = 0;
				}
				if(time == 0) {
					B = i/8;
					b = i%8;
					timing1s_event[B] |= code_msk[b];
				}
				timing1s[i].count = time;
			}
		}
		time1s_come_flag = 0;
	}
#endif
}
/**********************************************
 * 打开定时器，并设定触发时间的最大值
 */
static void timing_cell_start(unsigned int index,unsigned int event_count)
{
#if 0
	unsigned char B,b;
	if(index >= TIMING100MS_BASE && index < (TIMING100MS_BASE+TIMING100MS_MAX)) {
		index -= TIMING100MS_BASE;
	    B = index/8;
	    b = index%8;
		timing100ms[index].count = event_count;
		timeing100ms_event[B] &= ~code_msk[b];
	} else if(index >= TIMING1S_BASE && index < (TIMING1S_BASE+TIMING1S_MAX)) {
		index -= TIMING1S_BASE;
	    B = index/8;
	    b = index%8;
		timing1s[index].count = event_count;
		timing1s_event[B] &= ~code_msk[b];
	}
#endif
}

/**********************************************
 * 关闭定时器，并取消触发事件
 */
static void timing_cell_stop(unsigned int index)
{
#if 0
	unsigned char B,b;
	if(index >= TIMING100MS_BASE && index < (TIMING100MS_BASE+TIMING100MS_MAX)) {
		index -= TIMING100MS_BASE;
	    B = index/8;
	    b = index%8;
		timing100ms[index].count = 0;
		timeing100ms_event[B] &= ~code_msk[b];
	} else if(index >= TIMING1S_BASE && index < (TIMING1S_BASE+TIMING1S_MAX)) {
		index -= TIMING1S_BASE;
	    B = index/8;
	    b = index%8;
		timing1s[index].count = 0;
		timing1s_event[B] &= ~code_msk[b];
	}
#endif
}

/**********************************************
 * 加载输入端口的输入值
 */
void handle_plc_ld(void)
{
	unsigned int bit_index = plc_command_array[2];
	bit_index <<= 8;
	bit_index |= plc_command_array[1];
	bit_acc = get_bitval(bit_index);
	if(PLC_CODE == PLC_LDI) {
		bit_acc = !bit_acc;
	}
	plc_command_index += 3;
}
/**********************************************
 * 把位运算的结果输出到输出端口中
 */
void handle_plc_out(void)
{
	unsigned char B,b;
	unsigned int bit_index = plc_command_array[2];
	bit_index <<= 8;
	bit_index |= plc_command_array[1];
	set_bitval(bit_index,bit_acc);
	plc_command_index += 3;
}

/**********************************************
 * 与或与非运算
 */
void handle_plc_and_ani(void)
{
	unsigned char bittmp;
	unsigned int bit_index = plc_command_array[2];
	bit_index <<= 8;
	bit_index |= plc_command_array[1];
	bittmp = get_bitval(bit_index);
	if(PLC_CODE == PLC_AND) {
	    bit_acc = bit_acc && bittmp;
	} else {
		bit_acc = bit_acc && (!bittmp);
	}
	plc_command_index += 3;
}

/**********************************************
 * 或或或非运算
 */
void handle_plc_or_ori(void)
{
	unsigned char bittmp;
	unsigned int bit_index = plc_command_array[2];
	bit_index <<= 8;
	bit_index |= plc_command_array[1];
	bittmp = get_bitval(bit_index);
	if(PLC_CODE == PLC_OR) {
	    bit_acc = bit_acc || bittmp;
	} else if(PLC_CODE == PLC_ORI) {
		bit_acc = bit_acc || (!bittmp);
	} else {
		handle_plc_command_error();
		return ;
	}
	plc_command_index += 3;
}

/**********************************************
 * 加载输入端上升沿或下降沿
 */
void handle_plc_ldp_ldf(void)
{
	unsigned char reg;
	unsigned int bit_index = plc_command_array[2];
	bit_index <<= 8;
	bit_index |= plc_command_array[1];
	reg =  get_last_bitval(bit_index)?0x01:0x00;
	reg |= get_bitval(bit_index)?     0x02:0x00;
	if(PLC_CODE == PLC_LDP) {
		if(reg == 0x02) {
			bit_acc = 1;
		} else {
			bit_acc = 0;
		}
	} else if(PLC_CODE == PLC_LDF) {
		if(reg == 0x01) {
		    bit_acc = 1;
		} else {
			bit_acc = 0;
		}
	} else {
		handle_plc_command_error();
		return ;
	}
	plc_command_index += 3;
}


/**********************************************
 * 与上升沿或下降沿
 */
void handle_plc_andp_andf(void)
{
	unsigned char reg;
	unsigned char B,b;
	unsigned int bit_index = plc_command_array[2];
	bit_index <<= 8;
	bit_index |= plc_command_array[1];
	reg =  get_last_bitval(bit_index)?0x01:0x00;
	reg |= get_bitval(bit_index)?     0x02:0x00;
	if(PLC_CODE == PLC_ANDP) {
		if(reg == 0x02) {
			bit_acc = bit_acc && 1;
		} else {
			bit_acc = 0;
		}
	} else if(PLC_CODE == PLC_ANDF) {
		if(reg == 0x01) {
		    bit_acc = bit_acc && 1;
		} else {
			bit_acc = 0;
		}
	} else {
		handle_plc_command_error();
		return ;
	}
	plc_command_index += 3;
}
/**********************************************
 * 或上升沿或下降沿
 */
void handle_plc_orp_orf(void)
{
	unsigned char reg;
	unsigned char B,b;
	unsigned int bit_index = plc_command_array[2];
	bit_index <<= 8;
	bit_index |= plc_command_array[1];
	reg =  get_last_bitval(bit_index)?0x01:0x00;
	reg |= get_bitval(bit_index)?     0x02:0x00;
	if(PLC_CODE == PLC_ORP) {
		if(reg == 0x02) {
			bit_acc = bit_acc || 1;
		} else {
			bit_acc = bit_acc || 0;
		}
	} else if(PLC_CODE == PLC_ORF) {
		if(reg == 0x01) {
		    bit_acc = bit_acc || 1;
		} else {
			bit_acc = bit_acc || 0;
		}
	} else {
		handle_plc_command_error();
		return ;
	}
	plc_command_index += 3;
}

/**********************************************
 * 压栈、出栈、读栈
 */
void handle_plc_mps_mrd_mpp(void)
{
	unsigned char B,b;
	B = bit_stack_sp / 8;
	b = bit_stack_sp % 8;
	if(PLC_CODE == PLC_MPS) {
		if(bit_stack_sp >= BIT_STACK_LEVEL) {
		    handle_plc_command_error();
		    return ;
		}
		if(bit_acc) {
			bit_stack[B] |=  code_msk[b];
		} else {
			bit_stack[B] &= ~code_msk[b];
		}
		bit_stack_sp++;
	} else if(PLC_CODE == PLC_MRD) {
		bit_acc = (bit_stack[B] & code_msk[b])?1:0;
	} else if(PLC_CODE == PLC_MPP) {
		bit_acc = (bit_stack[B] & code_msk[b])?1:0;
		bit_stack_sp--;
	}
	plc_command_index += 1;
}
/**********************************************
 * 锁或解锁指令
 */
void handle_plc_set_rst(void)
{
	unsigned char B,b;
	unsigned int bit_index = plc_command_array[2];
	bit_index <<= 8;
	bit_index |= plc_command_array[1];
	if(PLC_CODE == PLC_SET) {
	    if(bit_acc) {
			set_bitval(bit_index,1);
	    }
	} else if(PLC_CODE == PLC_RST) {
	    if(bit_acc) {
			set_bitval(bit_index,0);
	    }
	} else {
		handle_plc_command_error();
		return ;
	}
	plc_command_index += 3;
}
/**********************************************
 * 取反结果
 */
void handle_plc_inv(void)
{
	bit_acc = !bit_acc;
	plc_command_index += 1;
}

/**********************************************
 * 输出到定时器
 * 根据编号，可能输出到100ms定时器，也可能输出到1s定时器
 */
void handle_plc_out_t(void)
{
	unsigned char B,b;
	unsigned int kval;
	unsigned int time_index;
	time_index = plc_command_array[2];
    time_index <<= 8;
	time_index |= plc_command_array[1];
	if(bit_acc) {
	    kval = plc_command_array[4];
	    kval <<= 8;
	    kval |= plc_command_array[3];
	    timing_cell_start(time_index,kval);
	} else {
		timing_cell_stop(time_index);
	}
	plc_command_index += 5;
}
/**********************************************
 * 输出到计数器
 * 根据编号，可能输出到100ms定时器，也可能输出到1s定时器
 */
void handle_plc_out_c(void)
{
#if 0
	unsigned char B,b;
	unsigned int kval;
	unsigned int index;
	index = plc_command_array[2];
    index <<= 8;
	index |= plc_command_array[1];
	if(bit_acc) {
	    kval = plc_command_array[4];
	    kval <<= 8;
	    kval |= plc_command_array[3];
		if(index >= COUNTER_MAX) {
		    handle_plc_command_error();
		    return ;
		}
		B = index / 8;
		b = index % 8;
		counter[index].count = kval;
		counter_event[B] |= code_msk[b];
		if(++counter[index].count > kval) {
			counter_event[B] |= code_msk[b];
		}
	} else {
		counter[index].count = 0;
		counter_event[B] &= ~code_msk[b];
	}
	plc_command_index += 5;
#endif
}

void PlcProcess(void)
{
	//输入处理,读取IO口的输入
	io_in_get_bits(0,inputs_new,IO_INPUT_COUNT);
	read_next_plc_code();
	//逻辑运算,调用一次，运行一次用户的程序
	switch(PLC_CODE)
	{
	case PLC_END: //指令结束，从头开始
		plc_command_index = 0;
		return 0;
	case PLC_LD: 
	case PLC_LDI:
		handle_plc_ld();
		break;
	case PLC_OUT:
		handle_plc_out();
		break;
	case PLC_AND:
	case PLC_ANI:
		handle_plc_and_ani();
		break;
	case PLC_OR:
	case PLC_ORI:
		handle_plc_or_ori();
		break;
	case PLC_LDP:
	case PLC_LDF:
		handle_plc_ldp_ldf();
		break;
	case PLC_ANDP:
	case PLC_ANDF:
		handle_plc_andp_andf();
		break;
	case PLC_ORP:
	case PLC_ORF:
		handle_plc_orp_orf();
		break;
	case PLC_MPS:
	case PLC_MRD:
	case PLC_MPP:
		handle_plc_mps_mrd_mpp();
		break;
	case PLC_SET:
	case PLC_RST:
		handle_plc_set_rst();
		break;
	case PLC_INV:
		handle_plc_inv();
		break;
	case PLC_OUTT:
		handle_plc_out_t();
		break;
	case PLC_OUTC:
		handle_plc_out_c();
		break;
	default:
	case PLC_NONE: //空指令，直接跳过
		return ;
	}
	//输出处理，把运算结果输出到继电器中
	io_out_set_bits(0,output_last,IO_OUTPUT_COUNT);
	//后续处理
	memcpy(inputs_last,inputs_new,sizeof(inputs_new));
	//系统时间处理
	sys_time_tick_process();
	//定时器处理
	timing_cell_prcess();
	//计数器处理
}

