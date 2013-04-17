// ZimuView.cpp : implementation of the CZimuView class
//

#include "stdafx.h"
#include <sstream>
#include <fstream>
#include <mmsystem.h>
#include <imm.h>

#include "Zimu.h"
#include "ZimuDoc.h"
#include "ZimuView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static UINT WM_FINDREPLACE = ::RegisterWindowMessage(FINDMSGSTRING);

void log2File(char *buf, int len)
{
	FILE *stream = fopen("C:\\zimu.log", "a+");
	if(!stream){
		MessageBox(NULL, "不能打开日志文件: C:\\zimu.log", "菩提字幕", MB_OK);
	}

	unsigned bytes = fwrite(buf, 1, len, stream);
	ASSERT(bytes == len);
	fclose(stream);
}


/////////////////////////////////////////////////////////////////////////////
// CZimuView

IMPLEMENT_DYNCREATE(CZimuView, CScrollView)

BEGIN_MESSAGE_MAP(CZimuView, CScrollView)
	//{{AFX_MSG_MAP(CZimuView)
	ON_COMMAND(ID_BUTTON_PLAY, OnButtonPlay)
	ON_COMMAND(ID_BUTTON_PAUSE, OnButtonPause)
	ON_COMMAND(ID_BUTTON_STOP, OnButtonStop)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_BUTTON_SKIP, OnButtonSkip)
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_BUTTON_ADD, OnButtonAdd)
	ON_COMMAND(ID_BUTTON_DEL, OnButtonDel)
	ON_COMMAND(ID_BUTTON_MERGE, OnButtonMerge)
	ON_COMMAND(ID_BUTTON_MOVEDOWN, OnButtonMovedown)
	ON_COMMAND(ID_BUTTON_MOVEUP, OnButtonMoveup)
	ON_COMMAND(ID_BUTTON_SPLIT, OnButtonSplit)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_FILE_IMPORT, OnFileImport)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(IDM_CHECK, OnErrorCheck)
	ON_WM_SETFOCUS()
	ON_REGISTERED_MESSAGE(WM_FINDREPLACE, OnFindReplace)
	ON_COMMAND(IDM_FIND, OnFind)
	ON_COMMAND(IDM_REPLACE, OnReplace)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_APP_EXIT, OnAppExit)
	ON_WM_KILLFOCUS()
	ON_WM_KEYUP()
	ON_COMMAND(ID_BUTTON_PROOF, OnButtonProof)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_PROOF, OnUpdateButtonProof)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_COMMAND(ID_BUTTON_TIME_ADJ, OnButtonTimeAdjust)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_TIME_ADJ, OnUpdateButtonTimeAdjust)
	ON_WM_CHAR()
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_ADD, OnUpdateButtonAdd)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_DEL, OnUpdateButtonDel)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_MERGE, OnUpdateButtonMerge)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_SPLIT, OnUpdateButtonSplit)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_MOVEDOWN, OnUpdateButtonMovedown)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_MOVEUP, OnUpdateButtonMoveup)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_PAUSE, OnUpdateButtonPause)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_PLAY, OnUpdateButtonPlay)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_SKIP, OnUpdateButtonSkip)
	ON_WM_TIMER()
	ON_COMMAND(ID_EDIT_TIMEGAP, OnEditTimegap)
	ON_COMMAND(ID_FILE_FINAL_VERSION, OnFileFinalVersion)
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
	ON_MESSAGE(WM_HOTKEY, OnHotkey)
	ON_MESSAGE(MM_MCINOTIFY, OnMCINotify)
	ON_MESSAGE(WM_GRAPHNOTIFY, OnDSNotify)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, OnUpdateFileOpen)
	ON_UPDATE_COMMAND_UI(ID_FILE_IMPORT, OnUpdateFileImport)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_TIME_OFFSET, &CZimuView::OnTimeOffset)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CZimuView construction/destruction
const int CZimuView::m_bmpH = 2000;
const int CZimuView::m_arrowBoxWidth = 15;
CZimuView::CZimuView()
: m_left(2)
, m_top(0)
, m_seqW(30)
, m_startW(120)
, m_endW(120)
, m_contentW(300)
, m_titleH(0)
, m_itemH(40)
, m_vscrollbarW(20)
, m_scrollPos(0)
, m_updated(false)
, m_sizeChanged(true)
, m_orgOff(0)
, m_lastVPos(0)
, m_thumbed(false)
, m_inEdit(false)
, m_dcMeminit(false)
, m_curItem(0)
, m_prevItem(0)
, m_playingItem(0)
, m_findLine(-1)
, m_mergeSecond(0)
, m_borderPos(-1)
, m_lastPos(0)
, m_isPaused(false)
, m_changeStart(true)
, m_player(NULL)
, m_adjustBoth(false)
, m_frDlg(NULL)
, m_lastKey(0)
, m_proofReading(FALSE)
, m_hasProofed(FALSE)
, m_timeAdjusting(TRUE)
, m_timerAutoSave(100)
, m_startPos(0)
, m_endPos(0)
{
	m_vscrollbarW = GetSystemMetrics(SM_CXVSCROLL);
	int left = m_seqW + m_startW + m_endW + m_contentW;
	m_upArrow.SetRect(left, -m_arrowBoxWidth-m_itemH/2, left + m_arrowBoxWidth, -m_itemH/2);
	m_downArrow.SetRect(left, -m_itemH/2+1, left + m_arrowBoxWidth, -m_itemH/2+m_arrowBoxWidth+1);

	m_player = DShowPlayer::CreateInstance(this);
}

CZimuView::~CZimuView()
{
	BmpPageType::iterator it = m_bmpPages.begin();
	for(; it != m_bmpPages.end(); it++){
		delete *it;
	}
}

BOOL CZimuView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CZimuView drawing

void CZimuView::OnDraw(CDC* pDC)
{
	CZimuDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here

	CRect rect;
	GetClientRect(rect);
	m_scrollPos = GetScrollPos(SB_VERT);
	if(!m_dcMeminit){
		//int width = GetSystemMetrics(SM_CXSCREEN);
		//int height = GetSystemMetrics(SM_CYSCREEN);
		m_dcMeminit = true;
		//m_dcMemory.DeleteDC();
		m_dcMemory.Detach();
		m_dcMemory.CreateCompatibleDC(pDC);
		m_bmpMemory.Detach();
		m_bmpMemory.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
		m_dcMemory.SelectObject(&m_bmpMemory);
		m_dcMemory.FillSolidRect(rect, pDC->GetBkColor());
	}

	//if(m_updated){
		DoDraw(m_dcMemory);
		//m_updated = false;
	//}

	int yOff = m_scrollPos % m_itemH;
 	pDC->BitBlt(0, m_scrollPos, rect.Width(), rect.Height(), &m_dcMemory, 0, yOff, SRCCOPY); //y, scrollPos
	
//	DrawArrow(*pDC);

	//if(m_player->GetCurFile().m_state != FS_CLOSED){
	if(m_player->IsOpen()){
		const int CCWIDTH = 16;
		CPen linePen(PS_SOLID, 1, RGB(192, 192, 192));
		CPoint pt1(m_left + m_seqW + m_startW + m_endW + CCWIDTH*MAX_CHINESE_CHARACTERS, m_scrollPos), pt2(0, m_scrollPos); // 12 Chinese character at most each line.
		pt2.x = pt1.x + 2*CCWIDTH;

		pDC->SelectObject(&linePen);
		//pDC->MoveTo(pt1);
		pt1.Offset(0, rect.Height());
		//pDC->LineTo(pt1);

		pDC->MoveTo(pt2);
		pt2.Offset(0, rect.Height());
		pDC->LineTo(pt2);}
	}

void CZimuView::DoDraw(CDC &dc)
{
	CRect rect;
	//dc.GetClipBox(rect);
	GetClientRect(rect);
	rect.InflateRect(2, m_itemH);
	//dc.SelectObject(GetStockObject(NULL_BRUSH));
	//dc.Rectangle(rect);
	dc.FillSolidRect(rect, RGB(255, 255, 255));
	DrawTitle(dc);
	DisplayZimu(dc);
	DrawProgressBar(dc);
}

void CZimuView::ChangeScollSize()
{
	DWORD err = 0;
	CRect rect;
	GetClientRect(rect);
	CSize sz = GetTotalSize();
	CSize szPage = sz;
	CSize szLine = sz;
	szPage.cy = rect.Height() - m_itemH/2;
	szLine.cy = m_itemH;
	sz.cy = m_titleH + m_zimu.GetItemCount() * m_itemH;
	SetScrollSizes(MM_TEXT, sz, szPage, szLine);

	m_dcMemory.DeleteDC();
	m_bmpMemory.DeleteObject();
	CClientDC dc(this);
	m_dcMemory.CreateCompatibleDC(&dc);
	BOOL rt = m_bmpMemory.CreateCompatibleBitmap(&dc, rect.Width(), m_bmpH);
	if(!rt){
		err = GetLastError();
		HLOCAL local = NULL;
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, err, 0, (LPSTR)&local, 0, NULL);
		AfxMessageBox((LPSTR)&local);
	}
	m_dcMemory.SelectObject(&m_bmpMemory);
	m_dcMemory.FillSolidRect(0, 0, rect.Width(), min(m_bmpH, sz.cy), dc.GetBkColor());
//	m_dcMemory.OffsetViewportOrg(0, -2000);

	//SetScrollPos(SB_VERT, sz.cy-rect.Height());
//	PostMessage(WM_VSCROLL, SB_BOTTOM);
	m_scrollPos = GetScrollPos(SB_VERT);
	m_sizeChanged = true;
}

void CZimuView::Refresh()
{
	Invalidate();
	UpdateWindow();
}

void CZimuView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CSize sizeTotal;
	sizeTotal.cx = sizeTotal.cy = 100;
	SetScrollSizes(MM_TEXT, sizeTotal);

}

void CZimuView::DrawTitle(CDC &dc)
{
}

void GetTextArea(CDC &dc, const string &text, const string &target, int pos, int tabWidth, SIZE &sz, CPoint &pt)
{
	if(text.empty() || target.empty())
		return;

	int lineCnt = 0;
	int i = 0, lineBegin = 0;
	while(i < pos){
		if(text.at(i) == '\n'){
			lineCnt++;
			lineBegin = i;
		}
		i++;
	}
	SIZE sz1;
	GetTextExtentPoint32(dc.m_hDC, text.c_str() + lineBegin, pos - lineBegin, &sz1);
	GetTextExtentPoint32(dc.m_hDC, text.c_str() + pos, target.length(), &sz);
	pt.x = sz1.cx;
	pt.y = sz1.cy * lineCnt;
}

