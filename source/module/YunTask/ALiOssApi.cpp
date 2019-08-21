#include "StdAfx.h"
#include "AliOssApi.h"

#pragma once  
#pragma execution_character_set("utf-8")  

CAliOssApi::CAliOssApi()
{

}
CAliOssApi::~CAliOssApi()
{
}

bool CAliOssApi::Init()
{
	// initialize http io system, call it olny once
	if (aos_http_io_initialize(NULL, 0) != AOSE_OK) 
	{
		return false;
	}
	if (m_ossInfo.sOssEndpoint.IsEmpty() || m_ossInfo.sAccessKeyId.IsEmpty() || m_ossInfo.sAccessKeySecret.IsEmpty() || m_ossInfo.sBucketName.IsEmpty())
	{
		return false;
	}

	return true;
}
void CAliOssApi::UnInit()
{
	aos_http_io_deinitialize();
}
void CAliOssApi::init_sample_request_options(oss_request_options_t *options, int is_cname, char* _pOssEndpoint, char* _pAccessKeyId, char* _pBucketName)
{
	options->config = oss_config_create(options->pool);

	_pOssEndpoint = ToMultiByte(m_ossInfo.sOssEndpoint.GetBuffer(), CP_UTF8);
	_pAccessKeyId = ToMultiByte(m_ossInfo.sAccessKeyId.GetBuffer(), CP_UTF8);
	_pBucketName = ToMultiByte(m_ossInfo.sAccessKeySecret.GetBuffer(), CP_UTF8);
	aos_str_set(&options->config->endpoint, _pOssEndpoint);
	aos_str_set(&options->config->access_key_id, _pAccessKeyId);
	aos_str_set(&options->config->access_key_secret, _pBucketName);
	options->config->is_cname = is_cname;
	options->ctl = aos_http_controller_create(options->pool, 0);
}

char * CAliOssApi::ToMultiByte(LPCWSTR pszSource, int _nLanguage)
{
	int nLanguage = _nLanguage/*CP_UTF8*/;

	int nLength = WideCharToMultiByte(nLanguage, 0, pszSource, wcslen(pszSource), NULL, 0, NULL, FALSE);

	char *pBuffer = NULL;
	pBuffer = new char[nLength + 2];
	memset(pBuffer, 0, nLength + 2);
	if (!pBuffer)
	{
		return NULL;
	}
	WideCharToMultiByte(nLanguage, 0, pszSource, wcslen(pszSource), pBuffer, nLength, NULL, FALSE);
	pBuffer[nLength + 1] = 0;

	return pBuffer;
}
// char* CAliOssApi::CStringToMutilChar(CString str, int nLanguage)
// {
// 	char* pszMultiByte;
// 	int iSize = WideCharToMultiByte(nLanguage, 0, str, -1, NULL, 0, NULL, NULL);
// 	pszMultiByte = (char*)malloc((iSize + 1));
// 	memset(pszMultiByte, 0, iSize + 1);
// 	WideCharToMultiByte(nLanguage, 0, str, -1, pszMultiByte, iSize, NULL, NULL);
// 	return pszMultiByte;
// }

bool CAliOssApi::put_object_from_file(CString _strFilePath, vector<CString> _vsFileName, vector<CString>& _vCacheUploadFailData, map<CString, CString>& _mapPageName)
{
	bool bReValue = false;
	if (!Init())
	{
		return false;
	}
	aos_pool_t *p = NULL;
	aos_string_t bucket;
	aos_string_t object;
	int is_cname = 0;
	aos_table_t *headers = NULL;
	aos_table_t *resp_headers = NULL;
	oss_request_options_t *options = NULL;
	aos_status_t *s = NULL;
	aos_string_t file;

	char* pcOssEndpoint = NULL;
	char* pcAccessKeyId = NULL;
	char* pcAccessKeySecret = NULL;
	aos_pool_create(&p, NULL);
	options = oss_request_options_create(p);
	init_sample_request_options(options, is_cname, pcOssEndpoint, pcAccessKeyId, pcAccessKeySecret);

	char* pBucketName = ToMultiByte(m_ossInfo.sBucketName.GetBuffer(), CP_UTF8);;
	aos_str_set(&bucket, pBucketName);

	CString strFilePath;
	char *objectNmae = NULL;
	char *filename = NULL;

	vector<CString> vsFileName = _vsFileName;
	int ivLen = vsFileName.size();

	for (int i = 0; i < ivLen; ++i)
	{
		//vsFileName[i].Replace(_T("http:oss"), _T(""));
// 		CString sOssPath = vsFileName[i];
// 		int ipos = vsFileName[i].Find(_T("/"));
// 		if (ipos > 0)
// 		{
// 			vsFileName[i] = vsFileName[i].Mid(ipos+1);
// 		}

		resp_headers = NULL;
		headers = NULL;
		headers = aos_table_make(options->pool, 1);
		apr_table_set(headers, /*OSS_CONTENT_TYPE*/"Content-Type", "text/html");

		strFilePath = _strFilePath;
		strFilePath = strFilePath + vsFileName[i];
//		strFilePath.Replace(_T("/"), _T("\\"));
		objectNmae = ToMultiByte(vsFileName[i].GetBuffer(), CP_UTF8);
		filename = ToMultiByte(strFilePath.GetBuffer(), CP_UTF8);
		aos_str_set(&object, objectNmae);
		aos_str_set(&file, filename);


		s = oss_put_object_from_file(options, &bucket, &object, &file,
			headers, &resp_headers);

		if (!aos_status_is_ok(s))
		{
			g_log.Trace(LOGL_LOW, LOGT_ERROR, __TFILE__, __LINE__, _T("提交单个快照失败，快照路径：%s, code=%d,error_msg=%s"), strFilePath, s->code, s->error_msg);
			CString strCache = _mapPageName[_vsFileName[i]];
			_vCacheUploadFailData.push_back(strCache);
		}
		else
		{
			if (!bReValue)
			{
				bReValue = true;
			}
		}
		//内存清理
		if (NULL != objectNmae)
		{
			delete[] objectNmae;
			objectNmae = NULL;
		}
		if (NULL != filename)
		{
			delete[] filename;
			filename = NULL;
		}
	}
	if (NULL != pBucketName)
	{
		delete[] pBucketName;
		pBucketName = NULL;
	}
	if (NULL != pcOssEndpoint)
	{
		delete[] pcOssEndpoint;
		pcOssEndpoint = NULL;
	}
	if (NULL != pcAccessKeyId)
	{
		delete[] pcAccessKeyId;
		pcAccessKeyId = NULL;
	}
	if (NULL != pcAccessKeySecret)
	{
		delete[] pcAccessKeySecret;
		pcAccessKeySecret = NULL;
	}
	aos_pool_destroy(p);

	return bReValue;
}
