// MontyTest.h : ������� ���� ��������� ��� ���������� MontyTest
//
#pragma once

#ifndef __AFXWIN_H__
	#error "�������� stdafx.h �� ��������� ����� ����� � PCH"
#endif

#include "resource.h"       // �������� �������


// CMontyTestApp:
// � ���������� ������� ������ ��. MontyTest.cpp
//

class CMontyTestApp : public CWinApp
{
public:
	CMontyTestApp();


// ���������������
public:
	virtual BOOL InitInstance();

// ����������
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CMontyTestApp theApp;