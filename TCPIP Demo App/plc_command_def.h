#ifndef __PLC_COMMAND_H__
#define __PLC_COMMAND_H__

//ָ��ϵͳ��Ϊ����ָ������ݴ���ָ��
//ָ��Ȳ���������ָ��ĵ�һ��ͷ�ֽڱ�ʾָ�����
//�������ݴ���ָ������
//����:LDָ��
//LD  X001
//LD  X3388
//LD  X7899
//��Щָ�������������16λ���ܱ�ʾ���������ֵ����������λ��2���ֽ�

#define    PLC_NONE    0     //��Чָ����ָ��
#define    PLC_LD      1     //����ָ��
#define    PLC_LDI     2
#define    PLC_OUT     3     //���ָ��
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
//�Զ���ָ��
#define    PLC_OUTT    20   //�������ʱ��
#define    PLC_OUTC    21   //�����������

#define    PLC_END     0xFF  //����ָ��


struct TIME_COUNT_TYPE
{
	WORD   time_count;
};
//��ʱ��ָ�������
typedef struct _TIME_OP
{
	WORD index;
	WORD kval;
} TIME_OP;

#endif


