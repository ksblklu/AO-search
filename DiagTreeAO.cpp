// DiagTreeAO.cpp: implementation of the CDiagTreeAO class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DiagTreeAO.h"




//////////////////////////////////////////////////////////////////////
//节点比对，在find中有用
BOOL CALLBACK CompareAONodeData(LPARAM p1,LPARAM p2)
{
	//	return ((*(CDTreeNode**)p1)->tId == (*(CDTreeNode**)p2)->tId);
	return FALSE;
}
//删除节点在堆中申请的内存空间
void CALLBACK DeleteAONodeData( POSITION pos, LPARAM lParam)
{
	CDTreeNode* pData=*((CDiagTreeAO*)lParam)->GetAt(pos);
	delete pData;
	pData=NULL;
}

CDiagTreeAO::CDiagTreeAO()
{
	CRGTreeT<CDTreeNode*>::SetCompareFunc(CompareAONodeData);
	CRGTreeT<CDTreeNode*>::SetDeleteFunc(DeleteAONodeData,(LPARAM)this);
	pRootNode = NULL;
	pMatrix = NULL;
	pProbs = NULL;
	pWeights= NULL;
	//pLevels = NULL;
	m_pCosts = NULL;
	m_pCostsWei=NULL;
	m_pSCntLevel=NULL;
	m_pUnuseTests=NULL;
	m_BacktrackCnt=0;
	m_SpendTime=0;

}

CDiagTreeAO::~CDiagTreeAO()
{
	FreeMemory();
}
void CDiagTreeAO::FreeMemory()
{
	if(pMatrix)
		delete[] pMatrix;
	pMatrix=NULL;
	if(pProbs)
		delete[] pProbs;
	pProbs=NULL;
	if (pWeights)
		delete[] pWeights;
	pWeights = NULL;
	//if (pLevels)
	//	delete[] pLevels;
	//pWeights = NULL;
	if(m_pCosts)
		delete[] m_pCosts;
	m_pCosts=NULL;

	if(m_pCostsWei)
		delete[] m_pCostsWei;
	m_pCostsWei=NULL;
	if(m_pSCntLevel)
		delete[] m_pSCntLevel;
	m_pSCntLevel=NULL;
	if(m_pUnuseTests)
		delete[] m_pUnuseTests;
	m_pUnuseTests=NULL;
	RemoveAll();//清空树
}

void CDiagTreeAO::SetData(int* matrix,int rows,int cols,double* probs/*=NULL*/,
	double* weights, double* costs/*=NULL*/)
{
	if(!matrix)
		return;
	int i;
	FreeMemory();//释放原有内存
	pRootNode=new CDTreeNode();
	//故障集
	pRootNode->sCnt=rows;
	pRootNode->pSIds=new int[pRootNode->sCnt];
	for (i=0;i<pRootNode->sCnt;i++)
		pRootNode->pSIds[i]=i;
	//数据copy
	m_Rows=rows;
	m_Cols=cols;
	pMatrix=new int[rows*cols];
	memcpy(pMatrix,matrix,sizeof(int)*rows*cols);
	//故障概率
	pProbs=new double[rows];
	if(probs){
		memcpy(pProbs,probs,sizeof(double)*rows);
	}else{
		double dTemp=m_Rows;
		if(m_Rows==0)
			dTemp=1000000.0;
		for (i=0;i<rows;i++)
			pProbs[i]=1/dTemp;
	}
	//故障权值
	pWeights = new double[rows];
	if (probs) {
		memcpy(pWeights, weights, sizeof(double)*rows);
	}
	else {
		double dTemp = m_Rows;
		if (m_Rows == 0)
			dTemp = 1000000.0;
		for (i = 0; i < rows; i++)
			pWeights[i] = 1 / dTemp;
	}
	//故障等级
	//pWeights = new double[rows];
	//if (levels) {
	//	memcpy(pLevels, levels, sizeof(int)*rows);
	//}

	//测试成本
	m_pCosts=new double[cols];
	if(costs){
		for (i=0;i<cols;i++)
		{
			if(costs[i]<-1000.0)
				m_pCosts[i]=1.0;
			else
				m_pCosts[i]=costs[i];
		}
	}else{
		for (i=0;i<cols;i++)
			m_pCosts[i]=1.0;
	}
	
	//测试代价（测试成本的加权值）
	m_pCostsWei=new double[cols];
	for (i = 0; i < cols; i++)
		m_pCostsWei[i] = m_pCosts[i];
	//每层隔离的故障源数，系统未用测试数
	m_pSCntLevel=new int[cols+1];//层数为测试数+1时，尺寸极限最大情况
	m_pUnuseTests=new int[cols];
	//生成诊断树
	SearchAODiagTree(/*Tselect, Tsequence*/);
}


