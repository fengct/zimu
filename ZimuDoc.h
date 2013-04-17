// ZimuDoc.h : interface of the CZimuDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_ZIMUDOC_H__3C082E25_72B6_409A_9C27_3DBF2D793019__INCLUDED_)
#define AFX_ZIMUDOC_H__3C082E25_72B6_409A_9C27_3DBF2D793019__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CZimuDoc : public CDocument
{
protected: // create from serialization only
	CZimuDoc();
	DECLARE_DYNCREATE(CZimuDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CZimuDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CZimuDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CZimuDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ZIMUDOC_H__3C082E25_72B6_409A_9C27_3DBF2D793019__INCLUDED_)
