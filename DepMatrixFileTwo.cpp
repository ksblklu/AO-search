
#include "StdAfx.h"
#include <stdio.h>
#include "DepMatrixFileTwo.h"

#ifndef NULL
#define NULL	((void*)0)
#endif

#ifdef __cplusplus
extern "C"{
#endif

//读*.DMDT(DepMatrixData)文件
//函数内部采用了new申请pDMDT成员对象的空间，使用完变量pDMDT后，可以调用DMDT_MemoryFree()释放pDMDT成员对象的内存
//返回值：	0	//运行成功
//			-1	//传入参数无效
//			-2	//打开文件失败

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
		return -1;			//传入参数无效

	pFile=fopen(fileName,"r");
	if (pFile==NULL)
	{
		return -2;			//打开文件失败
	}

	//行列数
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
			fscanf(pFile,"%d",&pMatrixtemp[i*col + j]);//依赖矩阵
		}
		
		fscanf(pFile, "%lf", &pCosttemp[i]);         //测试成本
	}

	for (i = 0; i < col; i++)
	{
		fscanf(pFile, "%lf", &pProb[i]);             //故障概率
	}
	for (i = 0; i < col; i++)
	{
		fscanf(pFile, "%lf", &pWeight[i]);           //故障权值
	}
	for (i = 0; i < col; i++)
	{
		fscanf(pFile, "%d", &pLevel[i]);             //故障等级
	}
	fclose(pFile);

	//计算选取的测试个数
	while (Tselect[TselectLen] != AE)
	{
		TselectLen++;
	}
	
	//测试选取后的依赖矩阵转置
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


//初始化pDMDT的成员变量
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

//释放pDMDT成员对象的内存
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

