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

		int TimeOut = 500;//���ý��ճ�ʱ500��
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
	// Windows�ļ�ʱ����һ��64λ��ֵ�����Ǵ�1601��1��1������12:00������ʱ������
	// ��λ��1/10,000,000�롣��1000���֮1��(100-nanosecond)
	FILETIME ft;
	SYSTEMTIME st;

	// ���Ƚ���׼ʱ�䣨1900��1��1��0��0��0��0���룩ת��ΪWindows�ļ�ʱ��
	st.wYear = 1900;
	st.wMonth = 1;
	st.wDay = 1;
	st.wHour = 0;
	st.wMinute = 0;
	st.wSecond = 0;
	st.wMilliseconds = 0;

	if (0 == SystemTimeToFileTime(&st, &ft))
		return -1;

	// Ȼ��Time Protocolʹ�õĻ�׼ʱ�������ȥ��ʱ�䣨ulTime��
	LONGLONG *pLLong = (LONGLONG *)&ft;

	/* ע�⣺
	�ļ�ʱ�䵥λ��1/1000 0000��(��100ns)��
	��Ҫ����ʱ��server�ϻ�ȡ������Ϊ��λ��ulTime��һ��ת��
	*/
	*pLLong += (LONGLONG)10000000 * ulTime;

	// �ٽ�ʱ��ת������������ϵͳʱ��
	if (0 == FileTimeToSystemTime(&ft, &st))
		return -1;

	TRACE(_T("%04d-%02d-%02d %02d:%02d:%02d\n"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	CTime timeTemp(st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	netTime = timeTemp + CTimeSpan(0, 8, 0, 0);	//��UTCʱ�����8��ʱ��
	return 0;

// 	if (0 == GetTimeDiff(timeTemp))
// 	{
// 		netTime = timeTemp;
// 		return 0;
// 	}
// 
// 	return -1;
}


//����ʱ���һ�£��ܹ�����GetTimeDiff�������㵱��ʱ��������ʱ����Ե�����
int CGetNetTime::GetTimeDiff(CTime &netTime)
{
	float bias;
	long sminute, shour;

	TIME_ZONE_INFORMATION tzinfo;
	DWORD dwStandardDaylight;

	/* ��ȡʱ����Ϣ */
	dwStandardDaylight = GetTimeZoneInformation(&tzinfo); //��ȡʱ����UTC��ʱ��� Ӧ�÷���-8
	if (dwStandardDaylight == TIME_ZONE_ID_INVALID) //��������ʧ��
	{
		return -1;
	}

	/* ʱ����� */
	bias = tzinfo.Bias;
	if (dwStandardDaylight == TIME_ZONE_ID_STANDARD) //��׼ʱ����Ч
		bias += tzinfo.StandardBias;

	if (dwStandardDaylight == TIME_ZONE_ID_DAYLIGHT) //����ʱ��
		bias += tzinfo.DaylightBias;

	shour = bias / 60;
	sminute = fmod(bias, (float)60);

	//netTime = netTime - CTimeSpan(0, shour, sminute, 0);

	return 0;
}