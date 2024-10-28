
// ipc2023Dlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "ipc2023.h"
#include "ipc2023Dlg.h"
#include "afxdialogex.h"
#define WM_UPDATE_UI (WM_USER + 1)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();

    // 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ABOUTBOX };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

    // 구현입니다.
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// Cipc2023Dlg 대화 상자



Cipc2023Dlg::Cipc2023Dlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_IPC2023_DIALOG, pParent)
    , CBaseLayer("ChatDlg")
    , m_bSendReady(FALSE)


{
    m_stMessage = _T("");

    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    //Protocol Layer Setting
    m_LayerMgr.AddLayer(new CChatAppLayer("ChatApp"));
    m_LayerMgr.AddLayer(new CFileAppLayer("FileApp"));
    m_LayerMgr.AddLayer(new CEthernetLayer("Ethernet"));
    m_LayerMgr.AddLayer(new CNILayer("NI"));
    m_LayerMgr.AddLayer(this);

    // 레이어를 연결한다. (레이어 생성)
    m_LayerMgr.ConnectLayers("NI ( *Ethernet ( *ChatApp ( *ChatDlg ) *FileApp ( *ChatDlg ) ) ) )");

    m_ChatApp = (CChatAppLayer*)m_LayerMgr.GetLayer("ChatApp");
    m_FileApp = (CFileAppLayer*)m_LayerMgr.GetLayer("FileApp");
    m_Eth = (CEthernetLayer*)m_LayerMgr.GetLayer("Ethernet");
    m_NI = (CNILayer*)m_LayerMgr.GetLayer("NI");

    //Protocol Layer Setting
}

void Cipc2023Dlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_NOWADDR, m_unSrcAddr);
    DDX_Text(pDX, IDC_EDIT_DST, m_unDstAddr);
    DDX_Text(pDX, IDC_EDIT_MSG, m_stMessage);
    DDX_Control(pDX, IDC_LIST_CHAT, m_ListChat);
    DDX_Control(pDX, IDC_LIST_FILELIST, m_ListFileList);
    DDX_Control(pDX, IDC_COMBO_SELSRCADDR, m_ComboEnetName);
}



BEGIN_MESSAGE_MAP(Cipc2023Dlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON_ADDR, &Cipc2023Dlg::OnBnClickedButtonSetDstAddr) //버튼 함수명 변경: OnBnClickedButtonAddr -> OnBnClickedButtonSetdstaddr --2024.10.02--
    ON_BN_CLICKED(IDC_BUTTON_SEND, &Cipc2023Dlg::OnBnClickedButtonSend)
    ON_CBN_SELCHANGE(IDC_COMBO_SELSRCADDR, &Cipc2023Dlg::OnCbnSelchangeComboSelSrcAddr)
    ON_BN_CLICKED(IDC_BUTTON_FILESELECT, &Cipc2023Dlg::OnBnClickedButtonFileSelect)
    ON_BN_CLICKED(IDC_BUTTON2_FILESEND, &Cipc2023Dlg::OnBnClickedButtonFileSend)
    ON_MESSAGE(WM_UPDATE_UI, &Cipc2023Dlg::OnUpdateUI)   // 사용자 정의 메시지 매핑

END_MESSAGE_MAP()


// Cipc2023Dlg 메시지 처리기

BOOL Cipc2023Dlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

    // IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != nullptr)
    {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    //progress bar 초기화
    m_progressBar = (CProgressCtrl*)GetDlgItem(IDC_PROGRESS_FILESEND);
    m_progressBar->SetRange(0, 100); // 범위 설정
    m_progressBar->SetPos(0); // 초기 위치 설정

    // 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
    //  프레임워크가 이 작업을 자동으로 수행합니다.
    SetIcon(m_hIcon, TRUE);         // 큰 아이콘을 설정합니다.
    SetIcon(m_hIcon, FALSE);      // 작은 아이콘을 설정합니다.

    // TODO: 여기에 추가 초기화 작업을 추가합니다.
    SetDlgState(IPC_INITIALIZING);
    SetDlgState(CFT_COMBO_SET);

    return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void Cipc2023Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void Cipc2023Dlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // 아이콘을 그립니다.
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR Cipc2023Dlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}




