#ifndef _FILEOPERATETION_H_
#define _FILEOPERATETION_H_

class CFileOperation
{
public:
	CFileOperation();
	~CFileOperation();

	// 判断文件是否存在
	BOOL IsFileExist(const CStdString& csFile);
	// 判断文件夹是否存在
	BOOL IsDirExist(const CStdString & csDir);
	// 判断文件或文件夹是否存在
	BOOL IsPathExist(const CStdString & csPath);
	// 判断文件或文件夹是否存在加强版(听说会更快一点)
	BOOL IsPathExistEx(const CStdString & csPath);

protected:
private:
};


#endif