//////////////////////////////////////////////////////////////////////////
void CDiagTreeAO::QSortUp(double* pD,int l,int r)
{
	int i=l,j=r;
	double mid,tmp;
	mid=pD[(l+r)/2];
	do{	while((pD[i]<mid) && (i<r))
		i++; 
	while((pD[j]>mid) && (j>l))
		j--; 
	if(i<=j){ 
		tmp = pD[i]; 
		pD[i] = pD[j]; 
		pD[j] = tmp; 
		i++; 
		j--; 
	} 
	}while(i<=j);
	if(l<j)	QSortUp(pD,l,j);
	if(r>i)	QSortUp(pD,i,r);
}
//快速排序 大->小
void CDiagTreeAO::QSortDown(double* pD,int l,int r)
{
	int i=l,j=r;
	double mid,tmp;
	mid=pD[(l+r)/2];
	do{	while((pD[i]>mid) && (i<r))
		i++; 
	while((pD[j]<mid) && (j>l))
		j--; 
	if(i<=j){ 
		tmp = pD[i]; 
		pD[i] = pD[j]; 
		pD[j] = tmp; 
		i++; 
		j--; 
	} 
	}while(i<=j);
	if(l<j)	QSortDown(pD,l,j);
	if(r>i)	QSortDown(pD,i,r);
}
//霍夫曼编码平均长度
double CDiagTreeAO::Wx(double* probs,double* weights,int cnt)
{
	int i;
	double avgHuffCodeLen=0.0;
	double probOptSum=0.0;
	//the Huffman average code word length
	for (i=1;i<cnt;i++)
		avgHuffCodeLen += probs[i - 1] * i;
		//avgHuffCodeLen+=probs[i-1]*weights[i-1]*i;

	//if(i>=2)
	//	avgHuffCodeLen += probs[i - 1] * (i - 1);
	//	//avgHuffCodeLen+=probs[i-1]*weights[i-1]*(i-1);
	//else
	//	return 0.0;
	//total probabilities
	probOptSum=0.0;
	for (i=0;i<cnt;i++)
		probOptSum+=probs[i];
	//the average conditional Huffman code word length Wx
	return (avgHuffCodeLen/probOptSum);
}
//启发函数
double CDiagTreeAO::HEF1(double* probs, double* weights, int cnt,double* costs/*=NULL*/,int costCnt/*=0*/)
{
	int i;
	QSortDown(probs,0,cnt-1);//排序 概率 大->小
	double wx=Wx(probs,weights,cnt);
// 	CHuffManTree huffMan;
// 	huffMan.CreateTree(probs,cnt);
// 	double wx=huffMan.GetAvgCodeLen();
	if(!costs)//不考虑测试成本时
		return wx;
	double costAll=0.0;
#ifdef _MFC_VER
	if(IsBadReadPtr(costs,(sizeof(double)*((int)wx+1))))
		ASSERT(FALSE);
#endif
	if(((int)wx)<costCnt)
		costCnt=(int)wx+1;
	QSortUp(costs,0,costCnt-1);//排序 成本 小->大
	for (i=0;i<costCnt;i++)
		costAll+=costs[i];
	return costAll+(wx-(int)wx)*costs[(int)wx];
}

