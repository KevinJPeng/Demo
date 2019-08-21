#ifndef _ALIOSSAPI_H_
#define _ALIOSSAPI_H_

#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"

// #pragma once  
// #pragma execution_character_set("utf-8")

#define HEAPSIZE (512)
typedef struct _OssAccessInfo
{
public:
	CString sOssEndpoint;
	CString sAccessKeyId;
	CString sAccessKeySecret;
	CString sBucketName;
	_OssAccessInfo()
	{
		sOssEndpoint.Empty();
		sAccessKeyId.Empty();
		sAccessKeySecret.Empty();
		sBucketName.Empty();
	}
}OssAccessInfo, *pOssAccessInfo;

class CAliOssApi
{
public:
	CAliOssApi();
	~CAliOssApi();
	bool Init();
	void UnInit();
	bool put_object_from_file(CString _strFilePath, vector<CString> _vsFileName, vector<CString>& _vCacheUploadFailData, map<CString, CString>& _mapPageName);
	char *ToMultiByte(LPCWSTR pszSource, int nLanguage);
//	char* CStringToMutilChar(CString str, int nLanguage);

private:
	void init_sample_request_options(oss_request_options_t *options, int is_cname, char* _pOssEndpoint, char* _pAccessKeyId, char* _pBucketName);
public:
	OssAccessInfo m_ossInfo;
};

#endif