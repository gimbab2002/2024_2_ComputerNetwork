// FileAppLayer.cpp: implementation of the CFileAppLayer class.
//
//////////////////////////////////////////////////////////////////////
#define _CRT_SECURE_NO_WARNINGS

#include "stdafx.h" // ǥ�� �ý��� ���� ���� (Windows ���α׷� ���� �� �Ϲ������� ����)
#include "pch.h"    // �̸� �����ϵ� ��� ����
#include "EthernetLayer.h"
#include "FileAppLayer.h"
#include "ipc2023Dlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__; // ���� �ҽ� ���� �̸��� �����ϴ� ��ũ��
#define new DEBUG_NEW // �޸� ���� �˻縦 ���� ����� ������ ����
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFileAppLayer::CFileAppLayer(char* pName)
    : CBaseLayer(pName) // �θ� Ŭ������ CBaseLayer�� �����ڸ� ȣ���ϸ鼭 pName ����
{
    ResetHeader(); // ��� ������ �ʱ�ȭ�ϴ� �Լ� ȣ��
}

// �Ҹ���: ChatAppLayer ��ü�� �Ҹ�� �� ȣ���
CFileAppLayer::~CFileAppLayer()
{
    // �Ҹ��ڿ� Ư���� �۾��� ������, �⺻ �޸� ������ �ڵ����� �����
}


// ��� �ʱ�ȭ �Լ�: ����� ��� �ʵ带 �ʱ�ȭ��
void CFileAppLayer::ResetHeader()
{   
    m_sHeader.total_length = 0x0000;      // ������ �� ���̸� 0���� �ʱ�ȭ
    m_sHeader.file_seq_num = 0x0000;      // ������ ������ 0���� �ʱ�ȭ
    m_sHeader.file_length = 0x0000;       // ������ ������ ���̸� 0���� �ʱ�ȭ
    m_sHeader.file_type = 0x00;          // ������ Ÿ���� 0���� �ʱ�ȭ
    memset(m_sHeader.file_data, 0, FILE_DATA_SIZE); // ������ ������ 0���� �ʱ�ȭ
}

// ������ ���� �Լ�: ���� ���̾�� ���޹��� �����͸� ���� ���̾�� ó���� �� ���� ���̾�� ������
BOOL CFileAppLayer::Send(CString filePath, CString fileName)
{
    m_filePath = filePath;
    m_fileName = fileName;

    // Mutex ���
    std::lock_guard<std::mutex> lock(sendMutex);

    AfxBeginThread(FileThread, this); //���� ����ȭ �� ������ ������� ����

    return TRUE;
}

