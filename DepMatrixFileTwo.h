/************************************************* 

File name:		DepMatrixFileTwo.h 


Function List:
	1��int DMDT_ReadFile(IN const char* fileName,OUT CDMDTStruct* pDMDT)
	2��int DMDT_WriteFile(IN const char* fileName,IN CDMDTStruct* pDMDT)
	3��void DMDT_Init(IN CDMDTStruct* pDMDT)
	4��void DMDT_MemoryFree(IN CDMDTStruct* pDMDT)
	5��void DMDT_RangeCheck(IN CDMDTStruct* pDMDT);	//�ļ�������Ч��Χ�淶��

History:          
1. Date: 
Author: 
Modification: 
2. ... 
*************************************************/
#ifndef DEPMATRIXFILE_HEAD
#define DEPMATRIXFILE_HEAD

#ifndef IN
#define IN		//��ǲ���Ϊ����
#endif
#ifndef OUT
#define OUT		//��ǲ���Ϊ���
#endif
#define AE 255  //���������β

#ifdef __cplusplus
extern "C"{
#endif

//������������
typedef struct CDMDTStruct__
{
	int		row;			//����
	int		col;			//����
	int*	pMatrix;		//����Դ-������������
	double*	pProb;			//���ϸ���
	double*	pWeight;		//�ɱ�ʱ��Ȩֵ
	int*    pLevel;         //���ϵȼ�
	double*	pCost;			//���Գɱ�
}CDMDTStruct;

//��*.DMD��*.DMDT(DepMatrixData)�ļ�
//�����ڲ�������new����pDMDT��Ա����Ŀռ䣬ʹ�������pDMDT�󣬿��Ե���DMDT_MemoryFree()�ͷ�pDMDT��Ա������ڴ�
//����ֵ��	0	//���гɹ�
//			-1	//���������Ч
//			-2	//���ļ�ʧ��
//			-3	//����������ȷ
int DMDT_ReadFile(IN const char* fileName, IN int* Tselect,OUT CDMDTStruct* pDMDT);



//��ʼ��pDMDT�ĳ�Ա����
void DMDT_Init(IN CDMDTStruct* pDMDT);


//�ͷ�pDMDT��Ա������ڴ�
void DMDT_MemoryFree(IN CDMDTStruct* pDMDT);


#ifdef __cplusplus
	};
#endif

#endif	//end of DEPMATRIXFILE_HEAD
