#include "stdafx.h"
#include "UrlOperate.h"

//Unicode CString URLEncode 
BYTE toHex(const BYTE &x)
{
	return x > 9 ? x + 55: x + 48;
}

CString  URLEncode(const CString &sIn)
{
	char* pUrl = WideToMulti(sIn.GetString(), CP_UTF8);
	CStringA strSrc(pUrl);

	CStringA sOut;
	const int nLen = strSrc.GetLength() + 1;

	register LPBYTE pOutTmp = NULL;
	LPBYTE pOutBuf = NULL;
	register LPBYTE pInTmp = NULL;
	LPBYTE pInBuf =(LPBYTE)strSrc.GetBuffer(nLen);

	//alloc out buffer
	pOutBuf = (LPBYTE)sOut.GetBuffer(nLen*3 - 2);//new BYTE [nLen  * 3];

	if(pOutBuf)
	{
		pInTmp   = pInBuf;
		pOutTmp = pOutBuf;

		// do encoding
		while (*pInTmp)
		{
			if(isalnum(*pInTmp))
				*pOutTmp++ = *pInTmp;
			else
				if(isspace(*pInTmp))
					*pOutTmp++ = '+';
				else
				{
					*pOutTmp++ = '%';
					*pOutTmp++ = toHex(*pInTmp>>4);
					*pOutTmp++ = toHex(*pInTmp%16);
				}
				pInTmp++;
		}
		*pOutTmp = '\0';
		//sOut=pOutBuf;
		//delete [] pOutBuf;
		sOut.ReleaseBuffer();
	}
	strSrc.ReleaseBuffer();
	if (pUrl != NULL)
	{
		delete pUrl;
		pUrl = NULL;
	}
	return CString(sOut);
}

CString EncodeUrlParam( const CString &strParam )
{
	CString strData;
	CString strTmp;
	int iPos1 = 0, iPos2 = 0;

	while (true)
	{
		strTmp.Empty();
		iPos1 = strParam.Find(_T('='), iPos2+1);

		if (iPos1 != -1)
		{
			strData += strParam.Mid(iPos2, iPos1-iPos2+1);
			iPos2 = strParam.Find(_T('&'), iPos1 + 1);

			if (iPos2 != -1)
			{
				strTmp = strParam.Mid(iPos1+1, iPos2-iPos1-1);
			}
			else
			{
				strTmp = strParam.Mid(iPos1+1, strParam.GetLength()-iPos1);
			}

			if (!strTmp.IsEmpty())
			{
				strData += URLEncode(strTmp);
			}
		}

		if (iPos1 == -1 || iPos2 == -1)
			break;
	}

	if (strData.IsEmpty() && (!strParam.IsEmpty()))  //如果通过解析之后，数据为空，将整个参数进行编码
	{
		strData += URLEncode(strParam);
	}

	return strData;
}


char * WideToMulti( const wchar_t *pWide , DWORD dwCode )
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

wchar_t * MultitoWide( const char *pMulti, DWORD dwCode /*= CP_ACP*/ )
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

CString DecodeString(CString &sIn )
{
	CString strDecodeData = stringcoding::StringBase64Decode(sIn);

	//进行一次异或操作
	TCHAR ch = _T('①');
	for (int i =0; i < strDecodeData.GetLength(); ++i)
	{	
		DWORD Temp = strDecodeData[i];
		Temp = Temp^ch;
		strDecodeData.SetAt(i,Temp);
	}
	return strDecodeData;
}