void CZimuView::DisplayZimu(CDC &dc)
{
	CRect rectSeq, rectStartTm, rectEndTm, rectZimu, rectWnd;
	rectSeq.left = m_left;
	rectSeq.top = m_top;
	rectSeq.right = m_left + m_seqW;
	rectSeq.bottom = m_top + m_itemH;
	rectStartTm.left = rectSeq.right;
	rectStartTm.top = m_top;
	rectStartTm.right = rectStartTm.left + m_startW;
	rectStartTm.bottom = rectStartTm.top + m_itemH;
	rectEndTm.left = rectStartTm.right;
	rectEndTm.top = m_top;
	rectEndTm.right = rectEndTm.left + m_endW;
	rectEndTm.bottom = m_top + m_itemH;
	rectZimu.left = rectEndTm.right;
	rectZimu.top = m_top;
	rectZimu.right = rectZimu.left + m_contentW;
	rectZimu.bottom = m_top + m_itemH;
	GetClientRect(rectWnd);
	
	char buf[8] = {0};
	CBrush brshHighLight(RGB(200, 200, 255));
	CBrush lightBrsh(RGB(240, 240, 240));//GetSysColor(COLOR_3DLIGHT));
	CBrush darkBrsh(RGB(225, 225, 225)); //GetSysColor(COLOR_3DSHADOW));
	CBrush warningBrsh(RGB(255, 255, 190));
	CBrush findTextBrsh(RGB(255, 255, 0));

	COLORREF clrTimeErr = RGB(234, 85, 32);
	COLORREF clrContentsLen = RGB(236, 135, 14);

	//for revise record display.
	CBrush reviseBrsh(RGB(255, 255, 0));
	CPen revisePen(PS_SOLID, 1, RGB(255, 0, 0));
	CPen *oldPen = NULL;
	const int reviseItemHeight = 20;
	const int reviseItemeGap = 5;
	int reviseY = 1;
	CRect rectRevise;
	rectRevise.left = rectZimu.right + 20;
	rectRevise.right = rectRevise.left + 200; 
	rectRevise.top = reviseY;
	rectRevise.bottom = rectRevise.top + reviseItemHeight;
	
	CFont newFont;
	newFont.CreatePointFont(120, "宋体");
	CFont* oldFont = dc.SelectObject(&newFont);
	dc.SetBkMode(TRANSPARENT);

	int cnt = 0, idx = 0;
	CBitmap *bmp = NULL;
	const CZimuFile::ContentsType &contents = m_zimu.GetContents();
	int reviseItemCnt = 0;

	//CZimuFile::ContentsType::const_iterator itFirst = contents.begin();
	CZimuFile::ContentsType::const_iterator it = contents.begin();
	for(; it != contents.end(); it++, cnt++){
/*		idx = m_itemH * cnt / m_bmpH;
		if(idx >= m_bmpPages.size())
			break;
		if(bmp != m_bmpPages[idx]){
			bmp = m_bmpPages[idx];
			dc.SelectObject(bmp);
			dc.SetViewportOrg(0, 0);
			dc.OffsetViewportOrg(0, idx > 0 ? (idx-1)*m_bmpH : 0);
		}*/
		if((cnt+1)*m_itemH <= m_scrollPos)
			continue;
// 		else
// 			if(itFirst == contents.begin())
// 				itFirst = it;
		if(cnt*m_itemH >= m_scrollPos + rectWnd.Height())
			break;

		const Sentence& st = *it;
		const Sentence *lastSent = NULL;
		CRect rect(m_left, rectSeq.top, rectZimu.right, rectSeq.bottom);
		if(st.m_seq == m_curItem || st.m_seq == m_mergeSecond){
			dc.FillRect(rect, &brshHighLight);
		}else if(st.m_seq % 2 == 1){
			CBrush *brsh = &lightBrsh;
			dc.FillRect(rect, brsh);
		}else{
			CBrush *brsh = &darkBrsh;
			dc.FillRect(rect, brsh);
		}

		if(st.m_seq == m_findLine+1){
			int pos = st.m_content.find(m_searchText);
			if(pos != string::npos){
				SIZE sz;
				CPoint pt;
				GetTextArea(dc, st.m_content, m_searchText.c_str(), pos, 4, sz, pt);

				CPoint pt1 = rectZimu.TopLeft(), pt2;
				pt1.Offset(pt);
				pt2 = pt1;
				pt2.Offset(sz);
				CRect rect0(pt1, pt2);

				dc.FillRect(rect0, &findTextBrsh);
			}
		}
		
		dc.SetTextColor(RGB(0, 0, 0));
		itoa(st.m_seq, buf, 10);
		dc.DrawText(buf, rectSeq, DT_CENTER | DT_VCENTER | DT_SINGLELINE );
		rectSeq.OffsetRect(0, m_itemH);

		//if((lastSent && st.m_startTime.m_msVal != lastSent->m_endTime.m_msVal) || (st.m_endTime.m_msVal - st.m_startTime.m_msVal < 1500)){
		if(st.m_mark & SM_TIME_MARK || st.m_mark & SM_TIME_MODIFIED1){
			dc.SetTextColor(clrTimeErr);
		}else{
			dc.SetTextColor(RGB(0, 0, 0));
		}
		dc.DrawText(st.m_startTime.m_dispVal.c_str(), rectStartTm, DT_CENTER | DT_VCENTER | DT_SINGLELINE );
		rectStartTm.OffsetRect(0, m_itemH);

		if(st.m_mark & SM_TIME_MARK || st.m_mark & SM_TIME_MODIFIED2){
			dc.SetTextColor(clrTimeErr);
		}else{
			dc.SetTextColor(RGB(0, 0, 0));
		}
		dc.DrawText(st.m_endTime.m_dispVal.c_str(), rectEndTm, DT_CENTER | DT_VCENTER | DT_SINGLELINE );
		rectEndTm.OffsetRect(0, m_itemH);

		//dc.Rectangle(rectZimu);
		if(st.m_mark & SM_CONTENT_LENGTH){
			dc.SetTextColor(clrContentsLen);
		}else{
			dc.SetTextColor(RGB(0, 0, 0));
		}
		dc.DrawText(st.m_content.c_str(), rectZimu, DT_LEFT);

		if(m_proofReading){
			lastSent = &st;
			reviseItemCnt += st.m_reviseRecords.size();
			rectRevise.top = max(reviseY, rectZimu.top);
			dc.SelectObject(&reviseBrsh);
			int indent = 0;
			oldPen = dc.SelectObject(&revisePen);
			ReviseRecords::const_iterator itr = st.m_reviseRecords.begin();
			for(; itr != st.m_reviseRecords.end(); itr++){
				const ReviseRecord &revise = *itr;
				int h = dc.DrawText(revise.m_contents.c_str(), rectRevise, DT_CALCRECT);
				reviseY = rectRevise.bottom + reviseItemeGap;
				//dc.Rectangle(rectRevise);
				//dc.FillRect(rectRevise, &reviseBrsh);
				dc.SetTextColor(RGB(255, 0, 0));
				dc.DrawText(revise.m_contents.c_str(), rectRevise, DT_LEFT);
				
				dc.MoveTo(rectZimu.right, rectZimu.top + rectZimu.Height()/2);
				dc.LineTo(rectRevise.left, rectRevise.top + rectRevise.Height()/2);

				indent += 5;
				rectRevise.OffsetRect(5, h + reviseItemeGap);
			}
			//dc.SelectObject(oldPen);
			rectRevise.OffsetRect(-indent, 0);
		}
		rectZimu.OffsetRect(0, m_itemH);
	}

//	ShowReviseRecords(dc, itFirst, it, reviseItemCnt);
}

void CZimuView::ShowReviseRecords(CDC &dc, CZimuFile::ContentsType::const_iterator start, CZimuFile::ContentsType::const_iterator end, int reviseItemCnt)
{

}

void CZimuView::DrawProgressBar(CDC &dc)
{
	
}

void CZimuView::RedrawItem(int seq, bool heighLight, CDC *dc)
{
	const CZimuFile::ContentsType &contents = m_zimu.GetContents();
	if(seq <= 0 || seq > contents.size())
		return;

	CClientDC clientDC(this);
	if(!dc){
		dc = &clientDC;
	}

	char buf[8] = {0};
	CBrush brshHighLight(RGB(200, 200, 255));
	CBrush lightBrsh(RGB(240, 240, 240));//GetSysColor(COLOR_3DLIGHT));
	CBrush darkBrsh(RGB(225, 225, 225)); //GetSysColor(COLOR_3DSHADOW));
	CFont newFont;
	newFont.CreatePointFont(120, "宋体");
	dc->SelectObject(&newFont);
	dc->SetBkMode(TRANSPARENT);

	m_scrollPos = GetScrollPos(SB_VERT);
	CRect rectSeq, rectStartTm, rectEndTm, rectZimu;
	rectSeq.top = rectStartTm.top = rectEndTm.top = rectZimu.top = (seq-1)*m_itemH + m_titleH + m_top;
	rectSeq.bottom = rectStartTm.bottom = rectEndTm.bottom = rectZimu.bottom = rectSeq.top + m_itemH;

	rectSeq.left = m_left;
	rectStartTm.left = rectSeq.right = rectSeq.left + m_seqW;
	rectEndTm.left = rectStartTm.right = rectStartTm.left + m_startW;
	rectZimu.left = rectEndTm.right = rectEndTm.left + m_endW;
	rectZimu.right = rectZimu.left + m_contentW;

	Sentence *st = m_zimu.GetSentence(seq);
	if(!st){
		//AfxMessageBox("");
		return;
	}
	ASSERT(st);

	CRect rect(m_left, rectSeq.top, rectZimu.right, rectSeq.bottom);
	if(heighLight){
		dc->FillRect(rect, &brshHighLight);
	}else if(seq % 2 == 1){
		dc->FillRect(rect, &lightBrsh);
	}else{
		dc->FillRect(rect, &darkBrsh);
	}

	itoa(seq, buf, 10);
	dc->DrawText(buf, rectSeq, DT_CENTER | DT_VCENTER | DT_SINGLELINE );
	dc->DrawText(st->m_startTime.m_dispVal.c_str(), rectStartTm, DT_CENTER | DT_VCENTER | DT_SINGLELINE );
	dc->DrawText(st->m_endTime.m_dispVal.c_str(), rectEndTm, DT_CENTER | DT_VCENTER | DT_SINGLELINE );
	dc->DrawText(st->m_content.c_str(), rectZimu, DT_LEFT);
}

void CZimuView::DrawArrow(CDC &dc)
{
	CPen pen(PS_SOLID, 1, RGB(170,180,205));
	CBrush arrBrush, bkBrush;
	COLORREF color1 = RGB(170,180,205);
	arrBrush.CreateSolidBrush(color1);
	COLORREF color2 = m_adjustBoth ? color1 : dc.GetBkColor();
	bkBrush.CreateSolidBrush(color2);

	dc.SelectObject(&pen);
	dc.SelectObject(&arrBrush);

	int xOff1, xOff2;
	if(m_changeStart){
		xOff1 = 0;
		xOff2 = -(m_arrowBoxWidth-1)/2;
	}else{
		xOff1 = (m_arrowBoxWidth+1)/2;
		xOff2 = 0;
	}
		
	CPoint pts[4];
	pts[0] = CPoint(m_upArrow.left+m_arrowBoxWidth/2, m_upArrow.top);
	pts[1] = CPoint(m_upArrow.left, m_upArrow.bottom);
	pts[2] = CPoint(m_upArrow.right, m_upArrow.bottom);
	pts[3] = pts[0];
	dc.SelectObject(&bkBrush);
	dc.Polygon(pts, 4);

	pts[1] = CPoint(m_upArrow.left+xOff1, m_upArrow.bottom);
	pts[2] = CPoint(m_upArrow.right+xOff2, m_upArrow.bottom);
	dc.SelectObject(&arrBrush);
	HRGN region = CreatePolygonRgn(pts, 4, ALTERNATE);
	PaintRgn(dc.m_hDC, region);

	pts[0] = CPoint(m_downArrow.left+m_arrowBoxWidth/2, m_downArrow.bottom);
	pts[1] = CPoint(m_downArrow.left, m_downArrow.top);
	pts[2] = CPoint(m_downArrow.right, m_downArrow.top);
	pts[3] = pts[0];
	dc.SelectObject(&bkBrush);
	dc.Polygon(pts, 4);

	pts[1] = CPoint(m_downArrow.left+xOff1, m_downArrow.top);
	pts[2] = CPoint(m_downArrow.right+xOff2, m_downArrow.top);
	dc.SelectObject(&arrBrush);
	region = CreatePolygonRgn(pts, 4, ALTERNATE);
	PaintRgn(dc.m_hDC, region);
}

