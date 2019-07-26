/**********************************************
*  KEYRW接口使用说明:
*  1.使用时只需要IKeyBase.h 和 KeyRW.dll放在对目录即可
*  2.成功失败均有对应的返回码，可以打印排查问题
*    读取接口失败后会使用默认值
*************************************************/

#pragma once
#include "IKeyBase.h"

typedef IKeyBase* (*CREATEOBJ)(_EM_KEYTYPE_ iType);
typedef void(*DELETEMEM)(IKeyBase*);

class IKeyRW :	public IKeyBase
{
public:
	IKeyRW();
	~IKeyRW();
public:
	//初始化dll
	E_KEYRET InitDll(_EM_KEYTYPE_ eType);

	/*
	@brief 加密数据
	@param strScrData: 要加密的数据
	@param strKey: 加密的秘钥
	@param strEncryptData: 加密后的数据；
	@return 返回结果
	*/
	 E_KEYRET EncryptData(const CString& strSrcData, const CString& strKey, CString& strEncryptData);

	 /*
	 @brief 数据解密
	 @param strEncrtptData: 要解密的数据
	 @param strKey: 解密的秘钥
	 @param strScrData: 解密后的数据；
	 @return 返回结果
	 */
	E_KEYRET DecryptData(const  CString& strEncrtptData, const CString& strKey, CString& strScrData);

private:
	TCHAR *GetCurPath(void);
private:
	IKeyBase* m_pKeyObj;
	HMODULE m_hDll;
};

