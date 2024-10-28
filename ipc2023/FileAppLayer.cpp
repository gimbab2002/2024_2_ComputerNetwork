// FileAppLayer.cpp: implementation of the CFileAppLayer class.
//
//////////////////////////////////////////////////////////////////////
#define _CRT_SECURE_NO_WARNINGS

#include "stdafx.h" // 표준 시스템 포함 파일 (Windows 프로그램 개발 시 일반적으로 포함)
#include "pch.h"    // 미리 컴파일된 헤더 파일
#include "EthernetLayer.h"
#include "FileAppLayer.h"
#include "ipc2023Dlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__; // 현재 소스 파일 이름을 저장하는 매크로
#define new DEBUG_NEW // 메모리 누수 검사를 위한 디버그 힙과의 통합
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFileAppLayer::CFileAppLayer(char* pName)
    : CBaseLayer(pName) // 부모 클래스인 CBaseLayer의 생성자를 호출하면서 pName 전달
{
    ResetHeader(); // 헤더 정보를 초기화하는 함수 호출
}

// 소멸자: ChatAppLayer 객체가 소멸될 때 호출됨
CFileAppLayer::~CFileAppLayer()
{
    // 소멸자에 특별한 작업은 없으며, 기본 메모리 정리가 자동으로 수행됨
}


// 헤더 초기화 함수: 헤더의 모든 필드를 초기화함
void CFileAppLayer::ResetHeader()
{   
    m_sHeader.total_length = 0x0000;      // 데이터 총 길이를 0으로 초기화
    m_sHeader.file_seq_num = 0x0000;      // 데이터 순서를 0으로 초기화
    m_sHeader.file_length = 0x0000;       // 보내는 데이터 길이를 0으로 초기화
    m_sHeader.file_type = 0x00;          // 데이터 타입을 0으로 초기화
    memset(m_sHeader.file_data, 0, FILE_DATA_SIZE); // 데이터 영역을 0으로 초기화
}

// 데이터 전송 함수: 상위 레이어에서 전달받은 데이터를 현재 레이어에서 처리한 후 하위 레이어로 전송함
BOOL CFileAppLayer::Send(CString filePath, CString fileName)
{
    m_filePath = filePath;
    m_fileName = fileName;

    // Mutex 잠금
    std::lock_guard<std::mutex> lock(sendMutex);

    AfxBeginThread(FileThread, this); //파일 단편화 및 전송을 스레드로 실행

    return TRUE;
}

UINT CFileAppLayer::FileThread(LPVOID pParam) {
    BOOL bSuccess = FALSE;
    CFileAppLayer* pFile = (CFileAppLayer*)pParam;
    
    int data_length = FILE_DATA_SIZE; // 보낼 data의 길이
    int seq_tot_num; // Sequential
    int temp = 0;
    TRACE("%s\n", pFile->m_fileName);
    std::ifstream file(pFile->m_filePath, std::ios::binary); // 바이너리 모드로 파일 열기
    if (!file) {
        TRACE("파일 열기 실패");
        return FALSE; // 파일 열기에 실패
    }
    // 파일 끝으로 이동
    file.seekg(0, std::ios::end);
    pFile->m_sHeader.total_length = (unsigned long)file.tellg(); // 현재 위치(파일 크기)를 가져옴
    file.seekg(0, std::ios::beg);

    seq_tot_num = (pFile->m_sHeader.total_length / FILE_DATA_SIZE) + 1;

    char* buffer = new char[pFile->m_sHeader.total_length];
    file.read(buffer, pFile->m_sHeader.total_length);

    for (int i = 0; i <= seq_tot_num + 1; i++) {
        ((CEthernetLayer*)(pFile->GetUnderLayer()))->SetFrameType(0x8020);
        // 보낼 data의 길이를 결정
        if (i == seq_tot_num) // 보낼 횟수의 가장 마지막일 때는, 남은 데이터의 길이만큼 보낸다.
            data_length = pFile->m_sHeader.total_length % FILE_DATA_SIZE;
        else // 처음, 중간 데이터 일 때, APP_DATA_SIZE만큼 보낸다.
            data_length = FILE_DATA_SIZE;

        if (i == 0) // 처음부분 : 타입은 0x00, 데이터의 총 길이를 전송한다.
        {
            pFile->m_sHeader.file_type = DATA_TYPE_BEGIN;
            pFile->m_sHeader.file_seq_num = 0;
            memset(pFile->m_sHeader.file_data, 0, data_length);
            CString filename;
            filename = pFile->m_fileName;
            data_length = strlen(filename);
            memcpy(pFile->m_sHeader.file_data, filename, data_length);
            TRACE("%s\n", pFile->m_sHeader.file_data);
            pFile->m_sHeader.file_length = data_length;
        }
        else if (i != 0 && i <= seq_tot_num) // 중간 부분 : 타입은 0x01, seq_num는 순서대로, 
        {
            pFile->m_sHeader.file_type = DATA_TYPE_CONT;
            pFile->m_sHeader.file_seq_num = i;
            pFile->m_sHeader.file_length = data_length;
            std::vector<char> str(buffer + temp, buffer + temp + data_length);

            memcpy(pFile->m_sHeader.file_data, str.data(), data_length);
            temp += data_length;
        }
        else // 마지막 부분 : 타입은 0x02
        {
            pFile->m_sHeader.file_seq_num = i;
            pFile->m_sHeader.file_type = DATA_TYPE_END;
            memset(buffer, 0, data_length);
            data_length = 0;
        }
        bSuccess = pFile->mp_UnderLayer->Send((unsigned char*)&pFile->m_sHeader, data_length + FILE_HEADER_SIZE);

        pFile->mp_aUpperLayer[0]->UpdateProgressBar(i, seq_tot_num);

        TRACE("FileAppLayer: Sent %d bytes\n", data_length + FILE_HEADER_SIZE);
    }
    delete[] buffer;
}