void CZimuView::EraseArrow(CDC &dc)
{
	dc.FillSolidRect(m_upArrow, dc.GetBkColor());
	dc.FillSolidRect(m_downArrow, dc.GetBkColor());
}

int CZimuView::PickItem(CPoint point, bool input)
{
	m_scrollPos = GetScrollPos(SB_VERT);
	m_borderPos = -1;

	int x = point.x, y = point.y + m_scrollPos;
	int right = m_left + m_seqW + m_startW + m_endW + m_contentW;
	int bt = m_zimu.GetItemCount() * m_itemH + m_top;

	if(x < m_left || y < m_top || y > bt)
		return 0;

	int seq = (y - m_top - m_titleH + m_itemH)/m_itemH;
	if(seq > m_zimu.GetItemCount())
		return 0;

	if((x > right) && (x <= right + m_itemH/2)){
		if(abs((y - m_top - m_titleH + m_itemH/4) % m_itemH) < m_itemH/2){
			m_adjustBoth = true;
			m_borderPos = (y - m_top - m_titleH + m_itemH/2)/m_itemH;
		}else{
			m_adjustBoth = false;
			m_borderPos = (y - m_top - m_titleH + m_itemH)/m_itemH;
		}

		seq = 0;
	}

	if(input && x < (m_left + m_seqW + m_startW + m_endW))
		return -seq;

	return seq;
}

/////////////////////////////////////////////////////////////////////////////
// CZimuView diagnostics

#ifdef _DEBUG
void CZimuView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CZimuView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CZimuDoc* CZimuView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CZimuDoc)));
	return (CZimuDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CZimuView message handlers

void CZimuView::OnFileOpen() 
{
	ResetStatus();
	m_input.Reset();

	CString path, name;
	CFileDialog fd(TRUE, NULL, NULL, OFN_FILEMUSTEXIST, "MP3 files (*.mp3)|*.mp3|all files (*.*)|*.*||");
	if(fd.DoModal() == IDOK){
		path = fd.GetPathName();
		name = fd.GetFileTitle();

		CString title;
		SetWindowText("正在播放：" + path);

		int rt = m_player->Open(path.GetBuffer(path.GetLength()));
		if(rt){
			string msg;
			m_player->GetLastError(msg);
			MessageBox(msg.c_str());
		}
//		rt = m_player.Play();
//		if(rt){
//			string msg;
//			m_player.GetLastError(msg);
//			MessageBox(msg.c_str());
//		}
		CString m_nameLen = "Now Playing: " + name + " total: " + TimeMS2Str(m_player->GetMediaDuration()).c_str();
		AfxGetMainWnd()->SetWindowText(m_nameLen);
		CMainFrame *frame = (CMainFrame*)AfxGetMainWnd();
		
		m_zimu.Open(path, true);
		if(m_zimu.GetItemCount() == 0){ //new file
			CStartPosDlg dlg;
			if(dlg.DoModal() == IDOK){
				m_startPos = dlg.GetOffset();
			}
		}
		m_curItem = 1;
		if (m_curItem > m_zimu.GetItemCount())
			m_curItem = m_zimu.GetItemCount();

		m_timerAutoSave = SetTimer(100, 60*1000, NULL);

		ChangeScollSize();
//		m_updated = true;
		Invalidate();
		UpdateWindow();
		return;
	}else{
		return;
	}
}


void CZimuView::OnFileImport() 
{
	ResetStatus();
	m_input.Reset();

	CString path, name;
	CFileDialog fd(TRUE, NULL, NULL, OFN_FILEMUSTEXIST, "srt files (*.srt)|*.srt|"); //, "*.mp3|*.*");
	if(fd.DoModal() == IDOK){
		path = fd.GetPathName();
		name = fd.GetFileTitle();

		if(path.Find(".srt") == -1)
			return;
		
		if(m_zimu.Open(path.GetBuffer(path.GetLength()), false) < 0){
			ChangeScollSize();
			Invalidate();
			UpdateWindow();
			return;
		}

		m_timerAutoSave = SetTimer(100, 60*1000, NULL);
		if(m_zimu.GetItemCount() > 0){
			m_startPos = m_zimu.GetSentence(1)->m_startTime.m_msVal / 1000;
			m_endPos = m_zimu.GetSentence(m_zimu.GetItemCount())->m_endTime.m_msVal / 1000;
			//m_player->SetRange(m_startPos*1000, m_endPos*1000);
		}

		string mp3File = m_zimu.GetSourcePath();
		int rt = m_player->Open(mp3File);
		if(rt){ //can't find file.
			CFileDialog fd2(TRUE, NULL, NULL, OFN_FILEMUSTEXIST, "MP3 files (*.mp3)|*.mp3|");
			if(fd2.DoModal() == IDOK){
				path = fd2.GetPathName();
				mp3File = path.GetBuffer(path.GetLength());
				rt = m_player->Open(mp3File);
				if (rt==0)
					m_player->Pause();
			}
		}
		if(rt){
			string msg;
			m_player->GetLastError(msg);
			MessageBox(msg.c_str());
		}
		m_curItem = 1;
		if (m_curItem > m_zimu.GetItemCount())
			m_curItem = m_zimu.GetItemCount();
		
		AfxGetMainWnd()->SetWindowText("Import file: " + name);

//		m_updated = true;
		ChangeScollSize();
		Invalidate();
		UpdateWindow();
	}	
}

void CZimuView::OnFileSave() 
{
	m_updated = false;
	if(!m_zimu.IsUpdated())
		return;
	
	if(!m_zimu.HasRevise()){
		if(m_hasProofed){
// 			int rt = MessageBox("文件新增了校对记录，需要存储为校订文件类型(.rev)文件，否则将会丢失校对记录，另存为校订文件类型吗？", 
// 				"菩提字幕", MB_YESNO|MB_ICONQUESTION);
// 			if(rt == IDYES){
				m_zimu.SaveAsReviseFile();
// 				return;
// 			}else{
// 				m_zimu.SaveAsSrtFile();
// 				return;
// 			}
		}
	}else if(m_zimu.HasRevise()){
		m_zimu.SaveAsReviseFile();
		return;
	}
	m_zimu.Save();
}

void CZimuView::OnFileSaveAs() 
{
	CString path, name;
	string defaultName = m_zimu.GetDestPath();
	int pos = defaultName.find_last_of(".");
	string baseName = defaultName.substr(0, pos);
	CFileDialog fd(FALSE, NULL, baseName.c_str(), OFN_FILEMUSTEXIST | OFN_EXPLORER, "srt files (*.srt)|*.srt|text files (*.txt)|*.txt|lrc files (*.lrc)|*.lrc|all files (*.*)|*.*||"); //, "*.mp3|*.*");
	if(fd.DoModal() == IDOK){
		path = fd.GetPathName();
		int idx = fd.m_ofn.nFilterIndex;
		CString ext = fd.GetFileExt();
		if(idx == 1){
			if(ext == "")
				path += ".srt";
			m_zimu.Save(path);
		}else if(idx == 2){
			if(ext == "")
				path += ".txt";
			m_zimu.SaveToText(path);
		}else if(idx == 3){
			if(ext == "")
				path += ".lrc";
			m_zimu.SaveToLrc(path);
		}else{
			m_zimu.Save(path);
		}
	}
		
// 		if(path.Find(".txt") == path.GetLength()-4)
// 			m_zimu.SaveToText(path.GetBuffer(path.GetLength()));
// 		else if(path.Find(".srt") == path.GetLength()-4)
// 			m_zimu.SaveAsSrtFile(path.GetBuffer(path.GetLength()));
// 		else if(path.Find(".rev") == path.GetLength()-4)
// 			m_zimu.SaveAsReviseFile(path.GetBuffer(path.GetLength()));

}

void CZimuView::OnButtonPlay() 
{
	DebugOut("Play Command" << endl);
	//m_player->Speed(1.0);

	if(m_player->IsPaused())
	{
		m_player->Resume();
		return;
	}

	unsigned long pos =	m_player->GetCurrentPos();
	if(pos == 0){
		pos = m_startPos * 1000;
	}

	Sentence *st = m_zimu.GetSentence(m_playingItem);
	if(!st){
		m_playingItem = 1;
		st = m_zimu.GetSentence(m_playingItem);
	}

	if(st){
		//ASSERT(pos >= st->m_startTime.m_msVal && pos <= st->m_endTime.m_msVal);
		m_player->Seek(st->m_startTime.m_msVal);
		m_player->Play(st->m_endTime.m_msVal - st->m_startTime.m_msVal);
		m_playingItem = m_curItem;
	}else{ //no record, play from start.
		m_player->Seek(0);
		m_player->Play(m_player->GetMediaDuration());
		m_playingItem = m_curItem;
	}
}

void CZimuView::OnButtonPause() 
{
	DebugOut("Pause Command" << endl);
	m_player->Pause();
}

void CZimuView::OnButtonStop() 
{
	DebugOut("Stop Command" << endl);
	m_player->Stop();
}

void CZimuView::OnButtonSkip() 
{
	DebugOut("Speed Command" << endl);
// 	unsigned long pos = 0;
// 	m_player->GetCurPos(pos);
// 	pos += 3000;
// 	m_player->Play(pos, 0, true);
//	m_player->Speed(2.0);
}

void CZimuView::ScrollToPos(int seq)
{
	CRect rect;
	GetClientRect(rect);

	m_scrollPos = GetScrollPos(SB_VERT);
	if(seq * m_itemH + m_top > m_scrollPos + rect.Height()){
		m_scrollPos = (m_itemH * seq + m_top) - rect.Height()/2;
		SetScrollPos(SB_VERT, m_scrollPos);
	}else if((seq-1) * m_itemH + m_top < m_scrollPos){
		m_scrollPos = (m_itemH * seq + m_top) - rect.Height()/2;
		SetScrollPos(SB_VERT, m_scrollPos);
	}else{
		;
	}
// 	
// 	int spos = (seq-1)*m_itemH+m_top;
// 	spos -= spos % clientRect.Height();
// 	SetScrollPos(SB_VERT, spos);

	Invalidate(FALSE);
	UpdateWindow();
}

void CZimuView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	m_lastKey = nChar;

	DebugOut("key down " << nChar << endl);
	SHORT ksCtrl = GetKeyState(VK_CONTROL);
	SHORT ksCaps = GetKeyState(VK_CAPITAL);
	if(nChar == VK_ADD || nChar == 0xBB){
		OnButtonAdd();
	}else if(nChar == VK_SUBTRACT || nChar == VK_DELETE || nChar == 0xBD){
		OnButtonDel();
	}else if(nChar == VK_SPACE){
		if(m_player->IsOpen()){
			m_isPaused = m_player->IsPaused();
			if(m_isPaused)
				m_player->Resume();
			else
				m_player->Pause();

			m_isPaused = m_player->IsPaused();
		}
	}else if(nChar == VK_HOME){
		if(ksCtrl & 0x8000)
			PostMessage(WM_VSCROLL, SB_TOP);
	}else if(nChar == VK_END){
		if(ksCtrl & 0x8000)
			PostMessage(WM_VSCROLL, SB_BOTTOM);
	}else if(nChar == VK_PRIOR){
		//if(ksCtrl & 0x8000)
			PostMessage(WM_VSCROLL, SB_PAGEUP);
	}else if(nChar == VK_NEXT){
		//if(ksCtrl & 0x8000)
			PostMessage(WM_VSCROLL, SB_PAGEDOWN);
	}else if(nChar == VK_UP){
		OnButtonMoveup();
	}else if(nChar == VK_DOWN){
		OnButtonMovedown();
	}else if(nChar == VK_F3){
		m_findLine = m_zimu.NextMarkSentence(SM_TIME_ERROR, m_findLine+1);
		if(m_findLine > 0){
			m_curItem = m_findLine;
			ScrollToPos(m_findLine);
		}else{
			m_findLine = 0;
			MessageBox("已经找到文件末尾！");
		}
	}else if(nChar == VK_F8){
		m_findLine = m_zimu.NextTimeGap(m_findLine+1);
		if(m_findLine > 0){
			m_curItem = m_findLine-1;
			ScrollToPos(m_findLine-1);
		}else{
			m_findLine = 0;
			MessageBox("已经找到文件末尾！");
		}
	}else if(nChar == VK_F4){
		m_player->Stop();
	}else if(nChar == VK_RETURN){
		StartEdit(m_curItem);
	}else if(nChar == VK_ESCAPE){
//		Shell_NotifyIcon(3, NULL);
		if(m_player->IsOpen()){
			if(m_player->IsPlaying()){
				m_player->Pause();
				m_isPaused = true;
			}/*else if(m_player->IsPaused()){
				m_player->Resume();
				m_isPaused = false;
			}*/
		}
		AfxGetMainWnd()->ShowWindow(SW_HIDE);
	}else if(nChar == VK_LEFT){
 		m_zimu.ClearTimeModifyMark();

		BOOL end = ksCaps & 0x01;
		BOOL curOnly = ksCtrl & 0x8000;
		if(end)
			curOnly = TRUE;
		Sentence *st1 = curOnly ? NULL : m_zimu.GetSentence(m_curItem-1);
		Sentence *st2 = m_zimu.GetSentence(m_curItem);
		DWORD mask1 = 0, mask2 = 0;
		if(!curOnly && st1){
			ModifyTime(m_curItem-1, false, -TIME_DELTA);
			st1->m_mark |= SM_TIME_MODIFIED2;
			mask1 = TAM_END;
		}
		mask2 = end ? TAM_END : TAM_START;

		if(end){
			st2->m_mark |= SM_TIME_MODIFIED2;
			mask2 = TAM_END;
			ModifyTime(m_curItem, false, -TIME_DELTA);
		}else{
			st2->m_mark |= SM_TIME_MODIFIED1;
			mask2 = TAM_START;
			ModifyTime(m_curItem, true, -TIME_DELTA);
		}
		CTimeAdjust *ta = new CTimeAdjust(*this, m_zimu, m_curItem-1, m_curItem, mask1, mask2, -TIME_DELTA);
		//ta->Execute();
		OnCommand(ta);
		DoDraw(m_dcMemory);
		Invalidate(FALSE);
		UpdateWindow();
	}else if(nChar == VK_RIGHT){
 		m_zimu.ClearTimeModifyMark();

		BOOL end = ksCaps & 0x01;
		BOOL curOnly = ksCtrl & 0x8000;
		if(end)
			curOnly = TRUE;
		Sentence *st1 = curOnly ? NULL : m_zimu.GetSentence(m_curItem-1);
		Sentence *st2 = m_zimu.GetSentence(m_curItem);
		DWORD mask1 = 0, mask2 = 0;

		if(end){
			st2->m_mark |= SM_TIME_MODIFIED2;
			mask2 = TAM_END;
			ModifyTime(m_curItem, false, TIME_DELTA);
		}else{
			st2->m_mark |= SM_TIME_MODIFIED1;
			mask2 = TAM_START;
			ModifyTime(m_curItem, true, TIME_DELTA);
		}
		
		if(!curOnly && st1){
			ModifyTime(m_curItem-1, false, TIME_DELTA);
			st1->m_mark |= SM_TIME_MODIFIED2;
			mask1 = TAM_END;
		}
		mask2 = end ? TAM_END : TAM_START;
		CTimeAdjust *ta = new CTimeAdjust(*this, m_zimu, m_curItem-1, m_curItem, mask1, mask2, TIME_DELTA);
		//ta->Execute();
		OnCommand(ta);
		DoDraw(m_dcMemory);
		Invalidate(FALSE);
		UpdateWindow();
	}
	
	CScrollView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CZimuView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CScrollView::OnKeyUp(nChar, nRepCnt, nFlags);
	DebugOut("key up " << nChar << endl);
//	char buf[24] = {0};
//	itoa(nChar, buf, 16);
//	AfxGetMainWnd()->SetWindowText(buf);
	if(m_lastKey == VK_PROCESSKEY && nChar == VK_SPACE){
		m_isPaused = m_player->IsPaused();
		if(m_isPaused)
			m_player->Resume();
		else
			m_player->Pause();

		m_isPaused = !m_isPaused;
	}
}

