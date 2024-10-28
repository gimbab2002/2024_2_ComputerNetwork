#pragma once
// LayerManager.cpp: implementation of the CLayerManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "pch.h"
#include "LayerManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLayerManager::CLayerManager()
	: m_nLayerCount(0),
	mp_sListHead(NULL),
	mp_sListTail(NULL),
	m_nTop(-1)
{
	// 생성자: 레이어 매니저 객체 생성 시 초기화
}

CLayerManager::~CLayerManager()
{
	// 소멸자: 레이어 매니저 객체가 파괴될 때 호출됨
	// 현재는 특별한 작업을 하지 않음
}

void CLayerManager::AddLayer(CBaseLayer* pLayer)
{
	// 레이어 배열에 새 레이어를 추가
	mp_aLayers[m_nLayerCount++] = pLayer;
}

CBaseLayer* CLayerManager::GetLayer(int nindex)
{
	// 인덱스를 기반으로 레이어를 반환
	return mp_aLayers[nindex];
}

CBaseLayer* CLayerManager::GetLayer(char* pName)
{
	// 레이어 이름으로 레이어를 검색하여 반환
	for (int i = 0; i < m_nLayerCount; i++)
	{
		if (!strcmp(pName, mp_aLayers[i]->GetLayerName()))
			return mp_aLayers[i];
	}

	return NULL;
}

void CLayerManager::ConnectLayers(char* pcList)
{
	// 레이어 연결을 설정하는 함수
	MakeList(pcList);         // 레이어 이름 목록을 생성
	LinkLayer(mp_sListHead);  // 레이어를 연결
	int arr;
	arr = 3;  // 현재 사용되지 않음
}

void CLayerManager::MakeList(char* pcList)
{
	// 주어진 문자열을 기반으로 레이어 목록을 생성
	while (pcList != (char*)0x01)
	{
		char sBuff[100];
		sscanf_s(pcList, "%s", sBuff, sizeof(sBuff));
		pcList = strchr(pcList, ' ') + 1;

		PNODE pNode = AllocNode(sBuff);
		AddNode(pNode);
	}
}

CLayerManager::PNODE CLayerManager::AllocNode(char* pcName)
{
	// 노드를 할당하고 초기화
	PNODE node = new NODE;
	ASSERT(node);

	strcpy_s(node->token, pcName);
	node->next = NULL;

	return node;
}

void CLayerManager::AddNode(PNODE pNode)
{
	// 노드를 리스트에 추가
	if (!mp_sListHead)
	{
		mp_sListHead = mp_sListTail = pNode;
	}
	else
	{
		mp_sListTail->next = pNode;
		mp_sListTail = pNode;
	}
}

void CLayerManager::Push(CBaseLayer* pLayer)
{
	// 스택에 레이어를 푸시
	if (m_nTop >= MAX_LAYER_NUMBER)
	{
#ifdef _DEBUG
		TRACE("The Stack is full.. so cannot run the push operation.. \n");
#endif
		return;
	}

	mp_Stack[++m_nTop] = pLayer;
}

CBaseLayer* CLayerManager::Pop()
{
	// 스택에서 레이어를 팝
	if (m_nTop < 0)
	{
#ifdef _DEBUG
		TRACE("The Stack is empty.. so cannot run the pop operation.. \n");
#endif
		return NULL;
	}

	CBaseLayer* pLayer = mp_Stack[m_nTop];
	mp_Stack[m_nTop] = NULL;
	m_nTop--;

	return pLayer;
}

CBaseLayer* CLayerManager::Top()
{
	// 스택의 상단 레이어를 반환
	if (m_nTop < 0)
	{
#ifdef _DEBUG
		TRACE("The Stack is empty.. so cannot run the top operation.. \n");
#endif
		return NULL;
	}

	return mp_Stack[m_nTop];
}

void CLayerManager::LinkLayer(PNODE pNode)
{
	// 레이어 노드를 순회하며 레이어 간의 연결을 설정
	CBaseLayer* pLayer = NULL;

	while (pNode)
	{
		if (!pLayer)
			pLayer = GetLayer(pNode->token);
		else
		{
			if (*pNode->token == '(')
				Push(pLayer);  // 현재 레이어를 스택에 푸시
			else if (*pNode->token == ')')
				Pop();  // 스택에서 레이어 팝
			else
			{
				char cMode = *pNode->token;
				char* pcName = pNode->token + 1;

				pLayer = GetLayer(pcName);

				// 레이어 연결 방식에 따라 레이어를 연결
				switch (cMode)
				{
				case '*': Top()->SetUpperUnderLayer(pLayer); break;
				case '+': Top()->SetUpperLayer(pLayer); break;
				case '-': Top()->SetUnderLayer(pLayer); break;
				}
			}
		}

		pNode = pNode->next;
	}
}

void CLayerManager::DeAllocLayer()
{
	// 모든 레이어 메모리 해제
	for (int i = 0; i < this->m_nLayerCount; i++)
		delete this->mp_aLayers[i];
}
