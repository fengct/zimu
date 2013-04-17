// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__87FB2ED9_3444_4EC1_B359_A207394ED1E5__INCLUDED_)
#define AFX_MAINFRM_H__87FB2ED9_3444_4EC1_B359_A207394ED1E5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	NOTIFYICONDATA m_notifyData;
public:
	CToolBar& GetToolBar() { return m_wndToolBar; }

// Generated message map functions
protected:
	LRESULT OnTrayNotification(WPARAM wp, LPARAM lp);
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnDebug();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

// CDebugDlg 对话框
#include "ReDebug.h"
//#include "CListView0.h"
class CDebugDlg : public CDialog
{
	DECLARE_DYNAMIC(CDebugDlg)

	//CListView0* m_list;
	CRichEditCtrl *m_edit;
public:
	CDebugDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDebugDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_DBG };

	void OnCancel()
	{
		DestroyWindow();
	}

	void PostNcDestroy()
	{
		delete this;
	}
protected:
	BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__87FB2ED9_3444_4EC1_B359_A207394ED1E5__INCLUDED_)
