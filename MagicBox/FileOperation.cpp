#include "stdafx.h"
#include "FileOperation.h"



CFileOperation::CFileOperation()
{

}
CFileOperation::~CFileOperation()
{

}

// �ж��ļ��Ƿ����
BOOL CFileOperation::IsFileExist(const CStdString& csFile)
{
	DWORD dwAttrib = GetFileAttributes(csFile);
	return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}
// �ж��ļ����Ƿ����
BOOL CFileOperation::IsDirExist(const CStdString & csDir)
{
	DWORD dwAttrib = GetFileAttributes(csDir);
	return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 != (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}
// �ж��ļ����ļ����Ƿ����
BOOL CFileOperation::IsPathExist(const CStdString & csPath)
{
	DWORD dwAttrib = GetFileAttributes(csPath);
	return INVALID_FILE_ATTRIBUTES != dwAttrib;
}

// �ж��ļ����ļ����Ƿ���ڼ�ǿ��(��˵�����һ��)
BOOL CFileOperation::IsPathExistEx(const CStdString & csPath)
{
	WIN32_FILE_ATTRIBUTE_DATA attrs = { 0 };
	return 0 != GetFileAttributesEx(csPath, GetFileExInfoStandard, &attrs);
}
