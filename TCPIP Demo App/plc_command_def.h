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
#define    PLC_LDKH    2
#define    PLC_LDI     3
#define    PLC_OUT     4     //���ָ��
#define    PLC_AND     5
#define    PLC_ANI     6
#define    PLC_OR      7
#define    PLC_ORI     8
#define    PLC_LDP     9
#define    PLC_LDF     10
#define    PLC_ANDP    11
#define    PLC_ANDF    12
#define    PLC_ORP     13
#define    PLC_ORF     14
#define    PLC_MPS     15
#define    PLC_MRD     16
#define    PLC_MPP     17
#define    PLC_SET     18
#define    PLC_RST     19
#define    PLC_INV     20
//�Զ���ָ��
#define    PLC_OUTT    21   //�������ʱ��
#define    PLC_OUTC    22   //�����������
//����ָ��
#define    PLC_ZRST    23
#define    PLC_BCMP    24
#define    PLC_BCMPE   25
#define    PLC_BCMPL   26
#define    PLC_BCMPB   27
#define    PLC_BACMPL  28
#define    PLC_BACMPE  29
#define    PLC_BACMPB  30
#define    PLC_BZCP    31
#define    PLC_BZCPS   32
#define    PLC_BAZCP   33
#define    PLC_MOV     34
#define    PLC_CML     35
#define    PLC_BMOV    36
#define    PLC_FMOV    37

#define    PLC_NETRB   38
#define    PLC_NETWB   39
#define    PLC_NETRW   40
#define    PLC_NETWW   41

#define    PLC_LDKL    42
#define    PLC_SEI     43 //�����룬��ȡ�� //2B

#define    PLC_JMP     44  //��������ת
#define    PLC_JMPS    45  //��������ת




#define    PLC_END     0xFF  //����ָ��







#endif


