#pragma once


/*
	@brief 对url参数进行编码
	@param strParam参数
	@return 返回已编码好的
	*/
CString EncodeUrlParam( const CString &strParam );

/*
@brief 用户名解密
*/
CString DecodeString(CString &sIn );


/*
@brief url编码
*/
CString  URLEncode(const CString &sIn);


/*
	@brief  宽字符转多字节
	@param  要转化的宽字符串
	@return 返回多字节
	*/
char * WideToMulti( const wchar_t *pWide , DWORD dwCode = CP_ACP );


/*
	@brief  多字节转宽字符
	@param  要转化的多字符串
	@return 返回宽字节
	*/
wchar_t * MultitoWide( const char *pMulti, DWORD dwCode = CP_ACP );