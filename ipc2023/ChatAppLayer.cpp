// ChatAppLayer.cpp: implementation of the CChatAppLayer class.
//
//////////////////////////////////////////////////////////////////////
#define _CRT_SECURE_NO_WARNINGS

#include "stdafx.h" // ǥ�� �ý��� ���� ���� (Windows ���α׷� ���� �� �Ϲ������� ����)
#include "pch.h"    // �̸� �����ϵ� ��� ����
#include "ChatAppLayer.h" // ChatAppLayer Ŭ������ ������ �����ϴ� ��� ����
#include "EthernetLayer.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__; // ���� �ҽ� ���� �̸��� �����ϴ� ��ũ��
#define new DEBUG_NEW // �޸� ���� �˻縦 ���� ����� ������ ����
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// ������: ChatAppLayer ��ü�� ������ �� ȣ���
CChatAppLayer::CChatAppLayer(char* pName)
	: CBaseLayer(pName), // �θ� Ŭ������ CBaseLayer�� �����ڸ� ȣ���ϸ鼭 pName ����
	mp_Dlg(NULL) // ��� ���� mp_Dlg �ʱ�ȭ (NULL ������ ����)
{
	ResetHeader(); // ��� ������ �ʱ�ȭ�ϴ� �Լ� ȣ��
}

// �Ҹ���: ChatAppLayer ��ü�� �Ҹ�� �� ȣ���
CChatAppLayer::~CChatAppLayer()
{
	// �Ҹ��ڿ� Ư���� �۾��� ������, �⺻ �޸� ������ �ڵ����� �����
}


// ��� �ʱ�ȭ �Լ�: ����� ��� �ʵ带 �ʱ�ȭ��
void CChatAppLayer::ResetHeader()
{
	m_sHeader.total_length = 0x0000;      // ������ ���̸� 0���� �ʱ�ȭ
	m_sHeader.app_length = 0x0000;
	m_sHeader.app_type = 0x00;          // ������ Ÿ���� 0���� �ʱ�ȭ
	memset(m_sHeader.app_data, 0, APP_DATA_SIZE); // ������ ������ 0���� �ʱ�ȭ
}


// ������ ���� �Լ�: ���� ���̾�� ���޹��� �����͸� ���� ���̾�� ó���� �� ���� ���̾�� ������
BOOL CChatAppLayer::Send(unsigned char* ppayload, int nlength)
{
	BOOL bSuccess = FALSE; // ���� ���� ���θ� ������ ����
	m_ppayload = ppayload;
	m_length = nlength;
	// Mutex ���
	std::lock_guard<std::mutex> lock(sendMutex);

	if (nlength <= APP_DATA_SIZE) {
		((CEthernetLayer*)GetUnderLayer())->SetFrameType(0x2080);
		m_sHeader.app_seq_num = 0;
		m_sHeader.total_length = (unsigned short)nlength; // ����� ������ �� ���̸� ����
		memcpy(m_sHeader.app_data, ppayload, nlength); 
		// Ethernet ���̾�(���� ���̾�)�� �����͸� �Ѱ���
		bSuccess = mp_UnderLayer->Send((unsigned char*)&m_sHeader, nlength + APP_HEADER_SIZE);
		TRACE("ChatAppLayer: Sent %d bytes\n", nlength);

	}
	else {
		AfxBeginThread(ChatThread, this);
	}
	return bSuccess; // ���� ���� ���θ� ��ȯ
}