void Cipc2023Dlg::OnBnClickedButtonSend()
{
    UpdateData(TRUE);

    if (!m_stMessage.IsEmpty())
    {

        SendData();
        m_stMessage = "";

        m_nAckReady = 1;
        m_AckTimeoutThread = std::thread(&Cipc2023Dlg::CheckAckTimeout, this, 5000); // 2초 대기
        m_AckTimeoutThread.detach();  // 별도 스레드로 실행
        (CEdit*)GetDlgItem(IDC_EDIT_MSG)->SetFocus();
    }

    UpdateData(FALSE);
}

void Cipc2023Dlg::CheckAckTimeout(int timeout)
{
    // TODO: Add your message handler code here and/or call default

    std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
    if (m_nAckReady == 1)
    {
        // Ack가 오지 않았으므로 송신 실패 처리
        PostMessage(WM_UPDATE_UI, 0, 0);
    }
}



void Cipc2023Dlg::SendData()
{
    CString MsgHeader;
    if (m_unDstAddr == (unsigned int)0xff)
        MsgHeader.Format(_T("[%s:BROADCAST] "), m_unSrcAddr);
    else
        MsgHeader.Format(_T("[%s:%s] "), m_unSrcAddr, m_unDstAddr);

    m_ListChat.AddString(MsgHeader + m_stMessage);
    CreateHorizontalScroll();
    int nlength = m_stMessage.GetLength();
    unsigned char* ppayload = new unsigned char[nlength + 1];
    memcpy(ppayload, (unsigned char*)(LPCTSTR)m_stMessage, nlength);
    ppayload[nlength] = '\0';


    // 보낼 data와 메시지 길이를 Send함수로 넘겨준다.
    m_ChatApp->Send(ppayload, m_stMessage.GetLength());
}

BOOL Cipc2023Dlg::ReceiveAck(unsigned char* ppayload) {
    TRACE("수신 확인 (Ack) 받음\n");
    m_nAckReady = 0;
    return TRUE;
}
LRESULT Cipc2023Dlg::OnUpdateUI(WPARAM wParam, LPARAM lParam)
{
    // Ack 수신 여부에 따라 UI를 업데이트
    if (m_nAckReady == 1) // Ack를 받지 못했을 경우
    {
        // 채팅 리스트에 송신 실패 메시지를 추가
        m_ListChat.AddString(_T("송신 실패"));
    }
    return 0; // 반환 값은 0
}

