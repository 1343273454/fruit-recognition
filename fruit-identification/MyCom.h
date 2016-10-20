// MyCom.h: interface for the MyCom class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYCOM_H__BF6B4ECC_762F_48D2_9365_F123560D9CAA__INCLUDED_)
#define AFX_MYCOM_H__BF6B4ECC_762F_48D2_9365_F123560D9CAA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxmt.h>

// flow control
#define FC_DTRDSR  0x01
#define FC_RTSCTS  0x02
#define FC_XONXOFF 0x04

#define ASCII_XON  0x11
#define ASCII_XOFF 0x13

class MyCom;
typedef struct _CONNECTION 
{
	HANDLE				hCommDev;	//��ǰ��ͨѶ���Ӿ��
	HANDLE				hEventCancel;//��ǰ�˳��ź��¼�
	DWORD				dwRefCount;	//��ǰͨѶ���ӵ����ô���
	MyCom*				pCom;		//MyCom�����ָ��
	OVERLAPPED			olWrite;	//д�ص�I/O����
	OVERLAPPED			olRead;		//���ص�I/O����
	CCriticalSection	CSec;		//��ǰ�ṹ֧�ֶ��̷߳��ʵĻ�����
}CONNECTION, *LPCONNECTION;
typedef const CONNECTION* LPCCONNECTION;

typedef struct _COMM_INFO{
	BYTE	byCommPort;	//ͨѶ�˿����ƣ���ΪCOM1-4
	BYTE	byByteSize;	//����λ��������Ϊ8
	BYTE	byParity;	//��ż���鷽ʽ������Ϊ0
	BYTE	byStopBits;	//ֹͣλ������Ϊ1
	DWORD	dwBaudRate;	//�����ʣ�����Ϊ38400
	BYTE	byXonXoff;	
	BYTE	byFlowCtrl;	//�����Ʒ�ʽ
}COMM_INFO;

//��ǰ�õ��Ĵ���ͨѶ����״̬
typedef enum{
	PS_NOREADY,			//��ǰ״̬��û��(��ʼ״̬)
	PS_READY,			//��ǰ���ھ���״̬
	PS_WAITTING,		//��ǰ�������ݵȴ�״̬
	PS_RECEIVINGDATA,	//��ǰ���ڽ�������
	PS_RECEIVED_FAULT	//��ǰ�������ݴ���
}PROCESS_STATUS;

class MyCom
{
public:
	enum{
		//��ǰ��ͨ�Ż�����һ��������ȡ���ݵ��ڴ滺���С
		MAX_BUF_BLOCK	=	128,
		//�쳣��ֹ��ǰ�ڴ򿪵Ķ˿��������е�����������������
		PURGE_FLAG		=	PURGE_TXABORT | PURGE_RXABORT 
							| PURGE_TXCLEAR | PURGE_RXCLEAR,
		//��ǰ���ڵ�ͨ��ʱ��Ҫ����֪ͨ���¼�����
		COMM_MASK		=	EV_RXCHAR,
		//��ǰ��ȴ�ʱ��(����)
		MAX_WAITTING	=	500,
		//��ǰ��С�ȴ�ʱ��(����)
		MIN_WAITTING	=	100,
		//��������ͬһ��ʱ��ʱ����
		RECONNECT_TIME	=	1000
	};
	//��ǰ���ڵ�������Ϣ
	COMM_INFO	m_CommInfo;
	CONNECTION	m_Connection;

	MyCom();
	virtual ~MyCom();

	//������ǰ��ͨѶ����
	virtual BOOL Connect();
	//���½�����ǰ��ͨѶ����
	virtual BOOL ReConnect();
	//ȡ����ǰ��ͨѶ����
	virtual void DisConnect();
	//��ʼ����ǰ�Ĵ�����Ϣ
	virtual BOOL InitCommInfo();
	//������ջ���������
	virtual void ProcessReceivedData(LPCSTR lpcByte/*�Ӵ��ڽ��ջ���������*/, 
		DWORD dwDataLenght/*�Ӵ��ڽ��ջ��������ݵĳ���*/);
	//�򴮿ڷ�������
	virtual BOOL WriteData2Comm(LPCSTR lpcByte/*��Ҫд�봮�ڵ�����*/, 
		DWORD dwDataLenght/*��Ҫд�봮�ڵ����ݵĳ���*/);

