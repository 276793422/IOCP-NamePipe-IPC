#include <windows.h>
#include <process.h>
#include "IOCPNamePipe.h"

#ifdef _DEBUG
#include "TestList.h"
#endif

#define BUFSIZE	0x1000	//	4096

CIOCPNamePipe::CIOCPNamePipe(void)
{
}

CIOCPNamePipe::~CIOCPNamePipe(void)
{
}

//////////////////////////////////////////////////////////////////////////

//	��ʼ��
DWORD CMessageArrayClass::Init()
{
	InitializeCriticalSection(&m_sMessageArray.cs);
	m_sMessageArray.dwRead = 0;
	m_sMessageArray.dwWrite = 0;
	m_sMessageArray.dwMessageCount = 0;
	return 0;
}

//	��ȡ��Ϣ����
DWORD CMessageArrayClass::GetMessageCount()
{
	return m_sMessageArray.dwMessageCount;
}

//	�Ž�ȥһ��MSG
DWORD CMessageArrayClass::SetAMessageIntoArray(LPVOID pPipe, BYTE *buf, DWORD dwLen)
{
	DWORD ret = -1;
	EnterCriticalSection(&m_sMessageArray.cs);
	if (m_sMessageArray.dwMessageCount > IOCP_NAMEPIPE_SERVER_INFO_ARR_MAX)
	{
		goto goto_error;
	}
	if (m_sMessageArray.dwMessageCount < 0)
	{
		goto goto_error;
	}
	if (m_sMessageArray.dwMessageCount == IOCP_NAMEPIPE_SERVER_INFO_ARR_MAX)
	{
		goto goto_error;
	}
	m_sMessageArray.dwMessageCount ++;
	m_sMessageArray.dwWrite ++;
	if ( m_sMessageArray.dwWrite >= IOCP_NAMEPIPE_SERVER_INFO_ARR_MAX )
	{
		m_sMessageArray.dwWrite = 0;
	}
	m_sMessageArray.MessageArr[m_sMessageArray.dwWrite].pPipe = pPipe;
	m_sMessageArray.MessageArr[m_sMessageArray.dwWrite].dwLen = dwLen;
	m_sMessageArray.MessageArr[m_sMessageArray.dwWrite].buf = buf;
	ret = 0;
goto_error:
	LeaveCriticalSection(&m_sMessageArray.cs);
	return ret;
}

//	ȡ��һ��MSG
DWORD CMessageArrayClass::GetAMessageFromArray(LPVOID *pPipe, BYTE **buf, DWORD *dwLen)
{
	DWORD ret = -1;
	EnterCriticalSection(&m_sMessageArray.cs);
	if (m_sMessageArray.dwMessageCount > IOCP_NAMEPIPE_SERVER_INFO_ARR_MAX)
	{
		goto goto_error;
	}
	if (m_sMessageArray.dwMessageCount < 0)
	{
		goto goto_error;
	}
	if (m_sMessageArray.dwMessageCount == 0)
	{
		goto goto_error;
	}
	m_sMessageArray.dwMessageCount --;
	m_sMessageArray.dwRead ++;
	if ( m_sMessageArray.dwRead >= IOCP_NAMEPIPE_SERVER_INFO_ARR_MAX )
	{
		m_sMessageArray.dwRead = 0;
	}
	*pPipe = m_sMessageArray.MessageArr[m_sMessageArray.dwRead].pPipe;
	*dwLen = m_sMessageArray.MessageArr[m_sMessageArray.dwRead].dwLen;
	*buf = m_sMessageArray.MessageArr[m_sMessageArray.dwRead].buf;
	ret = 0;
goto_error:
	LeaveCriticalSection(&m_sMessageArray.cs);
	return ret;
}

