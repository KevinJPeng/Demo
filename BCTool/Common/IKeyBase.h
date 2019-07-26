#pragma once


//定义返回码，以便调用者排除问题
typedef enum _EM_KEYRET_
{
	KRET_SUCCESS = 0,			//成功
	KRET_INITFAIL,				//初始化失败,或者是未进行对象初始化
	KRET_VALIDDATA,				//数据无效
	KRET_NOTSUMSZWFILE,			//不是有效的商舟网文件
	KRET_NOTFINDNODE,			//未找到对应节点
	KRET_OPENFILEFAIL,			//打开文件失败
	KRET_NOTSUMSZWXMLCONTENT,	//不是有效的商舟网XML内容
	KRET_LOADDLLFAIL,			//加载dll失败
}E_KEYRET;


typedef enum _EM_KEYTYPE_
{
	KTYPE_NUL = 0,      //无效类型
	KTYPE_LOG,			//加密日志类型
	KTYPE_DATA,		//加密采编数据类型
	KTYPE_CFG,			//加密配置文件类型

};

class IKeyBase
{
public:
	//文件加密
	virtual E_KEYRET EncryptData(const CString& strSrcData, const CString& strKey, CString& strEncryptData) = 0;

	//文件解密
	virtual E_KEYRET DecryptData(const  CString& strEncrtptData, const CString& strKey, CString& strScrData) = 0;

	// 析构函数
	virtual ~IKeyBase()
	{}
};