//////////////////////////////////////////////////////////////////////////
//测试的期望成本
BOOL CDiagTreeAO::ExpectCost(int* pSIds,int sCnt,int tId,double* pCostBuf,double* expCost)
{
	double*	pPassProbBuf=new double[sCnt];
	double* pPassWeightBuf = new double[sCnt];
	double*	pFailProbBuf=new double[sCnt];
	double* pFailWeightBuf = new double[sCnt];
	int		j=0,sId=0,passCnt=0;
	double	passProbSum=0.0;
	double  passWeightSum=0.0;
	double	failProbSum=0.0;
	double  failWeightSum=0.0;
	for (j=0;j<sCnt;j++)
	{
		sId=pSIds[j];
		if(!pMatrix[sId*m_Cols+tId]){//pass
			pPassProbBuf[passCnt]=pProbs[sId];
			pPassWeightBuf[passCnt] = pWeights[sId];
			passProbSum+=pProbs[sId];
			passWeightSum += pWeights[sId];

			passCnt++;
		}else{//fail
			pFailProbBuf[j-passCnt]=pProbs[sId];
			pFailWeightBuf[j - passCnt] = pWeights[sId];
			failProbSum+=pProbs[sId];
			failWeightSum += pWeights[sId];
		}
	}
	double	passHEF1,failHEF1;
	passHEF1=HEF1(pPassProbBuf,pPassWeightBuf,passCnt,pCostBuf);
	failHEF1=HEF1(pFailProbBuf, pFailWeightBuf,(sCnt-passCnt),pCostBuf);
	delete	pPassProbBuf;
	delete	pPassWeightBuf;
	delete	pFailProbBuf;
	delete	pFailWeightBuf;
	pPassProbBuf=NULL;
	pPassWeightBuf=NULL;
	pFailProbBuf=NULL;
	pFailWeightBuf=NULL;
	*expCost=(passWeightSum*passProbSum*passHEF1+ failWeightSum*failProbSum*failHEF1)/(passWeightSum*passProbSum+failWeightSum*failProbSum);
	return ((passCnt!=sCnt) && sCnt);
}
void CDiagTreeAO::TestSelect(CDTreeNode* pNode,int& leftTCnt,int* pLeftTIds,double* pExpTCosts)
{
	int i,j;
	//printf("当前剩余测试：%d", leftTCnt);
	double* pCostBuf=new double[leftTCnt];
	for (i=0;i<leftTCnt;i++)
	{
		pCostBuf[i]=m_pCostsWei[pLeftTIds[i]];
	}
	//隔离成功时
	if (pNode->sCnt==1)
	{
		//printf("leaf");
		pNode->costWei=0.0;
		pNode->cost=0.0;
		pNode->bLeaf=TRUE;
		pNode->prob=pProbs[pNode->pSIds[0]];
		pNode->weight=pWeights[pNode->pSIds[0]];
		delete[] pCostBuf;
		pCostBuf=NULL;
		return;
	}
	//模糊组时
	BOOL bRstAnd=TRUE,bRstOr=FALSE;
	for (i=0;i<leftTCnt;i++)
	{
		bRstAnd=bRstOr=pMatrix[ pNode->pSIds[0]*m_Cols + pLeftTIds[i] ];
		for (j=0;j<pNode->sCnt;j++)
		{
			bRstAnd&=pMatrix[ pNode->pSIds[j]*m_Cols + pLeftTIds[i] ];
			bRstOr|=pMatrix[ pNode->pSIds[j]*m_Cols + pLeftTIds[i] ];
			if(bRstAnd!=bRstOr)
				break;
		}
		if(bRstAnd!=bRstOr)
			break;
	}if (bRstAnd==bRstOr || !leftTCnt){
		//printf("Ambi");
		pNode->costWei=COST_INIT_AMBI*pNode->sCnt;//模糊组的测试代价该设置为多少？
		pNode->cost=0.0;
		pNode->bAmbiguity=TRUE;
		pNode->bLeaf=TRUE;
		pNode->prob=0.0;
		pNode->weight = 0.0;
		for (j = 0; j < pNode->sCnt; j++) {
			pNode->prob += pProbs[pNode->pSIds[j]];
			pNode->weight += pWeights[pNode->pSIds[j]];
		}
		delete[] pCostBuf;
		pCostBuf=NULL;
		return;
	}
	//找最低测试代价、最佳测试
	int		tId=0;
	double	expCost;
	ASSERT(pNode->costWei>=0);
	for (i=0;i<leftTCnt;i++)
	{
		tId=pLeftTIds[i];
		if (!ExpectCost(pNode->pSIds,pNode->sCnt,tId,pCostBuf,&expCost))
		{
			pExpTCosts[i]=-COST_INIT;
			continue;//当测试tId无法分辨故障pSIds时
		}
		pExpTCosts[i]=m_pCostsWei[tId]+expCost;
		if(pNode->costWei-pExpTCosts[i]>0)
		{
			pNode->costWei=pExpTCosts[i];
			pNode->selTId=tId;
		}
	}
	if (pNode->sCnt==2 && !pNode->bAmbiguity)
	{
		pNode->costWei=m_pCostsWei[pNode->selTId];
		pNode->cost=m_pCosts[pNode->selTId];
	}
    //printf("选取测试为%d\n", pNode->selTId);
	pNode->prob=0.0;
	pNode->weight = 0.0;
	for (j = 0; j < pNode->sCnt; j++) {
		pNode->prob += pProbs[pNode->pSIds[j]];
		pNode->weight += pWeights[pNode->pSIds[j]];
	}
	delete[] pCostBuf;
	pCostBuf=NULL;
}
void CDiagTreeAO::AOSearch(POSITION posParent,int& leftTCnt,int* pLeftTIds,double* pExpTCosts)
{

	if(!posParent)
		return;
	int i,j;
	POSITION	pos=NULL,posTemp=NULL;
	CDTreeNode* pParent=NULL;
	pParent=*GetAt(posParent);
	if(!pParent||!pParent->sCnt)
		return;

	
	int	passSCnt=0;
	for (j=0;j<pParent->sCnt;j++)
	{
		if(!pMatrix[pParent->pSIds[j]*m_Cols+pParent->selTId]){//pass
			passSCnt++;
		}
	}
	if (!passSCnt || passSCnt==pParent->sCnt)
	{
		//ASSERT(passSCnt && passSCnt!=pParent->sCnt);
		pParent->bLeaf=TRUE;
		pParent->costWei=COST_INIT_AMBI*pParent->sCnt;
		pParent->bAmbiguity=TRUE;
		pParent->prob=0.0;
		pParent->weight = 0.0;
		for (j = 0; j < pParent->sCnt; j++) {
			pParent->prob += pProbs[pParent->pSIds[j]];
			pParent->weight += pWeights[pParent->pSIds[j]];
		}
		return;
	}
	//////////////////////////////////////////////////////////////////////////
	//从剩余测试中剔除已选择的最佳测试
	int		leftTCntTemp=leftTCnt;
	double* pExpTCostsTemp=new double[leftTCntTemp];
	int*	pLeftTIdsTemp=new int[leftTCntTemp];
	for (i=0,j=0;i<leftTCntTemp;i++)
	{
		if(pLeftTIds[i]!=pParent->selTId){
			pLeftTIdsTemp[j]=pLeftTIds[i];
			pExpTCostsTemp[j]=pExpTCosts[i];
			j++;
		}
	}if(i!=j)
		leftTCntTemp--;
	
	//////////////////////////////////////////////////////////////////////////
	//左子节点
	CDTreeNode* pLNode=NULL;
	pLNode=new CDTreeNode();
	pLNode->sCnt=passSCnt;
	pLNode->pSIds=new int[pLNode->sCnt];
	for (i=0,j=0;j<pParent->sCnt;j++)
	{
		if(!pMatrix[pParent->pSIds[j]*m_Cols+pParent->selTId]){//pass
			pLNode->pSIds[i]=pParent->pSIds[j];
			i++;
		}
	}
	TestSelect(pLNode,leftTCntTemp,pLeftTIdsTemp,pExpTCostsTemp);
	pos=AddHead(&pLNode,posParent);
	AOSearch(pos,leftTCntTemp,pLeftTIdsTemp,pExpTCostsTemp);
	//////////////////////////////////////////////////////////////////////////
	//右子节点
	CDTreeNode* pRNode=NULL;
	pRNode=new CDTreeNode();
	pRNode->sCnt=pParent->sCnt-pLNode->sCnt;
	pRNode->pSIds=new int[pRNode->sCnt];
	for (i=0,j=0;j<pParent->sCnt;j++)
	{
		if(pMatrix[pParent->pSIds[j]*m_Cols+pParent->selTId]){//fail
			pRNode->pSIds[i]=pParent->pSIds[j];
			i++;
		}
	}
	TestSelect(pRNode,leftTCntTemp,pLeftTIdsTemp,pExpTCostsTemp);
	pos=AddTail(&pRNode,posParent);
	AOSearch(pos,leftTCntTemp,pLeftTIdsTemp,pExpTCostsTemp);
	//////////////////////////////////////////////////////////////////////////
	delete[] pLeftTIdsTemp;
	pLeftTIdsTemp=NULL;
	delete[] pExpTCostsTemp;
	pExpTCostsTemp=NULL;
	ASSERT(pParent->selTId>=0);
	//////////////////////////////////////////////////////////////////////////
	double revisedCost=0.0;//实测成本
	revisedCost=m_pCostsWei[pParent->selTId]+
		(pLNode->weight*pLNode->prob*pLNode->costWei+ pRNode->weight*pRNode->prob*pRNode->costWei)/(pLNode->weight*pLNode->prob+ pRNode->weight*pRNode->prob);
	//ASSERT(revisedCost>=1.0);
	pParent->cost=m_pCosts[pParent->selTId]+
		(pLNode->weight*pLNode->prob*pLNode->cost+ pRNode->weight*pRNode->prob*pRNode->cost)/(pLNode->weight*pLNode->prob+ pRNode->weight*pRNode->prob);
	if (pParent->costWei-revisedCost>0.01||revisedCost-pParent->costWei>0.01)
	{
		CostRevise(posParent,revisedCost,leftTCnt,pLeftTIds,pExpTCosts);//最优测试修正
	}
}

