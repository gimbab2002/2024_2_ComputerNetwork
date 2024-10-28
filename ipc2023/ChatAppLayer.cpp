// ChatAppLayer.cpp: implementation of the CChatAppLayer class.
//
//////////////////////////////////////////////////////////////////////
#define _CRT_SECURE_NO_WARNINGS

#include "stdafx.h" // 표준 시스템 포함 파일 (Windows 프로그램 개발 시 일반적으로 포함)
#include "pch.h"    // 미리 컴파일된 헤더 파일
#include "ChatAppLayer.h" // ChatAppLayer 클래스의 선언을 포함하는 헤더 파일
#include "EthernetLayer.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__; // 현재 소스 파일 이름을 저장하는 매크로
#define new DEBUG_NEW // 메모리 누수 검사를 위한 디버그 힙과의 통합
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// 생성자: ChatAppLayer 객체가 생성될 때 호출됨
CChatAppLayer::CChatAppLayer(char* pName)
	: CBaseLayer(pName), // 부모 클래스인 CBaseLayer의 생성자를 호출하면서 pName 전달
	mp_Dlg(NULL) // 멤버 변수 mp_Dlg 초기화 (NULL 값으로 설정)
{
	ResetHeader(); // 헤더 정보를 초기화하는 함수 호출
}

// 소멸자: ChatAppLayer 객체가 소멸될 때 호출됨
CChatAppLayer::~CChatAppLayer()
{
	// 소멸자에 특별한 작업은 없으며, 기본 메모리 정리가 자동으로 수행됨
}


// 헤더 초기화 함수: 헤더의 모든 필드를 초기화함
void CChatAppLayer::ResetHeader()
{
	m_sHeader.total_length = 0x0000;      // 데이터 길이를 0으로 초기화
	m_sHeader.app_length = 0x0000;
	m_sHeader.app_type = 0x00;          // 데이터 타입을 0으로 초기화
	memset(m_sHeader.app_data, 0, APP_DATA_SIZE); // 데이터 영역을 0으로 초기화
}


// 데이터 전송 함수: 상위 레이어에서 전달받은 데이터를 현재 레이어에서 처리한 후 하위 레이어로 전송함
BOOL CChatAppLayer::Send(unsigned char* ppayload, int nlength)
{
	BOOL bSuccess = FALSE; // 전송 성공 여부를 저장할 변수
	m_ppayload = ppayload;
	m_length = nlength;
	// Mutex 잠금
	std::lock_guard<std::mutex> lock(sendMutex);

	if (nlength <= APP_DATA_SIZE) {
		((CEthernetLayer*)GetUnderLayer())->SetFrameType(0x2080);
		m_sHeader.app_seq_num = 0;
		m_sHeader.total_length = (unsigned short)nlength; // 헤더에 데이터 총 길이를 설정
		memcpy(m_sHeader.app_data, ppayload, nlength); 
		// Ethernet 레이어(하위 레이어)로 데이터를 넘겨줌
		bSuccess = mp_UnderLayer->Send((unsigned char*)&m_sHeader, nlength + APP_HEADER_SIZE);
		TRACE("ChatAppLayer: Sent %d bytes\n", nlength);

	}
	else {
		AfxBeginThread(ChatThread, this);
	}
	return bSuccess; // 전송 성공 여부를 반환
}

