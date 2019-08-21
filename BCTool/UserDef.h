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
	const TCHAR* const g_sUpdateSelfUrl = _T("http://112.74.102.50:8075/BCTool.exe");
	//��װ�����ص�ַ
	const TCHAR* const g_sAPKDLUrl = _T("http://112.74.102.50:8075/MasterZ_Custom_93.exe");
#elif(RUN_DEBUG == 1)
	//web��������ַ��ClientUpdateService.exe���ڵ�ַ��
	const string g_sServerUrl = "http://192.168.1.253:2015/downloadfilelist/index";
	//�����������ص�ַ
	const TCHAR* const g_sUpdateSelfUrl = _T("http://112.74.102.50:8075/BCTool.exe");
	//��װ�����ص�ַ
	const TCHAR* const g_sAPKDLUrl = _T("http://112.74.102.50:8075/MasterZ_Custom_93.exe");
#else
	//web��������ַ��ClientUpdateService.exe���ڵ�ַ��
	const string g_sServerUrl = "http://127.0.0.1:2015";
	//�����������ص�ַ
	const TCHAR* const g_sUpdateSelfUrl = _T("http://112.74.102.50:8075/BCTool.exe");
	//��װ�����ص�ַ
	const TCHAR* const g_sAPKDLUrl = _T("http://112.74.102.50:8075/MasterZ_Custom_93.exe");
#endif


// web��������ַ��ClientUpdateService.exe���ڵ�ַ��
// const string g_sServerUrl = "http://127.0.0.1:2015";
// �����������ص�ַ
// const TCHAR* const g_sUpdateSelfUrl = _T("http://112.74.102.50:8075/BCTool.exe");
// ��װ�����ص�ַ
// const TCHAR* const g_sAPKDLUrl = _T("http://112.74.102.50:8075/MasterZ_Custom_93.exe");




//��������Ҽ���Ϣ
const LPARAM MOUSE_RIGHT_ACTIVE = 0x2040001;


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
// const LPARAM USERMSG_PROCESS_EXIT = 1000;
// const LPARAM USERMSG_SINGLE_RUN = 1100;


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
	L_BCTOOL,		//the user-defined logger.
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

enum
{
	//������ʾ��Ϣ
	//WPARAM:NULL   
	//LPARAM: CString ָ��  ��ʽ��TipTitle##TipContent##TipTime
	MSG_TIP_MSG = WM_USER + 10,

	//������Ϣ
	//lParam������Ϊ
	WM_TRAY_MSG,

	//�������ѡ����Ϣ
	//WPARAM:�˵����б��   
	//LPARAM:0
	MSG_TRAY_FUNCTION_MSG,

	//ģ̬�����Ի�����Ϣ
	//WPAPM: 0
	//LPARAM:0
	WM_SHOW_MODAL,

	//��������Ϣ
	//WPARAM: 0	��Ϣ����
	MSG_FROM_SERVWE,

	//�����˳�
	MSG_PROCESS_EXIT,
};

enum msgparam
{
	//WM_SHOW_MODA
	WPARAM_SHOW_SAFEEXIT_WMD = 0,						//������ȫ�˳��Ի���

	//MSG_TRAY_FUNCTION_MSG
	WPARAM_SHOW_CLIENT = 0,								//��ʾ�ͻ���
	WPARAM_SAFE_EXIT = 1,								//��ȫ�˳�

	//MSG_FROM_SERVWE
	WPARAM_NEWINFO_MSG = 0,

	//WM_TIMER
	WPARAM_TIMER_DL_CLIENTPACKAGE = 0,				//�ͻ��˰�װ������
	WPARAM_TRAYICON_TWINKLING = 1,					//������˸
	WPARAM_ICON_TWINKLING = 2,					//������˸

	//MSG_PROCESS_EXIT
	WPARAM_PROCESS_EXIT_MANUAL = 0,
	WPARAM_PROCESS_EXIT_AUTO = 1,
};
#endif