// EthernetLayer.cpp: implementation of the CEthernetLayer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "pch.h"
#include "EthernetLayer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEthernetLayer::CEthernetLayer(char* pName)
    : CBaseLayer(pName)
{
    ResetHeader();
}

CEthernetLayer::~CEthernetLayer()
{
}

void CEthernetLayer::ResetHeader()
{


    memset(&m_sHeader, 0, sizeof(ETHERNET_HEADER)); // ��ü ����� 0���� �ʱ�ȭ
    m_sHeader.enet_type = htons(0x0800); // ����� ��ȯ�� ���� Ethernet Ÿ�� ����
}

unsigned char* CEthernetLayer::GetEnetDstAddress() // Ethernet Destination Address
{
    return m_sHeader.enet_dstaddr.addrs;
}

unsigned char* CEthernetLayer::GetEnetSrcAddress() // Ethernet Source Address
{
    return m_sHeader.enet_srcaddr.addrs;
}

void CEthernetLayer::SetEnetSrcAddress(unsigned char* pAddress)
{

    memcpy(m_sHeader.enet_srcaddr.addrs, pAddress, 6);
}

void CEthernetLayer::SetEnetDstAddress(unsigned char* pAddress)
{

    memcpy(m_sHeader.enet_dstaddr.addrs, pAddress, 6);
}

void CEthernetLayer::SetFrameType(unsigned short type) {
    m_sHeader.enet_type = type;
}

BOOL CEthernetLayer::Send(unsigned char* ppayload, int nlength)
{
    memcpy(m_sHeader.enet_data, ppayload, nlength);

    BOOL bSuccess = FALSE;
    
    bSuccess = mp_UnderLayer->Send((unsigned char*)&m_sHeader, nlength + ETHER_HEADER_SIZE);
    if (bSuccess) {
        TRACE("EthernetLayer: Sending %d bytes\n", nlength);
    }
    if (!bSuccess) {
        TRACE("EthernetLayer: Send failed\n");
    }
    return bSuccess;
}

BOOL CEthernetLayer::Receive(unsigned char* ppayload)
{
    PETHERNET_HEADER pFrame = (PETHERNET_HEADER)ppayload;
    CHAT_DLG_ACK_HEADER* pHeader = (CHAT_DLG_ACK_HEADER*)ppayload;
    // �ڽ��� MAC �ּ� ��������
    unsigned char* mySrcAddr = GetEnetSrcAddress();

    // ������ �ּҰ� �ڽ��� ���� �ƴϸ� discard
    if (memcmp(pFrame->enet_dstaddr.addrs, mySrcAddr, 6) != 0) {
        return FALSE; // ���

    }

    // ����� �ּҰ� �ڽ��� ���̶�� ���
    if (memcmp(pFrame->enet_srcaddr.addrs, mySrcAddr, 6) == 0) {
        return FALSE; // ���
    }
    if (pHeader->ack_type == 0x2024) {
        TRACE("RECEIVING ACK MESSAGE.\n");
        mp_aUpperLayer[0]->Receive(ppayload);
        return TRUE;
    }
    
    // Frame Type�� 0x2080�̸� ChatApp Layer�� ������ ����
    if (pFrame->enet_type == 0x2080) {
        TRACE("Receive Success\n");
        return mp_aUpperLayer[0]->Receive((unsigned char*)pFrame->enet_data);
    }
    if (pFrame->enet_type == 0x8020) {
        TRACE("Receive Success\n");
        return mp_aUpperLayer[1]->Receive((unsigned char*)pFrame->enet_data);
    }
 
    return FALSE; // �ٸ� Frame Type�� ó������ ����


}