void CDiagTreeAO::CostRevise(POSITION posParent,double revisedCost,int& leftTCnt,int* pLeftTIds,double* pExpTCosts)
{
	int i;
	CDTreeNode* pParent=NULL;
	pParent=*GetAt(posParent);
	
	pParent->costWei=revisedCost;
	//ASSERT(revisedCost < pParent->costWei);
	int minCostTId=pParent->selTId;
	double costWei=pParent->costWei;
	for (i=0;i<leftTCnt;i++)
	{
		if (pParent->selTId==pLeftTIds[i]){
			pExpTCosts[i]=pParent->costWei;
		}
	}
	for (i=0;i<leftTCnt;i++)
	{
		if ((costWei-pExpTCosts[i]>0.01)&& pExpTCosts[i]>0){
			costWei=pExpTCosts[i];
			minCostTId=pLeftTIds[i];
		}
	}
	if (pParent->selTId!=minCostTId)
	{	
		clock_t spendTime=clock()-m_Clocktart;	//算法定时,分析超过此时间时，返回不再递归
		if (spendTime>5000)
		{
			return;
		}

		pParent->costWei=costWei;
		pParent->selTId=minCostTId;

		m_BacktrackCnt++;
		
		//删除被修正的诊断树部分
		POSITION pos=NULL;
		pos=GetChildPosition(posParent,FALSE);//先删右节点
		RemoveChilds(pos);
		((CTreeItem*)posParent)->pTIChildLast=NULL;
		if(!((CTreeItem*)posParent)->nChildCount)
			((CTreeItem*)posParent)->pTIChildFirst=NULL;
		pos=GetChildPosition(posParent);//再删左节点
		if (pos)
		{
			((CTreeItem*)pos)->pTINext=NULL;
			RemoveChilds(pos);
			((CTreeItem*)posParent)->pTIChildFirst=NULL;
		}
		AOSearch(posParent,leftTCnt,pLeftTIds,pExpTCosts);
	}
}

