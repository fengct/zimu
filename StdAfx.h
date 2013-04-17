// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__C04044BD_1F39_42F4_8D14_8FAFB07E5674__INCLUDED_)
#define AFX_STDAFX_H__C04044BD_1F39_42F4_8D14_8FAFB07E5674__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"strmbase.lib")
#pragma comment(lib,"Strmiids.lib")
#pragma comment(lib,"Quartz.lib")

#pragma warning (disable : 4786)

#include <string>
using namespace std;

string TimeMS2HMS(int timeMS);
string TimeMS2Str(int timeMS);
string TimeMS2HMS2(int timeSec);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__C04044BD_1F39_42F4_8D14_8FAFB07E5674__INCLUDED_)