BOOL Cipc2023Dlg::Receive(unsigned char* ppayload)
{   
    CHAT_DLG_ACK_HEADER* pHeader = (CHAT_DLG_ACK_HEADER*)ppayload;
    if (pHeader->ack_type == 0x2024) {
        ReceiveAck(ppayload);
        return TRUE;
    }
    else {
        CString Msg;
        int len_ppayload = strlen((char*)ppayload);

        unsigned char* GetBuff = (unsigned char*)malloc(len_ppayload);
        memset(GetBuff, 0, len_ppayload);
        memcpy(GetBuff, ppayload, len_ppayload);
        GetBuff[len_ppayload] = '\0';

        // App Header를 분석하여, 리스트 창에 뿌려줄 내용의 메시지를 구성한다.
        // 보내는 쪽 또는 받는 쪽과 GetBuff에 저장된 메시지 내용을 합친다.

        if (m_unSrcAddr == (unsigned int)0xff)
            Msg.Format(_T("[%s:BROADCAST] %s"), m_unDstAddr, (char*)GetBuff);
        else
            Msg.Format(_T("[%s:%s] %s"), m_unDstAddr, m_unSrcAddr, (char*)GetBuff);


        m_ListChat.AddString((LPCTSTR)Msg.GetBuffer(0));
        CreateHorizontalScroll();

        SendAck();
    }
    return TRUE;
}
void Cipc2023Dlg::SendAck() {
    // Ack 메시지 생성 (예: sequence number 포함)
    CHAT_DLG_ACK_HEADER ackMessage;
    ackMessage.ack_type = 0x2024;  // Ack 타입

    // MAC 주소 변환 후 ether_srcaddr와 ether_dstaddr에 할당
    unsigned char* srcAddr = MacAddrToHexInt(m_unSrcAddr);  // 근원지 MAC 주소 변환
    unsigned char* dstAddr = MacAddrToHexInt(m_unDstAddr);  // 목적지 MAC 주소 변환

    // 변환된 MAC 주소를 ackMessage 구조체에 복사
    memcpy(ackMessage.ether_srcaddr, srcAddr, 6);  // 근원지 MAC 주소 복사
    memcpy(ackMessage.ether_dstaddr, dstAddr, 6);  // 목적지 MAC 주소 복사


    // MAC 주소 출력 (디버깅용)
    TRACE("dstaddr: %02X:%02X:%02X:%02X:%02X:%02X\n", ackMessage.ether_dstaddr[0], ackMessage.ether_dstaddr[1], ackMessage.ether_dstaddr[2], ackMessage.ether_dstaddr[3], ackMessage.ether_dstaddr[4], ackMessage.ether_dstaddr[5]);
    TRACE("srcaddr: %02X:%02X:%02X:%02X:%02X:%02X\n", ackMessage.ether_srcaddr[0], ackMessage.ether_srcaddr[1], ackMessage.ether_srcaddr[2], ackMessage.ether_srcaddr[3], ackMessage.ether_srcaddr[4], ackMessage.ether_srcaddr[5]);
    
    unsigned char paddedAck[20];
    memset(paddedAck, 0, 20);
    memcpy(paddedAck, &ackMessage, sizeof(ackMessage));
    m_NI->Send(paddedAck, 20);  // 64바이트로 패딩된 패킷 전송
    // Ack 메시지를 NI 레이어로 송신
    m_NI->Send(paddedAck, 20);
}
BOOL Cipc2023Dlg::PreTranslateMessage(MSG* pMsg)
{
    // TODO: Add your specialized code here and/or call the base class
    switch (pMsg->message)
    {
    case WM_KEYDOWN:
        switch (pMsg->wParam)
        {
        case VK_RETURN:
            if (::GetDlgCtrlID(::GetFocus()) == IDC_EDIT_MSG)
                OnBnClickedButtonSend();
            return FALSE;
        case VK_ESCAPE: return FALSE;
        }
        break;
    }

    return CDialog::PreTranslateMessage(pMsg);
}


