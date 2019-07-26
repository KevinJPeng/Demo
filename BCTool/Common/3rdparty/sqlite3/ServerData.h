#pragma once
#include <vector>
#define DATETIMELEN 11
#define MAX_USER_PATH 256

#define MID_LENGTH  50



typedef struct UserInfo
{
	char szUName[MAX_USER_PATH];       //用户名
	char szPwd[MAX_USER_PATH];         //用户密码
	int iSavePwd;                      //是否保存账号密码      0不保存  1保存
	int iAutoLogin;                    //是否自动登录          0不自动登录 1自动登录
	int iProduceId;                    //产品id

	UserInfo()
	{
		memset(szUName, 0, sizeof(szUName));
		memset(szPwd, 0, sizeof(szPwd));
		iSavePwd = 1;
		iAutoLogin = 1;
		iProduceId = 0;
	}
}UserInfo;