void CZimuView::AddItem()
{
	if(!m_player->IsOpen())
		return;

	Sentence st, *last;
	last = m_zimu.GetSentence(m_zimu.GetItemCount());
	if(last){
		m_lastPos = m_player->GetCurrentPos();
		if(m_lastPos <= last->m_startTime.m_msVal){
			MessageBeep(-1);
			return;
		}
		last->m_endTime.SetTime(m_lastPos);
		m_zimu.UpdateSentence(*last);
	}

	//add to tail.
	st.m_seq = m_zimu.GetItemCount() + 1;
	st.m_startTime.SetTime(m_lastPos);
	st.m_endTime.SetTime(m_player->GetMediaDuration()); //to tail
	st.m_content.assign(Sentence::m_initialText);
	st.m_mark = 0;

	m_zimu.AppendSentence(st);


//	ChangeScollSize();

	//RedrawItem(st.m_seq, false, NULL);
	m_updated = true;
//	Invalidate();
//	UpdateWindow();
}

void CZimuView::ModifyTime(int seq, bool startTime, int delta)
{
	m_zimu.ModifyTime(seq, startTime, delta);
	m_updated = true;
}

void CZimuView::OnPlayResult(int result)
{
	OnMCINotify(MCI_NOTIFY_SUCCESSFUL, 0);
}

LRESULT CZimuView::OnDSNotify(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

void CZimuView::OnPlay(int code)
{
	//OnMCINotify(MCI_NOTIFY_SUCCESSFUL, 0);
	PostMessage(MM_MCINOTIFY, MCI_NOTIFY_SUCCESSFUL);
}

LRESULT CZimuView::OnMCINotify(WPARAM wParam, LPARAM lParam)
{
	char buf[256] = {0};
	sprintf(buf, "OnMCINotify, %d\n", wParam);
	OutputDebugString(buf);
	//log2File(buf, strlen(buf));

	if(wParam == MCI_NOTIFY_SUCCESSFUL){
		if(m_playingItem != m_zimu.GetItemCount()){
			if(!m_inEdit)
				m_playingItem++;
			Sentence *st = m_zimu.GetSentence(m_playingItem);
			Sentence *nextSt = m_zimu.GetSentence(m_playingItem+1);
			if(st){
				DebugOut("Play Command when edit." << endl);
				//m_player->Play(st->m_startTime.m_msVal, nextSt ? nextSt->m_startTime.m_msVal : st->m_endTime.m_msVal, m_inEdit);
				if(m_inEdit)
					m_player->Seek(st->m_startTime.m_msVal);
				m_player->Play(st->m_endTime.m_msVal - st->m_startTime.m_msVal);
			}
			m_curItem = m_playingItem;
			AdjustScrollPos();

			Invalidate(FALSE);
			UpdateWindow();
		}
	}
	return 0;
}

void CZimuView::OnButtonAdd() 
{
	if(!m_player->IsOpen())
		return;

	unsigned long pos, len;
	pos = m_player->GetCurrentPos();
	len = m_player->GetMediaDuration();
	if(m_endPos != 0)
		len = m_endPos * 1000;

	CAddRecordCommand *command = new CAddRecordCommand(*this, m_zimu, pos, len);
	command->Execute();

	OnCommand(command);
	m_curItem = m_zimu.GetItemCount();

//	m_scrollPos = GetScrollPos(SB_VERT);
}

void CZimuView::OnButtonDel() 
{
	if(!m_player->IsOpen())
		return;
	if(m_curItem == 0)
		return;

	CDelRecordCommand *command = new CDelRecordCommand(*this, m_zimu, m_curItem);
	command->Execute();

	if(m_curItem > m_zimu.GetItemCount())
		m_curItem = m_zimu.GetItemCount();
	
	if(m_player->IsPlaying()){
		Sentence *st = m_zimu.GetSentence(m_curItem);
		if(st)
		{
			m_player->Seek(st->m_startTime.m_msVal);
			m_player->Play(st->m_endTime.m_msVal - st->m_startTime.m_msVal);
			m_playingItem = m_curItem;
		}
	}

	OnCommand(command);
}

void CZimuView::OnButtonMerge() 
{
	if(m_curItem == 0 || m_mergeSecond == 0){
		AfxMessageBox("请选择相邻两句话合并。");
		return;
	}

	if(m_curItem > m_mergeSecond){
		m_curItem = m_mergeSecond;
		m_mergeSecond = m_curItem + 1;
	}
	
	CMergeCommand *command = new CMergeCommand(*this, m_zimu, m_curItem, m_mergeSecond);
	command->Execute();

	m_mergeSecond = 0;

	OnCommand(command);
}

void CZimuView::OnButtonSplit() 
{
	if(!m_player->IsOpen())
		return;

	if(m_curItem == 0)
		return;

	unsigned long pos;
	if(!m_player->IsOpen()){
			AfxMessageBox("文件未打开或已关闭，只能在播放或暂停状态下拆分！");
		return;
	}
	pos = m_player->GetCurrentPos();

	CSplitCommand *command = new CSplitCommand(*this, m_zimu, m_curItem, pos);
	command->Execute();

	OnCommand(command);
}

void CZimuView::OnButtonMovedown() 
{
	m_curItem++;
	if(m_curItem > m_zimu.GetItemCount())
		m_curItem = m_zimu.GetItemCount();

	Sentence *st = m_zimu.GetSentence(m_curItem);
	if(st){
		m_player->Seek(st->m_startTime.m_msVal);
		m_player->Play(st->m_endTime.m_msVal - st->m_startTime.m_msVal);
		m_playingItem = m_curItem;
		AdjustScrollPos();
	}

	Invalidate();
	UpdateWindow();
}

void CZimuView::OnButtonMoveup() 
{
	m_curItem--;
	if(m_curItem < 1)
		m_curItem = 1;

	Sentence *st = m_zimu.GetSentence(m_curItem);
	if(st){
		m_player->Seek(st->m_startTime.m_msVal);
		m_player->Play(st->m_endTime.m_msVal - st->m_startTime.m_msVal);
		m_playingItem = m_curItem;
		AdjustScrollPos();
	}

	Invalidate();
	UpdateWindow();
}

void CZimuView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SetFocus();

	bool update = false;
	int seq = PickItem(point, false);
	if(seq != 0){
		m_mergeSecond = 0;

		SHORT ks = GetKeyState(VK_LCONTROL);
		if(ks & 0x8000){
			if(m_curItem == 0){
				m_prevItem = m_curItem;
				m_curItem = seq;
			}else{
				if(abs(seq - m_curItem) == 1){
					RedrawItem(m_mergeSecond, false, &m_dcMemory);
					m_mergeSecond = seq;
				}else{
					RedrawItem(m_curItem, false, &m_dcMemory);
					m_curItem = seq;
					m_mergeSecond = 0;
				}
			}
		}else{
			if(m_curItem != seq){
				m_prevItem = m_curItem;
				m_curItem = seq;
				RedrawItem(m_prevItem, false, &m_dcMemory);
				RedrawItem(m_curItem, true, &m_dcMemory);
			}
		}

		EraseArrow(m_dcMemory);
		int yOff = (m_upArrow.bottom == m_itemH/2) ? 0 : m_upArrow.bottom;
		if(m_adjustBoth)
			yOff += m_itemH;
		m_upArrow.OffsetRect(0, (abs(seq-1))*m_itemH - yOff);
		m_downArrow.OffsetRect(0, (abs(seq-1))*m_itemH - yOff);
		
		update = true;
	}

	m_inEdit = false;
	OnAdjustTime(point, 200, m_borderPos, true);
	//if(update){
		Invalidate(FALSE);
		UpdateWindow();
	//}
	CScrollView::OnLButtonDown(nFlags, point);
}