//	ȡ��һ��MSG����� dwMaxLen < dwLen ����ʧ�ܣ�dwLen �������賤��
DWORD CMessageArrayClass::GetAMessageFromArrayTestLen(LPVOID *pPipe, BYTE **buf, DWORD *dwLen, DWORD dwMaxLen)
{
	DWORD ret = -1;
	DWORD tmpRead;
	EnterCriticalSection(&m_sMessageArray.cs);
	if (m_sMessageArray.dwMessageCount > IOCP_NAMEPIPE_SERVER_INFO_ARR_MAX)
	{
		*dwLen = 0;
		goto goto_error;
	}
	if (m_sMessageArray.dwMessageCount < 0)
	{
		*dwLen = 0;
		goto goto_error;
	}
	if (m_sMessageArray.dwMessageCount == 0)
	{
		*dwLen = 0;
		goto goto_error;
	}
	tmpRead = m_sMessageArray.dwRead;
	m_sMessageArray.dwMessageCount --;
	m_sMessageArray.dwRead ++;
	if ( m_sMessageArray.dwRead >= IOCP_NAMEPIPE_SERVER_INFO_ARR_MAX )
	{
		m_sMessageArray.dwRead = 0;
	}
	//	��� dwMaxLen < m_sMessageArray.MessageArr[m_sMessageArray.dwRead].dwLen
	//		Ҳ�����ṩ�ĳ��Ȳ�����ô���ظոոı������������Ȼ��ȡ������
	//		��󷵻�ʧ��
	if (dwMaxLen < m_sMessageArray.MessageArr[m_sMessageArray.dwRead].dwLen)
	{
		*dwLen = m_sMessageArray.MessageArr[m_sMessageArray.dwRead].dwLen;
		m_sMessageArray.dwMessageCount ++;
		m_sMessageArray.dwRead = tmpRead;
		goto goto_error;
	}
	*pPipe = m_sMessageArray.MessageArr[m_sMessageArray.dwRead].pPipe;
	*dwLen = m_sMessageArray.MessageArr[m_sMessageArray.dwRead].dwLen;
	*buf = m_sMessageArray.MessageArr[m_sMessageArray.dwRead].buf;
	ret = 0;
goto_error:
	LeaveCriticalSection(&m_sMessageArray.cs);
	return ret;
}

//////////////////////////////////////////////////////////////////////////

DWORD CPointArrayClass::Init()
{
	InitializeCriticalSection(&m_sArray.cs);
	m_sArray.dwOnUse = 0;
	memset(&m_sArray.sArray , 0 , sizeof(m_sArray.sArray));

	return 0;
}

//	����
DWORD CPointArrayClass::Insert( PVOID point , PVOID info )
{
	DWORD i = -1;
	EnterCriticalSection(&m_sArray.cs);
	if ( point == NULL || point == 0 )
	{
		goto goto_error;
	}
	if ( m_sArray.dwOnUse > IOCP_POINT_ARR_MAX )
	{
		goto goto_error;
	}
	for ( i = 0 ; i < IOCP_POINT_ARR_MAX ; i++ )
	{
		if ( m_sArray.sArray[i].point == NULL && m_sArray.sArray[i].info == NULL )
		{
			m_sArray.dwOnUse++;
			m_sArray.sArray[i].point = point;
			m_sArray.sArray[i].info = info;
			break;
		}
	}
goto_error:
	LeaveCriticalSection(&m_sArray.cs);
	return i;
}
//	��ȡ
DWORD CPointArrayClass::GetOn( DWORD index , PVOID *point , PVOID *info )
{
	DWORD i = -1;
	EnterCriticalSection(&m_sArray.cs);
	if ( m_sArray.dwOnUse > IOCP_POINT_ARR_MAX || m_sArray.dwOnUse == 0 )
	{
		goto goto_error;
	}
	if ( index < 0 || IOCP_POINT_ARR_MAX <= index )
	{
		goto goto_error;
	}
	if ( point == NULL || point == 0 || info == NULL || info == 0 )
	{
		goto goto_error;
	}
	if ( m_sArray.sArray[index].point == NULL )
	{
		goto goto_error;
	}
	*point = m_sArray.sArray[index].point;
	*info = m_sArray.sArray[index].info;
	i = index;
goto_error:
	LeaveCriticalSection(&m_sArray.cs);
	return i;
}

