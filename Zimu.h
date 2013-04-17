// Zimu.h : main header file for the ZIMU application
//

#if !defined(AFX_ZIMU_H__D7CB0030_6F1A_43A0_96D9_205DBE5B54FE__INCLUDED_)
#define AFX_ZIMU_H__D7CB0030_6F1A_43A0_96D9_205DBE5B54FE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CZimuApp:
// See Zimu.cpp for the implementation of this class
//

class CZimuApp : public CWinApp
{
public:
	CZimuApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CZimuApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CZimuApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ZIMU_H__D7CB0030_6F1A_43A0_96D9_205DBE5B54FE__INCLUDED_)
