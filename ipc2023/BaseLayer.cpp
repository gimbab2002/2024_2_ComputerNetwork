// BaseLayer.cpp: implementation of the CBaseLayer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"  // ǥ�� �ý��� ���� ���� (Windows ���α׷� ���� �� ���)
#include "ipc2023.h" // ������Ʈ ���� ��� ���� (������Ʈ�� ���� ���ǵ�)
#include "BaseLayer.h" // CBaseLayer Ŭ���� ������ �����ϴ� ��� ����
#include "pch.h" // �̸� �����ϵ� ��� ����

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__; // ���� ���� �̸��� �����ϴ� ��ũ��
#define new DEBUG_NEW // �޸� ���� �˻縦 ���� ����� ���� ����
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// ������: CBaseLayer ��ü�� ������ �� ȣ���
CBaseLayer::CBaseLayer(char* pName)
	: m_nUpperLayerCount(0),  // ���� ���̾��� ������ 0���� �ʱ�ȭ
	mp_UnderLayer(NULL)       // ���� ���̾ NULL�� �ʱ�ȭ
{
	m_pLayerName = pName; // ���̾� �̸��� ���ڷ� ���� ������ ����
}

// �Ҹ���: CBaseLayer ��ü�� �Ҹ�� �� ȣ���
CBaseLayer::~CBaseLayer()
{
	// �Ҹ��ڿ��� Ư���� �۾��� ������, �⺻ �޸� ������ �ڵ����� �̷����
}

// SetUnderUpperLayer �Լ�: ���ڷ� ���� ���̾ ���� ���̾�� �����ϰ�, 
// ���� ���̾ �ش� ���̾��� ���� ���̾�� ����
void CBaseLayer::SetUnderUpperLayer(CBaseLayer* pUULayer)
{
	if (!pUULayer) // ���ڷ� ���� �����Ͱ� NULL�� ���
	{
#ifdef _DEBUG
		TRACE("[CBaseLayer::SetUnderUpperLayer] The variable 'pUULayer' is NULL");
#endif
		return; // NULL�� ��� �۾��� �ߴ��ϰ� ����
	}

	//////////////////////// fill the blank ///////////////////////////////
	// ���ڷ� ���� ������ ���� ������ ���� ���̾�� ����
	// �׸��� ���� ������ ���ڷ� ���� ������ ���� ���̾�� ����
	this->mp_UnderLayer = pUULayer;        // ���� ���̾� ����
	pUULayer->SetUpperLayer(this);         // ���� ���̾� ����
	///////////////////////////////////////////////////////////////////////
}

// SetUpperUnderLayer �Լ�: ���ڷ� ���� ���̾ ���� ���̾�� �����ϰ�, 
// ���� ���̾ �ش� ���̾��� ���� ���̾�� ����
void CBaseLayer::SetUpperUnderLayer(CBaseLayer* pUULayer)
{
	if (!pUULayer) // ���ڷ� ���� �����Ͱ� NULL�� ���
	{
#ifdef _DEBUG
		TRACE("[CBaseLayer::SetUpperUnderLayer] The variable 'pUULayer' is NULL");
#endif
		return; // NULL�� ��� �۾��� �ߴ��ϰ� ����
	}

	//////////////////////// fill the blank ///////////////////////////////
	// ���ڷ� ���� ������ ���� ���̾�� ����
	// �׸��� ���� ������ �ش� ������ ���� ���̾�� ����
	SetUpperLayer(pUULayer); // ���� ���̾� ����
	pUULayer->SetUnderLayer(this); // ���� ���̾� ����
	///////////////////////////////////////////////////////////////////////
}

// SetUpperLayer �Լ�: ���ڷ� ���� ���̾ ���� ���̾� �迭�� �߰�
void CBaseLayer::SetUpperLayer(CBaseLayer* pUpperLayer)
{
	if (!pUpperLayer) // ���ڷ� ���� �����Ͱ� NULL�� ���
	{
#ifdef _DEBUG
		TRACE("[CBaseLayer::SetUpperLayer] The variable 'pUpperLayer' is NULL");
#endif
		return; // NULL�� ��� �۾��� �ߴ��ϰ� ����
	}

	// ���� ���̾� �迭�� ���ڷ� ���� ���̾ �߰��ϰ�, ���� ���̾� ������ ������Ŵ
	this->mp_aUpperLayer[m_nUpperLayerCount++] = pUpperLayer;
}

// SetUnderLayer �Լ�: ���ڷ� ���� ���̾ ���� ���̾�� ����
void CBaseLayer::SetUnderLayer(CBaseLayer* pUnderLayer)
{
	if (!pUnderLayer) // ���ڷ� ���� �����Ͱ� NULL�� ���
	{
#ifdef _DEBUG
		TRACE("[CBaseLayer::SetUnderLayer] The variable 'pUnderLayer' is NULL\n");
#endif
		return; // NULL�� ��� �۾��� �ߴ��ϰ� ����
	}

	// ���� ���̾ ���ڷ� ���� ���̾�� ����
	this->mp_UnderLayer = pUnderLayer;
}

// GetUpperLayer �Լ�: ���� ���̾� �迭���� �ε����� �ش��ϴ� ���̾ ��ȯ
CBaseLayer* CBaseLayer::GetUpperLayer(int nindex)
{
	// �ε����� ��ȿ���� ������ NULL�� ��ȯ
	if (nindex < 0 ||
		nindex > m_nUpperLayerCount || // �迭 ������ ��� ���
		m_nUpperLayerCount < 0) // ���� ���̾ ���� ���
	{
#ifdef _DEBUG
		TRACE("[CBaseLayer::GetUpperLayer] There is no UpperLayer in Array..\n");
#endif 
		return NULL; // ��ȿ���� ���� �ε����� NULL ��ȯ
	}

	return mp_aUpperLayer[nindex]; // �ش� �ε����� ���� ���̾ ��ȯ
}

// GetUnderLayer �Լ�: ������ ���� ���̾ ��ȯ
CBaseLayer* CBaseLayer::GetUnderLayer()
{
	if (!mp_UnderLayer) // ���� ���̾ �����Ǿ� ���� ���� ���
	{
#ifdef _DEBUG
		TRACE("[CBaseLayer::GetUnderLayer] There is not a UnderLayer..\n");
#endif 
		return NULL; // NULL ��ȯ
	}

	return mp_UnderLayer; // ������ ���� ���̾ ��ȯ
}

// GetLayerName �Լ�: ���̾��� �̸��� ��ȯ
char* CBaseLayer::GetLayerName()
{
	return m_pLayerName; // ���̾� �̸��� ��ȯ
}
