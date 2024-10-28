
// ipc2023Dlg.h: 헤더 파일
//

#pragma once

#include "LayerManager.h"	// Added by ClassView
#include "ChatAppLayer.h"	// Added by ClassView
#include "EthernetLayer.h"	// Added by ClassView
#include "NILayer.h"	// Added by ClassView
#include "FileAppLayer.h"
// Cipc2023Dlg 대화 상자
class Cipc2023Dlg : public CDialogEx, public CBaseLayer
{
// 생성입니다.
public:
	Cipc2023Dlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.



// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_IPC2023_DIALOG };
#endif

	public:

	virtual BOOL PreTranslateMessage(MSG* pMsg);


	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()
public:
	CString m_unSrcAddr;
	CString m_unDstAddr;
	CString m_stMessage;
	CListBox m_ListChat;
	CListBox m_ListFileList;
	CComboBox m_ComboEnetName;
	CIPAddressCtrl m_unDstIPAddr;
	CIPAddressCtrl m_unSrcIPAddr;
	CProgressCtrl* m_progressBar;
	typedef struct _CHAT_DLG_ACK_HEADER {
		unsigned char ether_dstaddr[6];
		unsigned char ether_srcaddr[6];
		unsigned short   ack_type;     // Ack type (e.g., 1 for Ack)
	} CHAT_DLG_ACK_HEADER;

	CProgressCtrl m_ProgressCtrl;
public:
	BOOL			Receive(unsigned char* ppayload);
	inline void		SendData(); 
	unsigned char* MacAddrToHexInt(CString ehter);
	BOOL			ReceiveAck(unsigned char* ppayload);
	void			SendAck();
	void			CheckAckTimeout(int timeout);   // 타임아웃 후 Ack 확인 함수
	void			CreateHorizontalScroll();
	void			CreateHorizontalScroll2();
	void			UpdateProgressBar(int currentSeq, int totalSeq);


private:
	CLayerManager	m_LayerMgr;
	static UINT nRegAckMsg;  // Ack 메시지를 저장하는 변수
	static UINT nRegSendMsg;
	std::thread m_AckTimeoutThread;      // Ack 타임아웃을 처리할 스레드
	std::atomic<int> m_nAckReady;        // Ack 수신 여부 플래그 (0: 수신 완료, 1: 대기 중)
	std::atomic<int> f_nAckReady;        // Ack 수신 여부 플래그 (0: 수신 완료, 1: 대기 중)

	// UI 업데이트를 위한 사용자 정의 메시지 핸들러
	enum {
		IPC_INITIALIZING,
		IPC_READYTOSEND,
		IPC_WAITFORACK,
		IPC_ERROR,
		IPC_BROADCASTMODE,
		IPC_UNICASTMODE,
		IPC_ADDR_SET,
		IPC_ADDR_RESET,
		CFT_COMBO_SET,
	};

	void			SetDlgState(int state);
	inline void		EndofProcess();


	BOOL			m_bSendReady;

	// Object App
	CChatAppLayer* m_ChatApp;
	CNILayer* m_NI;
	CEthernetLayer* m_Eth;
	CFileAppLayer* m_FileApp;

	// Implementation
	UINT			m_wParam;
	DWORD			m_lParam;
	
public:
	afx_msg void OnBnClickedButtonSend();
	afx_msg void OnBnClickedButtonSetDstAddr();
	afx_msg void OnCbnSelchangeComboSelSrcAddr();
	afx_msg void OnBnClickedButtonFileSelect();
	afx_msg void OnBnClickedButtonFileSend();
	afx_msg LRESULT OnUpdateUI(WPARAM wParam, LPARAM lParam);

};
