#pragma once
class CGetNetTime
{
public:
	CGetNetTime();
	~CGetNetTime();

public:
	int GetNetTime(CTime &netTime);	//获取网络时间

private:
	int ConvertTime(unsigned long ulTime, CTime &netTime);
	int GetTimeDiff(CTime &netTime);
};

