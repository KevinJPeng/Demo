/*************************************************
* 说明：执行任务信息结构
* 时间：2015/8/14
* 作者：周林
**************************************************/

#ifndef _TASKINFO_H_
#define _TASKINFO_H_

#include "stdafx.h"
#include <vector>   
#include <algorithm> 
#include <functional> 

class CTaskBase;

// 任务类型
typedef enum _EM_TASKTYPE
{
	eType_Null          = 0,     // 没有类型初始化TaskInfo
	eType_QuickPhoto    = 1,     // 关键字排名类型
	eType_ShopTraffic   = 2,     // 刷新商铺信息类型
	eType_Inforefr      = 3,     // 信息刷新类型
	eType_MainTask		= 4,	 // 网站推广类型

}E_TASKTYPE;

// 任务执行优先级
typedef enum _EM_TASKPRI
{
	ePri_Ring0 = 0,              // 最高优先级
	ePri_Ring1 = 1,              // 其次
	ePri_Ring2 = 2,              // 再次
	ePri_Ring3 = 3,              // 最低优先级
}E_TASKPRI;

// 执行任务信息结构
typedef struct ST_TaskInfo
{
	CTaskBase* _pTask;           // 类型任务指针
	DWORD      _EntryTime;       // 进入任务列表时间
	int        _iPri;            // 任务执行优先级
	int        _iType;           // 任务类型

	ST_TaskInfo()
	{
		_pTask     = NULL;
		_EntryTime = 0xFFFFFFFF;
		_iPri      = ePri_Ring3;
		_iType     = eType_Null;
	}

	~ST_TaskInfo()
	{
		if (_pTask != NULL)
		{
			delete _pTask;
			_pTask = NULL;
		}
	}
}TaskInfo, *pTaskInfo;


// 比较函数来比较pTaskInfo的优先级
bool CompareTaskInfo(pTaskInfo pA, pTaskInfo pB);


#endif //_TASKINFO_H_