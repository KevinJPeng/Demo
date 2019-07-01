#ifndef _USERDEFINE_H_
#define _USERDEFINE_H_
//#include <minwindef.h>


//0�����ϣ�50��	1�����£�253��	2������	
#define RUN_DEBUG 0		


//����汾��
const TCHAR* const g_sVersion = _T("1&1.0.00");


#if(RUN_DEBUG == 0)
	//web��������ַ��ClientUpdateService.exe���ڵ�ַ��
	const string g_sServerUrl = "http://112.74.102.50:2015/downloadfilelist/index";
	//�����������ص�ַ
	const TCHAR* const g_sUpdateSelfUrl = _T("http://112.74.102.50:8075/MagicBox.exe");
	//��װ�����ص�ַ
	const TCHAR* const g_sAPKDLUrl = _T("http://112.74.102.50:8075/MasterZ_Custom_93.exe");
#elif(RUN_DEBUG == 1)
	//web��������ַ��ClientUpdateService.exe���ڵ�ַ��
	const string g_sServerUrl = "http://192.168.1.253:2015/downloadfilelist/index";
	//�����������ص�ַ
	const TCHAR* const g_sUpdateSelfUrl = _T("http://112.74.102.50:8075/MagicBox.exe");
	//��װ�����ص�ַ
	const TCHAR* const g_sAPKDLUrl = _T("http://112.74.102.50:8075/MasterZ_Custom_93.exe");
#else
	//web��������ַ��ClientUpdateService.exe���ڵ�ַ��
	const string g_sServerUrl = "http://127.0.0.1:2015";
	//�����������ص�ַ
	const TCHAR* const g_sUpdateSelfUrl = _T("http://112.74.102.50:8075/MagicBox.exe");
	//��װ�����ص�ַ
	const TCHAR* const g_sAPKDLUrl = _T("http://112.74.102.50:8075/MasterZ_Custom_93.exe");
#endif


// web��������ַ��ClientUpdateService.exe���ڵ�ַ��
// const string g_sServerUrl = "http://127.0.0.1:2015";
// �����������ص�ַ
// const TCHAR* const g_sUpdateSelfUrl = _T("http://112.74.102.50:8075/MagicBox.exe");
// ��װ�����ص�ַ
// const TCHAR* const g_sAPKDLUrl = _T("http://112.74.102.50:8075/MasterZ_Custom_93.exe");




//��������Ҽ���Ϣ
const LPARAM MOUSE_RIGHT_ACTIVE = 0x2040001;

/*-------------------------------------��ض�ʱ��ID����--------------------------------------*/
//���ذ�װ��
const UINT ID_TIMER_DL_CLIENTPACKAGE = 200;
//����ˢ�¶�ʱ��
//const UINT ID_TIMER_UI_REDRAW = 201;
//
const UINT ID_TIMER_UI_REFRESH = 202;
const UINT ID_TIMER_UI_HOWLONG = 203;

/*--------------------------------------������س�������-------------------------------------*/
//Common
const TCHAR* const sButtonControlName_Ok = _T("Button_OK");
const TCHAR* const sProgressControlName_DL = _T("Progress_DL");
//SKIN_DEFAULT
const TCHAR* const sLabel_Show_Start = _T("Label_Show_Start");
const TCHAR* const sLabel_Show_Exit = _T("Label_Show_Exit");
const TCHAR* const sControl_Loading = _T("Control_loading");
const TCHAR* const sControl_Black = _T("Control_Blak");
const TCHAR* const sButtonGif_Loading = _T("ButtonGif");

const TCHAR* const sLabel_FunctionDeclaration = _T("Label_Function_Declaration");
const TCHAR* const sLabel_ExitTip = _T("Label_Exit_Tip");
const int c_iExit_initTime = 5;

/*--------------------------------------�Զ�����Ϣ-------------------------------------*/
//SKIN_DEFAULT
const LPARAM USERMSG_PROCESS_EXIT = 1000;
const LPARAM USERMSG_SINGLE_RUN = 1100;


enum
{
	CLIENT_NULL = 0,
	CLIENT_GET_SERVERINFO_FAIL,	//��ȡ��������Ϣʧ��
	CLIENT_EXIST,				//�ͻ����Ѱ�װ
	CLIENT_OPEN_MUTEX_FAIL,		//����������ʧ��
	CLIENT_CLIENT_RUN,			//�ͻ�������������
	CLIENT_START_SUCCESS,		//�����ͻ��˳ɹ�
	CLIENT_START_FAIL,			//�����ͻ���ʧ��
	CLIENT_SELECTSERVERFAIL,	//ѡ�������ʧ��
	CLIENT_DL_FAIL,				//���ؿͻ��˰�װ��ʧ��
	CLIENT_INSTALL_FAIL,		//�ͻ��˰�װʧ��
	CLIENT_INSTALL_SUCCESS		//�ͻ��˰�װ�ɹ�
};


enum ENUM_LOGGER
{
	L_MAIN = 0,		//the main logger, It away exist.
	L_MagicBox,		//the user-defined logger.
	L_LOG_SUM
};
//��������״̬
//ui״̬
enum E_UI_STATUS
{
	CLIENT_UI_NULL = 0,		//��ʼ״̬
	CLIENT_UI_START,		//UI����
	CLIENT_UI_EXIT,			//UI�˳�
};
//����״̬
enum E_DL_STATUS
{
	CLIENT_DL_NULL = 0,		//��ʼ״̬
	CLIENT_DL_START,		//���ؿ�ʼ
	CLIENT_DL_OVER,			//�������
};

#endif