
#include "StdAfx.h"
#include <stdio.h>
#include "DepMatrixFileTwo.h"

#ifndef NULL
#define NULL	((void*)0)
#endif

#ifdef __cplusplus
extern "C"{
#endif

//��*.DMDT(DepMatrixData)�ļ�
//�����ڲ�������new����pDMDT��Ա����Ŀռ䣬ʹ�������pDMDT�󣬿��Ե���DMDT_MemoryFree()�ͷ�pDMDT��Ա������ڴ�
//����ֵ��	0	//���гɹ�
//			-1	//���������Ч
//			-2	//���ļ�ʧ��

int DMDT_ReadFile(IN const char* fileName, IN int* Tselect,OUT CDMDTStruct* pDMDT)
{
	int row = 0;
	int col = 0;
	int TselectLen = 0;
	int* pMatrix = NULL;
	int* pMatrixtemp = NULL;
	double* pProb = NULL;
	double* pWeight = NULL;
	int*    pLevel = NULL;
	double* pCost = NULL;
	double* pCosttemp = NULL;

	int i,j;
	FILE* pFile=NULL;

	if(fileName==NULL || pDMDT==NULL || Tselect==NULL)
		return -1;			//���������Ч

	pFile=fopen(fileName,"r");
	if (pFile==NULL)
	{
		return -2;			//���ļ�ʧ��
	}

	//������
	fscanf(pFile, "%d", &row);
	fscanf(pFile, "%d", &col);
	pMatrix = new int[row*col];
	pMatrixtemp = new int[row*col];
	pProb=new double[col];
	pWeight = new double[col];
	pLevel = new int[col];
	pCost=new double[row];
	pCosttemp = new double[row];

	
	for (i=0;i<row;i++)
	{
		for (j=0;j<col;j++)
		{
			fscanf(pFile,"%d",&pMatrixtemp[i*col + j]);//��������
		}
		
		fscanf(pFile, "%lf", &pCosttemp[i]);         //���Գɱ�
	}

	for (i = 0; i < col; i++)
	{
		fscanf(pFile, "%lf", &pProb[i]);             //���ϸ���
	}
	for (i = 0; i < col; i++)
	{
		fscanf(pFile, "%lf", &pWeight[i]);           //����Ȩֵ
	}
	for (i = 0; i < col; i++)
	{
		fscanf(pFile, "%d", &pLevel[i]);             //���ϵȼ�
	}
	fclose(pFile);

	//����ѡȡ�Ĳ��Ը���
	while (Tselect[TselectLen] != AE)
	{
		TselectLen++;
	}
	
	//����ѡȡ�����������ת��
	for(i = 0; i < col; i++)
	{ 
		for (j = 0; j < TselectLen; j++) 
		{
			pMatrix[i*TselectLen + j] = pMatrixtemp[(Tselect[j] - 1)*col + i];
			pCost[j] = pCosttemp[Tselect[j] - 1];
		}

	}

	pDMDT->row = col;
	pDMDT->col = TselectLen;
	pDMDT->pMatrix = pMatrix;
	pDMDT->pProb = pProb;
	pDMDT->pWeight = pWeight;
	pDMDT->pLevel = pLevel;
	pDMDT->pCost = pCost;

	delete[] pMatrixtemp;
	pMatrixtemp = NULL;
	delete[] pCosttemp;
	pCosttemp = NULL;

	return 0;
}


//��ʼ��pDMDT�ĳ�Ա����
void DMDT_Init(IN CDMDTStruct* pDMDT)
{
	pDMDT->row = 0;
	pDMDT->col = 0;
	pDMDT->pMatrix = NULL;
	pDMDT->pProb = NULL;
	pDMDT->pWeight = NULL;
	pDMDT->pLevel = NULL;
	pDMDT->pCost = NULL;
}

//�ͷ�pDMDT��Ա������ڴ�
void DMDT_MemoryFree(IN CDMDTStruct* pDMDT)
{
	if (pDMDT == NULL)
		return;

	if (pDMDT->pMatrix)
	{
		delete[] pDMDT->pMatrix;
		pDMDT->pMatrix = NULL;
	}

	if (pDMDT->pProb)
	{
		delete[] pDMDT->pProb;
		pDMDT->pProb = NULL;
	}

	if (pDMDT->pWeight)
	{
		delete[] pDMDT->pWeight;
		pDMDT->pWeight = NULL;
	}
	if (pDMDT->pLevel)
	{
		delete[] pDMDT->pLevel;
		pDMDT->pLevel = NULL;
	}
	if (pDMDT->pCost)
	{
		delete[] pDMDT->pCost;
		pDMDT->pCost = NULL;
	}
}

#ifdef __cplusplus
	};
#endif

