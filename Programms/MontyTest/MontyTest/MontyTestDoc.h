// MontyTestDoc.h : ��������� ������ CMontyTestDoc
//


#pragma once


class CMontyTestDoc : public CDocument
{
protected: // ������� ������ �� ������������
	CMontyTestDoc();
	DECLARE_DYNCREATE(CMontyTestDoc)

// ��������
public:

// ��������
public:

// ���������������
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// ����������
public:
	virtual ~CMontyTestDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ��������� ������� ����� ���������
protected:
	DECLARE_MESSAGE_MAP()
};


