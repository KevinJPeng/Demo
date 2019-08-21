#include "stdafx.h"
#include "TaskInfo.h"


// 比较函数来比较pTaskInfo的优先级
bool CompareTaskInfo(pTaskInfo pA, pTaskInfo pB)
{
	return pA->_iPri <= pB->_iPri;
}
