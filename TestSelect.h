#pragma once
#include <stdio.h>
#include "DiagTreeAO.h"

CDiagTreeAO		m_DiagTreeAO;		// AO��������ݶ���
CDMDTStruct		m_DMData;			// �����������
int* uSelect;                       // �û�ѡ�ò�������
int* uSequence;                     // ���Ų�������
int  uStepCount;                    // ���Բ�


//��ʼ���������
//return 1���������� 
//      -1������ʧ��
//      -2: ��������
extern "C" {
	_declspec(dllexport) int test_init(unsigned char* Tselect, unsigned char* Tsequence)
	{
		
		int i;
		
		extern int* uSequence;
		uSequence = NULL;
		uSequence = new int[100];//�洢�㷨������У�֮������û�����������
		extern int* uSelect;
		uSelect = NULL;
		uSelect= new int[101];//�洢�û��������У��ͷ��ڴ��ڷ��ع���ģʽ��

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
		//��ʼ�����������Ա
		DMDT_Init(&m_DMData);

		//���ļ�����������
		if (DMDT_ReadFile("DM.txt", uSelect, &m_DMData) < 0)
			return -1;
		
		CDMDTStruct* pDMD = (CDMDTStruct*)(&m_DMData);

		int row = pDMD->row;
		int col = pDMD->col;
		int* pMatrix = pDMD->pMatrix;
		double* pProb = pDMD->pProb;	
		double* pWeight = pDMD->pWeight;
		double* pTestCost = pDMD->pCost;

		//����AO*�����
		m_DiagTreeAO.SetData(pMatrix, row, col, pProb, pWeight, pTestCost);
		//��ʼ���������
		m_DiagTreeAO.GetInfo(uSelect, uSequence);

		i = 0;
		while (uSequence[i] != AE) {
			Tsequence[i] = uSequence[i];
			i++;
		}
		Tsequence[i] = AE;//�����β

		//delete[] uSequence;
		//uSequence = NULL;

		return 1;
	}

	//�����У�ÿ����һ������
	//return 1���������� 
	//     -1������ʧ�� 
	//     2������Ҷ�ӽڵ�
	_declspec(dllexport) int get_test_next_step(unsigned char Tstep, unsigned char Tstate, unsigned char* Tnum)
	{
		int i=0;
		int j,uScount;
		int uState = (int)Tstate;
		//���Բ���Ϣд��ȫ�ֱ���
		extern int  uStepCount;
		extern int* uSequence;

		uScount = 0;
		while (uSequence[i] != AE)
		{
			if ((int)Tstep == uSequence[i])
			{
				uStepCount += i+1;  //���Թ��Ĳ�����
				uScount++;
				break;
			}
			i++;
		}
		if (uScount == 0)
			return -1;
		else
		   j = m_DiagTreeAO.GetInfoTest(uStepCount, uState, uSequence);
		//���������û���������
		i = 0;
		while (uSequence[i] != AE) {
			Tnum[i]=uSequence[i];
			i++;
		}
		Tnum[i] = AE;

		return j;
	}

	//���Խ��������ع���ģʽ��������ģ���飩
	//if (2 = get_next_step()); then:
	_declspec(dllexport) int get_test_result(unsigned char* Fids,unsigned char* Flevels) {
		int i = 0;
		int j = 0;
		extern int* uSelect;

		int* uFids = NULL;
		uFids = new int[100];
		j=m_DiagTreeAO.GetTestResult(uFids);//���ع���ģʽ��uFids��

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