BOOL CChatAppLayer::Receive(unsigned char* ppayload)
{
	TRACE("a\n");
	// ppayload를 ChatApp 헤더 구조체로 넣는다.
	CHAT_DLG_ACK_HEADER* ack_hdr = (CHAT_DLG_ACK_HEADER*)ppayload;
	if (ack_hdr->ack_type == 0x2024) {
		TRACE("Sending ack to chatdlglayer");
		mp_aUpperLayer[0]->Receive(ppayload);
		return TRUE;
	}
	PCHAT_APP_HEADER app_hdr = (PCHAT_APP_HEADER)ppayload;
	static unsigned char* GetBuff;
	static int seq_num;
	static int index;
	if (app_hdr->total_length <= APP_DATA_SIZE) { //단편화 필요x
		GetBuff = (unsigned char*)malloc(app_hdr->total_length); //app_hdr->app_length크기만큼 메모리 할당
		memset(GetBuff, 0, app_hdr->total_length); //getbuff를 app_hdr->app_length크기만큼 0으로 초기화
		memcpy(GetBuff, app_hdr->app_data, app_hdr->total_length); //app_hdr->app_data에서 app_hdr->app_length크기만큼 getbuff에 복사
		GetBuff[app_hdr->total_length] = '\0'; //getbuff의 app_length번째 인덱스에 null(문자열의 끝)을 설정
		TRACE("%d app\n", app_hdr->total_length);
		TRACE("%s\n", GetBuff);
		TRACE("%s\n", app_hdr->app_data);
		mp_aUpperLayer[0]->Receive((unsigned char*)GetBuff); //ipc2023Dlg의 receive에 getbuff 전송 
		return TRUE;
	}
	// 밑 계층에서 넘겨받은 ppayload를 분석하여 ChatDlg 계층으로 넘겨준다.
	if (app_hdr->app_type == DATA_TYPE_BEGIN) // 데이터 첫 부분
	{
		seq_num = 0;
		index = 0;
		// 첫 부분 일 경우 그 크기만큼 버퍼 할당
		GetBuff = (unsigned char*)malloc(app_hdr->total_length+1);
		if (GetBuff == NULL) {
			TRACE("메모리 할당 실패\n");
			return FALSE;
		}
		memset(GetBuff, 0, app_hdr->total_length);  // GetBuff를 초기화해준다.
	}
	else if (app_hdr->app_type == DATA_TYPE_CONT) // 데이터 중간 부분
	{
		seq_num++;
		if (seq_num != app_hdr->app_seq_num)
			TRACE("%d 데이터 패킷 누락\n", seq_num);
		else
			TRACE("%d 데이터 패킷 수신\n", seq_num);
		// 계속 버퍼에 쌓는다.
		memcpy(GetBuff + index, app_hdr->app_data, app_hdr->app_length);
		index += app_hdr->app_length;
	}
	else if (app_hdr->app_type == DATA_TYPE_END) // 데이터 끝 부분
	{
		// 버퍼에 쌓인 데이터를 다시 GetBuff에 넣는다.
		GetBuff[app_hdr->total_length] = '\0';
		TRACE("%d app\n", app_hdr->total_length);
		TRACE("%d buff\n", index);
		// 위에서 만들어진 메시지 포맷을 ipc2023Dlg로 넘겨준다.
		mp_aUpperLayer[0]->Receive((unsigned char*)GetBuff);
		free(GetBuff);
		TRACE("free\n");
	}
	else
		return FALSE;

	return TRUE;
}
// 데이터 수신 함수: 하위 레이어로부터 데이터를 수신하여 상위 레이어로 전달
UINT CChatAppLayer::ChatThread(LPVOID pParam) {
	// Thread 함수가 static으로 사용되기에, 인자로 넘겨받은 pParam에 들어있는 Chatapp의 클래스를 이용하여 간접 접근한다.
	BOOL bSuccess = FALSE;
	CChatAppLayer* pChat = (CChatAppLayer*)pParam;
	
	int data_length = APP_DATA_SIZE; // 보낼 data의 길이
	int seq_tot_num; // Sequential
	int temp = 0;
	
	seq_tot_num = (pChat->m_length / APP_DATA_SIZE) + 1;

	for (int i = 0; i <= seq_tot_num + 1; i++)
	{
		((CEthernetLayer*)(pChat->GetUnderLayer()))->SetFrameType(0x2080);
		// 보낼 data의 길이를 결정
		if (i == seq_tot_num) // 보낼 횟수의 가장 마지막일 때는, 남은 데이터의 길이만큼 보낸다.
				data_length = pChat->m_length % APP_DATA_SIZE;
		else // 처음, 중간 데이터 일 때, APP_DATA_SIZE만큼 보낸다.
				data_length = APP_DATA_SIZE;

		memset(pChat->m_sHeader.app_data, 0, data_length);

		if (i == 0) // 처음부분 : 타입은 0x00, 데이터의 총 길이를 전송한다.
		{
			pChat->m_sHeader.total_length = pChat->m_length;
			pChat->m_sHeader.app_type = DATA_TYPE_BEGIN;
			pChat->m_sHeader.app_seq_num = 0;
			memset(pChat->m_sHeader.app_data, 0, data_length);
			data_length = 0;
		}
		else if (i != 0 && i <= seq_tot_num) // 중간 부분 : 타입은 0x01, seq_num는 순서대로, 
		{
			pChat->m_sHeader.app_type = DATA_TYPE_CONT;
			pChat->m_sHeader.app_seq_num = i;
			pChat->m_sHeader.app_length = data_length;
			TRACE("data_length: %d\n", data_length);
			TRACE("%d\n", pChat->m_sHeader.app_length);
			CString str;
			str = pChat->m_ppayload;
			str = str.Mid(temp, temp + data_length);

			memcpy(pChat->m_sHeader.app_data, str, data_length);
			temp += data_length;
		}
		else // 마지막 부분 : 타입은 0x02
		{
			pChat->m_sHeader.app_seq_num = i;
			pChat->m_sHeader.app_type = DATA_TYPE_END;
			memset(pChat->m_ppayload, 0, data_length);
			data_length = 0;
		}
		bSuccess = pChat->mp_UnderLayer->Send((unsigned char*)&pChat->m_sHeader, data_length + APP_HEADER_SIZE);
		TRACE("ChatAppLayer: Sending %d bytes\n", data_length+ APP_HEADER_SIZE);
	}
	TRACE("스레드 종료");
	return bSuccess;
}