BOOL CDiagTreeAO::GetInfoNode(POSITION posParent,int& level, int* Testselect, int* Tsequence)
{
	if(!posParent)
		return FALSE;
	level++;
	m_NodeCnt++;
	int lvll=level,lvlr=level;
	CDTreeNode*	pNode=*GetAt(posParent);
	
	if (pNode->bLeaf)
	{
		m_pSCntLevel[level-1]+=pNode->sCnt;		//第level层包含的故障源数
		m_LeafCnt++;
		Tsequence[level - 1] = AE;
		if(pNode->bAmbiguity)
			m_AmbCnt++;
	}
	else {
		//m_pUnuseTests[pNode->selTId] = 1;			//已被使用的测试置1
		Tsequence[level - 1] = Testselect[pNode->selTId];
		GetInfoNode(GetChildPosition(posParent, TRUE), lvll, Testselect, Tsequence);//遍历左子树
	}
	
	//GetInfoNode(GetChildPosition(posParent,FALSE),lvlr);

	return TRUE;
}
void CDiagTreeAO::GetInfo(int* Tselect, int* Tsequence)
{

	m_MaxLevel=0;
	m_NodeCnt=0;
	m_LeafCnt=0;
	m_AmbCnt=0;
	m_UnuseTestCnt=0;
	//printf("最优测试序列：");
	GetInfoNode(GetRootPosition(),m_MaxLevel, Tselect, Tsequence);

}
int  CDiagTreeAO::GetInfoTest(int& Tstep, int& Tstate, int* Tnum)
{
	int i;
	m_MaxLevel = 0;
	i=GetInfoNextNode(GetRootPosition(),m_MaxLevel,Tstep,Tstate,Tnum);
	return i;

}
int  CDiagTreeAO::GetInfoNextNode(POSITION posParent, int& level,int& Tstep,int& Tstate, int* Tnum)
{
	if (!posParent)
		return -1;
	level++;
	CDTreeNode*	pNode = *GetAt(posParent);
	if (level == Tstep)
	{
		int i = 0;
		pNode->Tstate = Tstate;  //更新当前测试步的测试通过状态
		POSITION posNext = GetChildPosition(posParent, !Tstate);//定位到当前测试步下一步测试
		CDTreeNode* pNodeNext = *GetAt(posNext);
		if (pNodeNext->bLeaf)
		{
			Tnum[0] = AE;
			return 2;
		}
		else
		{
			extern int* uSelect;
			GetInfoNode(posNext, i, uSelect, Tnum);//返回后续
			return 1;
		}
	}
	else
		GetInfoNextNode(GetChildPosition(posParent, !pNode->Tstate), level, Tstep, Tstate, Tnum);
	}

