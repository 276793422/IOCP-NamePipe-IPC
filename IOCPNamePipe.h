//
//					����˹�������
//
//	��������ˣ�
//			��������������̣߳�һ���߳����������ͻ�����һ���߳�������������
//
//	�������ݣ�
//			���﷢�Ͳ������ף���Ҫ��һ����֪��HANDLE�������ͣ�
//			���HANDLE���Ǵӿͻ����б���������ȡ�õģ�
//			���߽������ݵ�ʱ�򣬲���1���صģ�
//			�ҷ���ʱ����ȥ�ͻ����б�����ȷ�����HANDLE�Ƿ���Ч��
//			�����Ч�ŷ���
//
//	�������ݣ�
//			����һ��פ���߳�һֱ����IOCP�ķ�ʽ�������ݣ�
//				���յ�����֮�󣬰����ݷŵ�һ�������У�
//			Ȼ���û��Ľ��ղ�����ʵ����Ϊ�Ӷ����еĻ�ȡ������
//
//	�رշ���ˣ�
//			����˹رյ�ʱ�򣬻�رս����̺߳����ݽ����߳�
//			���ȹر�ѭ����ʶ��
//			Ȼ�󴴽�һ���µ����ӣ�ռ�ý����̵߳����ӣ������߳��˳�
//			���ڹر���ѭ����ʶ�����Խ����߳�Ҳ�����˳�
//			���������̶߳����԰�ȫ�˳�
//			�����̶߳���ȫ�˳�֮��
//			ѭ����Ѱ�ͻ����б�������������пͻ��ˣ�һ��һ���رգ�
//			��Դ�������
//
//	�ͻ��˹ر����ӣ�
//			����˻��֪���ͻ��˹رգ�Ȼ��ӿͻ����б����������ͻ���
//			������Դ
//
//
//
//					�ͻ��˹�������
//
//	�����ͻ��ˣ�
//			���ӷ���ˣ������߳̽�������
//
//	�������ݣ�
//			ֱ�ӷ���
//
//	�������ݣ�
//			����һ��פ���߳�һֱ�����ص�IO�ķ�ʽ��������
//				���յ�����֮�󣬰����ݷŵ�һ��������
//			Ȼ���û��Ľ��ղ�����ʵ����Ϊ�Ӷ����еĻ�ȡ����
//
//	�رտͻ��ˣ�
//			���߳�һ���źţ������˳�
//			Ȼ��������Դ
//
//	����˹ر����ӣ�
//			��ʱ���̻߳��֪������˹رգ�Ȼ����ͣ��ǰ�ͻ���״̬
//			��ʱ������Ϣ��ʱ�򣬻��жϳ���ǰ״̬Ϊֹͣ״̬�����Է���Ϣ��ʧ��
//			������Ϣ�ǴӶ���������գ�������������������ݣ���Ȼ���ã�����ʧ��
//



#include <windows.h>

#ifdef _DEBUG
#include <stdio.h>
#define DBGPRINT(XXX) printf XXX
#else
#define DBGPRINT(XXX)
#endif

#pragma once

enum NAME_PIPE_STATUS
{
	NAME_PIPE_STATUS_RUNNING		= 1 ,
	NAME_PIPE_STATUS_STOPPING		= 2 ,
};

//	����Ԫ�ص�������
enum{ IOCP_NAMEPIPE_SERVER_INFO_ARR_MAX = 256 };

class CMessageArrayClass
{
	//	��Ϣ���нṹ�壬���洢 IOCP_NAMEPIPE_SERVER_INFO_ARR_MAX ��
	struct 
	{
		struct
		{
			LPVOID pPipe;
			DWORD dwLen;
			BYTE *buf;
		}MessageArr[IOCP_NAMEPIPE_SERVER_INFO_ARR_MAX];
		DWORD dwRead;	//	��ǰ��ȡ������������ٴε���ȡ����ʱ����Ҫ����ƫ�ƣ�Ȼ���ٻ�ȡ
		DWORD dwWrite;	//	��ǰд�������������ٴε���д����ʱ����Ҫ����ƫ�ƣ�Ȼ���ٻ�ȡ
		DWORD dwMessageCount;
		CRITICAL_SECTION cs;
	}m_sMessageArray;

public:
	//	��ʼ��
	DWORD Init();

	//	��ȡ��Ϣ����
	DWORD GetMessageCount();
	//	�Ž�ȥһ��MSG
	DWORD SetAMessageIntoArray(LPVOID pPipe, BYTE *buf, DWORD dwLen);
	//	ȡ��һ��MSG
	DWORD GetAMessageFromArray(LPVOID *pPipe, BYTE **buf, DWORD *dwLen);
	//	ȡ��һ��MSG����� dwMaxLen < dwLen ����ʧ�ܣ�dwLen �������賤��
	//		��������µ�ʧ�ܣ�dwLen ���� 0
	DWORD GetAMessageFromArrayTestLen(LPVOID *pPipe, BYTE **buf, DWORD *dwLen, DWORD dwMaxLen);
};