BOOL CFileAppLayer::Receive(unsigned char* ppayload) {
    // ppayload를 ChatApp 헤더 구조체로 넣는다.
    PFILE_APP_HEADER file_hdr = (PFILE_APP_HEADER)ppayload;
    static unsigned char* GetBuff;
    static unsigned char* filename;

    static int seq_num;
    static int index;
    BOOL fileReceiveSuccess = TRUE;
    // 밑 계층에서 넘겨받은 ppayload를 분석하여 ChatDlg 계층으로 넘겨준다.
    if (file_hdr->file_type == DATA_TYPE_BEGIN) // 데이터 첫 부분
    {
        seq_num = 0;
        index = 0;
        TRACE("%s\n", file_hdr->file_data);

        // 첫 부분 일 경우 그 크기만큼 버퍼 할당      
        GetBuff = (unsigned char*)malloc(file_hdr->total_length);
        filename = (unsigned char*)malloc(file_hdr->file_length + 1);
        memset(GetBuff, 0, file_hdr->total_length);  // GetBuff를 초기화해준다.
        memset(filename, 0, file_hdr->file_length);
        memcpy(filename, file_hdr->file_data, file_hdr->file_length);
        filename[file_hdr->file_length] = '\0';


        unsigned char fileReceiving[20] = ("File Receiving...");
        mp_aUpperLayer[0]->Receive(fileReceiving);
    }
    else if (file_hdr->file_type == DATA_TYPE_CONT) // 데이터 중간 부분
    {
        seq_num++;
        if (seq_num != file_hdr->file_seq_num) {
            TRACE("%d 데이터 패킷 누락\n", seq_num);
            fileReceiveSuccess = FALSE;
        }
        else
            TRACE("%d 데이터 패킷 수신\n", seq_num);
        // 계속 버퍼에 쌓는다.
        memcpy(GetBuff + index, file_hdr->file_data, file_hdr->file_length);
        index += file_hdr->file_length;
    }
    else if (file_hdr->file_type == DATA_TYPE_END) // 데이터 끝 부분
    {
        TRACE("%d app\n", file_hdr->total_length);
        TRACE("%d buff\n", index);
        // 위에서 만들어진 메시지 포맷을 ipc2023Dlg로 넘겨준다.
        FILE* file = fopen((const char*)filename, "wb");
        if (file == NULL) {
            TRACE("파일을 열 수 없습니다");
        }
        if (fileReceiveSuccess) {
            fwrite(GetBuff, sizeof(unsigned char), file_hdr->total_length, file);
            mp_aUpperLayer[0]->Receive((unsigned char*)filename);
            unsigned char fileReceivingSuccess[20] = ("File Received.");
            mp_aUpperLayer[0]->Receive(fileReceivingSuccess);

            // 파일을 닫습니다
            fclose(file);
            free(filename);
            free(GetBuff);
            TRACE("free\n");
        }
        else {
            unsigned char fileReceivingFail[25] = ("File Receiving fail...");
            mp_aUpperLayer[0]->Receive(fileReceivingFail);
           
        }
    }
    else
        return FALSE;

    return TRUE;
}

