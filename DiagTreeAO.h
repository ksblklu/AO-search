// DiagTreeAO.h: interface for the CDiagTreeAO class.
//
//////////////////////////////////////////////////////////////////////


#pragma once

//#include <math.h>
#include "RGTreeT.h"
//#include "HuffManTree.h"
#include "DepMatrixFileTwo.h"

#define		COST_INIT		(1000.0)
#define		COST_INIT_AMBI	0	//ģ����Ĳ��Դ���,������Ϊ���٣�
//////////////////////////////////////////////////////////////////////////
//������ڵ�
class CDTreeNode
{
public:
	//����Դ
	int*		pSIds;			//����Դid
	int			sCnt;			//�����Ĺ���Դ��
	double		prob;			//�ڵ�����Ĺ���Դ�ĸ��ʺ�
	double      weight;         //�ڵ�����Ĺ���Դ��Ȩֵ��
	//����
	int			selTId;			//ѡ��Ĳ���id
	double		costWei;		//���Դ���(��Ȩ��cost*weight,����Ŀ��weight��Ϊ1)
	double		cost;			//���Գɱ�
	BOOL        Tused;          //���Ա�ʹ�ù����
	BOOL        Tstate;         //����ʵ��ͨ�����
	//������Ϣ
	BOOL		bAmbiguity;		//ģ����
	BOOL		bLeaf;			//Ҷ�ӽڵ�
	int			level;			//�㼶
	double		costCumulate;	//�ۻ��ɱ�
	CDTreeNode() :pSIds(NULL), sCnt(0), prob(0.0), selTId(-1), costWei(COST_INIT), cost(0.0),weight(0.0),
		Tused(FALSE), Tstate(FALSE),bAmbiguity(FALSE), bLeaf(FALSE), level(0), costCumulate(0.0) {};
	virtual ~CDTreeNode(){
		if (pSIds){
			delete[] pSIds;
			pSIds=NULL;
		}
	};
};
//////////////////////////////////////////////////////////////////////////
class CDiagTreeAO : public CRGTreeT<CDTreeNode*>
{
public:
	CDiagTreeAO();
	virtual ~CDiagTreeAO();
public:
	//��ʼ���������������
	//weight	���Գɱ���ʱ��Ȩֵ��0~1��Ĭ��1��ʾCostռ100%
	//rate		���Գɱ���ʱ���Ч�Ա��ʣ�Ĭ��10��ʾCost/Time=10
	void	SetData(int* matrix,int rows,int cols,double* probs=NULL,
		double* weights=NULL, double* costs=NULL);
	
	void	FreeMemory();
public:
	int			m_NodeCnt;				//�ܽڵ���
	int			m_LeafCnt;				//Ҷ�ӽڵ���
	int			m_AmbCnt;				//ģ������
	int			m_MaxLevel;				//����������1��ʼ����
	int			*m_pSCntLevel;			//ÿ������Ҷ�ӽڵ�����Ĺ���Դ����������m_pSCntLevel[0]��Ӧ��һ�㼴���ڵ�
	int			m_UnuseTestCnt;			//δ�ò�����
	int			*m_pUnuseTests;			//δ�ò��Լ���0|1=δ��|���ã���ʼ��ʱȫΪ0���ѱ�ʹ�õĲ�����1

	int				m_BacktrackCnt;		//�ص�����
	clock_t			m_Clocktart;
	clock_t			m_SpendTime;		//�㷨����ʱ��
	CDTreeNode*		pRootNode;			//���ڵ㣬���а������������Ĳ���ָ��

//private:
	int			m_Rows;					//����Դ��
	int			m_Cols;					//������
	int*		pMatrix;				//���copy����������
	double*		pProbs;					//���ϸ���
	double*     pWeights;               //����Ȩֵ
	int*        pLevels;                //���ϵȼ�
	double*		m_pCosts;				//���Գɱ�
	double*		m_pCostsWei;			//��Ȩ��Ĳ��Գɱ�

private:
	//�������� С->�����ڳɱ�����
	void	QSortUp(double* pD,int l,int r);
	//�������� ��->С�����ڸ�������
	void	QSortDown(double* pD,int l,int r);
	//����������ƽ������
	double	Wx(double* probs,double* weights,int cnt);
	//��������
	double	HEF1(double* probs, double* weights, int cnt,double* costs=NULL,int costCnt=0);
	//-----------
	//������������HEF1����Ԥ�ڲ��Դ���expCost������ֵ��ʾtId�ܷ�ֱ���ϼ�pSIds
	BOOL	ExpectCost(int* pSIds,int sCnt,int tId,double* pCostBuf, double* expCost);
	//����Ԥ�ڲ��Դ���ExpectCostѡ����Ѳ���
	void	TestSelect(CDTreeNode* pNode,int& leftTCnt,int* pLeftTIds,double* pExpTCosts);
	//AO*�ݹ����������������н�Ԥ�ڲ��Դ�������Ϊʵ�����
	void	AOSearch(POSITION pos,int& leftTCnt,int* pLeftTIds,double* pExpTCosts);
	//����ʵ����ۣ�������Ѳ���
	void	CostRevise(POSITION posParent,double revisedCost,int& leftTCnt,int* pLeftTIds,double* pExpTCosts);
	//����AO*�����
	void	SearchAODiagTree();
public:
	//��ȡ�����ָ��
	void	GetInfo(int* Tselect, int* Tsequence);
	int     GetInfoTest(int& Tstep, int& Tstate,int* Tnum);
	int     GetTestResult(int* Fids);
	//�ݹ������������GetInfo()����
	BOOL	GetInfoNode(POSITION posParent,int& level, int* Tselect, int* Tsequence);
	int     GetInfoNextNode(POSITION posParent, int& level,int& Tstep,  int& Tstate, int* Tnum);
	int     GetInfoFNode(POSITION posParent, int* Fids);
};
//////////////////////////////////////////////////////////////////////////
//���ļ���ȡ����������
//BOOL ReadDataFromFile(LPCTSTR fileName,int** ppDM,int& row,int& col,double** ppProbs);
//////////////////////////////////////////////////////////////////////////