//	�Ƴ�
DWORD CPointArrayClass::Remove( DWORD index , PVOID *point , PVOID *info )
{
	DWORD i = -1;
	EnterCriticalSection(&m_sArray.cs);
	if ( m_sArray.dwOnUse > IOCP_POINT_ARR_MAX || m_sArray.dwOnUse == 0 )
	{
		goto goto_error;
	}
	if ( index < 0 || IOCP_POINT_ARR_MAX <= index )
	{
		goto goto_error;
	}
	if ( m_sArray.sArray[index].point == NULL )
	{
		goto goto_error;
	}
	i = index;
	if ( point != NULL && point != 0 )
	{
		*point = m_sArray.sArray[index].point;
	}
	if ( info != NULL && info != 0 )
	{
		*info = m_sArray.sArray[index].info;
	}
	m_sArray.sArray[index].point = NULL;
	m_sArray.sArray[index].info = NULL;
	m_sArray.dwOnUse--;
goto_error:
	LeaveCriticalSection(&m_sArray.cs);
	return i;
}

//	�Ƴ�
DWORD CPointArrayClass::FindRemove( PVOID point , PVOID *info )
{
	DWORD i = -1;
	EnterCriticalSection(&m_sArray.cs);
	if ( point == NULL || point == 0 )
	{
		goto goto_error;
	}
	for (i = 0 ; i < IOCP_POINT_ARR_MAX ; i++)
	{
		if ( m_sArray.sArray[i].point )
		{
			if ( m_sArray.sArray[i].point == point )
			{
				if ( info )
				{
					*info = m_sArray.sArray[i].info;
				}
				m_sArray.dwOnUse--;
				m_sArray.sArray[i].point = NULL;
				m_sArray.sArray[i].info = NULL;
				break;
			}
		}
	}
	if ( i >= IOCP_POINT_ARR_MAX )
	{
		i = -1;
	}

goto_error:
	LeaveCriticalSection(&m_sArray.cs);
	return i;
}

//	�ж���Ч
DWORD CPointArrayClass::IsHere( PVOID point )
{
	DWORD i = -1;
	EnterCriticalSection(&m_sArray.cs);
	if ( point == NULL || point == 0 )
	{
		goto goto_error;
	}
	for (i = 0 ; i < IOCP_POINT_ARR_MAX ; i++)
	{
		if ( m_sArray.sArray[i].point )
		{
			if ( m_sArray.sArray[i].point == point )
			{
				break;
			}
		}
	}
	if ( i >= IOCP_POINT_ARR_MAX )
	{
		i = -1;
	}

goto_error:
	LeaveCriticalSection(&m_sArray.cs);
	return i;
}

//	��ȡ����
DWORD CPointArrayClass::GetCount()
{
	return m_sArray.dwOnUse;
}

//////////////////////////////////////////////////////////////////////////

typedef struct
{
	OVERLAPPED	ov ;
	HANDLE		hPipe ;
	DWORD		dwUse;
	DWORD		dwTmp;
	BYTE		*buf;
} PIPE_INSTRUCT, *PPIPE_INSTRUCT;

