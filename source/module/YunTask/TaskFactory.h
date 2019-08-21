/*******************************************************
* 文件说明:    任务工厂类,根据任务类型创建对应的类
* 时间    ：   2015/8/14
* 作者    :    周林
*******************************************************/

#ifndef _TASTFACTORY_H_
#define _TASTFACTORY_H_

#include "TaskBase.h"
#include "YunTaskStreamCtr.h"
#include "TaskThread.h"
#include "TaskQuickPhoto.h"
#include "TaskGeneral.h"
#include "TaskInfo.h"



// 任务工厂类,根据任务类型创建对应的任务信息结构指针
class CTastFactory
{
public:
	static pTaskInfo Create(int iType, CTaskThread *pTaskThread, CYunTaskStreamCtr *pYunTaskStreamCtr);
};

#endif // _TASTFACTORY_H_