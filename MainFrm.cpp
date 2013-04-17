// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "Zimu.h"

#include "MainFrm.h"
#include "ZimuView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static UINT WM_MYSYSTRAYNOTIFY = 299; //::RegisterWindowMessage("MySysTrayNotify");

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_MESSAGE(WM_MYSYSTRAYNOTIFY, OnTrayNotification)
	ON_COMMAND(IDM_DEBUG, OnDebug)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
	Shell_NotifyIcon(NIM_DELETE, &m_notifyData);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	NOTIFYICONDATA &tnd = m_notifyData;
	tnd.cbSize=sizeof(NOTIFYICONDATA);
	tnd.hWnd=this->m_hWnd;
	tnd.uID=IDR_MAINFRAME;
	tnd.uFlags=NIF_MESSAGE|NIF_ICON|NIF_TIP;
	tnd.uCallbackMessage=WM_MYSYSTRAYNOTIFY;
	tnd.hIcon=LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME));
	strcpy(tnd.szTip,"菩提字幕");//图标提示为"测试程序"
	BOOL rt = Shell_NotifyIcon(NIM_ADD,&tnd);//向任务栏添加图标
	if(!rt)
		TRACE0("Add systray icon failed!");

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;

//	int W = ::GetSystemMetrics(SM_CXSCREEN);
//	int H = ::GetSystemMetrics(SM_CYSCREEN);
	cs.cx = 720;
	cs.cy = 560;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

LRESULT CMainFrame::OnTrayNotification(WPARAM wp, LPARAM lp)
{
	UINT uID = wp;
	UINT uMouseMsg = lp;
	if(uID == IDR_MAINFRAME){
		if(uMouseMsg == WM_LBUTTONDOWN || 
			uMouseMsg == WM_RBUTTONDOWN){
			//AfxGetApp( )->m_pMainWnd->ShowWindow(SW_SHOWNORMAL);
			AfxGetMainWnd()->ShowWindow(SW_NORMAL);
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers



void CMainFrame::OnClose() 
{
	CZimuView *view = (CZimuView*)GetActiveView();
	if(!view->OnCloseWindow())
		return;
	
	CFrameWnd::OnClose();
}


void CMainFrame::OnDebug()
{
	CDebugDlg *dlg = new CDebugDlg();
	if(!dlg->Create(IDD_DIALOG_DBG)){
		MessageBeep(MB_ICONWARNING);
		return;
	}
	dlg->ShowWindow(SW_SHOW);
	//dlg->DoModal();
}