void CIOCPNamePipeServer::WorkerProcCallBack( void *pParam )
{
	PPIPE_INSTRUCT pPI = (PPIPE_INSTRUCT)pParam;
	//	TODO: ��ǰ�и��������ˣ�
	//		pPI->hPipe		��Ӧ����ľ��
	//		pPI->buf		���õ�һ��ռ䣬���Բ���
	//		pPI->dwUse		��Ӧһ��DWORD�����Լ�¼�������ݳ���
	BYTE *buf;
	BYTE *nbuf;
	DWORD dwBufLen = BUFSIZE;
	DWORD dwBufIndex = 0;
	buf = (BYTE*)malloc(dwBufLen);

	while ( ReadFile(pPI->hPipe , pPI->buf , BUFSIZE , &pPI->dwUse , NULL ) )
	{
		memmove( buf + dwBufIndex * BUFSIZE , pPI->buf , pPI->dwUse );
		if (pPI->dwUse <= BUFSIZE)
		{
			dwBufLen = dwBufIndex * BUFSIZE + pPI->dwUse;
			DBGPRINT(("\nĿ�� : %p ����: %s\n" , pPI->hPipe , buf));
			break;
		}
		else
		{
			dwBufLen <<= 1;
			dwBufIndex ++;
			//	FIXME	���������⣬��Ӧ�����޴ε������ڴ棬Ӧ������һ����������
			nbuf = NULL;
			while ( nbuf == NULL )
			{
				nbuf = (BYTE *)realloc(buf, dwBufLen);
			}
			buf = nbuf;
		}
	}

	if ( -1 == m_sMessageArray.SetAMessageIntoArray(pPI, buf, dwBufLen) )
	{
		free( buf );
	}
}

void CIOCPNamePipeServer::WorkerThreadProc( void *pParam )
{
	LPOVERLAPPED pOV;
	PPIPE_INSTRUCT pPI;
	DWORD cbTrans;
	BOOL bRet = FALSE;
	DBGPRINT(("\n���Ͽ�ʼ����IOCP\n"));
	while ( m_dwRun )
	{
		bRet = GetQueuedCompletionStatus( m_hCompletionPort, &cbTrans, (LPDWORD)&pPI, &pOV, /*INFINITE*/ /*5**/1000 );
		if ( bRet == FALSE )
		{
			if ( GetLastError() == WAIT_TIMEOUT )
			{
				continue;
			}
		}
		if ( pOV == NULL )
		{
			continue;
		}
		if ( (DWORD)pPI == -1 )
		{
			break;
		}
		if ( !m_dwRun )
		{
			break;
		}
		// ��֤�ܵ��ڲ��Ƿ�ȷʵ����Ϣ�������ǶԷ��Ѿ���ص���
		bRet = ReadFile(pPI->hPipe , pPI->buf , 0 , &pPI->dwUse , NULL );
		if ( bRet == TRUE )
		{
			WorkerProcCallBack( pPI );
		}
		else
		{
			if ( GetLastError() == 109 )	//	�ܵ��ر�
			{
				DBGPRINT(("\nĿ�� : %p �ر�\n", pPI->hPipe));
				if ( m_sPointArray.FindRemove( pPI , NULL ) != -1 )
				{
					if ( pPI->buf )
					{
						free(pPI->buf);
					}
					FlushFileBuffers(pPI->hPipe); 
					DisconnectNamedPipe(pPI->hPipe);
					CloseHandle(pPI->hPipe);
					CloseHandle(pPI->ov.hEvent);
					GlobalFree(pPI);
				}
				continue;
			}
		}
		//	����һ�ε����ע��
		bRet = ReadFile(pPI->hPipe , pPI->buf , 0 , &pPI->dwUse , pOV );
		if ( bRet == TRUE )
		{
		}
		else
		{
			// ���ע��ʧ��
		}
	}
	//m_dwStatus = NAME_PIPE_STATUS_STOPPING;
	DBGPRINT(("\n����IOCP����\n"));
}

static void start_address_worker( void *pParam )
{
	CIOCPNamePipeServer *pThis = (CIOCPNamePipeServer*)pParam;
	pThis->WorkerThreadProc(pParam);
	_endthread();
}

