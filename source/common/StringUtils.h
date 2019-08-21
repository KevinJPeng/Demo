#ifndef __STRINGUTILS_H_
#define __STRINGUTILS_H_

#include "StdString.h"
#include "StdStrUtils.h"

//账号表现形式
enum E_Account_Style
{
	asNOT_INCLUDE_CHINESE = 0,  //正常
	asFIRST_CHAR,    //以字符开头
	asFIRST_DIGITAL, // 以数字开头
	asFIRST_CHINESE, //以汉字开头
	asFIRST_VALUE_UNKNOWN,//始终放在最后
};

//密码组合形式
enum E_Password_Style
{
	psNO_INCLUDE_CHINESE,  //常、混合(不包含汉字)
	psALL_UPPER_CHART,     //纯大写字符
	psALL_LOWER_CHART,    //纯小写字符
	psALL_DIGITAL,        //纯数字
	psALL_CHINESE,      //纯汉字
	psUPPER_CHART_AND_DIGITAL,  //大写字符和数字
	psLOWER_CHART_AND_DIGITAL,  //小写字符和数字
	psUPPER_CHART_AND_CHINESE,  //大写字符和汉字
	psLOWER_CHART_AND_CHINESE,  //小写字符和汉字
	psDIGITAL_AND_CHINESE,    //  数字和汉字
	psALL_VALUE_UNKNOWN,//始终放在最后
};

class StringUtils :public CStdStrUtils
{
public:
	DWORD SplitStringToInt(const CStdString& input, const CStdString& delimiter, std::vector<int>& results, bool includeEmpties = true);

	DWORD SplitStringToIntArray(const CStdString& input, const CStdString& delimiter, int * & intresult , bool includeEmpties = true);

	CStdString GetYunMu(int Num);
	DWORD GetRandEName(CStdString &strOut);
	DWORD GetRandCName(CStdString &strOut);
	DWORD CreateUserName(CStdString& strOut);
	DWORD CreateUserName2(CStdString& strOut, E_Account_Style eFirstChType);

	//add 20080923
	//生成随机字符串
	DWORD RandStr(int nLen, E_Password_Style eType, CStdString& strOut);

	//判断字符串是否都是数字类型
	BOOL IsNumeric(const TCHAR *pszInput);

	//判断字符串是否都是英文字符类型
	BOOL IsAlpha(const TCHAR *pszInput);
//end add 20080923


private:
	int mrand();
	int GetRand(int nStart,int nEnd);
	DWORD MatchFirstCharStr(CStdString &strInAndOut, E_Account_Style eFlag,const CStdString &strZN,const CStdString &strEN);
};

#endif