void CZimuView::OnAdjustTime(CPoint pt, int val, int item, bool left)
{
	m_changeStart = left;

	const int changeMS = 200;
	m_scrollPos = GetScrollPos(SB_VERT);
	pt.Offset(0, m_scrollPos);
	if(PtInRect(m_upArrow, pt)){
		if(m_adjustBoth){
			ModifyTime(item, false, -changeMS);
			ModifyTime(item+1, true, -changeMS);
		}else{
			ModifyTime(item, left, -changeMS);
		}
	}else if(PtInRect(m_downArrow, pt)){
		if(m_adjustBoth){
			ModifyTime(item+1, true, changeMS);
			ModifyTime(item, false, changeMS);
		}else{
			ModifyTime(item, left, changeMS);
		}
	}
	
	int yPos = m_borderPos * m_itemH - m_itemH / 2;
	if(m_adjustBoth)
		yPos += m_itemH / 2;
	m_upArrow.top = yPos - m_arrowBoxWidth;
	m_upArrow.bottom = yPos;
	m_upArrow.OffsetRect(0, -1);
	m_downArrow.top = yPos;
	m_downArrow.bottom = yPos + m_arrowBoxWidth;
	m_downArrow.OffsetRect(0, 1);
	
//	m_updated = true;
}

void CZimuView::StartEdit(int seq)
{
	if(seq == 0)
		return;

	//if(seq < 0){
		Sentence *st = m_zimu.GetSentence(abs(seq));
		if(!st)
			return;
		
		m_playingItem = abs(seq);
		Sentence *nextSt = m_zimu.GetSentence(m_playingItem+1);
		//m_player->Play(st->m_startTime.m_msVal, nextSt ? nextSt->m_startTime.m_msVal : st->m_endTime.m_msVal, true);
		m_player->Seek(st->m_startTime.m_msVal);
		m_player->Play(st->m_endTime.m_msVal - st->m_startTime.m_msVal);
		m_playingItem = m_curItem;
	//}
	m_prevItem = m_curItem;
	m_curItem = abs(seq);
	m_mergeSecond = 0;
	if(seq < 0)
		return;

	m_scrollPos = GetScrollPos(SB_VERT);
	int left = m_left + m_seqW + m_startW + m_endW;
	CRect rect(left, m_top+m_titleH+(seq-1)*m_itemH, left+m_contentW, m_top+seq*m_itemH);
	rect.OffsetRect(0, -m_scrollPos);

	static bool s_bCreated = false;
	if(!s_bCreated){
		s_bCreated = true;
		m_input.m_view = this;
		m_input.Create(ES_MULTILINE | ES_AUTOVSCROLL, rect, this, 0);
		m_input.Initial();
		m_input.MySubclassWindow(m_input.m_hWnd);
		m_input.LineScroll(3);
		m_input.FmtLines(TRUE);
	}

	m_playingItem = m_curItem;
	st = m_zimu.GetSentence(m_curItem);
	ASSERT(st);
	DebugOut("start edit. " << st->m_content << endl);

	m_input.m_sentence = st;
	
	if(st->m_content != Sentence::m_initialText)
		m_input.SetWindowText(st->m_content.c_str());
	else
		m_input.SetWindowText("");
	m_input.MoveWindow(rect);
	m_input.OnChange();
	m_input.ShowWindow(SW_SHOW);
	//m_edit.m_pos = point;
	m_input.SetSel(st->m_content.size(), st->m_content.size(), FALSE);
	//m_edit.ShowCaret();
	m_input.SetFocus();
	m_inEdit = true;
}

void CZimuView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	int seq = PickItem(point, true);
	
	StartEdit(seq);
	CScrollView::OnLButtonDblClk(nFlags, point);
}

/////////////////////////////////////////////////////////////////////////////
// CZimuEdit
const int ID_COPY = 100;
const int ID_CUT = 101;
const int ID_PASTE = 102;
const int ID_SELALL = 103;
const int ID_UNDO = 104;
const int ID_REDO = 105;
const int ID_CHECK = 106;
const int ID_SEARCH = 110;
const int ID_REPLACE = 111;

BEGIN_MESSAGE_MAP(CZimuEdit, CEdit)
	//{{AFX_MSG_MAP(CZimuEdit)
	ON_WM_KILLFOCUS()
	ON_CONTROL_REFLECT(EN_CHANGE, OnChange)
	ON_MESSAGE(WM_HOTKEY, OnHotKey)
	ON_WM_KEYDOWN()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CZimuEdit::CZimuEdit()
: m_sentence(NULL)
, m_view(NULL)
, m_oldWndProc(NULL)
{
}

CZimuEdit::~CZimuEdit()
{
}

void CZimuEdit::Initial()
{
	if(m_sentence){
		CString tmp(m_sentence->m_content.c_str());
		m_history.push_back(tmp);
	}
}

void CZimuEdit::ReplaceSelection(LPTSTR sel)
{
	int selStart, selEnd;
	CString text;

	GetWindowText(text);
	GetSel(selStart, selEnd);
	char *buf = text.GetBuffer(text.GetLength());
	int lenSel = strlen(sel);
	int len = text.GetLength() - (selEnd-selStart) + lenSel;
	char *buf2 = new char[len+1];
	memcpy(buf2, buf, selStart);
	memcpy(buf2+selStart, sel, lenSel);
	memcpy(buf2+selStart+lenSel, buf+selEnd, text.GetLength() - selEnd);
	buf2[len] = (TCHAR)0;
	SetWindowText(buf2);

	SetSel(selStart+lenSel, selStart+lenSel, TRUE);
	delete [] buf2;

	OnChange();
}

LRESULT CZimuEdit::OnHotKey(WPARAM wParam, LPARAM lParam)
{
//	MessageBeep(-1);
	int selStart, selEnd;
	CString text;
	HGLOBAL hGlobal;
	LPTSTR strGlobal;
	int selLen;

	BOOL rt = OpenClipboard();
	if(!rt){
		MessageBox("can't open clipboard!");
		return 0;
	}

	GetWindowText(text);
	GetSel(selStart, selEnd);
	char *buf = text.GetBuffer(text.GetLength());
	
	if(wParam == ID_COPY){ //COPY
		EmptyClipboard();
		selLen = selEnd - selStart;
		if(!selLen){
			CloseClipboard();
			return 0;
		}
		hGlobal = GlobalAlloc(GMEM_MOVEABLE, selLen+1);
		if(hGlobal == NULL){
			CloseClipboard();
			return 0;
		}

		strGlobal = (LPTSTR)GlobalLock(hGlobal);
		memcpy(strGlobal, buf+selStart, selLen);
		strGlobal[selLen] = '\0';
		GlobalUnlock(hGlobal);

		SetClipboardData(CF_TEXT, strGlobal);
	}else if(wParam == ID_CUT){ //CUT
		EmptyClipboard();
		selLen = selEnd - selStart;
		if(!selLen){
			CloseClipboard();
			return 0;
		}
		hGlobal = GlobalAlloc(GMEM_MOVEABLE, selLen+1);
		if(hGlobal == NULL){
			CloseClipboard();
			return 0;
		}

		strGlobal = (LPTSTR)GlobalLock(hGlobal);
		memcpy(strGlobal, buf+selStart, selLen);
		strGlobal[selLen] = '\0';
		GlobalUnlock(hGlobal);

		SetClipboardData(CF_TEXT, strGlobal);

		ReplaceSelection("");
	}else if(wParam == ID_PASTE){ //PASTE
		if(!IsClipboardFormatAvailable(CF_TEXT))
			return 0;

		hGlobal = GetClipboardData(CF_TEXT);
		if(hGlobal){
			strGlobal = (LPTSTR)GlobalLock(hGlobal);
			if(strGlobal){
				ReplaceSelection(strGlobal);
				GlobalUnlock(hGlobal);
			}
		}
	}else if(wParam == ID_SELALL){
		SetSel(0, text.GetLength(), FALSE);
	}else if(wParam == ID_UNDO){
		if(!m_history.empty()){
			CString &tmp = m_history.back();
			m_future.push_front(tmp);
			m_history.pop_back();

			if(m_history.size())
				SetWindowText(m_history.back());
		}
	}else if(wParam == ID_REDO){
		if(!m_future.empty()){
			CString &tmp = m_future.front();
			SetWindowText(tmp);
			m_history.push_back(tmp);
			m_future.pop_front();
		}
	}

	CloseClipboard();
	return 0;
}

void CZimuEdit::OnSetFocus(CWnd* pOldWnd) 
{
	CEdit::OnSetFocus(pOldWnd);
	
	RegisterHotKey(m_hWnd, ID_COPY, MOD_CONTROL, 'C');
	RegisterHotKey(m_hWnd, ID_CUT, MOD_CONTROL, 'X');
	RegisterHotKey(m_hWnd, ID_PASTE, MOD_CONTROL, 'V');
	RegisterHotKey(m_hWnd, ID_SELALL, MOD_CONTROL, 'A');
	RegisterHotKey(m_hWnd, ID_UNDO, MOD_CONTROL, 'Z');
	RegisterHotKey(m_hWnd, ID_REDO, MOD_CONTROL, 'Y');
}

/////////////////////////////////////////////////////////////////////////////
// CZimuEdit message handlers

