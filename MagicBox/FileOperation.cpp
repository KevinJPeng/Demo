#include "stdafx.h"
#include "FileOperation.h"



CFileOperation::CFileOperation()
{

}
CFileOperation::~CFileOperation()
{

}

// 判断文件是否存在
BOOL CFileOperation::IsFileExist(const CStdString& csFile)
{
	DWORD dwAttrib = GetFileAttributes(csFile);
	return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}
// 判断文件夹是否存在
BOOL CFileOperation::IsDirExist(const CStdString & csDir)
{
	DWORD dwAttrib = GetFileAttributes(csDir);
	return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 != (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}
// 判断文件或文件夹是否存在
BOOL CFileOperation::IsPathExist(const CStdString & csPath)
{
	DWORD dwAttrib = GetFileAttributes(csPath);
	return INVALID_FILE_ATTRIBUTES != dwAttrib;
}

// 判断文件或文件夹是否存在加强版(听说会更快一点)
BOOL CFileOperation::IsPathExistEx(const CStdString & csPath)
{
	WIN32_FILE_ATTRIBUTE_DATA attrs = { 0 };
	return 0 != GetFileAttributesEx(csPath, GetFileExInfoStandard, &attrs);
}