void CIOCPNamePipeServer::ControllerThreadProc( void *pParam )
{
	PPIPE_INSTRUCT pPI;
	BOOL fConnected;
	HANDLE hRet;

	m_hCompletionPort = CreateIoCompletionPort ( INVALID_HANDLE_VALUE, NULL, 0, 4 ) ;

	//	����������
	m_hWorkerThread = (HANDLE)_beginthread(start_address_worker , 0 , (LPVOID)this);

	DBGPRINT(("\n�������ѭ��\n"));

	//	��������ѭ��������Ҫ��������ֻ��һ�������ǰ����˵�����ӵ�IOCP��
	while ( m_dwRun )
	{
		pPI = NULL;
		while( !pPI )
		{
			pPI = (PPIPE_INSTRUCT)GlobalAlloc(GPTR , sizeof(PIPE_INSTRUCT));
		}

		pPI->buf = NULL;
		//	FIXME	���ʧ�ܾ�һֱ���������ﲻ�ף������޸�
		pPI->hPipe = INVALID_HANDLE_VALUE;
		while ( pPI->hPipe == INVALID_HANDLE_VALUE )
		{
			pPI->hPipe = CreateNamedPipe(m_wsName, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
											PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
											PIPE_UNLIMITED_INSTANCES, 0, 0, 0, NULL);
		}
		//	FIXME	���ʧ�ܾ�һֱ�ȴ����ӣ����ﲻ�ף������޸�
		fConnected = FALSE;
		while ( !fConnected )
		{
			fConnected = ConnectNamedPipe ( pPI->hPipe, NULL ) ; 
		}
		if ( !m_dwRun )
		{
			CloseHandle(pPI->hPipe);
			GlobalFree(pPI);
			break;
		}

		DBGPRINT(("\nĿ�� : %p ����\n" , pPI->hPipe));
		pPI->ov.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL ) ;

		hRet = CreateIoCompletionPort ( pPI->hPipe, m_hCompletionPort, (ULONG_PTR)pPI, 1 ) ;
		if ( hRet == NULL )
		{
			CloseHandle(pPI->hPipe);
			CloseHandle(pPI->ov.hEvent);
			GlobalFree(pPI);
			continue;
		}
		m_sPointArray.Insert( pPI , NULL );
		pPI->buf = (BYTE *)malloc(sizeof(BYTE)*BUFSIZE);
		ReadFile(pPI->hPipe , pPI->buf , 0 , &pPI->dwUse , &(pPI->ov) );
	}

	//	����5�룬��Ȼû�����˳�����ôֻ��ǿɱ���������һ�㲻�����
	if ( WaitForSingleObject(m_hWorkerThread , 5*1000) == WAIT_TIMEOUT )
	{
		TerminateThread(m_hWorkerThread, -1);
	}
	CloseHandle(m_hCompletionPort);
	DBGPRINT(("\n�����߳��˳�\n"));
}

static void start_address_controller( void *pParam )
{
	CIOCPNamePipeServer *pThis = (CIOCPNamePipeServer*)pParam;
	pThis->ControllerThreadProc(pParam);
	_endthread();
}

CIOCPNamePipeServer::CIOCPNamePipeServer()
{
	m_dwRun = 0;
	m_wsName[0] = L'\0';
	m_dwStatus = NAME_PIPE_STATUS_STOPPING;
}

DWORD CIOCPNamePipeServer::Create(WCHAR *name, DWORD dwMode)
{
	if ( m_dwStatus == NAME_PIPE_STATUS_RUNNING )
	{
		return -1;
	}
	if ( name == NULL || name == 0 )
	{
		return -1;
	}
	wcscpy_s(m_wsName, name);

	m_dwRun = 1;
	m_hControllerThread = NULL;
	m_hCompletionPort = NULL;

	m_sPointArray.Init();

	m_sMessageArray.Init();
	m_dwStatus = NAME_PIPE_STATUS_RUNNING;

	m_hControllerThread = (HANDLE)_beginthread(start_address_controller, 0, (void *)this);

	DBGPRINT(("\n��ʼ������\n"));
	return 0;
}