void CZimuEdit::OnKillFocus(CWnd* pNewWnd) 
{
	CEdit::OnKillFocus(pNewWnd);
	ShowWindow(SW_HIDE);
	
	if(!m_sentence)
		return;

	CString text;
	GetWindowText(text);
	string tmp;
	tmp.assign(text.GetBuffer(text.GetLength()));
	if(tmp == "")
		tmp.assign(Sentence::m_initialText);

	if(tmp != m_sentence->m_content){
		if(m_view->IsProofReading()){
			ReviseRecord revise;
			revise.m_time = time(NULL);
			revise.m_contents = m_sentence->m_content;
			m_sentence->m_reviseRecords.push_back(revise);

			m_view->SetProofed(TRUE);
		}

		//m_sentence->m_content.assign(tmp);
		m_view->OnInputContent(tmp, *m_sentence);
		m_view->SetUpdated(true);
	}

   int lineCnt = GetLineCount();
   while (lineCnt)
   {
      int len = LineLength(lineCnt);
      if(len > MAX_CHINESE_CHARACTERS*2)
		  m_sentence->m_mark |= SM_CONTENT_LENGTH;
	  lineCnt--;
   }

	UnregisterHotKey(m_hWnd, ID_COPY);
	UnregisterHotKey(m_hWnd, ID_CUT);
	UnregisterHotKey(m_hWnd, ID_PASTE);
	UnregisterHotKey(m_hWnd, ID_SELALL);
	UnregisterHotKey(m_hWnd, ID_UNDO);
	UnregisterHotKey(m_hWnd, ID_REDO);

}

void CZimuEdit::OnChange() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CEdit::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	CString tmp;
	GetWindowText(tmp);

	int lineCnt = GetLineCount();
	if(lineCnt == 3){
		::CTempBuffer buff;
		std::string lines[3];
		int len;
		for(int i = 0; i < lineCnt; i++){
			len = GetLine(i, buff.Data(), buff.Size());
			buff.Data()[len] = 0;
			lines[i].assign(buff.Data());
		}
		int posStart, posEnd;
		GetSel(posStart, posEnd);
		if(posStart == LineLength(0)+2){ //the first line
			lines[1].append(lines[2]);
		}else{
			lines[0].append(lines[1]);
			std::swap(lines[1], lines[2]);
		}
		
		std::string text(lines[0]);
		text += "\r\n";
		text += lines[1];

		SetWindowText(text.c_str());
	}

	//if(tmp != m_history.back()){
		m_history.push_back(tmp);
		//m_view->SetUpdated(true);
	//}

	m_future.clear();
}

void CZimuEdit::Reset()
{
	m_sentence = NULL;
	m_history.clear();
	m_future.clear();
}

void CZimuEdit::MySubclassWindow(HWND hWnd)
{
	if(m_oldWndProc)
		return;

	m_oldWndProc = (WNDPROC)::SetWindowLong(hWnd, GWL_WNDPROC,
		(DWORD)AfxGetAfxWndProc());
}

void CZimuView::OnSize(UINT nType, int cx, int cy) 
{
	ChangeScollSize();
	CScrollView::OnSize(nType, cx, cy);
}

BOOL CZimuView::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	if(m_thumbed)
		return FALSE;

	return CScrollView::OnEraseBkgnd(pDC);
}

void CZimuView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{

	UINT yOrig = GetScrollPos(SB_VERT);
	nPos = nPos & 0xFFFF;
	WORD hiWord = yOrig >> 16;
	UINT minDistance = 0xFFFF, pos = 0;
	for(int i = 0; i <= hiWord+1; i++){
		UINT y = (i<<16) + nPos;
		if(abs(y - yOrig) < minDistance){
			minDistance = abs(y - yOrig);
			pos = y;
		}
	}

	nPos = pos;

	char buf[256] = {0};
	sprintf(buf, "OnVScroll, %d %d %d %d\n", nSBCode, nPos, yOrig, nPos-yOrig);
	OutputDebugString(buf);
	
	if(nSBCode == SB_THUMBTRACK){
		BOOL bResult = OnScrollBy(CSize(0, nPos - yOrig));
		//if (bResult)
		//	Invalidate();
		UpdateWindow();
		return;
	}else{
		Invalidate();
	}

	CScrollView::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CZimuView::AdjustCurrentItem()
{
}

void CZimuView::AdjustScrollPos()
{
	CRect rect;
	GetClientRect(rect);

	m_scrollPos = GetScrollPos(SB_VERT);
	if(m_curItem * m_itemH + m_top > m_scrollPos + rect.Height()){
		m_scrollPos = (m_itemH * m_curItem + m_top) - rect.Height()/2;
		SetScrollPos(SB_VERT, m_scrollPos);
	}else if((m_curItem-1) * m_itemH + m_top < m_scrollPos){
		m_scrollPos = (m_itemH * m_curItem + m_top) - rect.Height()/2;
		SetScrollPos(SB_VERT, m_scrollPos);
	}else{
		return;
	}
}

void CZimuView::SetUpdated(bool updated)
{
	m_updated = updated;
	m_zimu.SetUpdated(updated);
	CMainFrame *frame = (CMainFrame*)AfxGetMainWnd();
	if(frame){
		frame->GetToolBar().OnUpdateCmdUI(frame, TRUE);
	}
}

BOOL CZimuView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	// TODO: Add your message handler code here and/or call default
	return CScrollView::OnMouseWheel(nFlags, zDelta, pt);
}

void CZimuView::OnInputContent(string &content, Sentence &st)
{
	if(content.size() > MAX_TOTAL_BYTES){
		st.m_mark |= SM_CONTENT_LENGTH;
	}

	string old;
	std::swap(st.m_content, old);

	m_zimu.Trim(content);
	if(content == "")
		content = Sentence::m_initialText;
	st.m_content.assign(content);
	m_zimu.CheckContentLength(st);
	CContentChange *cc = new CContentChange(*this, m_zimu, st.m_seq, old);
	
	RedrawItem(st.m_seq, m_curItem == st.m_seq);
	m_updated = true;
//	DoDraw(m_dcMemory);

	m_inEdit = false;
	m_zimu.ClearTimeModifyMark();

	OnCommand(cc);

// 	Invalidate(FALSE);
// 	UpdateWindow();
}

bool CZimuView::OnCloseWindow()
{
	if(m_zimu.IsUpdated()){
		int rt = MessageBox("文件修改过，想要保存吗？", NULL, MB_YESNOCANCEL);
		if(rt == IDYES)
			m_zimu.Save();
		else if(rt == IDCANCEL)
			return false;
	}
	return true;
}


void CZimuView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	OnAdjustTime(point, 200, m_borderPos, false);
	Invalidate(FALSE);
	UpdateWindow();
	
	CScrollView::OnRButtonDown(nFlags, point);
}

LRESULT CZimuView::OnHotkey(WPARAM wParam, LPARAM lParam)
{
	MessageBeep(-1);
	switch(wParam){
	case ID_REPLACE:
	case ID_SEARCH:{
		if(m_frDlg == NULL){
			m_frDlg = new CFindReplaceDialog();
			m_frDlg->Create(wParam == ID_SEARCH, m_lastSearch, m_lastReplace, FR_DOWN, this);
		}
	}
		break;
	default:
		break;
	}
	return 0;
}


void CZimuView::OnErrorCheck() 
{
	bool rt = m_zimu.CheckContents(true, true, this);
	if(!rt){
		m_updated = true;
// 		Invalidate(FALSE);
// 		UpdateWindow();
	}else{
		AfxMessageBox("没有发现时间错误！");
	}
}

void CZimuView::OnButtonConvertSl() 
{
	m_zimu.ConvertToSingleLine();

	Invalidate();
	UpdateWindow();
}

void CZimuView::OnSetFocus(CWnd* pOldWnd) 
{
	CScrollView::OnSetFocus(pOldWnd);
	
	RegisterHotKey(m_hWnd, ID_SEARCH, MOD_CONTROL, 'F');
	RegisterHotKey(m_hWnd, ID_REPLACE, MOD_CONTROL, 'R');

	m_inEdit = false;
}

LONG CZimuView::OnFindReplace(WPARAM wParam, LPARAM lParam)
{
	m_lastSearch = m_frDlg->GetFindString();
	m_lastReplace = m_frDlg->GetReplaceString();
	if(m_frDlg->IsTerminating()){
//		VERIFY(m_frDlg->DestroyWindow());
		//delete m_frDlg;
		m_frDlg = NULL;
		m_findLine = -1;

		//Invalidate(FALSE);
		//UpdateWindow();

		return -1;
	}

	if(m_frDlg->FindNext()){
		CString target = m_frDlg->GetFindString();
		m_searchText.assign(target.GetBuffer(target.GetLength()));

		FindPos nextPos;
		m_findLine = m_zimu.Find(m_searchText.c_str(), m_lastFindPos, nextPos);
		if(m_findLine < 0){
			m_findLine = m_zimu.Find(m_searchText.c_str(), nextPos, nextPos);
			if(m_findLine < 0){
				string prompt = "找不到\"";
				prompt.append(m_searchText);
				prompt.append("\"!");
				MessageBox(prompt.c_str());
			}else{
				MessageBox("已经找到文件末尾！");
			}
		}
		m_lastFindPos = nextPos;

		CRect clientRect;
		GetClientRect(clientRect);
		int spos = m_findLine*m_itemH+m_top;
		spos -= spos % clientRect.Height();
		SetScrollPos(SB_VERT, spos);

	}else if(m_frDlg->ReplaceCurrent()){
		CString target = m_frDlg->GetFindString();
		CString replacer = m_frDlg->GetReplaceString();
		int rv = m_zimu.Replace(m_lastFindPos, target.GetBuffer(target.GetLength()),
			replacer.GetBuffer(replacer.GetLength()));
		if(rv < 0){
			
		}
	}else if(m_frDlg->ReplaceAll()){
		CString target = m_frDlg->GetFindString();
		CString replacer = m_frDlg->GetReplaceString();
		int cnt = m_zimu.ReplaceAll(target.GetBuffer(target.GetLength()), replacer.GetBuffer(replacer.GetLength()));
		if(cnt > 0){
			stringstream ss;
			ss << cnt << "处替换完成!";
			MessageBox(ss.str().c_str());
		}else if(cnt ==0){
			MessageBox("没找到可替换内容!");
		}
	}
	Invalidate(FALSE);
	UpdateWindow();

	return 0;
}

void CZimuView::OnFind() 
{
	if(m_frDlg == NULL){
		m_frDlg = new CFindReplaceDialog();
		m_frDlg->Create(TRUE, m_lastSearch, m_lastReplace, FR_DOWN, this);
	}
}

void CZimuView::OnReplace() 
{
	if(m_frDlg == NULL){
		m_frDlg = new CFindReplaceDialog();
		m_frDlg->Create(FALSE, m_lastSearch, m_lastReplace, FR_DOWN, this);
	}
}

void CZimuView::OnFileNew() 
{
	ResetStatus();
	if(m_zimu.IsUpdated()){
		int rt = MessageBox("文件修改过，想要保存吗？", NULL, MB_YESNOCANCEL);
		if(rt == IDYES)
			m_zimu.Save();
		if(rt == IDCANCEL)
			return;
	}
	m_zimu.Close();
	m_player->Close();
	ResetStatus();

	AfxGetMainWnd()->SetWindowText("菩提字幕");
	Invalidate();
	UpdateWindow();
}

void CZimuView::OnAppExit() 
{
	if(OnCloseWindow())
		PostQuitMessage(0);
}

