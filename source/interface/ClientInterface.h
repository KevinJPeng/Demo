#pragma once
#define REG_USER_ROOT               HKEY_CURRENT_USER
#define REG_KEY_VERSION             _T("version")
#define IsHexNum(c) ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))  

//用于由登录界面传给主界面登录参数使用
typedef struct tuserinfo
{
	CString strURL;               //登录UR
	CString strUserName;		  //用户名
	CString strPassWord;		  //密码
	int bIsSavePassWord;		  //是否保存密码
	int bIsAutoPassWord;		  //是否自动登录
	tuserinfo()
	{
		strURL = _T("");
		strUserName=_T("");
		strPassWord = _T("");
		bIsSavePassWord = 0;
		bIsAutoPassWord = 0;
	}
}_TUSERINFO,*PTUSERINTO;

//存放到数据库中的用户名和密码
typedef struct tuserInfotodb
{
	CString strUserName;		  //用户名
	CString strPassWord;		  //密码
	int iProductID;			  //产品ID--舟大师
	tuserInfotodb()
	{
		strUserName = _T("");
		strPassWord = _T("");
		iProductID = 0;
	}
};

//存放配置信息
typedef struct tclientsetting
{
	CString strRebootStart;			   //是否开机启动
	CString strMasterZPlain;		   //参与体验计划
	tclientsetting()
	{
		strRebootStart = _T("1");
		strMasterZPlain =  _T("1");
	}
};

//格式化日期取出月份和日期
CString FormatDate(char* pDate);

//取出当前天
CString GetDay(char* pDate);

//写入相关用户信息到数据库
bool WriteUserInfo(CString& strUserName,CString& strPassWord,int iAutoFlag,int iPassWordFlag,int iProductid);

//从数据库中读取用户的相关信息
bool ReadUserInfo(CString& strUserName,CString& strPassWord,int&iAutoFlag,int&iPassWordFlag);

//从数据库中获取用户名
CString GetUserName();

//从配置文件中获取URL
bool GetURL(CStdString& strURL);

//从配置文件中获取与服务器的连接时间
DWORD GetConnectServer();

//将明文的字符串和密码封装为密文的相关数据格式
DWORD GetEncodeDate(const CString& strUserName,const CString&strPassWord,CString& strEnCodeData);

//将密文的用户名和密码保存为密文的相关数据格式
CString GetEncodeDate(CString& strEncodeUserName,CString&strEncodePassWord);

//加密字符串
CStdString EncodeString( CString &strSource);

//解密字符串
CStdString DecodeString( CString& strDest);

//解密登录模块发过来的登录成功之后的数据，获取正确的登录用户名和密码
void DecodeString(const CString& strParam,tuserInfotodb&tuserdb);

//获取近七天的日期
void GetDateSeven(std::vector<CString>& vecDate);

//宽字节CString转Char*
char* CStringToMutilChar(CString& str,int& chLength,WORD wPage=CP_ACP);

//检测网络是否连接
bool IsInternetConnect();

//解析tips字符串
bool AnysisTipsInfo(const CString& strInfo, CString& strTitle, CString& strContent, CString& strTime);

//获取一个一定范围内的随机数 dwBase随机KEY,dwmin最小的数
int GetRand(DWORD dwBase, DWORD dwMin);

//获取随机数加在URL后面
CString GeLocalTimeRand();

//操作开机启动项注册表 0表示添加客户端开机启动,1表示取消客户端开机启动
void OperateRegistOfStart(WORD wtype = 0); 

//从配置文件中读取设置信息
void GetSettingInfo(tclientsetting& tsSetting);

//写入用户配置信息到配置文件
void WriteConfig(tclientsetting& tsSetting);

//判断某个进程是否正在运行
BOOL CheckProcessRun(TCHAR* chProName);

//封装登录结构体信息 strUserName:密文  strEncodeString:密文
void ChangeLoginfo(PassInfo& passinfo, CString strEncodeString, CString strUserName);

//获取当前时间并格式化输出;格式化当前时间
CString GetLocalTimeFormat();

//URLEncode---UTF-8编码:要换编码方式只需要改变Page页
BYTE toHex(const BYTE &x);
CString  URLEncode(CString sIn);

//URLDecode---UTF-8解码:要换解码方式只需要改变Page页
CString Utf8ToStringT(LPSTR str);
CString UrlDecode(LPCTSTR url);

//比较两个图表数据是否一致
BOOL IsChartDataChange (const KEYWORD_PRODUCT_LIST& tOld,const KEYWORD_PRODUCT_LIST& tNew,DWORD wtype);
BOOL IsWeixinUserCareDataChange(const WEIXINTONG_DATA& told,const WEIXINTONG_DATA& tnew,DWORD wtype);

//获取当前版本号
int GetRegCurentVersionID();

//获取舟大师注册表中当前用户名
TCHAR* GetRegCurrentUserName();

//用户切换账号之后先判断是否与注册表一致，不一致就更新注册表
BOOL UpdateRegUserInfo(CString strUserName,int ProDuctId);

//获取版本标志 customize.dat version type:0主线  type:1 定制
DWORD GetClientType(); 

//获取是否需要默认启动SyncDat.exe程序下载dat文件
BOOL GetUpdateDatCfg();

//获取隐藏版客户的的标记
DWORD GetHideType();

//设置隐藏版客户的标记
void SetHideType();

//封装升级请求结构体
void GetEncodeUpdateData(CString& strEnCodeData, DWORD dwClientType);

//将简写目录转换为实际的目录
CString GetDirectory(CString strShortDir);