// MontyTestDoc.cpp : ���������� ������ CMontyTestDoc
//

#include "stdafx.h"
#include "MontyTest.h"

#include "MontyTestDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMontyTestDoc

IMPLEMENT_DYNCREATE(CMontyTestDoc, CDocument)

BEGIN_MESSAGE_MAP(CMontyTestDoc, CDocument)
END_MESSAGE_MAP()


// ��������/����������� CMontyTestDoc

CMontyTestDoc::CMontyTestDoc()
{
	// TODO: �������� ��� ��� ������������ ������ ������������

}

CMontyTestDoc::~CMontyTestDoc()
{
}

BOOL CMontyTestDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: �������� ��� ��������� �������������
	// (��������� SDI ����� �������� ������������ ���� ��������)

	return TRUE;
}




// ������������ CMontyTestDoc

void CMontyTestDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: �������� ��� ����������
	}
	else
	{
		// TODO: �������� ��� ��������
	}
}


// ����������� CMontyTestDoc

#ifdef _DEBUG
void CMontyTestDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMontyTestDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// ������� CMontyTestDoc
