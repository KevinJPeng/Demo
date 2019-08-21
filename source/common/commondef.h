#pragma once

#include "Directory.h"
#include "SQLiteOperate.h"


struct T_GLOBAL_DATA
{
	CDirectory dir;
	SQLiteOperate  sqlIte;
	//其它全局数据...
};




class IThreadUnit;
typedef IThreadUnit* (*GETITHREADOBJECT)(T_GLOBAL_DATA *);



#ifndef _NEW
#define _NEW(p, type) \
	(p) = new type;\
	if (!(p))\
{\
	printf("new operator fail!!");\
}
#endif

#ifndef _NEWA
#define _NEWA(p, type, count) \
	(p) = new type[count];\
	if (!(p))\
{\
	printf("new operator fail!!");\
}
#endif

#ifndef _DELETE
#define _DELETE(p) \
	if (p)\
{\
	delete (p);\
	(p) = NULL;\
}
#endif

#ifndef _DELETEA
#define _DELETEA(p) \
	if (p)\
{\
	delete[] (p);\
	(p) = NULL;\
}
#endif