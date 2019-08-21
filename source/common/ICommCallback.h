#pragma once
class ICommCallback
{
public:

	//处理收到的结果数据
	virtual void OnReceive(CString strData) = 0;
};

