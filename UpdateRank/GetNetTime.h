#pragma once
class CGetNetTime
{
public:
	CGetNetTime();
	~CGetNetTime();

public:
	int GetNetTime(CTime &netTime);	//��ȡ����ʱ��

private:
	int ConvertTime(unsigned long ulTime, CTime &netTime);
	int GetTimeDiff(CTime &netTime);
};

