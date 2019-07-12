#include "stdafx.h"
#include "GetNetTime.h"


CGetNetTime::CGetNetTime()
{
}


CGetNetTime::~CGetNetTime()
{
}

int CGetNetTime::GetNetTime(CTime &netTime)
{
	int nRet;

	/* Initialize Winsock */
	WORD wVersionRequested;
	WSADATA wsaData;
	int nErrCode;

	wVersionRequested = MAKEWORD(2, 2);
	nErrCode = WSAStartup(wVersionRequested, &wsaData);
	if (0 != nErrCode)
	{
		return -1;
	}

	/* Get server IP */
	struct hostent *host;
	char *pServerIP;

	host = gethostbyname("time.nist.gov");
	if (NULL == host)
	{
		return -1;
	}

	pServerIP = inet_ntoa(*(struct in_addr*)host->h_addr_list[0]);

	/* Connect to time server, and get time */
	SOCKET sockfd;

	char cTimeBuf[40] = { 0 };
	unsigned long ulTime = 0;
	int nTry = 0;

	do
	{
		if (2 == nTry++)
		{
			return -1;
		}

		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (INVALID_SOCKET == sockfd)
		{
			continue;
		}

		int TimeOut = 500;//设置接收超时500秒
		setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&TimeOut, sizeof(TimeOut));

		sockaddr_in    addr;

		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(37);
		addr.sin_addr.s_addr = inet_addr(pServerIP);

		nRet = connect(sockfd, (sockaddr *)&addr, sizeof(addr));
		if (SOCKET_ERROR == nRet)
		{
			continue;
		}

		nRet = recv(sockfd, (char *)&ulTime, sizeof(ulTime), 0);
		if ((SOCKET_ERROR != nRet) && (0 != nRet))
		{
			break;
		}

		int nErr = WSAGetLastError();
		//TRACE(_T("[%d]%s"), nErr, ConvertErrcodeToString(nErr));

		closesocket(sockfd);
	} while (1);

	closesocket(sockfd);

	unsigned long ulTimehl = ntohl(ulTime);
	if (0 != ConvertTime(ulTimehl, netTime))
		return -1;

	return 0;
}

int CGetNetTime::ConvertTime(unsigned long ulTime, CTime &netTime)
{
	// Windows文件时间是一个64位的值，它是从1601年1月1日中午12:00到如今的时间间隔。
	// 单位是1/10,000,000秒。即1000万分之1秒(100-nanosecond)
	FILETIME ft;
	SYSTEMTIME st;

	// 首先将基准时间（1900年1月1日0点0分0秒0毫秒）转化为Windows文件时间
	st.wYear = 1900;
	st.wMonth = 1;
	st.wDay = 1;
	st.wHour = 0;
	st.wMinute = 0;
	st.wSecond = 0;
	st.wMilliseconds = 0;

	if (0 == SystemTimeToFileTime(&st, &ft))
		return -1;

	// 然后将Time Protocol使用的基准时间加上逝去的时间（ulTime）
	LONGLONG *pLLong = (LONGLONG *)&ft;

	/* 注意：
	文件时间单位是1/1000 0000秒(即100ns)。
	须要将从时间server上获取的以秒为单位的ulTime做一下转换
	*/
	*pLLong += (LONGLONG)10000000 * ulTime;

	// 再将时间转化回来。更新系统时间
	if (0 == FileTimeToSystemTime(&ft, &st))
		return -1;

	TRACE(_T("%04d-%02d-%02d %02d:%02d:%02d\n"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	CTime timeTemp(st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	netTime = timeTemp + CTimeSpan(0, 8, 0, 0);	//与UTC时间相差8个时区
	return 0;

// 	if (0 == GetTimeDiff(timeTemp))
// 	{
// 		netTime = timeTemp;
// 		return 0;
// 	}
// 
// 	return -1;
}


//各地时差都不一致，能够依据GetTimeDiff函数计算当地时差，对上面的时间加以调整：
int CGetNetTime::GetTimeDiff(CTime &netTime)
{
	float bias;
	long sminute, shour;

	TIME_ZONE_INFORMATION tzinfo;
	DWORD dwStandardDaylight;

	/* 获取时区信息 */
	dwStandardDaylight = GetTimeZoneInformation(&tzinfo); //获取时区与UTC的时间差 应该返回-8
	if (dwStandardDaylight == TIME_ZONE_ID_INVALID) //函数运行失败
	{
		return -1;
	}

	/* 时差调整 */
	bias = tzinfo.Bias;
	if (dwStandardDaylight == TIME_ZONE_ID_STANDARD) //标准时间有效
		bias += tzinfo.StandardBias;

	if (dwStandardDaylight == TIME_ZONE_ID_DAYLIGHT) //夏令时间
		bias += tzinfo.DaylightBias;

	shour = bias / 60;
	sminute = fmod(bias, (float)60);

	//netTime = netTime - CTimeSpan(0, shour, sminute, 0);

	return 0;
}