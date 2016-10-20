// MyCom.cpp: implementation of the MyCom class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MyCom.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//������Ϣ���
inline int MessageError(LPCSTR lpcszError, UINT nType = MB_OK | MB_ICONWARNING)
{
	CWnd *pWnd = ::AfxGetMainWnd();
	if(pWnd != NULL)
		return pWnd->MessageBox(lpcszError, "Error", nType);
	else
		return ::MessageBox(NULL, lpcszError, "Error", nType);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MyCom::MyCom()
: m_bConnected(false), 
m_ePStatus(PS_NOREADY)
{
	memset(&m_Connection, 0, sizeof(m_Connection) - sizeof(CCriticalSection));
}

MyCom::~MyCom()
{

}

//��ȡ��ǰͨѶ�˿ڻ����е�����
UINT MyCom::CommReadBlock(HANDLE hFile, LPOVERLAPPED lpOLRead, LPSTR lpszBlock, UINT nMaxLength)
{
	BOOL       fReadStat;
	COMSTAT    ComStat;
	DWORD      dwErrorFlags;
	DWORD      dwLength;
	DWORD      dwError;

	if(hFile == NULL)
		return false;

	//��ѯ��ǰͨѶ�������Ķ����е������ֽ���
	::ClearCommError(hFile, &dwErrorFlags, &ComStat);
	//���㵱ǰ�����ȡ���ݵ��ֽ���
	//����������е������ֽ����ϵ�ǰ�ڴ滺���ֽ���С, 
	//�Ͷ�ȡ��ǰ��������;  
	//����������е������ֽ����ϵ�ǰ�ڴ滺���ֽ�����,
	//�Ͷ�ȡ��ǰ�ڴ滺������֧�ֵ�����ֽ���.)
	dwLength = min(nMaxLength, ComStat.cbInQue);
	
	if(dwLength <= 0)
		return 0;

	//����ӵ�ǰ��ͨѶ��������ȡ����
	fReadStat = ::ReadFile(hFile, lpszBlock, dwLength, &dwLength, lpOLRead);
	if(fReadStat)
		return dwLength;

	if(GetLastError() == ERROR_IO_PENDING)
	{
		//��ǰ����I/O�ص������������ȴ�I/O����...
		while(!GetOverlappedResult(hFile, lpOLRead, &dwLength, true))
		{
			dwError = GetLastError();
			if(dwError == ERROR_IO_INCOMPLETE)
				continue;
			else
			{
				//��ǰͨѶ�������Ĵ�����
				TRACE("<CE-%u>", dwError);
				::ClearCommError(hFile, &dwErrorFlags, &ComStat);
				if(dwErrorFlags > 0)
					TRACE("<CE-%u>", dwErrorFlags);
				
				break;
			}
		}
	}
	else
	{
		//��ǰͨѶ������һЩ�����Ĵ���
		dwLength = 0 ;
		::ClearCommError(hFile, &dwErrorFlags, &ComStat);
		if(dwErrorFlags > 0)
			TRACE("<CE-%u>", dwErrorFlags);
	}

	return dwLength;
}

//��ǰ��ͨѶ�˿ڼ����̺߳���
UINT MyCom::CommMonitorProc(void* pData)
{
	LPCONNECTION	pMPParam = (LPCONNECTION)pData;
	int				nLength;
	char			szBuffer[MAX_BUF_BLOCK + 1];
	
	//������ǰ��������ͨѶ������������
	while(pMPParam->hCommDev != NULL)
	{
		DWORD dwEvtMask = 0;

		//�ȴ���ǰ����ͨѶ��һ�����¼�������
		WaitCommEvent(pMPParam->hCommDev, &dwEvtMask, NULL);
		
		if((dwEvtMask & EV_RXCHAR) != EV_RXCHAR)
			continue;
		
		//�Ӵ��ڶ�ȡ���ݲ�����
		while((nLength = CommReadBlock(pMPParam->hCommDev, 
			&pMPParam->olRead, (LPSTR)szBuffer, MAX_BUF_BLOCK)) > 0)
		{
			//����Ӵ��ڶ���������
			if(pMPParam->pCom != NULL)
			{
				pMPParam->pCom->LockThread();
				pMPParam->pCom->ProcessReceivedData(szBuffer, nLength);
				pMPParam->pCom->UnlockThread();
			}

			if(::WaitForSingleObject(pMPParam->hEventCancel, 0) == WAIT_OBJECT_0)
				break;
		}
	}

	//֪ͨ���̵߳�ǰ�߳��ѽ���
	//....

	//��ǰ�߳���ȷ����������0
	return 0;
}

//������ǰ��ͨѶ����
BOOL MyCom::Connect()
{
	//��ǰ�Ѿ�ͨѶ�����Ѿ�����
	if(m_bConnected)
		return true;
	
	__try
	{
		m_Connection.CSec.Lock();

		//��ʼ����ǰ������Ϣ������
		if(!this->InitCommInfo())
		{
			ASSERT(false);
			__leave;
		}
		
		if(m_Connection.hCommDev != NULL)
		{
			ASSERT(false);
			__leave;
		}

		//������ǰ��ͨѶ����

		//��������I/O�첽ͨѶʱ�Ķ���д��ȡ���¼�
		m_Connection.olRead.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		m_Connection.olWrite.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		m_Connection.hEventCancel = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		if(m_Connection.olRead.hEvent == NULL
		|| m_Connection.olWrite.hEvent == NULL
		|| m_Connection.hEventCancel == NULL)
			__leave;

		//��ͨѶ�˿�
		if(!this->OpenComm(m_Connection.hCommDev))
			__leave;

		//���õ�ǰ���ڵ�ͨ�ţ���COMM_MASKָ�����¼�
		//��WaitCommEvent���÷���ʱ����������Щ�¼�
		if(!::SetCommMask(m_Connection.hCommDev, COMM_MASK))
		{
			ASSERT(false);
			__leave;
		}

		//���ô������������������������Ĵ�С
		if(!::SetupComm(m_Connection.hCommDev, 1024, 1024))
		{
			ASSERT(false);
			__leave;
		}
			
		//�쳣��ֹ��ǰ�ڴ򿪵Ķ˿��������е�������������
		if(!::PurgeComm(m_Connection.hCommDev, PURGE_FLAG))
		{
			ASSERT(false);
			__leave;
		}

		//���õ�ǰ���ڵĶ�д��ʱ
		if(!this->SetCommTimeouts(m_Connection.hCommDev))
		{
			ASSERT(false);
			__leave;
		}

		//�趨��ǰ���ڵ�ͨ�Ų���
		if(!this->SetCommState(m_Connection.hCommDev))
		{
			ASSERT(false);
			__leave;
		}

		m_Connection.pCom = this;

		//���������߳�
		DWORD dwThreadID;
		m_hMonitorThread = CreateThread(
			(LPSECURITY_ATTRIBUTES)NULL,
			0, (LPTHREAD_START_ROUTINE)CommMonitorProc,
			&m_Connection, 0, &dwThreadID);
		if(m_hMonitorThread == NULL)
		{
			ASSERT(false);
			__leave;
		}

		//���������ϵ�DTR��
		EscapeCommFunction(m_Connection.hCommDev, SETDTR);

		m_Connection.dwRefCount++;
		m_bConnected = true;
		m_ePStatus = PS_READY;
	}
	__finally
	{
		//��ǰ���Ӵ���ʧ��
		if(!m_bConnected)
		{
			//����˿��Ѿ��򿪣���ر��Ѵ򿪵Ķ˿�
			if(m_Connection.hCommDev != NULL)
			{
				::CloseHandle(m_Connection.hCommDev);
				m_Connection.hCommDev = NULL;
			}

			//�������д�¼��Ѿ���������ɾ����ǰ�Ķ���д�¼�
			if(m_Connection.olRead.hEvent != NULL)
			{
				//�رն��¼�
				::CloseHandle(m_Connection.olRead.hEvent);
				m_Connection.olRead.hEvent = NULL;
			}

			if(m_Connection.olWrite.hEvent != NULL)
			{
				//�ر�д�¼�
				::CloseHandle(m_Connection.olWrite.hEvent);
				m_Connection.olWrite.hEvent = NULL;
			}

			if(m_Connection.hEventCancel != NULL)
			{
				//�ر�ȡ���¼�
				::CloseHandle(m_Connection.hEventCancel);
				m_Connection.hEventCancel = NULL;
			}
		}

		m_Connection.CSec.Unlock();
		return m_bConnected;
	}
}

//���½�����ǰ��ͨѶ����
BOOL MyCom::ReConnect()
{
	//��ǰ�Ѿ�ͨѶ�����Ѿ�����
	if(m_bConnected)
	{
		this->DisConnect();
		this->DelayReply(RECONNECT_TIME);
	}
	
	return this->Connect();
}

//ȡ����ǰ��ͨѶ����
void MyCom::DisConnect()
{
	m_Connection.CSec.Lock();

	//��ǰ��û��ͨѶ���ӣ�ֱ�ӷ���
	if(!m_bConnected)
	{
		m_Connection.CSec.Unlock();
		return;
	}

	m_Connection.dwRefCount--;
	
	//�ر�ͨѶ�¼�
	if(m_Connection.dwRefCount == 0
	&& m_Connection.hCommDev != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(m_Connection.hCommDev);
		::CloseHandle(m_Connection.olRead.hEvent);
		::CloseHandle(m_Connection.olWrite.hEvent);
				
		m_Connection.hCommDev = NULL;
		m_Connection.olRead.hEvent = NULL;
		m_Connection.olWrite.hEvent = NULL;

		::SetEvent(m_Connection.hEventCancel);
		::CloseHandle(m_Connection.hEventCancel);
		m_Connection.hEventCancel = NULL;
	}

	//�õ�ǰ��ͨѶ���ӱ�־
	m_bConnected = false;
	m_ePStatus = PS_NOREADY;
	m_Connection.CSec.Unlock();
}

//��ȡ��ǰ��ͨѶ�˿�����
CString MyCom::GetCommName()
{
	return this->GetCommName(m_CommInfo.byCommPort);
}

//�жϵ�ǰ��ͨѶ�����Ƿ��Ѿ�����
BOOL MyCom::IsConnected()
{
	return m_bConnected;
}

//��ȡ��ǰ��ͨѶ����ָ��,�������ڵ�ǰ�߳�
//(�˺���������UnLockConnection()��Ե���, ʱ��������̫��)
LPCONNECTION MyCom::LockConnection()
{
	m_Connection.CSec.Lock();
	return &m_Connection;
}

//�ͷŵ�ǰ��ͨѶ����ָ��(�����ǰ�̵߳�����)
//(�˺���������LockConnection()��Ե���, ʱ��������̫��)
void MyCom::UnLockConnection()
{
	m_Connection.CSec.Unlock();
}

//��ʼ����ǰ�Ĵ�����Ϣ
BOOL MyCom::InitCommInfo()
{
	if(m_bConnected)
		return false;

	memset(&m_CommInfo, 0, sizeof(m_CommInfo));

	m_CommInfo.byByteSize = 8;
	m_CommInfo.byCommPort = 1;
	m_CommInfo.byParity = NOPARITY;
	m_CommInfo.byStopBits = TWOSTOPBITS;
	m_CommInfo.dwBaudRate = CBR_9600;
	m_CommInfo.byFlowCtrl = FC_XONXOFF;
	m_CommInfo.byXonXoff = false;

	return true;
}

//���ݴ��ڵ���Ż�ȡ��ǰ���ڵ�����
CString MyCom::GetCommName(BYTE bySN)
{
	CString strOut;
	
	if(bySN <= 0 || bySN > 20)
	{
		ASSERT(false);
		bySN = 1;
	}

	strOut.Format("COM%d", bySN);
	return strOut;
}

//�趨��ǰ���ڵ�ͨ�Ų���
BOOL MyCom::SetCommState(HANDLE hFile)
{
	DCB dcb;
	::ZeroMemory(&dcb, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);
	
	if(!::GetCommState(hFile, &dcb))
		return false;
	
	dcb.BaudRate = this->m_CommInfo.dwBaudRate;
	dcb.ByteSize = this->m_CommInfo.byByteSize;
	dcb.Parity = this->m_CommInfo.byParity;
	dcb.StopBits = this->m_CommInfo.byStopBits;
	
	//setup hardware flow control
/*	dcb.fOutxDsrFlow = (BYTE)((m_CommInfo.byFlowCtrl & FC_DTRDSR) != 0);
	if(dcb.fOutxDsrFlow)
		dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
	else
		dcb.fDtrControl = DTR_CONTROL_ENABLE;
	
	dcb.fOutxCtsFlow = (BYTE)((m_CommInfo.byFlowCtrl & FC_RTSCTS) != 0);
	if(dcb.fOutxCtsFlow)
		dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
	else
		dcb.fRtsControl = RTS_CONTROL_ENABLE;
	
	//setup software flow control
	dcb.fInX = dcb.fOutX = (BYTE)((m_CommInfo.byFlowCtrl & FC_XONXOFF) != 0);
	dcb.XonChar = ASCII_XON;
	dcb.XoffChar = ASCII_XOFF;
	dcb.XonLim = 100;
	dcb.XoffLim = 100;
	
	//other various settings
	dcb.fBinary = TRUE;
	dcb.fParity = m_CommInfo.byParity;
*/	
	return ::SetCommState(hFile, &dcb);
}

//���õ�ǰ���ڵĶ�д��ʱ
BOOL MyCom::SetCommTimeouts(HANDLE hFile)
{
	COMMTIMEOUTS CommTimeOuts;
	
	CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
	CommTimeOuts.ReadTotalTimeoutConstant = 1000;
	CommTimeOuts.WriteTotalTimeoutMultiplier = 20;
	CommTimeOuts.WriteTotalTimeoutConstant = 0;
	
	return ::SetCommTimeouts(hFile, &CommTimeOuts);
}

//�򿪴���
BOOL MyCom::OpenComm(HANDLE &hFile)
{
	CString strError;

	//�ҿ�����
	HANDLE h = ::CreateFile(
		this->GetCommName(), 
		GENERIC_READ|GENERIC_WRITE, 
		0, NULL, OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED, 
		NULL);
			
	//��ָ������ʧ��
	if(h == INVALID_HANDLE_VALUE)
	{
		strError.Format("��ͨѶ�˿�%s����!\n", 
			this->GetCommName());
		MessageError(strError);
		return false;
	}

	hFile = h;
	return true;
}

//��ǰͨѶ�˿ڷ�������
BOOL MyCom::CommWriteBlock(HANDLE hFile, LPCSTR lpcByte, 
							DWORD dwBytesToWrite,
							OVERLAPPED *pOLWrite)
{
	DWORD       dwBytesWritten;
	DWORD   	dwError;
	DWORD       dwBytesSent = 0;
	COMSTAT     ComStat;
	char        szError[128];
	
	//�򴮿�д������
	if(WriteFile(hFile, lpcByte, dwBytesToWrite, 
		&dwBytesWritten, pOLWrite))
		return true;
	
	//���д���ݺ������к��ȫ��ϵͳ������Ϣ
	//����з��ص���Ϣ�����򱨴��˳���ǰ����������
	if(GetLastError() != ERROR_IO_PENDING)
	{
		ClearCommError(hFile, &dwError, &ComStat);
		if(dwError > 0)
		{
			sprintf(szError, "<CE-%x>", dwError);
			TRACE0(szError);
		}
			
		return false;
	}

	//��ȡ��ǰIO�ص������Ľ��
	while(!GetOverlappedResult(hFile, pOLWrite, 
		&dwBytesWritten, true))
	{
		dwError = GetLastError();

		//���ɴ���IO�ص����������У������ȴ�
		if(dwError == ERROR_IO_INCOMPLETE)
		{
			dwBytesSent += dwBytesWritten;
			continue;
		}
		//���������������˳���ǰ������ִ��
		else
		{
			sprintf(szError, "<CE-%x>\n", dwError);
			TRACE0(szError);
			ClearCommError(hFile, &dwError, &ComStat);
			if(dwError > 0)
			{
				sprintf(szError, "<CE-%x>\n", dwError);
				TRACE0(szError);
			}
			break;
		}
	}
	
	//������д�����ݵ��ֽ��������ж��Ƿ�д�������ݳɹ�
	dwBytesSent += dwBytesWritten;
	if(dwBytesSent != dwBytesToWrite)
	{
		sprintf(szError, "Probable Write Timeout: Total of %ld bytes sent\n", dwBytesSent);
		TRACE0(szError);
		return false;
	}
	else
	{
//		sprintf(szError, "%ld bytes written\n", dwBytesSent);
//		TRACE0(szError);
		return true;
	}
}

//������ջ���������
void MyCom::ProcessReceivedData(LPCSTR lpcByte, 
								 DWORD dwDataLenght)
{
#ifdef _DEBUG
	char szBuf[128];
	char szTemp[8];
	CTime timeCur = time(NULL);
	sprintf(szBuf, "(R_%2d:%2d:%2d)", timeCur.GetHour(), timeCur.GetMinute(), timeCur.GetSecond());
	for(DWORD i = 0; i < dwDataLenght; i++)
	{
		sprintf(szTemp, "%02x ", (unsigned char)lpcByte[i]);
		strcat(szBuf, szTemp);
	}

	TRACE(szBuf);
#endif
}

//�򴮿ڷ�������
BOOL MyCom::WriteData2Comm(LPCSTR lpcByte, 
							DWORD dwDataLenght)
{
#ifdef _DEBUG
	char szBuf[128];
	char szTemp[8];
	CTime timeCur = time(NULL);
	sprintf(szBuf, "(S_%2d:%2d:%2d)", timeCur.GetHour(), timeCur.GetMinute(), timeCur.GetSecond());
	for(DWORD i = 0; i < dwDataLenght; i++)
	{
		sprintf(szTemp, "%02x ", (unsigned char)lpcByte[i]);
		strcat(szBuf, szTemp);
	}

	TRACE(szBuf);
#endif

	BOOL bRet = false;
	//����ǰ���������ڵ�ǰ�߳�
	m_CSec.Lock();

	__try
	{
		if(!m_bConnected)
		{
			TRACE("���Ƚ�����ȷ�Ĵ���ͨѶ����!\n");
			__leave;
		}
		
		LPCONNECTION lpConnection = this->LockConnection();
		
		bRet = MyCom::CommWriteBlock(lpConnection->hCommDev,
			lpcByte, dwDataLenght, &lpConnection->olWrite);
		
		this->UnLockConnection();
	}
	__finally
	{
		m_CSec.Unlock();
		return bRet;
	}
}

//�趨��ǰ��ʱ����
void MyCom::DelayReply(DWORD dwTime)
{
	DWORD dwStart = GetTickCount();

	while(1)
	{
		MSG msg;
		//��Ӧ������Ϣ�Ĵ���
		while(::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}

		if(GetTickCount() - dwStart > dwTime)
			return;
	
		::Sleep(1);
	}
}