BOOL CChatAppLayer::Receive(unsigned char* ppayload)
{
	TRACE("a\n");
	// ppayload�� ChatApp ��� ����ü�� �ִ´�.
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
	if (app_hdr->total_length <= APP_DATA_SIZE) { //����ȭ �ʿ�x
		GetBuff = (unsigned char*)malloc(app_hdr->total_length); //app_hdr->app_lengthũ�⸸ŭ �޸� �Ҵ�
		memset(GetBuff, 0, app_hdr->total_length); //getbuff�� app_hdr->app_lengthũ�⸸ŭ 0���� �ʱ�ȭ
		memcpy(GetBuff, app_hdr->app_data, app_hdr->total_length); //app_hdr->app_data���� app_hdr->app_lengthũ�⸸ŭ getbuff�� ����
		GetBuff[app_hdr->total_length] = '\0'; //getbuff�� app_length��° �ε����� null(���ڿ��� ��)�� ����
		TRACE("%d app\n", app_hdr->total_length);
		TRACE("%s\n", GetBuff);
		TRACE("%s\n", app_hdr->app_data);
		mp_aUpperLayer[0]->Receive((unsigned char*)GetBuff); //ipc2023Dlg�� receive�� getbuff ���� 
		return TRUE;
	}
	// �� �������� �Ѱܹ��� ppayload�� �м��Ͽ� ChatDlg �������� �Ѱ��ش�.
	if (app_hdr->app_type == DATA_TYPE_BEGIN) // ������ ù �κ�
	{
		seq_num = 0;
		index = 0;
		// ù �κ� �� ��� �� ũ�⸸ŭ ���� �Ҵ�
		GetBuff = (unsigned char*)malloc(app_hdr->total_length+1);
		if (GetBuff == NULL) {
			TRACE("�޸� �Ҵ� ����\n");
			return FALSE;
		}
		memset(GetBuff, 0, app_hdr->total_length);  // GetBuff�� �ʱ�ȭ���ش�.
	}
	else if (app_hdr->app_type == DATA_TYPE_CONT) // ������ �߰� �κ�
	{
		seq_num++;
		if (seq_num != app_hdr->app_seq_num)
			TRACE("%d ������ ��Ŷ ����\n", seq_num);
		else
			TRACE("%d ������ ��Ŷ ����\n", seq_num);
		// ��� ���ۿ� �״´�.
		memcpy(GetBuff + index, app_hdr->app_data, app_hdr->app_length);
		index += app_hdr->app_length;
	}
	else if (app_hdr->app_type == DATA_TYPE_END) // ������ �� �κ�
	{
		// ���ۿ� ���� �����͸� �ٽ� GetBuff�� �ִ´�.
		GetBuff[app_hdr->total_length] = '\0';
		TRACE("%d app\n", app_hdr->total_length);
		TRACE("%d buff\n", index);
		// ������ ������� �޽��� ������ ipc2023Dlg�� �Ѱ��ش�.
		mp_aUpperLayer[0]->Receive((unsigned char*)GetBuff);
		free(GetBuff);
		TRACE("free\n");
	}
	else
		return FALSE;

	return TRUE;
}
// ������ ���� �Լ�: ���� ���̾�κ��� �����͸� �����Ͽ� ���� ���̾�� ����
UINT CChatAppLayer::ChatThread(LPVOID pParam) {
	// Thread �Լ��� static���� ���Ǳ⿡, ���ڷ� �Ѱܹ��� pParam�� ����ִ� Chatapp�� Ŭ������ �̿��Ͽ� ���� �����Ѵ�.
	BOOL bSuccess = FALSE;
	CChatAppLayer* pChat = (CChatAppLayer*)pParam;
	
	int data_length = APP_DATA_SIZE; // ���� data�� ����
	int seq_tot_num; // Sequential
	int temp = 0;
	
	seq_tot_num = (pChat->m_length / APP_DATA_SIZE) + 1;

	for (int i = 0; i <= seq_tot_num + 1; i++)
	{
		((CEthernetLayer*)(pChat->GetUnderLayer()))->SetFrameType(0x2080);
		// ���� data�� ���̸� ����
		if (i == seq_tot_num) // ���� Ƚ���� ���� �������� ����, ���� �������� ���̸�ŭ ������.
				data_length = pChat->m_length % APP_DATA_SIZE;
		else // ó��, �߰� ������ �� ��, APP_DATA_SIZE��ŭ ������.
				data_length = APP_DATA_SIZE;

		memset(pChat->m_sHeader.app_data, 0, data_length);

		if (i == 0) // ó���κ� : Ÿ���� 0x00, �������� �� ���̸� �����Ѵ�.
		{
			pChat->m_sHeader.total_length = pChat->m_length;
			pChat->m_sHeader.app_type = DATA_TYPE_BEGIN;
			pChat->m_sHeader.app_seq_num = 0;
			memset(pChat->m_sHeader.app_data, 0, data_length);
			data_length = 0;
		}
		else if (i != 0 && i <= seq_tot_num) // �߰� �κ� : Ÿ���� 0x01, seq_num�� �������, 
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
		else // ������ �κ� : Ÿ���� 0x02
		{
			pChat->m_sHeader.app_seq_num = i;
			pChat->m_sHeader.app_type = DATA_TYPE_END;
			memset(pChat->m_ppayload, 0, data_length);
			data_length = 0;
		}
		bSuccess = pChat->mp_UnderLayer->Send((unsigned char*)&pChat->m_sHeader, data_length + APP_HEADER_SIZE);
		TRACE("ChatAppLayer: Sending %d bytes\n", data_length+ APP_HEADER_SIZE);
	}
	TRACE("������ ����");
	return bSuccess;
}