UINT CFileAppLayer::FileThread(LPVOID pParam) {
    BOOL bSuccess = FALSE;
    CFileAppLayer* pFile = (CFileAppLayer*)pParam;
    
    int data_length = FILE_DATA_SIZE; // ���� data�� ����
    int seq_tot_num; // Sequential
    int temp = 0;
    TRACE("%s\n", pFile->m_fileName);
    std::ifstream file(pFile->m_filePath, std::ios::binary); // ���̳ʸ� ���� ���� ����
    if (!file) {
        TRACE("���� ���� ����");
        return FALSE; // ���� ���⿡ ����
    }
    // ���� ������ �̵�
    file.seekg(0, std::ios::end);
    pFile->m_sHeader.total_length = (unsigned long)file.tellg(); // ���� ��ġ(���� ũ��)�� ������
    file.seekg(0, std::ios::beg);

    seq_tot_num = (pFile->m_sHeader.total_length / FILE_DATA_SIZE) + 1;

    char* buffer = new char[pFile->m_sHeader.total_length];
    file.read(buffer, pFile->m_sHeader.total_length);

    for (int i = 0; i <= seq_tot_num + 1; i++) {
        ((CEthernetLayer*)(pFile->GetUnderLayer()))->SetFrameType(0x8020);
        // ���� data�� ���̸� ����
        if (i == seq_tot_num) // ���� Ƚ���� ���� �������� ����, ���� �������� ���̸�ŭ ������.
            data_length = pFile->m_sHeader.total_length % FILE_DATA_SIZE;
        else // ó��, �߰� ������ �� ��, APP_DATA_SIZE��ŭ ������.
            data_length = FILE_DATA_SIZE;

        if (i == 0) // ó���κ� : Ÿ���� 0x00, �������� �� ���̸� �����Ѵ�.
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
        else if (i != 0 && i <= seq_tot_num) // �߰� �κ� : Ÿ���� 0x01, seq_num�� �������, 
        {
            pFile->m_sHeader.file_type = DATA_TYPE_CONT;
            pFile->m_sHeader.file_seq_num = i;
            pFile->m_sHeader.file_length = data_length;
            std::vector<char> str(buffer + temp, buffer + temp + data_length);

            memcpy(pFile->m_sHeader.file_data, str.data(), data_length);
            temp += data_length;
        }
        else // ������ �κ� : Ÿ���� 0x02
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
    // ppayload�� ChatApp ��� ����ü�� �ִ´�.
    PFILE_APP_HEADER file_hdr = (PFILE_APP_HEADER)ppayload;
    static unsigned char* GetBuff;
    static unsigned char* filename;

    static int seq_num;
    static int index;
    BOOL fileReceiveSuccess = TRUE;
    // �� �������� �Ѱܹ��� ppayload�� �м��Ͽ� ChatDlg �������� �Ѱ��ش�.
    if (file_hdr->file_type == DATA_TYPE_BEGIN) // ������ ù �κ�
    {
        seq_num = 0;
        index = 0;
        TRACE("%s\n", file_hdr->file_data);

        // ù �κ� �� ��� �� ũ�⸸ŭ ���� �Ҵ�      
        GetBuff = (unsigned char*)malloc(file_hdr->total_length);
        filename = (unsigned char*)malloc(file_hdr->file_length + 1);
        memset(GetBuff, 0, file_hdr->total_length);  // GetBuff�� �ʱ�ȭ���ش�.
        memset(filename, 0, file_hdr->file_length);
        memcpy(filename, file_hdr->file_data, file_hdr->file_length);
        filename[file_hdr->file_length] = '\0';


        unsigned char fileReceiving[20] = ("File Receiving...");
        mp_aUpperLayer[0]->Receive(fileReceiving);
    }
    else if (file_hdr->file_type == DATA_TYPE_CONT) // ������ �߰� �κ�
    {
        seq_num++;
        if (seq_num != file_hdr->file_seq_num) {
            TRACE("%d ������ ��Ŷ ����\n", seq_num);
            fileReceiveSuccess = FALSE;
        }
        else
            TRACE("%d ������ ��Ŷ ����\n", seq_num);
        // ��� ���ۿ� �״´�.
        memcpy(GetBuff + index, file_hdr->file_data, file_hdr->file_length);
        index += file_hdr->file_length;
    }
    else if (file_hdr->file_type == DATA_TYPE_END) // ������ �� �κ�
    {
        TRACE("%d app\n", file_hdr->total_length);
        TRACE("%d buff\n", index);
        // ������ ������� �޽��� ������ ipc2023Dlg�� �Ѱ��ش�.
        FILE* file = fopen((const char*)filename, "wb");
        if (file == NULL) {
            TRACE("������ �� �� �����ϴ�");
        }
        if (fileReceiveSuccess) {
            fwrite(GetBuff, sizeof(unsigned char), file_hdr->total_length, file);
            mp_aUpperLayer[0]->Receive((unsigned char*)filename);
            unsigned char fileReceivingSuccess[20] = ("File Received.");
            mp_aUpperLayer[0]->Receive(fileReceivingSuccess);

            // ������ �ݽ��ϴ�
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