DWORD CIOCPNamePipeServer::Close()
{
	if ( m_dwStatus == NAME_PIPE_STATUS_STOPPING )
	{
		return -1;
	}
	m_dwRun = 0;
	{
		HANDLE hPipe;
		DWORD dwT = 0;
		hPipe = CreateFile(m_wsName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (hPipe != INVALID_HANDLE_VALUE)
		{
		}
		if (GetLastError() != ERROR_PIPE_BUSY) 
		{
		}
		CloseHandle(hPipe); 
	}
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_hControllerThread , 5*1000))
	{
	}
	else
	{
		TerminateThread(m_hControllerThread , -1);
	}
	DWORD i;
	PVOID point;
	PVOID info;
	PPIPE_INSTRUCT pPI;
	for ( i = 0 ; i < IOCP_POINT_ARR_MAX ; i++ )
	{
		if ( m_sPointArray.Remove(i , &point , & info) != -1 )
		{
			pPI = (PPIPE_INSTRUCT)point;
			if ( pPI->buf )
			{
				free(pPI->buf);
			}
			CloseHandle(pPI->hPipe);
			CloseHandle(pPI->ov.hEvent);
			GlobalFree(pPI);
		}
	}
	m_wsName[0] = L'\0';
	m_dwStatus = NAME_PIPE_STATUS_STOPPING;

#ifdef _DEBUG
	DEBUG_printf();
#endif
	return 0;
}

DWORD CIOCPNamePipeServer::Send(HANDLE hTo, BYTE *bBuf, DWORD dwLen)
{
	if ( m_dwStatus == NAME_PIPE_STATUS_STOPPING )
	{
		return -1;
	}
	DWORD cbWritten = 0;
	BOOL b = FALSE;
	PPIPE_INSTRUCT pPI = (PPIPE_INSTRUCT)hTo;
	if ( hTo == (HANDLE)-1 )
	{
		DWORD i;
		PVOID info;
		for ( i = 0 ; i < IOCP_POINT_ARR_MAX ; i++ )
		{
			if ( m_sPointArray.GetOn( i , (PVOID *)&pPI , &info ) != -1 )
			{
				WriteFile( pPI->hPipe, bBuf, dwLen, &cbWritten, NULL);
				DBGPRINT(("\n��Ⱥ����Ŀ�� : %p ���� : %s\n" , pPI->hPipe , bBuf ));
			}
		}
		return 0;
	}
	else
	{
		if ( m_sPointArray.IsHere(pPI) == -1 )
		{
			return -1;
		}
		b = WriteFile( pPI->hPipe, bBuf, dwLen, &cbWritten, NULL);
		DBGPRINT(("\nĿ�� : %p ���� : %s\n" , pPI->hPipe , bBuf ));
		if (b)
		{
			return cbWritten;
		}
		else
		{
			return -1;
		}
	}
	return 0;
}

