// FileSystem.cpp: ���������� ����� ����� ��� ����������� ����������.
//

#include "stdafx.h"
#include "FileSystem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ������������ ������ ����������

CWinApp theApp;

using namespace std;

BYTE FS[256*65536];

typedef struct _DIR_RECORD
{
    WORD wInodeID;					// ������ �����
    BYTE bNameLength;               // ������ ����� �����
    char * pcFileName;              // ��� �����
}

typedef struct _INODE_RECORD
{
    WORD wInodeID;					// ������ �����        
    WORD wFlags;					// �����
    DWORD dwSize;                   // ������ �����
    WORD pwTable[12];					// ������� ���������� �������� 
									// [0..8] ������ ���������
									// [9] ���������
									// [10] ������� ���������
									// [11] ������� ���������
} INODE_RECORD;

int uFSReadFile(
			  INODE_RECORD Inode,	// Inode �����
			  DWORD Address,		// ����� ������ �����
			  void * Dest,			// ����� ������
			  DWORD Count			// ���������� � ������
			 )						// ���������� ���������� ������� ��������� ���� ��� �������� < 0 ��� ������
{
    
}




int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// ���������������� MFC, � ����� ������ � ��������� �� ������� ��� ����
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO. �������� ��� ������ �������������� ����� ������������
		_tprintf(_T("����������� ������: ���� ��� ������������� MFC\n"));
		nRetCode = 1;
	}
	else
	{		
		memset(FS,0xFF,sizeof(FS));
	}

	return nRetCode;
}