int CDiagTreeAO::GetTestResult(int *Fids)//隔离出的故障模式
{
	int i=0;
	i=GetInfoFNode(GetRootPosition(), Fids);
	return i;
}

int CDiagTreeAO::GetInfoFNode(POSITION posParent,int* Fids)
{
	int i;
	CDTreeNode*	pNode = *GetAt(posParent);
	if (pNode->bLeaf)
	{
		//printf("Fids:");
		for (i = 0; i < pNode->sCnt; i++) 
			Fids[i] = pNode->pSIds[i]+1;
		Fids[pNode->sCnt] = AE;//数组结尾

		return 1;
	}
	else 
		GetInfoFNode(GetChildPosition(posParent, !pNode->Tstate), Fids);

	
}
void CDiagTreeAO::SearchAODiagTree()
{
	if(!pRootNode || !m_Rows)
	{
		delete pRootNode;
		pRootNode=NULL;
		return;
	}
	m_BacktrackCnt=0;
	//
	int i;
	int leftTCnt=m_Cols;
	int* pLeftTIds=new int[leftTCnt];
	double* pExpTCosts=new double[leftTCnt];
	POSITION posRoot=NULL;
	printf("算法开始");
	m_Clocktart=clock();				//算法开始,计时
	for (i = 0; i < leftTCnt; i++)
	{
		pLeftTIds[i] = i;
		//printf("%d ", pLeftTIds[i]);
	}
	printf("\n");
	TestSelect(pRootNode,leftTCnt,pLeftTIds,pExpTCosts);
	posRoot=AddHead(&pRootNode,NULL);
	AOSearch(posRoot,leftTCnt,pLeftTIds,pExpTCosts);
	//GetInfo(Tselect, Tsequence);
	m_SpendTime=clock()-m_Clocktart;	//算法结束,计时
	
	delete[] pLeftTIds;
	pLeftTIds=NULL;
	delete[] pExpTCosts;
	pExpTCosts=NULL;
// 	CString msgBox;
// 	msgBox.Format("SpendTime=%d",m_SpendTime);
// 	AfxMessageBox(msgBox);
}
//////////////////////////////////////////////////////////////////////////
