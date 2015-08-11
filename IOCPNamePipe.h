//
//					服务端工作流程
//
//	创建服务端：
//			服务端启动两个线程，一个线程用来接引客户，另一个线程用来接收数据
//
//	发送数据：
//			这里发送并不容易，需要有一个已知的HANDLE，来发送，
//			这个HANDLE，是从客户端列表里面里面取得的，
//			或者接收数据的时候，参数1返回的，
//			且发送时，会去客户端列表里面确认这个HANDLE是否有效，
//			如果有效才发送
//
//	接收数据：
//			会有一个驻留线程一直在用IOCP的方式接收数据，
//				接收到数据之后，把数据放到一个队列中，
//			然后用户的接收操作，实际上为从队列中的获取操作，
//
//	关闭服务端：
//			服务端关闭的时候，会关闭接引线程和数据接收线程
//			首先关闭循环标识，
//			然后创建一个新的连接，占用接引线程的连接，接引线程退出
//			由于关闭了循环标识，所以接收线程也可以退出
//			这样两个线程都可以安全退出
//			两个线程都安全退出之后，
//			循环搜寻客户端列表里面里面的所有客户端，一个一个关闭，
//			资源清理结束
//
//	客户端关闭连接：
//			服务端会感知到客户端关闭，然后从客户端列表中清除这个客户端
//			清理资源
//
//
//
//					客户端工作流程
//
//	创建客户端：
//			连接服务端，启动线程接收数据
//
//	发送数据：
//			直接发送
//
//	接收数据：
//			会有一个驻留线程一直在用重叠IO的方式接收数据
//				接收到数据之后，把数据放到一个队列中
//			然后用户的接收操作，实际上为从队列中的获取操作
//
//	关闭客户端：
//			给线程一个信号，让它退出
//			然后清理资源
//
//	服务端关闭连接：
//			这时候线程会感知到服务端关闭，然后暂停当前客户端状态
//			这时发送信息的时候，会判断出当前状态为停止状态，所以发信息会失败
//			接收信息是从队列里面接收，如果队列里面仍有数据，仍然可拿，否则失败
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

//	队列元素的最大个数
enum{ IOCP_NAMEPIPE_SERVER_INFO_ARR_MAX = 256 };

class CMessageArrayClass
{
	//	消息队列结构体，最多存储 IOCP_NAMEPIPE_SERVER_INFO_ARR_MAX 个
	struct 
	{
		struct
		{
			LPVOID pPipe;
			DWORD dwLen;
			BYTE *buf;
		}MessageArr[IOCP_NAMEPIPE_SERVER_INFO_ARR_MAX];
		DWORD dwRead;	//	当前读取的索引，如果再次调用取函数时，需要先做偏移，然后再获取
		DWORD dwWrite;	//	当前写入的索引，如果再次调用写函数时，需要先做偏移，然后再获取
		DWORD dwMessageCount;
		CRITICAL_SECTION cs;
	}m_sMessageArray;

public:
	//	初始化
	DWORD Init();

	//	获取消息数量
	DWORD GetMessageCount();
	//	放进去一个MSG
	DWORD SetAMessageIntoArray(LPVOID pPipe, BYTE *buf, DWORD dwLen);
	//	取出一个MSG
	DWORD GetAMessageFromArray(LPVOID *pPipe, BYTE **buf, DWORD *dwLen);
	//	取出一个MSG，如果 dwMaxLen < dwLen 返回失败，dwLen 返回所需长度
	//		其他情况下的失败，dwLen 返回 0
	DWORD GetAMessageFromArrayTestLen(LPVOID *pPipe, BYTE **buf, DWORD *dwLen, DWORD dwMaxLen);
};

//	指针队列元素的最大个数，也就是说客户端个数上限为 IOCP_POINT_ARR_MAX 个
enum{ IOCP_POINT_ARR_MAX = 256 };

class CPointArrayClass
{
	struct
	{
		struct
		{
			PVOID point;	//	当前元素
			PVOID info;		//	当前元素信息
		}sArray[IOCP_POINT_ARR_MAX];
		DWORD dwOnUse;		//	内部正在使用的个数
		CRITICAL_SECTION cs;
	} m_sArray;

public:
	//	初始化
	DWORD Init();
	//	插入
	DWORD Insert( PVOID point , PVOID info );
	//	获取，根据索引，只获取，不删除
	DWORD GetOn( DWORD index , PVOID *point , PVOID *info );
	//	移出，根据索引，获取并且删除
	DWORD Remove( DWORD index , PVOID *point , PVOID *info );
	//	移出，根据指针，获取并且删除
	DWORD FindRemove( PVOID point , PVOID *info );
	//	判断有效
	DWORD IsHere( PVOID point );
	//	获取数量
	DWORD GetCount();
};

class CIOCPNamePipe
{
public:
	CIOCPNamePipe();
	virtual ~CIOCPNamePipe();
	virtual DWORD Create(WCHAR *name, DWORD dwMode) = 0;				//	创建/开启管道
	virtual DWORD Recv(HANDLE *hFrom, BYTE *bBuf, DWORD *dwLen) = 0;	//	接收数据
	virtual DWORD Send(HANDLE hTo, BYTE *bBuf, DWORD dwLen) = 0;		//	发送数据
	virtual DWORD Close() = 0;											//	关闭管道
};

//////////////////////////////////////////////////////////////////////////

class CIOCPNamePipeServer : public CIOCPNamePipe
{
public:
	CIOCPNamePipeServer();

	//	创建连接
	DWORD Create(WCHAR *name, DWORD dwMode = PIPE_READMODE_MESSAGE);

	//	关闭连接
	DWORD Close();

	//	发送数据，如果 hTo 参数为 -1 ，则为广播消息
	DWORD Send(HANDLE hTo, BYTE *bBuf, DWORD dwLen);

	//	接收数据
	DWORD Recv(HANDLE *hFrom, BYTE *bBuf, DWORD *dwLen);

private:
	WCHAR m_wsName[MAX_PATH];		//	管道名字

	DWORD m_dwStatus;				//	状态

private:
	DWORD m_dwRun;								//	线程正在执行，如果为0 ，则线程退出
	HANDLE m_hCompletionPort;					//	完成端口实例

	HANDLE m_hControllerThread;					//	管理者线程句柄
	void ControllerThreadProc( void *pParam );	//	管理者线程执行体

	HANDLE m_hWorkerThread;						//	工作者线程句柄
	void WorkerThreadProc( void *pParam );		//	工作者线程执行体

	void WorkerProcCallBack( void *pParam );	//	工作者回调
												//	当这个函数被执行时，就是有个请求已经被收到了

	//	消息队列
	CMessageArrayClass m_sMessageArray;

	//	内部保存的数据列表
	CPointArrayClass m_sPointArray;

	//	管理者线程 shell
	friend void start_address_controller( void *pParam );
	//	工作者线程 shell
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
	WCHAR m_wsName[MAX_PATH];					//	管道名字

	DWORD m_dwStatus;							//	状态

private:
	HANDLE m_hPipe;								//	管道句柄

	DWORD m_dwRun;								//	线程正在执行，如果为0 ，则线程退出

	HANDLE m_hRecvThread;						//	接收线程句柄
	void ClientRecvThreadProc( void *pParam );	//	工作者线程执行体

	//	消息队列
	CMessageArrayClass m_sMessageArray;

	PIPE_CLIENT_CONTROL m_pipeClient;			//	客户端的的完成结构，为了防止客户端读写时出现问题

	friend void start_address_worker_clinet( void *pParam );
};