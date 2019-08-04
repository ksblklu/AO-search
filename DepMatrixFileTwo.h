/************************************************* 

File name:		DepMatrixFileTwo.h 


Function List:
	1、int DMDT_ReadFile(IN const char* fileName,OUT CDMDTStruct* pDMDT)
	2、int DMDT_WriteFile(IN const char* fileName,IN CDMDTStruct* pDMDT)
	3、void DMDT_Init(IN CDMDTStruct* pDMDT)
	4、void DMDT_MemoryFree(IN CDMDTStruct* pDMDT)
	5、void DMDT_RangeCheck(IN CDMDTStruct* pDMDT);	//文件数据有效范围规范化

History:          
1. Date: 
Author: 
Modification: 
2. ... 
*************************************************/
#ifndef DEPMATRIXFILE_HEAD
#define DEPMATRIXFILE_HEAD

#ifndef IN
#define IN		//标记参数为输入
#endif
#ifndef OUT
#define OUT		//标记参数为输出
#endif
#define AE 255  //定义数组结尾

#ifdef __cplusplus
extern "C"{
#endif

//依赖矩阵数据
typedef struct CDMDTStruct__
{
	int		row;			//行数
	int		col;			//列数
	int*	pMatrix;		//故障源-测试依赖矩阵
	double*	pProb;			//故障概率
	double*	pWeight;		//成本时间权值
	int*    pLevel;         //故障等级
	double*	pCost;			//测试成本
}CDMDTStruct;

//读*.DMD或*.DMDT(DepMatrixData)文件
//函数内部采用了new申请pDMDT成员对象的空间，使用完变量pDMDT后，可以调用DMDT_MemoryFree()释放pDMDT成员对象的内存
//返回值：	0	//运行成功
//			-1	//传入参数无效
//			-2	//打开文件失败
//			-3	//行列数不正确
int DMDT_ReadFile(IN const char* fileName, IN int* Tselect,OUT CDMDTStruct* pDMDT);



//初始化pDMDT的成员变量
void DMDT_Init(IN CDMDTStruct* pDMDT);


//释放pDMDT成员对象的内存
void DMDT_MemoryFree(IN CDMDTStruct* pDMDT);


#ifdef __cplusplus
	};
#endif

#endif	//end of DEPMATRIXFILE_HEAD
