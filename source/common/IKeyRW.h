/**********************************************
*  KEYRW�ӿ�ʹ��˵��:
*  1.ʹ��ʱֻ��ҪIKeyBase.h �� KeyRW.dll���ڶ�Ŀ¼����
*  2.�ɹ�ʧ�ܾ��ж�Ӧ�ķ����룬���Դ�ӡ�Ų�����
*    ��ȡ�ӿ�ʧ�ܺ��ʹ��Ĭ��ֵ
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
	//��ʼ��dll
	E_KEYRET InitDll(_EM_KEYTYPE_ eType);

	/*
	@brief ��������
	@param strScrData: Ҫ���ܵ�����
	@param strKey: ���ܵ���Կ
	@param strEncryptData: ���ܺ�����ݣ�
	@return ���ؽ��
	*/
	 E_KEYRET EncryptData(const CString& strSrcData, const CString& strKey, CString& strEncryptData);

	 /*
	 @brief ���ݽ���
	 @param strEncrtptData: Ҫ���ܵ�����
	 @param strKey: ���ܵ���Կ
	 @param strScrData: ���ܺ�����ݣ�
	 @return ���ؽ��
	 */
	E_KEYRET DecryptData(const  CString& strEncrtptData, const CString& strKey, CString& strScrData);

private:
	TCHAR *GetCurPath(void);
private:
	IKeyBase* m_pKeyObj;
	HMODULE m_hDll;
};