void CZimuView::OnKillFocus(CWnd* pNewWnd) 
{
	CScrollView::OnKillFocus(pNewWnd);
	
	UnregisterHotKey(m_hWnd, ID_SEARCH);
	UnregisterHotKey(m_hWnd, ID_REPLACE);
}

void CZimuEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if(nChar == VK_F2){
		m_view->SetFocus();
	}else if(nChar == VK_RETURN){
		return;
	}else
		CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CZimuView::OnButtonProof() 
{
	m_proofReading ^= 1;
	Invalidate(FALSE);
	UpdateWindow();
}

void CZimuView::OnUpdateButtonProof(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_proofReading);
	//pCmdUI->Enable(!m_player->GetCurFile().m_filePath.empty());
}

void CZimuView::OnUpdateFileSave(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	//pCmdUI->SetCheck(m_updated);
	pCmdUI->Enable(m_zimu.IsUpdated());
}

void CZimuView::OnButtonTimeAdjust() 
{
	m_timeAdjusting = !m_timeAdjusting;
}

void CZimuView::OnUpdateButtonTimeAdjust(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_timeAdjusting);
}

void CZimuView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	ProcessKeyDown(nChar);
	CScrollView::OnChar(nChar, nRepCnt, nFlags);
}

BOOL CZimuView::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_IME_COMPOSITION || pMsg->message == WM_IME_STARTCOMPOSITION)   
    {   
        HIMC hIMC;   
        HWND hWnd=pMsg->hwnd;   
        DWORD dwSize;   
        WCHAR lpWideStr[20];   
		
        hIMC = ImmGetContext(hWnd);   
        dwSize = ImmGetCompositionStringW(hIMC, GCS_RESULTSTR, NULL, 0);   
        dwSize += sizeof(WCHAR);   
		
        memset(lpWideStr, 0, sizeof(lpWideStr));   
		
        //get string in Unicode   
        ImmGetCompositionStringW(hIMC, GCS_RESULTSTR, lpWideStr, dwSize);   
		
//         //transfer to ANSI code   
//         int   iSize;   
//         char*   pszMultiByte;   
//         int ChineseSimpleAcp = 936;   
// 		
//         iSize = WideCharToMultiByte(ChineseSimpleAcp,0,lpWideStr,-1,NULL,0, NULL,NULL);   
//         pszMultiByte = new char[iSize+1]/**sizeof(char)*/;   
//         WideCharToMultiByte(ChineseSimpleAcp, 0, lpWideStr, -1, pszMultiByte, iSize, NULL, NULL );   
		
		ProcessKeyDown(lpWideStr[0]);

//         delete [] pszMultiByte;   
        ImmReleaseContext(hWnd, hIMC);   
		
        return CScrollView::PreTranslateMessage(pMsg);;   
    }   
	return CScrollView::PreTranslateMessage(pMsg);
}

BOOL CZimuView::ProcessKeyDown(UINT key)
{
	if(!m_timeAdjusting)
		return FALSE;

	SHORT ksCtrl = GetKeyState(VK_CONTROL);	
	SHORT ksShift = GetKeyState(VK_SHIFT);
	BOOL curLineOnly = ksCtrl & 0x8000 || ksShift & 0x8000;
	BOOL modifyEndTime = ksShift & 0x8000;
	
	int delta = TIME_DELTA; //ms
	switch (key){
	case ',':
	case '<':
	case 0xFF0C:
// 		m_zimu.ClearTimeModifyMark();
// 
// 		if(!curLineOnly && !modifyEndTime)
// 			ModifyTime(m_curItem-1, false, -delta);
// 		ModifyTime(m_curItem, !modifyEndTime, -delta);
// 		if(!curLineOnly && modifyEndTime)
// 			ModifyTime(m_curItem+1, true, -delta);
// 
// 		Invalidate();
// 		UpdateWindow();
		break;
	case '.':
	case '>':
	case 0x3002:
// 		m_zimu.ClearTimeModifyMark();
// 
// 		if(!curLineOnly && modifyEndTime)
// 			ModifyTime(m_curItem+1, true, delta);
// 		ModifyTime(m_curItem, !modifyEndTime, delta);
// 		if(!curLineOnly && !modifyEndTime)
// 			ModifyTime(m_curItem-1, false, delta);
// 
// 		Invalidate();
// 		UpdateWindow();
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

void CZimuView::OnCommand(CCommand *command, bool isNewTask)
{
	if(!command)
		return;

	if(isNewTask){
		m_undoList.push_back(command);

		ClearCommandHistory(true, false); //clear redo list.
	}

	if(!command->IsTimeModify())
		m_zimu.ClearTimeModifyMark(); 
	ChangeScollSize();

	m_curItem = command->GetActiveLine();
	ScrollToPos(m_curItem);

	DoDraw(m_dcMemory);
	Invalidate(FALSE);
	UpdateWindow();
}

CCommand::CCommand(CZimuView &view, CZimuFile &data)
:m_view(view)
,m_zimu(data)
{
	m_activeLine = m_view.GetActiveLine();
}

CAddRecordCommand::CAddRecordCommand(CZimuView &view, CZimuFile &data, unsigned long pos, unsigned long length)
:CCommand(view, data)
,m_pos(pos)
,m_length(length)
{
}

void CAddRecordCommand::Execute()
{
	int seq = m_zimu.GetItemCount();
	Sentence *last = m_zimu.GetSentence(seq);
	if(last){
		if(m_pos == 0 || m_pos < last->m_startTime.m_msVal || m_pos > last->m_endTime.m_msVal){
			MessageBeep(-1);
			m_sentence.m_seq = 0;
			return;
		}
		last->m_endTime.SetTime(m_pos);
	}

	if(seq == 0)
		m_sentence.m_startTime.SetTime(m_view.GetStartPos()*1000);
	else
		m_sentence.m_startTime.SetTime(m_pos);
	
	m_sentence.m_endTime.SetTime(m_length);
	m_sentence.m_seq = seq + 1;
	m_sentence.m_content = Sentence::m_initialText;
	m_sentence.m_mark = 0;

	m_zimu.AppendSentence(m_sentence);
	//m_sentence = *(m_zimu.GetSentence(m_zimu.GetItemCount()));
	m_view.PostMessage(WM_VSCROLL, SB_BOTTOM);
}

void CAddRecordCommand::Undo()
{
	int cnt = m_zimu.GetItemCount();
	if(cnt == m_sentence.m_seq){
		m_zimu.RemoveSentence(m_sentence.m_seq);
		m_view.SetActiveLine(m_activeLine);
	}else
		MessageBox(NULL, "撤销时出错!", "菩提字幕", MB_OK);
}

void CAddRecordCommand::Redo()
{
	int cnt = m_zimu.GetItemCount();
	if(cnt+1 == m_sentence.m_seq){
		Sentence *last = m_zimu.GetSentence(cnt);
		if(last)
			last->m_endTime.SetTime(m_pos);
		m_zimu.AppendSentence(m_sentence);
		m_view.SetActiveLine(cnt+1);
		m_view.PostMessage(WM_VSCROLL, SB_BOTTOM);
	}else{
		MessageBox(NULL, "重做时出错!", "菩提字幕", MB_OK);
	}
}

CDelRecordCommand::CDelRecordCommand(CZimuView &view, CZimuFile &data, int seq)
:CCommand(view, data)
{
	m_sentence = m_zimu.GetSentence(seq);
	if(!m_sentence){
		MessageBox(NULL, "删除错误，无效的序号!", "菩提字幕", MB_OK);
	}
}

void CDelRecordCommand::Execute()
{
	if(m_sentence)
		m_zimu.RemoveSentence(m_sentence->m_seq);
}

void CDelRecordCommand::Undo()
{
	if(m_sentence)
		m_zimu.InsertSentence(m_sentence->m_seq-1, *m_sentence);
	m_view.SetActiveLine(m_activeLine);
}

void CDelRecordCommand::Redo()
{
	Execute();
	m_view.SetActiveLine(m_activeLine);
}

CSplitCommand::CSplitCommand(CZimuView &view, CZimuFile &data, int seq, unsigned long pos)
:CCommand(view, data)
,m_pos(pos)
{
	Sentence *sent = m_zimu.GetSentence(seq);
	if(sent){
		m_originSentence = *sent;
	}else{
		m_originSentence.m_seq = 0;
		MessageBox(NULL, "拆分错误，无效的序号!", "菩提字幕", MB_OK);
	}
}

void CSplitCommand::Execute()
{
	int seq = m_originSentence.m_seq;
	if(seq == m_zimu.GetItemCount() || seq == 0)
		return;
	if(m_pos <= m_originSentence.m_startTime.m_msVal ||
		m_pos >= m_originSentence.m_endTime.m_msVal){
		MessageBox(NULL, "拆分错误，当前时间不在被拆分记录的时间范围内!", "菩提字幕", MB_OK);
		return;
	}

	m_splitSentence1 = m_originSentence;
	m_splitSentence1.m_endTime.SetTime(m_pos);
	m_zimu.UpdateSentence(m_splitSentence1);

	m_splitSentence2.m_seq = seq + 1;
	m_splitSentence2.m_startTime.SetTime(m_pos);
	m_splitSentence2.m_endTime.SetTime(m_originSentence.m_endTime.m_msVal);
	m_splitSentence2.m_content.assign(Sentence::m_initialText);
	m_zimu.InsertSentence(seq, m_splitSentence2);
}

void CSplitCommand::Undo()
{
	if(m_originSentence.m_seq == 0)
		return;

	m_zimu.RemoveSentence(m_splitSentence2.m_seq);
	m_zimu.UpdateSentence(m_originSentence);
	m_view.SetActiveLine(m_activeLine);
}

void CSplitCommand::Redo()
{
	m_zimu.UpdateSentence(m_splitSentence1);
	m_zimu.InsertSentence(m_splitSentence1.m_seq, m_splitSentence2);
	m_view.SetActiveLine(m_activeLine);
}

CMergeCommand::CMergeCommand(CZimuView &view, CZimuFile &data, int seq1, int seq2)
:CCommand(view, data)
{
	Sentence *sent1 = m_zimu.GetSentence(seq1);
	Sentence *sent2 = m_zimu.GetSentence(seq2);
	if(sent1 && sent2){
		m_originSentence1 = *sent1;
		m_originSentence2 = *sent2;
	}else{
		m_originSentence1.m_seq = 0;
		m_originSentence2.m_seq = 0;
		MessageBox(NULL, "合并错误，无效的序号!", "菩提字幕", MB_OK);
	}
}

void CMergeCommand::Execute()
{
	m_mergedSentence = m_originSentence1;
	m_mergedSentence.m_endTime = m_originSentence2.m_endTime;
	m_mergedSentence.m_content += m_originSentence2.m_content;

	m_zimu.UpdateSentence(m_mergedSentence);
	m_zimu.RemoveSentence(m_originSentence2.m_seq);
	m_view.SetActiveLine(m_activeLine);
}

void CMergeCommand::Undo()
{
	m_zimu.UpdateSentence(m_originSentence1);
	m_zimu.InsertSentence(m_originSentence1.m_seq, m_originSentence2);
	m_view.SetActiveLine(m_activeLine);
}

void CMergeCommand::Redo()
{
	m_zimu.UpdateSentence(m_mergedSentence);
	m_zimu.RemoveSentence(m_originSentence2.m_seq);
	m_view.SetActiveLine(m_activeLine);
}

CContentChange::CContentChange(CZimuView &view, CZimuFile &data, int seq, const string &contentOld)
:CCommand(view, data)
, m_seq(seq)
, m_content(contentOld)
, m_proof(view.IsProofReading())
{
}

void CContentChange::Execute()
{
}
void CContentChange::Undo()
{
	Sentence *st = m_zimu.GetSentence(m_seq);
	if(st){
		std::swap(st->m_content, m_content); //now m_conent contains new.
		m_zimu.CheckContentLength(*st);
		if(m_proof){
			st->m_reviseRecords.pop_back();
		}
	}
}

void CContentChange::Redo()
{
	Sentence *st = m_zimu.GetSentence(m_seq);
	if(st){
		std::swap(st->m_content, m_content);
		m_zimu.CheckContentLength(*st);
		if(m_proof){
			ReviseRecord rr;
			rr.m_contents.assign(m_content); //now m_conent became old.
			st->m_reviseRecords.push_back(rr);
		}
	}
}

CTimeAdjust::CTimeAdjust(CZimuView &view, CZimuFile &data, int seq1, int seq2, DWORD mask1, DWORD mask2, int delta)
:CCommand(view, data)
,m_seq1(seq1)
,m_seq2(seq2)
,m_mask1(mask1)
,m_mask2(mask2)
,m_delta(delta)
{
}
void CTimeAdjust::Execute()
{
	m_zimu.ClearTimeModifyMark();

	if(m_delta > 0){
		AdjustSentence(m_seq2, m_mask2, m_delta);
		AdjustSentence(m_seq1, m_mask1, m_delta);
	}else{
		AdjustSentence(m_seq1, m_mask1, m_delta);
		AdjustSentence(m_seq2, m_mask2, m_delta);
	}
}
void CTimeAdjust::Undo()
{
	m_zimu.ClearTimeModifyMark();

	if(m_delta > 0){
		AdjustSentence(m_seq1, m_mask1, -m_delta);
		AdjustSentence(m_seq2, m_mask2, -m_delta);
	}else{
		AdjustSentence(m_seq2, m_mask2, -m_delta);
		AdjustSentence(m_seq1, m_mask1, -m_delta);
	}
}
void CTimeAdjust::Redo()
{
	Execute();
}
void CTimeAdjust::AdjustSentence(int seq, DWORD mask, int delta)
{
	if(seq != 0){
		//ASSERT(mask & TAM_BOTH);
		if((mask & TAM_BOTH) == 0)
			return;

// 		DWORD time;
// 		if(mask & TAM_START){
// 			time = st.m_startTime.m_msVal;
// 			time += delta;
// 			st.m_startTime.SetTime(time);
// 			st.m_mark |= SM_TIME_MODIFIED1;
// 		}
// 
// 		if(mask & TAM_END){
// 			time = st.m_endTime.m_msVal;
// 			time += delta;
// 			st.m_endTime.SetTime(time);
// 			st.m_mark |= SM_TIME_MODIFIED2;
// 		}
// 
// 		m_zimu.UpdateSentence(st);
		m_view.ModifyTime(seq, (mask & TAM_START), delta);
	}
}

void CZimuView::OnEditUndo() 
{
	if(m_undoList.empty())
		return;
	CCommand *command = m_undoList.back();
	command->Undo();

	m_redoList.push_front(command);
	m_undoList.pop_back();

	OnCommand(command, false);
}

void CZimuView::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!m_undoList.empty());
}

