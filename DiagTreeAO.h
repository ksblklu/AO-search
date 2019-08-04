// DiagTreeAO.h: interface for the CDiagTreeAO class.
//
//////////////////////////////////////////////////////////////////////


#pragma once

//#include <math.h>
#include "RGTreeT.h"
//#include "HuffManTree.h"
#include "DepMatrixFileTwo.h"

#define		COST_INIT		(1000.0)
#define		COST_INIT_AMBI	0	//模糊组的测试代价,该设置为多少？
//////////////////////////////////////////////////////////////////////////
//诊断树节点
class CDTreeNode
{
public:
	//故障源
	int*		pSIds;			//故障源id
	int			sCnt;			//包含的故障源数
	double		prob;			//节点包含的故障源的概率和
	double      weight;         //节点包含的故障源的权值和
	//测试
	int			selTId;			//选择的测试id
	double		costWei;		//测试代价(加权：cost*weight,本项目中weight设为1)
	double		cost;			//测试成本
	BOOL        Tused;          //测试被使用过情况
	BOOL        Tstate;         //测试实测通过情况
	//附加信息
	BOOL		bAmbiguity;		//模糊组
	BOOL		bLeaf;			//叶子节点
	int			level;			//层级
	double		costCumulate;	//累积成本
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
	//初始化依赖矩阵等数据
	//weight	测试成本、时间权值，0~1，默认1表示Cost占100%
	//rate		测试成本、时间等效性比率，默认10表示Cost/Time=10
	void	SetData(int* matrix,int rows,int cols,double* probs=NULL,
		double* weights=NULL, double* costs=NULL);
	
	void	FreeMemory();
public:
	int			m_NodeCnt;				//总节点数
	int			m_LeafCnt;				//叶子节点数
	int			m_AmbCnt;				//模糊组数
	int			m_MaxLevel;				//最大层数，从1开始计数
	int			*m_pSCntLevel;			//每层所有叶子节点包含的故障源总数，其中m_pSCntLevel[0]对应第一层即根节点
	int			m_UnuseTestCnt;			//未用测试数
	int			*m_pUnuseTests;			//未用测试集，0|1=未用|已用，初始化时全为0，已被使用的测试置1

	int				m_BacktrackCnt;		//回调次数
	clock_t			m_Clocktart;
	clock_t			m_SpendTime;		//算法花费时间
	CDTreeNode*		pRootNode;			//根节点，其中包含了整棵树的部分指标

//private:
	int			m_Rows;					//故障源数
	int			m_Cols;					//测试数
	int*		pMatrix;				//存放copy的依赖矩阵
	double*		pProbs;					//故障概率
	double*     pWeights;               //故障权值
	int*        pLevels;                //故障等级
	double*		m_pCosts;				//测试成本
	double*		m_pCostsWei;			//加权后的测试成本

private:
	//快速排序 小->大，用于成本排序
	void	QSortUp(double* pD,int l,int r);
	//快速排序 大->小，用于概率排序
	void	QSortDown(double* pD,int l,int r);
	//霍夫曼编码平均长度
	double	Wx(double* probs,double* weights,int cnt);
	//启发函数
	double	HEF1(double* probs, double* weights, int cnt,double* costs=NULL,int costCnt=0);
	//-----------
	//调用启发函数HEF1计算预期测试代价expCost，返回值表示tId能否分辨故障集pSIds
	BOOL	ExpectCost(int* pSIds,int sCnt,int tId,double* pCostBuf, double* expCost);
	//根据预期测试代价ExpectCost选择最佳测试
	void	TestSelect(CDTreeNode* pNode,int& leftTCnt,int* pLeftTIds,double* pExpTCosts);
	//AO*递归搜索主函数，其中将预期测试代价修正为实测代价
	void	AOSearch(POSITION pos,int& leftTCnt,int* pLeftTIds,double* pExpTCosts);
	//根据实测代价，修正最佳测试
	void	CostRevise(POSITION posParent,double revisedCost,int& leftTCnt,int* pLeftTIds,double* pExpTCosts);
	//生成AO*诊断树
	void	SearchAODiagTree();
public:
	//获取诊断树指标
	void	GetInfo(int* Tselect, int* Tsequence);
	int     GetInfoTest(int& Tstep, int& Tstate,int* Tnum);
	int     GetTestResult(int* Fids);
	//递归诊断树，仅被GetInfo()调用
	BOOL	GetInfoNode(POSITION posParent,int& level, int* Tselect, int* Tsequence);
	int     GetInfoNextNode(POSITION posParent, int& level,int& Tstep,  int& Tstate, int* Tnum);
	int     GetInfoFNode(POSITION posParent, int* Fids);
};
//////////////////////////////////////////////////////////////////////////
//从文件读取待分析数据
//BOOL ReadDataFromFile(LPCTSTR fileName,int** ppDM,int& row,int& col,double** ppProbs);
//////////////////////////////////////////////////////////////////////////
