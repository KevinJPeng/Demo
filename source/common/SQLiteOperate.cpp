#include "StdAfx.h"
#include "SQLiteOperate.h"


SQLiteOperate::SQLiteOperate(void)
{
}


SQLiteOperate::~SQLiteOperate(void)
{
}

/*
	@brief 初始化数据库
	@return true成功初始化
	*/
void SQLiteOperate::InitDb()
{
	memset(szDbPath, 0, sizeof(szDbPath));
	GetModuleFileName(NULL, szDbPath, MAX_PATH);

	PathAppend(szDbPath, _T("..\\..\\"));
	PathAppend(szDbPath, _T("db\\post.db"));

	char *pBuf = WideToMulti(szDbPath, CP_UTF8);

	if (pBuf != NULL)
	{
		sqlite.Open(pBuf);


		if (!IsExistTable("ServerData"))          //初始化表格
		{
			InitTables();
		}

		delete []pBuf;
	}

	

}


/*
	@brief  创建数据库表格
	*/
void SQLiteOperate::InitTables()
{
	try
	{
		std::string  strSql;

		strSql = "create table ServerData(id INTEGER PRIMARY KEY AUTOINCREMENT \
			, szData    VARCHAR(100000)\
			, type  INTEGER\
			, UserName    VARCHAR(256)\
			, insertDate VARCHAR(11));\
			\
			create table UserInfo(UserName varchar(256) primary key\
			, PassWord varchar(256)\
			, SavePwd INTEGER\
			, AutoLogin INTEGER\
			, ProDuctId INTEGER);";
			
		sqlite.DirectStatement(strSql);
	}
	catch (...)
	{}
}

/*
	@brief  判断表是否存在
	@param  pTableName 表名
	@return true为存在
	*/
bool SQLiteOperate::IsExistTable( const char *pTableName )
{
	bool bRes = false;

	try
	{
		std::string  strSql;
		ResultTable  resTable;

		strSql = "SELECT * FROM sqlite_master where type='table' and name='";
		strSql.append(pTableName);
		strSql.append("'");

		SelectStmt(strSql, resTable);

		if (resTable.next() != NULL)
		{
			bRes = true;
		}

	}
	catch (...)
	{

	}


	return bRes;
}


/*
	@brief 查询结果函数
	@param  stmt  查询sql语句
	@param  [in/out]  数据返回结果
	@return  true为执行成功
	*/
bool SQLiteOperate::SelectStmt( std::string const& stmt, ResultTable& res )
{
	CLocalLock local(&m_Lock);

	return sqlite.SelectStmt(stmt, res);
}

/*
	@brief 直接执行sql语句函数
	@param  stmt  sql语句
	@return  true为执行成功
	*/
bool SQLiteOperate::DirectStatement( std::string const& stmt )
{
	CLocalLock local(&m_Lock);

	return sqlite.DirectStatement(stmt);
}


/*
	@brief  保存用户名和密码
	@param  userInfo 用户信息结构体
	*/
void SQLiteOperate::SaveUserInfo(UserInfo const &userInfo )
{
	try
	{
		CStringA  strSql;
		ResultTable  restTable;

		strSql.Format("SELECT UserName,PassWord  FROM UserInfo where UserName='%s'", userInfo.szUName);

		if (SelectStmt(strSql.GetString(), restTable))
		{
			if (restTable.next() != NULL)
			{
				strSql.Format("update UserInfo set PassWord='%s',SavePwd=%d,AutoLogin=%d,ProDuctId=%d  where UserName='%s'"
					, userInfo.szPwd
					, userInfo.iSavePwd
					, userInfo.iAutoLogin
					, userInfo.iProduceId
					, userInfo.szUName);
			}
			else
			{
				strSql.Format("insert into UserInfo(UserName,PassWord,SavePwd,AutoLogin,ProDuctId) values('%s','%s',%d,%d,%d)"
					, userInfo.szUName
					, userInfo.szPwd
					, userInfo.iSavePwd
					, userInfo.iAutoLogin
					, userInfo.iProduceId);
			}


			DirectStatement(strSql.GetString());				
		}
	}
	catch (...)
	{
	}	
}


/*
	@brief  取用户名和密码
	@param  [in out]userInfo 保存结果
	@return true取得成功
	*/
bool SQLiteOperate::GetUserInfo(UserInfo &userInfo )
{
	try
	{
		CStringA  strSql;
		ResultTable resTable;
		ResultRecord *resRecord = NULL;
		strSql.Format("SELECT UserName,PassWord,SavePwd,AutoLogin,ProDuctId  FROM UserInfo");

		SelectStmt(strSql.GetString(), resTable);
		if ((resRecord = resTable.next()) != NULL)
		{
			
			strncpy(userInfo.szUName, resRecord->fields_[0].c_str(), MAX_USER_PATH-1);
			strncpy(userInfo.szPwd, resRecord->fields_[1].c_str(), MAX_USER_PATH-1);

			userInfo.iSavePwd = atoi(resRecord->fields_[2].c_str());
			userInfo.iAutoLogin = atoi(resRecord->fields_[3].c_str());
			userInfo.iProduceId = atoi(resRecord->fields_[4].c_str());

			return true;
			
		}
	}
	catch (...)
	{
	}

	return false;

}

/*
	@brief  删除用户信息
	*/
void SQLiteOperate::DeleteUserInfo()
{
	try
	{
		CStringA  strSql;
		strSql.Format("delete from UserInfo");
	
		DirectStatement(strSql.GetString());

	}
	catch (...)
	{
		//g_log.Trace(LOGL_TOP, LOGT_PROMPT, __TFILE__, __LINE__, _T("删除用户信息出错"));
	}
}

/*
	@brief  宽字符转多字节
	@param  要转化的宽字符串
	@return 返回多字节
	*/
char * SQLiteOperate::WideToMulti( const wchar_t *pWide, DWORD dwCode /*= CP_ACP*/ )
{
	char *pChar = NULL;
	int  iWlen = 0;

	if (pWide == NULL
		|| (iWlen = wcslen(pWide)) == 0)
	{
		return pChar;
	}

	int iLen = WideCharToMultiByte( dwCode, 0, pWide, iWlen, NULL, NULL, NULL, NULL );
	if (iLen > 0)
	{
		pChar = new char[iLen + 1];
		if (pChar != NULL)
		{
			memset(pChar, 0, iLen+1);
			WideCharToMultiByte( dwCode, 0, pWide, iWlen, pChar, iLen, NULL, NULL );
		}
	}

	return pChar;
}


/*
	@brief  多字节转宽字符
	@param  要转化的多字符串
	@return 返回宽字节
	*/
wchar_t * SQLiteOperate::MultitoWide( const char *pMulti, DWORD dwCode /*= CP_ACP*/ )
{
	wchar_t *pWide = NULL;
	int iAlen = 0;

	if (pMulti == NULL
		|| (iAlen = strlen(pMulti)) == 0)
	{
		return pWide;
	}

	int iLen = MultiByteToWideChar( dwCode, 0, pMulti, iAlen, NULL, NULL );
	if (iLen > 0)
	{
		pWide = new wchar_t[iLen + 1];
		if (pWide != NULL)
		{
			memset(pWide, 0, (iLen+1)*sizeof(wchar_t));
			MultiByteToWideChar( dwCode, 0, pMulti, iAlen, pWide, iLen );
		}

	}

	return pWide;
}
