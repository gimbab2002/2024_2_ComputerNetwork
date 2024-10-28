#pragma once
// ChatAppLayer.h: interface for the CChatAppLayer class.
//
//////////////////////////////////////////////////////////////////////

#include "BaseLayer.h"
#include "pch.h"
#include "mutex"
#include <fstream>
#include <iostream>
#include <vector>

class CFileAppLayer
    : public CBaseLayer
{
private:
    inline void ResetHeader();
    std::mutex sendMutex; // ������ �������� ���� mutex �߰�

public:
    CFileAppLayer(char* pName);
    virtual ~CFileAppLayer();

    BOOL Send(CString filePath, CString fileName);  // ���� ���� �޼���
    BOOL Receive(unsigned char* ppayload);          // ���� ���� �޼���

    static UINT      FileThread(LPVOID pParam); // ������ �޼���

    CString m_fileName; // ������ ���� �̸�
    CString m_filePath; //���� ���

    //FileApp header
    typedef struct _FILE_APP_HEADER {

        unsigned long   total_length; // total length of the data
        unsigned short  file_seq_num; // sequence number when fragmented
        unsigned short  file_length;
        unsigned char   file_type; // type of application data
        unsigned char   file_data[FILE_DATA_SIZE]; // application data

    } FILE_APP_HEADER, * PFILE_APP_HEADER;
   
protected:
    FILE_APP_HEADER      m_sHeader;

    enum {
        DATA_TYPE_BEGIN = 0x00,
        DATA_TYPE_CONT = 0x01,
        DATA_TYPE_END = 0x02
    };
};