void Cipc2023Dlg::SetDlgState(int state)
{
    UpdateData(TRUE);
    int i;
    CString device_description;

    CButton* pSendButton = (CButton*)GetDlgItem(IDC_BUTTON_SEND);
    CButton* pSetAddrButton = (CButton*)GetDlgItem(IDC_BUTTON_SETDSTADDR);
    CEdit* pMsgEdit = (CEdit*)GetDlgItem(IDC_EDIT_MSG);
    CEdit* pSrcEdit = (CEdit*)GetDlgItem(IDC_NOWADDR);
    CEdit* pDstEdit = (CEdit*)GetDlgItem(IDC_EDIT_DST);

    CComboBox* pEnetNameCombo = (CComboBox*)GetDlgItem(IDC_COMBO_SELSRCADDR);
    CComboBox* pFileNameCombo = (CComboBox*)GetDlgItem(IDC_LIST_FILELIST);

    CButton* pFileSelButton = (CButton*)GetDlgItem(IDC_BUTTON_FILESELECT);
    CButton* pFileSendButton = (CButton*)GetDlgItem(IDC_BUTTON2_FILESEND);
    switch (state)
    {
    case IPC_INITIALIZING:
        pSendButton->EnableWindow(FALSE);
        pMsgEdit->EnableWindow(FALSE);
        m_ListChat.EnableWindow(TRUE);
        pFileSelButton->EnableWindow(FALSE);
        pFileSendButton->EnableWindow(FALSE);
        break;
    case IPC_READYTOSEND:
        pSendButton->EnableWindow(TRUE);
        pMsgEdit->EnableWindow(TRUE);
        m_ListChat.EnableWindow(TRUE);
        pFileSelButton->EnableWindow(TRUE);
        pFileSendButton->EnableWindow(TRUE);
        break;
    case IPC_WAITFORACK:   break;
    case IPC_ERROR:      break;
    case IPC_UNICASTMODE:
        m_unDstAddr.Format(_T("%.2x%.2x%.2x%.2x%.2x%.2x"), 0x00, 0x00, 0x00, 0x00, 0x00, 0x01);
        pDstEdit->EnableWindow(TRUE);
        break;
    case IPC_BROADCASTMODE:
        m_unDstAddr.Format(_T("%.2x%.2x%.2x%.2x%.2x%.2x"), 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
        pDstEdit->EnableWindow(FALSE);
        break;
    case IPC_ADDR_SET:
        pSetAddrButton->SetWindowText(_T("재설정(&R)"));
        pSrcEdit->EnableWindow(FALSE);
        pDstEdit->EnableWindow(FALSE);
        //pChkButton->EnableWindow(FALSE);
        pEnetNameCombo->EnableWindow(FALSE);
        m_NI->m_thrdSwitch = TRUE;

        break;
    case IPC_ADDR_RESET:
        pSetAddrButton->SetWindowText(_T("설정(&O)"));
        pSrcEdit->EnableWindow(TRUE);
        pDstEdit->EnableWindow(TRUE);
        pEnetNameCombo->EnableWindow(TRUE);
        m_NI->m_thrdSwitch = FALSE;
        break;
    case CFT_COMBO_SET:
        for (i = 0; i < NI_COUNT_NIC; i++) {
            if (!m_NI->GetAdapterObject(i))
                break;
            device_description = m_NI->GetAdapterObject(i)->description;
            device_description.Trim();
            pEnetNameCombo->AddString(device_description);
            pEnetNameCombo->SetCurSel(0);
        }


        break;
    }

    UpdateData(FALSE);
}


void Cipc2023Dlg::EndofProcess()
{
    m_LayerMgr.DeAllocLayer();
}


void Cipc2023Dlg::OnBnClickedButtonSetDstAddr()
{
    UpdateData(TRUE);


    if (!m_unDstAddr ||
        !m_unSrcAddr)
    {
        MessageBox(_T("주소를 설정 오류발생"),
            _T("경고"),
            MB_OK | MB_ICONSTOP);

        return;
    }

    if (m_bSendReady) {
        SetDlgState(IPC_ADDR_RESET);
        SetDlgState(IPC_INITIALIZING);
    }
    else {


        int nIndex = m_ComboEnetName.GetCurSel();
        m_NI->SetAdapterNumber(nIndex);

        CString inNicName = m_NI->GetAdapterObject(nIndex)->name;

        CEdit* pSrcEdit = (CEdit*)GetDlgItem(IDC_NOWADDR);

        pSrcEdit->SetWindowTextA(m_NI->GetNICardAddress((char*)inNicName.GetString()));

        //선택한 adapter에 맞는 ethernet주소

        //

        m_Eth->SetEnetSrcAddress(MacAddrToHexInt(m_unSrcAddr));
        m_Eth->SetEnetDstAddress(MacAddrToHexInt(m_unDstAddr));

        // 패킷 시작
        m_NI->PacketStartDriver();



        SetDlgState(IPC_ADDR_SET);
        SetDlgState(IPC_READYTOSEND);
    }

    m_bSendReady = !m_bSendReady;
}



unsigned char* Cipc2023Dlg::MacAddrToHexInt(CString ether)
{

    CString cstr;
    unsigned char* arp_ether = (u_char*)malloc(sizeof(u_char) * 6);

    for (int i = 0; i < 6; i++) {
        AfxExtractSubString(cstr, ether, i, ':');
        arp_ether[i] = (unsigned char)strtoul(cstr.GetString(), NULL, 16);
    }
    arp_ether[6] = '\0';

    return arp_ether;
}









void Cipc2023Dlg::OnCbnSelchangeComboSelSrcAddr()
{
    // 시작 주소를 선택하는 combo 박스 --2024.10.02--

    int nIndex = m_ComboEnetName.GetCurSel();
    m_NI->SetAdapterNumber(nIndex);

    CString inNicName = m_NI->GetAdapterObject(nIndex)->name;

    CEdit* pSrcEdit = (CEdit*)GetDlgItem(IDC_EDIT_SRC);

    pSrcEdit->SetWindowTextA(m_NI->GetNICardAddress((char*)inNicName.GetString()));

    //선택한 adapter에 맞는 ethernet주소

    //

    m_Eth->SetEnetSrcAddress(MacAddrToHexInt(m_unSrcAddr));

}



void Cipc2023Dlg::OnBnClickedButtonFileSelect()
{

    CFileDialog fileDlg(TRUE); // 파일 대화상자 열기

    fileDlg.m_ofn.lpstrFilter = _T("모든 파일 (*.*)|*.*||"); //파일 형식:모든 파일

    if (fileDlg.DoModal() == IDOK)
    {
        // 사용자가 선택한 파일 경로 가져오기
        CString filePath = fileDlg.GetPathName();

        CListBox* pListFilelist = (CListBox*)GetDlgItem(IDC_LIST_FILELIST);
        pListFilelist->AddString(filePath); //경로를 출력
        CreateHorizontalScroll2();

        UpdateData(FALSE);
        // 선택한 파일 경로로 원하는 작업 수행

    }
}


void Cipc2023Dlg::OnBnClickedButtonFileSend()
{
    CListBox* pListFilelist = (CListBox*)GetDlgItem(IDC_LIST_FILELIST); //파일 경로 저장된 리스트 박스 불러오기

    int nSel = pListFilelist->GetCurSel(); //ListBox에서 선택된 파일의 경로를 가져오는 시도

    if (nSel == LB_ERR) //선택된 경로가 없다면 오류 메세지 
    {
        AfxMessageBox(_T("전송할 파일을 선택해 주세요."));
        return;
    }

    CString filePath;
    pListFilelist->GetText(nSel, filePath); //가져온 경로를 복사

    CString fileName = filePath.Mid(filePath.ReverseFind('\\') + 1); //파일명

    CFile file; // 파일 열기
    if (!file.Open(filePath, CFile::modeRead | CFile::typeBinary))
    {
        AfxMessageBox(_T("파일을 열 수 없습니다."));
        return;
    }

    m_nAckReady = 1;
    m_AckTimeoutThread = std::thread(&Cipc2023Dlg::CheckAckTimeout, this, 2000); // 2초 대기
    m_AckTimeoutThread.detach();  // 별도 스레드로 실행
    m_ListChat.AddString(fileName);
    CreateHorizontalScroll();
    m_FileApp->Send(filePath, fileName); //파일 내용, 길이, 이름 전달


    file.Close();


}
//가로스크롤 만들기
void Cipc2023Dlg::CreateHorizontalScroll()
{
    CString str;
    CSize sz;
    int dx = 0;

    // 리스트박스의 DC(Device Context) 얻기
    CDC* pDC = m_ListChat.GetDC();

    // 리스트박스에 있는 모든 아이템의 너비를 확인
    for (int i = 0; i < m_ListChat.GetCount(); i++)
    {
        m_ListChat.GetText(i, str);
        sz = pDC->GetTextExtent(str);

        // 가장 넓은 문자열의 너비를 저장
        if (sz.cx > dx)
            dx = sz.cx;
    }

    // DC 해제
    m_ListChat.ReleaseDC(pDC);
    m_ListChat.SetHorizontalExtent(dx);

}
void Cipc2023Dlg::CreateHorizontalScroll2()
{
    CString str;
    CSize sz;
    int dx = 0;

    // 리스트박스의 DC(Device Context) 얻기
    CDC* pDC = m_ListFileList.GetDC();

    // 리스트박스에 있는 모든 아이템의 너비를 확인
    for (int i = 0; i < m_ListFileList.GetCount(); i++)
    {
        m_ListFileList.GetText(i, str);
        sz = pDC->GetTextExtent(str);

        // 가장 넓은 문자열의 너비를 저장
        if (sz.cx > dx)
            dx = sz.cx;
    }

    // DC 해제
    m_ListFileList.ReleaseDC(pDC);
    m_ListFileList.SetHorizontalExtent(dx);

}

//progress bar 진행
void Cipc2023Dlg::UpdateProgressBar(int currentSeq, int totalSeq) {

    int progressPercentage = (currentSeq * 100) / totalSeq;

    if (::IsWindow(m_progressBar->m_hWnd)) {
        m_progressBar->SetPos(progressPercentage);
    }
}