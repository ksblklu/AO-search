#pragma once
#include <stdio.h>
#include "DiagTreeAO.h"

CDiagTreeAO		m_DiagTreeAO;		// AO诊断树数据对象
CDMDTStruct		m_DMData;			// 依赖矩阵变量
int* uSelect;                       // 用户选用测试序列
int* uSequence;                     // 最优测试序列
int  uStepCount;                    // 测试步


//初始化测试序贯
//return 1：返回正常 
//      -1：返回失败
//      -2: 参数出错
extern "C" {
	_declspec(dllexport) int test_init(unsigned char* Tselect, unsigned char* Tsequence)
	{
		
		int i;
		
		extern int* uSequence;
		uSequence = NULL;
		uSequence = new int[100];//存储算法输出序列，之后存入用户输入数组中
		extern int* uSelect;
		uSelect = NULL;
		uSelect= new int[101];//存储用户输入序列，释放内存在返回故障模式后

		i = 0;
		while (Tselect[i] != AE) {
			if (Tselect[i] < 1 || Tselect[i] > 100)
				return -2;
			else
			    uSelect[i] = (int)Tselect[i];
			//printf("%d ", uSelect[i]);
			i++;
		}
		uSelect[i] = AE;
		//初始化依赖矩阵成员
		DMDT_Init(&m_DMData);

		//从文件读矩阵数据
		if (DMDT_ReadFile("DM.txt", uSelect, &m_DMData) < 0)
			return -1;
		
		CDMDTStruct* pDMD = (CDMDTStruct*)(&m_DMData);

		int row = pDMD->row;
		int col = pDMD->col;
		int* pMatrix = pDMD->pMatrix;
		double* pProb = pDMD->pProb;	
		double* pWeight = pDMD->pWeight;
		double* pTestCost = pDMD->pCost;

		//生成AO*诊断树
		m_DiagTreeAO.SetData(pMatrix, row, col, pProb, pWeight, pTestCost);
		//初始化测试序贯
		m_DiagTreeAO.GetInfo(uSelect, uSequence);

		i = 0;
		while (uSequence[i] != AE) {
			Tsequence[i] = uSequence[i];
			i++;
		}
		Tsequence[i] = AE;//数组结尾

		//delete[] uSequence;
		//uSequence = NULL;

		return 1;
	}

	//测试中，每测试一步调用
	//return 1：返回正常 
	//     -1：返回失败 
	//     2：到达叶子节点
	_declspec(dllexport) int get_test_next_step(unsigned char Tstep, unsigned char Tstate, unsigned char* Tnum)
	{
		int i=0;
		int j,uScount;
		int uState = (int)Tstate;
		//测试步信息写入全局变量
		extern int  uStepCount;
		extern int* uSequence;

		uScount = 0;
		while (uSequence[i] != AE)
		{
			if ((int)Tstep == uSequence[i])
			{
				uStepCount += i+1;  //测试过的测点个数
				uScount++;
				break;
			}
			i++;
		}
		if (uScount == 0)
			return -1;
		else
		   j = m_DiagTreeAO.GetInfoTest(uStepCount, uState, uSequence);
		//结果输出到用户输入数组
		i = 0;
		while (uSequence[i] != AE) {
			Tnum[i]=uSequence[i];
			i++;
		}
		Tnum[i] = AE;

		return j;
	}

	//测试结束，返回故障模式（单个或模糊组）
	//if (2 = get_next_step()); then:
	_declspec(dllexport) int get_test_result(unsigned char* Fids,unsigned char* Flevels) {
		int i = 0;
		int j = 0;
		extern int* uSelect;

		int* uFids = NULL;
		uFids = new int[100];
		j=m_DiagTreeAO.GetTestResult(uFids);//返回故障模式到uFids中

		while (uFids[i] != AE) {
			Fids[i] = uFids[i];
			Flevels[i] = m_DMData.pLevel[uFids[i]-1];
			i++;
		}
		Fids[i] = AE;
		Flevels[i] = AE;

		delete[] uFids;
		uFids = NULL;
		delete[] uSelect;
		uSelect = NULL;
		delete[] uSequence;
		uSequence = NULL;

		m_DiagTreeAO.FreeMemory();
		DMDT_MemoryFree(&m_DMData);

		return j;
	}

}