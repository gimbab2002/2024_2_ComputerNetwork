// ipc2023.cpp: 애플리케이션에 대한 클래스 동작을 정의합니다.
//

#include "pch.h"           // 미리 컴파일된 헤더
#include "framework.h"     // MFC 애플리케이션을 위한 기본 프레임워크 제공
#include "ipc2023.h"       // Cipc2023App 클래스의 선언이 포함된 헤더
#include "ipc2023Dlg.h"    // 애플리케이션의 메인 대화 상자 클래스 선언

#ifdef _DEBUG
#define new DEBUG_NEW  // 메모리 누수 검출을 위해 MFC 디버그 힙을 사용
#endif


// Cipc2023App 클래스 메시지 맵

BEGIN_MESSAGE_MAP(Cipc2023App, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)  // ID_HELP 명령을 처리하는 매핑
END_MESSAGE_MAP()


// Cipc2023App 생성자

Cipc2023App::Cipc2023App()
{
	// 다시 시작 관리자 지원
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;  // 애플리케이션이 비정상 종료 후 다시 시작 가능하도록 지원

	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.
}


// 유일한 Cipc2023App 개체입니다.

Cipc2023App theApp;  // 전역 애플리케이션 객체 생성 (Cipc2023App 클래스의 인스턴스)


// Cipc2023App 초기화

BOOL Cipc2023App::InitInstance()
{
	// 애플리케이션 매니페스트가 ComCtl32.dll 버전 6 이상을 사용하여 비주얼 스타일을
	// 사용하도록 지정하는 경우, Windows XP 상에서 반드시 InitCommonControlsEx()가 필요합니다.
	// InitCommonControlsEx()를 사용하지 않으면 창을 만들 수 없습니다.
	INITCOMMONCONTROLSEX InitCtrls;  // 공용 컨트롤 초기화를 위한 구조체
	InitCtrls.dwSize = sizeof(InitCtrls);  // 구조체의 크기 설정
	// 응용 프로그램에서 사용할 모든 공용 컨트롤 클래스를 포함하도록 이 항목을 설정하십시오.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;  // Windows 공용 컨트롤 사용 설정
	InitCommonControlsEx(&InitCtrls);  // 공용 컨트롤 클래스 초기화

	CWinApp::InitInstance();  // 부모 클래스(CWinApp)의 초기화 함수 호출


	AfxEnableControlContainer();  // MFC의 컨트롤 컨테이너 활성화

	// 대화 상자에 셸 트리 뷰 또는 셸 목록 뷰 컨트롤이 포함되어 있는 경우 셸 관리자를 만듭니다.
	CShellManager* pShellManager = new CShellManager;  // 셸 매니저 생성 (대화 상자에 셸 컨트롤이 있을 경우 필요)

	// MFC 컨트롤의 테마를 사용하기 위해 "Windows 원형" 비주얼 관리자 활성화
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));  // 비주얼 스타일을 적용하기 위한 설정

	// 표준 초기화
	// 이들 기능을 사용하지 않고 최종 실행 파일의 크기를 줄이려면
	// 아래에서 필요 없는 특정 초기화 루틴을 제거해야 합니다.
	// 해당 설정이 저장된 레지스트리 키를 변경하십시오.
	SetRegistryKey(_T("로컬 애플리케이션 마법사에서 생성된 애플리케이션"));  // 레지스트리에 설정을 저장할 키 지정 (주로 회사명 또는 조직명)

	// 메인 대화 상자 생성 및 실행
	Cipc2023Dlg dlg;  // 메인 대화 상자 객체 생성
	m_pMainWnd = &dlg;  // 애플리케이션의 메인 윈도우로 대화 상자 설정
	INT_PTR nResponse = dlg.DoModal();  // 대화 상자를 모달로 실행 (응답을 기다림)
	if (nResponse == IDOK)  // 사용자가 [확인] 버튼을 클릭했을 경우
	{
		// TODO: 여기에 [확인]을 클릭하여 대화 상자가 없어질 때 처리할 코드를 배치합니다.
	}
	else if (nResponse == IDCANCEL)  // 사용자가 [취소] 버튼을 클릭했을 경우
	{
		// TODO: 여기에 [취소]를 클릭하여 대화 상자가 없어질 때 처리할 코드를 배치합니다.
	}
	else if (nResponse == -1)  // 대화 상자를 만들지 못했을 경우
	{
		// 경고 메시지 출력 (디버그 모드에서만)
		TRACE(traceAppMsg, 0, "경고: 대화 상자를 만들지 못했으므로 애플리케이션이 예기치 않게 종료됩니다.\n");
		TRACE(traceAppMsg, 0, "경고: 대화 상자에서 MFC 컨트롤을 사용하는 경우 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS를 수행할 수 없습니다.\n");
	}

	// 위에서 만든 셸 관리자를 삭제합니다.
	if (pShellManager != nullptr)
	{
		delete pShellManager;  // 셸 매니저 메모리 해제
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();  // 사용된 모든 컨트롤 바를 정리 (MFC 라이브러리 사용 시)
#endif

	// 대화 상자가 닫혔으므로 응용 프로그램의 메시지 펌프를 시작하지 않고 응용 프로그램을 끝낼 수 있도록 FALSE를 반환합니다.
	return FALSE;  // 대화 상자가 종료된 후 프로그램이 종료되도록 설정
}
