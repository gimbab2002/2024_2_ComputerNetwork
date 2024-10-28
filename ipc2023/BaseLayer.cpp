// BaseLayer.cpp: implementation of the CBaseLayer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"  // 표준 시스템 포함 파일 (Windows 프로그램 개발 시 사용)
#include "ipc2023.h" // 프로젝트 관련 헤더 파일 (프로젝트명에 맞춰 정의됨)
#include "BaseLayer.h" // CBaseLayer 클래스 선언을 포함하는 헤더 파일
#include "pch.h" // 미리 컴파일된 헤더 파일

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__; // 현재 파일 이름을 저장하는 매크로
#define new DEBUG_NEW // 메모리 누수 검사를 위한 디버그 힙과 통합
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// 생성자: CBaseLayer 객체가 생성될 때 호출됨
CBaseLayer::CBaseLayer(char* pName)
	: m_nUpperLayerCount(0),  // 상위 레이어의 개수를 0으로 초기화
	mp_UnderLayer(NULL)       // 하위 레이어를 NULL로 초기화
{
	m_pLayerName = pName; // 레이어 이름을 인자로 받은 값으로 설정
}

// 소멸자: CBaseLayer 객체가 소멸될 때 호출됨
CBaseLayer::~CBaseLayer()
{
	// 소멸자에는 특별한 작업이 없으며, 기본 메모리 정리가 자동으로 이루어짐
}

// SetUnderUpperLayer 함수: 인자로 받은 레이어를 하위 레이어로 설정하고, 
// 현재 레이어를 해당 레이어의 상위 레이어로 설정
void CBaseLayer::SetUnderUpperLayer(CBaseLayer* pUULayer)
{
	if (!pUULayer) // 인자로 받은 포인터가 NULL인 경우
	{
#ifdef _DEBUG
		TRACE("[CBaseLayer::SetUnderUpperLayer] The variable 'pUULayer' is NULL");
#endif
		return; // NULL인 경우 작업을 중단하고 리턴
	}

	//////////////////////// fill the blank ///////////////////////////////
	// 인자로 받은 계층을 현재 계층의 하위 레이어로 설정
	// 그리고 현재 계층을 인자로 받은 계층의 상위 레이어로 설정
	this->mp_UnderLayer = pUULayer;        // 하위 레이어 설정
	pUULayer->SetUpperLayer(this);         // 상위 레이어 설정
	///////////////////////////////////////////////////////////////////////
}

// SetUpperUnderLayer 함수: 인자로 받은 레이어를 상위 레이어로 설정하고, 
// 현재 레이어를 해당 레이어의 하위 레이어로 설정
void CBaseLayer::SetUpperUnderLayer(CBaseLayer* pUULayer)
{
	if (!pUULayer) // 인자로 받은 포인터가 NULL인 경우
	{
#ifdef _DEBUG
		TRACE("[CBaseLayer::SetUpperUnderLayer] The variable 'pUULayer' is NULL");
#endif
		return; // NULL인 경우 작업을 중단하고 리턴
	}

	//////////////////////// fill the blank ///////////////////////////////
	// 인자로 받은 계층을 상위 레이어로 설정
	// 그리고 현재 계층을 해당 계층의 하위 레이어로 설정
	SetUpperLayer(pUULayer); // 상위 레이어 설정
	pUULayer->SetUnderLayer(this); // 하위 레이어 설정
	///////////////////////////////////////////////////////////////////////
}

// SetUpperLayer 함수: 인자로 받은 레이어를 상위 레이어 배열에 추가
void CBaseLayer::SetUpperLayer(CBaseLayer* pUpperLayer)
{
	if (!pUpperLayer) // 인자로 받은 포인터가 NULL인 경우
	{
#ifdef _DEBUG
		TRACE("[CBaseLayer::SetUpperLayer] The variable 'pUpperLayer' is NULL");
#endif
		return; // NULL인 경우 작업을 중단하고 리턴
	}

	// 상위 레이어 배열에 인자로 받은 레이어를 추가하고, 상위 레이어 개수를 증가시킴
	this->mp_aUpperLayer[m_nUpperLayerCount++] = pUpperLayer;
}

// SetUnderLayer 함수: 인자로 받은 레이어를 하위 레이어로 설정
void CBaseLayer::SetUnderLayer(CBaseLayer* pUnderLayer)
{
	if (!pUnderLayer) // 인자로 받은 포인터가 NULL인 경우
	{
#ifdef _DEBUG
		TRACE("[CBaseLayer::SetUnderLayer] The variable 'pUnderLayer' is NULL\n");
#endif
		return; // NULL인 경우 작업을 중단하고 리턴
	}

	// 하위 레이어를 인자로 받은 레이어로 설정
	this->mp_UnderLayer = pUnderLayer;
}

// GetUpperLayer 함수: 상위 레이어 배열에서 인덱스에 해당하는 레이어를 반환
CBaseLayer* CBaseLayer::GetUpperLayer(int nindex)
{
	// 인덱스가 유효하지 않으면 NULL을 반환
	if (nindex < 0 ||
		nindex > m_nUpperLayerCount || // 배열 범위를 벗어난 경우
		m_nUpperLayerCount < 0) // 상위 레이어가 없는 경우
	{
#ifdef _DEBUG
		TRACE("[CBaseLayer::GetUpperLayer] There is no UpperLayer in Array..\n");
#endif 
		return NULL; // 유효하지 않은 인덱스면 NULL 반환
	}

	return mp_aUpperLayer[nindex]; // 해당 인덱스의 상위 레이어를 반환
}

// GetUnderLayer 함수: 설정된 하위 레이어를 반환
CBaseLayer* CBaseLayer::GetUnderLayer()
{
	if (!mp_UnderLayer) // 하위 레이어가 설정되어 있지 않은 경우
	{
#ifdef _DEBUG
		TRACE("[CBaseLayer::GetUnderLayer] There is not a UnderLayer..\n");
#endif 
		return NULL; // NULL 반환
	}

	return mp_UnderLayer; // 설정된 하위 레이어를 반환
}

// GetLayerName 함수: 레이어의 이름을 반환
char* CBaseLayer::GetLayerName()
{
	return m_pLayerName; // 레이어 이름을 반환
}
