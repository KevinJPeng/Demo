#include "stdafx.h"
#include "TaskFactory.h"

pTaskInfo CTastFactory::Create(int iType, CTaskThread *pTaskThread, CYunTaskStreamCtr *pYunTaskStreamCtr)
{
	pTaskInfo pInfo = new TaskInfo();
	CTaskBase *pTask = NULL;

	switch (iType)
	{
	case eType_QuickPhoto:
		pTask = new CTaskQuickPhoto(pTaskThread, pYunTaskStreamCtr);
		pInfo->_pTask = pTask;
		pInfo->_EntryTime = GetTickCount();
		pInfo->_iPri = ePri_Ring0;
		pInfo->_iType = eType_QuickPhoto;
		break;

	case eType_ShopTraffic:
		pTask = new CTaskGeneral(pTaskThread, pYunTaskStreamCtr);
		pInfo->_pTask = pTask;
		pInfo->_EntryTime = GetTickCount();
		pInfo->_iPri = ePri_Ring2;
		pInfo->_iType = eType_ShopTraffic;
		break;

	case eType_Inforefr:
		pTask = new CTaskGeneral(pTaskThread, pYunTaskStreamCtr);
		pInfo->_pTask = pTask;
		pInfo->_EntryTime = GetTickCount();
		pInfo->_iPri = ePri_Ring3;
		pInfo->_iType = eType_Inforefr;
		break;
	case eType_MainTask:
		pTask = new CTaskGeneral(pTaskThread, pYunTaskStreamCtr);
		pInfo->_pTask = pTask;
		pInfo->_EntryTime = GetTickCount();
		pInfo->_iPri = ePri_Ring1;
		pInfo->_iType = eType_MainTask;
	default:
		break;
	}

	return pInfo;
}