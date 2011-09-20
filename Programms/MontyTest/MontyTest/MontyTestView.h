// MontyTestView.h : ��������� ������ CMontyTestView
//


#pragma once


class CMontyTestView : public CView
{
protected: // ������� ������ �� ������������
	CMontyTestView();
	DECLARE_DYNCREATE(CMontyTestView)

// ��������
public:
	CMontyTestDoc* GetDocument() const;

// ��������
public:

// ���������������
public:
	virtual void OnDraw(CDC* pDC);  // �������������� ��� ��������� ����� �������������
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// ����������
public:
	virtual ~CMontyTestView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ��������� ������� ����� ���������
protected:
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // ���������� ������ � MontyTestView.cpp
inline CMontyTestDoc* CMontyTestView::GetDocument() const
   { return reinterpret_cast<CMontyTestDoc*>(m_pDocument); }
#endif