void CZimuView::OnEditRedo() 
{
	if(m_redoList.empty())
		return;
	CCommand *command = m_redoList.front();
	command->Redo();

	m_undoList.push_back(command);
	m_redoList.pop_front();

	OnCommand(command, false);
}

void CZimuView::ClearCommandHistory(bool redo, bool undo)
{
	if(redo){
		CommandList::iterator it = m_redoList.begin();
		for(; it != m_redoList.end(); it++)
			delete *it;
		m_redoList.clear();
	}

	if(undo){
		CommandList::iterator it = m_undoList.begin();
		for(; it != m_undoList.end(); it++)
			delete *it;
		m_undoList.clear();
	}
}

void CZimuView::ResetStatus()
{
	m_curItem = 0;
	m_prevItem = 0;
	m_playingItem = 0;
	m_findLine = 0;
	m_mergeSecond = 0;
	m_lastPos = 0;
	m_isPaused = false;
	m_changeStart = false;
	
	m_curItem = 0;
	m_findLine = 0;
	m_mergeSecond = 0;

	m_startPos = 0;
	m_endPos = 0;
	ClearCommandHistory(true, true);
}

void CZimuView::OnUpdateEditRedo(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!m_redoList.empty());
}

void CZimuView::OnUpdateButtonAdd(CCmdUI* pCmdUI) 
{
	// there is file open.
	pCmdUI->Enable(m_player->IsOpen());
}

void CZimuView::OnUpdateButtonDel(CCmdUI* pCmdUI) 
{
	// selected
	pCmdUI->Enable(m_curItem && m_curItem <= m_zimu.GetItemCount() && m_mergeSecond == 0);
}

void CZimuView::OnUpdateButtonMerge(CCmdUI* pCmdUI) 
{
	// selected two lines.
	pCmdUI->Enable(m_mergeSecond != 0);
}

void CZimuView::OnUpdateButtonSplit(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_curItem && m_curItem <= m_zimu.GetItemCount() && m_mergeSecond == 0);
}

void CZimuView::OnUpdateButtonMovedown(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_player->IsOpen());
}

void CZimuView::OnUpdateButtonMoveup(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_player->IsOpen());
}

void CZimuView::OnUpdateButtonPause(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_player->IsPlaying());
}

void CZimuView::OnUpdateButtonPlay(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	BOOL enable = m_player->IsOpen() && !m_player->IsPlaying();
	//if(m_player->GetCurFile().m_filePath.empty())
	//	enable = FALSE;
	pCmdUI->Enable(enable);
}

void CZimuView::OnUpdateButtonSkip(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	BOOL enable = m_player->IsOpen();
	pCmdUI->Enable(enable);
}

void CZimuView::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent == m_timerAutoSave){
		OnFileSave();
	}
	CScrollView::OnTimer(nIDEvent);
}

void CZimuView::OnEditTimegap() 
{
	// TODO: Add your command handler code here
	int cnt = m_zimu.TimeGapFill();
	CString msg = "没有时间缝隙。";
	if(cnt > 0)
		msg.Format("%d条时间对齐！", cnt);
	AfxMessageBox(msg);
}

void CZimuView::OnFileFinalVersion() 
{
	// TODO: Add your command handler code here
	if(IDCANCEL == MessageBox("终审定稿将会删除所有修订记录，确定要定稿吗？", "菩提字幕", MB_OKCANCEL))
		return;

	m_zimu.ClearReviseRecord();

	ChangeScollSize();
	Invalidate();
	UpdateWindow();
}

void CZimuView::OnFileClose() 
{
	// TODO: Add your command handler code here
	if(m_zimu.IsUpdated()){
		int rt = MessageBox("字幕文件有过改动，想要保存吗？", "菩提字幕", MB_YESNOCANCEL);
		if(IDCANCEL == rt)
			return;
		if(IDYES == rt)
			m_zimu.Save();
	}
	m_zimu.Close();
	m_player->Close();
	ResetStatus();

	CMainFrame *frame = (CMainFrame*)AfxGetMainWnd();	
	frame->SetWindowText("菩提字幕");

	Invalidate();
	UpdateWindow();
}

void CZimuView::OnUpdateFileSaveAs(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_zimu.GetDestPath() != "");
}

void CZimuView::OnUpdateFileOpen(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_zimu.GetSourcePath() == "");
}

void CZimuView::OnUpdateFileImport(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_zimu.GetSourcePath() == "");
}

void CZimuView::OnTimeOffset()
{
	// TODO: 在此添加命令处理程序代码
	CTimeOffsetDlg dlg(this);
	if(dlg.DoModal() == IDOK){
		int offset = dlg.GetOffset();
		if(offset == 0)
			return;
		if(m_zimu.OffsetTime(offset*1000) == -1)
			::MessageBeep(-1);
		else{
			Invalidate();
			UpdateWindow();
		}
	}
}
// E:\字幕工作区\Zimu_921src\ZimuView.cpp : 实现文件
//

#include "stdafx.h"
#include "Zimu.h"
#include "ZimuView.h"


// CTimeOffsetDlg 对话框

IMPLEMENT_DYNAMIC(CTimeOffsetDlg, CDialog)

CTimeOffsetDlg::CTimeOffsetDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTimeOffsetDlg::IDD, pParent)
	, m_direction(-1)
	, m_min(0)
	, m_sec(0)
{

}

CTimeOffsetDlg::~CTimeOffsetDlg()
{
}

void CTimeOffsetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_TO_MINUTE, m_min);
	DDX_Text(pDX, IDC_TO_SECOND, m_sec);
}


BEGIN_MESSAGE_MAP(CTimeOffsetDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CTimeOffsetDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_RD_DEC, &CTimeOffsetDlg::OnBnClickedRdDec)
	ON_BN_CLICKED(IDC_RD_INC, &CTimeOffsetDlg::OnBnClickedRdInc)
END_MESSAGE_MAP()

int CTimeOffsetDlg::GetOffset() const
{
	int off = m_min * 60 + m_sec;
	if(m_direction == -1)
		return 0;
	else if(m_direction == 0)
		off = -off;
	return off;
}

void CTimeOffsetDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	OnOK();
}


void CTimeOffsetDlg::OnBnClickedRdDec()
{
	m_direction = 0;
}

void CTimeOffsetDlg::OnBnClickedRdInc()
{
	m_direction = 1;
}
// E:\字幕工作区\Zimu_921src\ZimuView.cpp : 实现文件
//

// CStartPosDlg 对话框

IMPLEMENT_DYNAMIC(CStartPosDlg, CDialog)

CStartPosDlg::CStartPosDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CStartPosDlg::IDD, pParent)
	, m_min(0)
	, m_sec(0)
{

}

CStartPosDlg::~CStartPosDlg()
{
}

void CStartPosDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_MIN, m_min);
	DDX_Text(pDX, IDC_SEC, m_sec);
}


BEGIN_MESSAGE_MAP(CStartPosDlg, CDialog)
END_MESSAGE_MAP()

int CStartPosDlg::GetOffset() const
{
	int off = m_min * 60 + m_sec;
	return off;
}
// CStartPosDlg 消息处理程序
// ZimuView.cpp : 实现文件
//

#include "stdafx.h"
#include "Zimu.h"
#include "ZimuView.h"


// CDebugDlg 对话框

IMPLEMENT_DYNAMIC(CDebugDlg, CDialog)

CDebugDlg::CDebugDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDebugDlg::IDD, pParent)
	//, m_list(new CListView0())
	, m_edit(new CRichEditCtrl)
{

}

CDebugDlg::~CDebugDlg()
{
	delete m_edit;
	m_edit = NULL;
}

BOOL CDebugDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect rect;
	GetClientRect(rect);
	m_edit->Create(0, rect, this, IDC_RICHEDIT_DBG);
	return TRUE;
}

void CDebugDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDebugDlg, CDialog)
END_MESSAGE_MAP()


// CDebugDlg 消息处理程序