	//��ǰ��ͨѶ�˿ڼ����̺߳���
	static UINT CommMonitorProc(void* pData/*���̴߳��ݸ�����(����)�̵߳Ĳ���*/);
	//��ȡ��ǰͨѶ�˿ڻ����е�����
	static UINT CommReadBlock(HANDLE hFile/*�򿪵Ĵ����ļ����*/, 
		LPOVERLAPPED lpOL/*���ڶ�����ʱ���첽ͨѶ�¼�*/, 
		LPSTR lpszBlock/*���ڶ��������ݵĻ�����*/, 
		UINT nMaxLength/*���ڶ��������ݵĻ������Ĵ�С*/);
	//��ǰͨѶ�˿ڷ�������
	static BOOL CommWriteBlock(HANDLE hFile/*�򿪵Ĵ����ļ����*/, 
		LPCSTR lpcByte/*��Ҫд�����ڵ�����*/, 
		DWORD dwBytesToWrite/*��Ҫд�����ڵ����ݵ��ֽ���*/, 
		OVERLAPPED *pOLWrite/*����д����ʱ���첽ͨѶ�¼�*/);
	
	//��ȡ��ǰ��ͨѶ����ָ��,�������ڵ�ǰ�߳�
	//(�˺���������UnLockConnection()��Ե���, ʱ��������̫��)
	LPCONNECTION LockConnection();
	//�ͷŵ�ǰ��ͨѶ����ָ��(�����ǰ�̵߳�����)
	//(�˺���������LockConnection()��Ե���, ʱ��������̫��)
	void UnLockConnection();

	//�жϵ�ǰ��ͨѶ�����Ƿ��Ѿ�����
	BOOL IsConnected();
	//�趨��ǰ�Ĵ���״̬
	inline void SetProcessStatus(PROCESS_STATUS ePStatus)
	{m_ePStatus = ePStatus;}
	//��ȡ��ǰ�Ĵ���״̬
	inline PROCESS_STATUS GetProcessStatus()
	{return m_ePStatus;}
	//����ǰ���������ڵ�ǰ�߳�
	BOOL LockThread(){return m_CSec.Lock();}
	//����ǰ��������ڵ�ǰ�߳�
	void UnlockThread(){m_CSec.Unlock();}

protected:
	//��ǰͨѶ�����ͨѶ����״̬(true����, false������)
	volatile BOOL m_bConnected;
	//��ǰ�ļ����߳̾��
	HANDLE m_hMonitorThread;
	//��ǰ�߳�ͨѶ�����ڵ�״̬
	volatile PROCESS_STATUS m_ePStatus;
	//��ǰ���������̻߳����ĵ��ٽ���
	CCriticalSection	m_CSec;

	//��ȡ��ǰ��ͨѶ�˿�����
	CString GetCommName();
	//���ݴ��ڵ���Ż�ȡ��ǰ���ڵ�����
	CString GetCommName(BYTE bySN);
	//�趨��ǰ��ʱ����
	void DelayReply(DWORD dwTime);

private:
	//�趨��ǰ���ڵ�ͨ�Ų���
	BOOL SetCommState(HANDLE hFile);
	//���õ�ǰ���ڵĶ�д��ʱ
	BOOL SetCommTimeouts(HANDLE hFile);
	//�򿪴���
	BOOL OpenComm(HANDLE &hFile);
};

#endif // !defined(AFX_MYCOM_H__BF6B4ECC_762F_48D2_9365_F123560D9CAA__INCLUDED_)
