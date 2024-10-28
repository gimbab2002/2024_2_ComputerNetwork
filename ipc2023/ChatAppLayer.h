#pragma once
// ChatAppLayer.h: interface for the CChatAppLayer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHATAPPLAYER_H__E78615DE_0F23_41A9_B814_34E2B3697EF2__INCLUDED_)
#define AFX_CHATAPPLAYER_H__E78615DE_0F23_41A9_B814_34E2B3697EF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseLayer.h"
#include "pch.h"
#include "mutex"

class CChatAppLayer
	: public CBaseLayer
{
private:
	inline void		ResetHeader();
	CObject* mp_Dlg;
	std::mutex sendMutex; // 스레드 안전성을 위한 mutex 추가


public:
	unsigned char* m_ppayload;
	int				m_length;
	BOOL			Receive(unsigned char* ppayload);
	BOOL			Send(unsigned char* ppayload, int nlength);

	static UINT		ChatThread(LPVOID pParam);

	CChatAppLayer(char* pName);
	virtual ~CChatAppLayer();

	typedef struct _CHAT_APP_HEADER {

		unsigned short  app_seq_num; // sequence number when fragmented
		unsigned short	total_length; // total length of the data
		unsigned short  app_length; //length of data
		unsigned char	app_type; // type of application data
		unsigned char	app_data[APP_DATA_SIZE]; // application data4

	} CHAT_APP_HEADER, * PCHAT_APP_HEADER;

	typedef struct _CHAT_DLG_ACK_HEADER {
		unsigned char ether_dstaddr[6];
		unsigned char ether_srcaddr[6];
		unsigned short   ack_type;     // Ack type (e.g., 1 for Ack)
	} CHAT_DLG_ACK_HEADER;

protected:
	CHAT_APP_HEADER		m_sHeader;

	enum {
		DATA_TYPE_BEGIN = 0x00,
		DATA_TYPE_CONT = 0x01,
		DATA_TYPE_END = 0x02
	};
};

#endif // !defined(AFX_CHATAPPLAYER_H__E78615DE_0F23_41A9_B814_34E2B3697EF2__INCLUDED_)










