#pragma once


//���巵���룬�Ա�������ų�����
typedef enum _EM_KEYRET_
{
	KRET_SUCCESS = 0,			//�ɹ�
	KRET_INITFAIL,				//��ʼ��ʧ��,������δ���ж����ʼ��
	KRET_VALIDDATA,				//������Ч
	KRET_NOTSUMSZWFILE,			//������Ч���������ļ�
	KRET_NOTFINDNODE,			//δ�ҵ���Ӧ�ڵ�
	KRET_OPENFILEFAIL,			//���ļ�ʧ��
	KRET_NOTSUMSZWXMLCONTENT,	//������Ч��������XML����
	KRET_LOADDLLFAIL,			//����dllʧ��
}E_KEYRET;


typedef enum _EM_KEYTYPE_
{
	KTYPE_NUL = 0,      //��Ч����
	KTYPE_LOG,			//������־����
	KTYPE_DATA,		//���ܲɱ���������
	KTYPE_CFG,			//���������ļ�����

};

class IKeyBase
{
public:
	//�ļ�����
	virtual E_KEYRET EncryptData(const CString& strSrcData, const CString& strKey, CString& strEncryptData) = 0;

	//�ļ�����
	virtual E_KEYRET DecryptData(const  CString& strEncrtptData, const CString& strKey, CString& strScrData) = 0;

	// ��������
	virtual ~IKeyBase()
	{}
};