DWORD CIOCPNamePipeServer::Recv(HANDLE *hFrom, BYTE *bBuf, DWORD *dwLen)
{
	if ( m_dwStatus == NAME_PIPE_STATUS_STOPPING )
	{
		return -1;
	}
	if ( m_sMessageArray.GetMessageCount() )
	{
		HANDLE hPipe;
		BYTE *buf;
		DWORD dwBufLen;
		if ( -1 == m_sMessageArray.GetAMessageFromArrayTestLen(&hPipe, &buf, &dwBufLen, *dwLen) )
		{
			*hFrom = NULL;
			*bBuf = NULL;
			*dwLen = dwBufLen;
			return -1;
		}
		DBGPRINT(("\nĿ�� : %p ȡ����Ϣ: %s\n" , ((PPIPE_INSTRUCT)hPipe)->hPipe , buf));
		memmove(bBuf , buf , dwBufLen);
		*hFrom = hPipe;
		*dwLen = dwBufLen;
		free(buf);
		return 0;
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////////

void CIOCPNamePipeClient::ClientRecvThreadProc( void *pParam )
{
	BYTE *buf = NULL;
	BYTE *nbuf;
	DWORD dwBufLen = BUFSIZE;
	DWORD dwBufIndex = 0;
	DWORD dwUse;
	int brk = 0;
	DBGPRINT(("\n��ʼ������Ϣ\n"));
	while ( m_dwRun )
	{
		dwBufLen = BUFSIZE;
		if ( 0 == ReadFile( m_hPipe , &dwUse , 0 , &dwUse , &m_pipeClient.oOverlap ) )
		{
			if ( GetLastError() != 997 )
			{
				break;
			}
		}
		if ( WAIT_OBJECT_0 != WaitForSingleObject(m_pipeClient.oOverlap.hEvent, -1) )
		{
			continue;
		}
		if ( m_dwRun == 0 )
		{
			break;
		}
		if ( buf == NULL )
		{
			buf = (BYTE*)malloc(dwBufLen);
		}
		brk = 0;
		dwUse = 0;
		while ( ReadFile(m_hPipe , buf , BUFSIZE , &dwUse , NULL ) )
		{
			memmove( buf + dwBufIndex * BUFSIZE , buf , dwUse );
			if (dwUse <= BUFSIZE)
			{
				dwBufLen = dwBufIndex * BUFSIZE + dwUse;
				brk = 1;
				break;
			}
			else
			{
				dwBufLen <<= 1;
				dwBufIndex ++;
				//	FIXME	���������⣬��Ӧ�����޴ε������ڴ棬Ӧ������һ����������
				nbuf = NULL;
				while ( nbuf == NULL )
				{
					nbuf = (BYTE *)realloc(buf, dwBufLen);
				}
				buf = nbuf;
			}
		}
		if ( GetLastError() == 109 )	//	�ܵ��ر�
		{
			DBGPRINT(("\nĿ�� : %p �ر�\n", m_hPipe));
			free(buf);
			buf = NULL;
			CloseHandle(m_hPipe);
			m_hPipe = NULL;
			break;
		}
		if ( !m_dwRun )
		{
			free(buf);
			buf = NULL;
			break;
		}
		if ( 0 == brk )
		{
			free( buf );
			buf = NULL;
		}
		else
		{
			if ( -1 == m_sMessageArray.SetAMessageIntoArray(NULL, buf, dwBufLen) )
			{
				free( buf );
				buf = NULL;
			}
		}
		buf = NULL;
		ResetEvent(m_pipeClient.oOverlap.hEvent);
	}
	CloseHandle(m_pipeClient.oOverlap.hEvent);

	m_dwStatus = NAME_PIPE_STATUS_STOPPING;
}

static void start_address_worker_clinet( void *pParam )
{
	CIOCPNamePipeClient *pThis = (CIOCPNamePipeClient*)pParam;
	pThis->ClientRecvThreadProc(pParam);
	_endthread();
}

CIOCPNamePipeClient::CIOCPNamePipeClient()
{
	m_dwStatus = NAME_PIPE_STATUS_STOPPING;
}

DWORD CIOCPNamePipeClient::Create(WCHAR *name, DWORD dwMode)
{
	if ( m_dwStatus == NAME_PIPE_STATUS_RUNNING )
	{
		return -1;
	}

	if ( name == NULL || name == 0 )
	{
		goto goto_error;
	}
	m_wsName[0] = L'\0';
	wcscpy_s(m_wsName, name);

	//	FIXME	ѭ������5�Σ�����δζ�ʧ�ܣ��Ͳ�����
	int i;
	for (i = 0; i < 5; i++)
	{
		//	Create ֮ǰ���Ե��� WaitNamedPipe ���һ��ʵ���ģ�Ҳ���Բ����ã������û�е���
		m_hPipe = CreateFile(m_wsName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

		if (m_hPipe != INVALID_HANDLE_VALUE)
		{
			break;
		}

		//	����˲�����
		if (GetLastError() == ERROR_FILE_NOT_FOUND) 
		{
			goto goto_error;
		}

		if (GetLastError() != ERROR_PIPE_BUSY) 
		{
			goto goto_error;
		}
	}
	if (i >= 5)
	{
		goto goto_error;
	}
	if (!SetNamedPipeHandleState( m_hPipe, &dwMode, NULL, NULL))
	{
		goto goto_error2;
	}
	m_dwStatus = NAME_PIPE_STATUS_RUNNING;

	m_sMessageArray.Init();

	{
		//	���λ����Ҫһ���ص�IO���ȴ�readfile
		memset(&m_pipeClient , 0 , sizeof(m_pipeClient));
		m_pipeClient.oOverlap.hEvent = CreateEvent(NULL , TRUE , FALSE , NULL);
	}

	m_dwRun = 1;
	m_hRecvThread = (HANDLE)_beginthread(start_address_worker_clinet, 0, (void *)this);
	return 0;

goto_error2:
	CloseHandle(m_hPipe);
goto_error:
	DBGPRINT(("\nGetLastError() = %d\n" , GetLastError()));
	m_wsName[0] = L'\0';
	m_hPipe = NULL;
	return -1;
}

DWORD CIOCPNamePipeClient::Close()
{
	//	״̬Ϊ�����٣����˳�
	if ( m_dwStatus == NAME_PIPE_STATUS_STOPPING )
	{
		return -1;
	}
	m_dwRun = 0;
	if (m_hPipe)
	{
		if ( m_hRecvThread )
		{
			SetEvent(m_pipeClient.oOverlap.hEvent);
			if ( WAIT_OBJECT_0 != WaitForSingleObject(m_hRecvThread , 5*1000) )
			{
				TerminateThread(m_hRecvThread , -1);
			}
			m_hRecvThread = NULL;
		}
		CloseHandle(m_hPipe);
		m_hPipe = NULL;
	}
	m_wsName[0] = L'\0';

	//	����ɼӿɲ��ӣ���Ϊ�߳�����������ֵ
	//	�߳����渳ֵ�ĺô��Ƿ���������˳�֮�󣬿ͻ��˻����Թ���
	//m_dwStatus = NAME_PIPE_STATUS_STOPPING;

#ifdef _DEBUG
	DEBUG_printf();
#endif
	return 0;
}

DWORD CIOCPNamePipeClient::Send(HANDLE hTo, BYTE *bBuf, DWORD dwLen)
{
	//	״̬��Ϊ�������У����˳�
	if ( m_dwStatus != NAME_PIPE_STATUS_RUNNING )
	{
		return -1;
	}

	DWORD cbWritten = 0;
	BOOL b = FALSE;
	b = WriteFile( m_hPipe, bBuf, dwLen, &cbWritten, NULL);
	DBGPRINT(("\nĿ�� : %p ���� : %s\n" , m_hPipe , bBuf ));
	if (b)
	{
		return cbWritten;
	}
	else
	{
		return -1;
	}
	return 0;
}

DWORD CIOCPNamePipeClient::Recv(HANDLE *hFrom, BYTE *bBuf, DWORD *dwLen)
{
	//	״̬��Ϊ�������У����˳�
	if ( m_dwStatus != NAME_PIPE_STATUS_RUNNING )
	{
		return -1;
	}
	if ( m_sMessageArray.GetMessageCount() )
	{
		HANDLE hPipe;
		BYTE *buf;
		DWORD dwBufLen;
		if ( -1 == m_sMessageArray.GetAMessageFromArrayTestLen(&hPipe, &buf, &dwBufLen, *dwLen) )
		{
			*hFrom = NULL;
			*bBuf = NULL;
			*dwLen = dwBufLen;
			return -1;
		}
		DBGPRINT(("\nĿ�� : %p ȡ����Ϣ: %s\n" , hPipe , buf));
		memmove(bBuf , buf , dwBufLen);
		*dwLen = dwBufLen;
		free(buf);
		return 0;
	}
	return -1;
}