//	ָ�����Ԫ�ص���������Ҳ����˵�ͻ��˸�������Ϊ IOCP_POINT_ARR_MAX ��
enum{ IOCP_POINT_ARR_MAX = 256 };

class CPointArrayClass
{
	struct
	{
		struct
		{
			PVOID point;	//	��ǰԪ��
			PVOID info;		//	��ǰԪ����Ϣ
		}sArray[IOCP_POINT_ARR_MAX];
		DWORD dwOnUse;		//	�ڲ�����ʹ�õĸ���
		CRITICAL_SECTION cs;
	} m_sArray;

public:
	//	��ʼ��
	DWORD Init();
	//	����
	DWORD Insert( PVOID point , PVOID info );
	//	��ȡ������������ֻ��ȡ����ɾ��
	DWORD GetOn( DWORD index , PVOID *point , PVOID *info );
	//	�Ƴ���������������ȡ����ɾ��
	DWORD Remove( DWORD index , PVOID *point , PVOID *info );
	//	�Ƴ�������ָ�룬��ȡ����ɾ��
	DWORD FindRemove( PVOID point , PVOID *info );
	//	�ж���Ч
	DWORD IsHere( PVOID point );
	//	��ȡ����
	DWORD GetCount();
};

class CIOCPNamePipe
{
public:
	CIOCPNamePipe();
	virtual ~CIOCPNamePipe();
	virtual DWORD Create(WCHAR *name, DWORD dwMode) = 0;				//	����/�����ܵ�
	virtual DWORD Recv(HANDLE *hFrom, BYTE *bBuf, DWORD *dwLen) = 0;	//	��������
	virtual DWORD Send(HANDLE hTo, BYTE *bBuf, DWORD dwLen) = 0;		//	��������
	virtual DWORD Close() = 0;											//	�رչܵ�
};

//////////////////////////////////////////////////////////////////////////

class CIOCPNamePipeServer : public CIOCPNamePipe
{
public:
	CIOCPNamePipeServer();

	//	��������
	DWORD Create(WCHAR *name, DWORD dwMode = PIPE_READMODE_MESSAGE);

	//	�ر�����
	DWORD Close();

	//	�������ݣ���� hTo ����Ϊ -1 ����Ϊ�㲥��Ϣ
	DWORD Send(HANDLE hTo, BYTE *bBuf, DWORD dwLen);

	//	��������
	DWORD Recv(HANDLE *hFrom, BYTE *bBuf, DWORD *dwLen);

private:
	WCHAR m_wsName[MAX_PATH];		//	�ܵ�����

	DWORD m_dwStatus;				//	״̬

private:
	DWORD m_dwRun;								//	�߳�����ִ�У����Ϊ0 �����߳��˳�
	HANDLE m_hCompletionPort;					//	��ɶ˿�ʵ��

	HANDLE m_hControllerThread;					//	�������߳̾��
	void ControllerThreadProc( void *pParam );	//	�������߳�ִ����

	HANDLE m_hWorkerThread;						//	�������߳̾��
	void WorkerThreadProc( void *pParam );		//	�������߳�ִ����

	void WorkerProcCallBack( void *pParam );	//	�����߻ص�
												//	�����������ִ��ʱ�������и������Ѿ����յ���

	//	��Ϣ����
	CMessageArrayClass m_sMessageArray;

	//	�ڲ�����������б�
	CPointArrayClass m_sPointArray;

	//	�������߳� shell
	friend void start_address_controller( void *pParam );
	//	�������߳� shell
	friend void start_address_worker( void *pParam );
};

//////////////////////////////////////////////////////////////////////////

typedef struct 
{ 
	OVERLAPPED oOverlap;
} PIPE_CLIENT_CONTROL;

class CIOCPNamePipeClient : public CIOCPNamePipe
{
public:
	CIOCPNamePipeClient();

	DWORD Create(WCHAR *name, DWORD dwMode = PIPE_READMODE_MESSAGE);

	DWORD Close();

	DWORD Send(HANDLE hTo, BYTE *bBuf, DWORD dwLen);

	DWORD Recv(HANDLE *hFrom, BYTE *bBuf, DWORD *dwLen);

private:
	WCHAR m_wsName[MAX_PATH];					//	�ܵ�����

	DWORD m_dwStatus;							//	״̬

private:
	HANDLE m_hPipe;								//	�ܵ����

	DWORD m_dwRun;								//	�߳�����ִ�У����Ϊ0 �����߳��˳�

	HANDLE m_hRecvThread;						//	�����߳̾��
	void ClientRecvThreadProc( void *pParam );	//	�������߳�ִ����

	//	��Ϣ����
	CMessageArrayClass m_sMessageArray;

	PIPE_CLIENT_CONTROL m_pipeClient;			//	�ͻ��˵ĵ���ɽṹ��Ϊ�˷�ֹ�ͻ��˶�дʱ��������

	friend void start_address_worker_clinet( void *pParam );
};