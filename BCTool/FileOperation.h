#ifndef _FILEOPERATETION_H_
#define _FILEOPERATETION_H_

class CFileOperation
{
public:
	CFileOperation();
	~CFileOperation();

	// �ж��ļ��Ƿ����
	BOOL IsFileExist(const CStdString& csFile);
	// �ж��ļ����Ƿ����
	BOOL IsDirExist(const CStdString & csDir);
	// �ж��ļ����ļ����Ƿ����
	BOOL IsPathExist(const CStdString & csPath);
	// �ж��ļ����ļ����Ƿ���ڼ�ǿ��(��˵�����һ��)
	BOOL IsPathExistEx(const CStdString & csPath);

protected:
private:
};


#endif