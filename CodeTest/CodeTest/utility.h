#ifndef _UTILITY_H_
#define _UTILITY_H_

class CUtility
{
public:
	CUtility();
	~CUtility();
public:
	void log4zTest();
	void tinyxml2Test();
	void StringConvertTest();
	void JsonTest();

protected:
private:
	int m_logId;	//for log4zTest
};
#endif
