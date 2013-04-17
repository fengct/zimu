// ZimuView.h : interface of the CZimuView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_ZIMUVIEW_H__C6BC10F9_611C_4081_9D37_266625FAD0E3__INCLUDED_)
#define AFX_ZIMUVIEW_H__C6BC10F9_611C_4081_9D37_266625FAD0E3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <vector>
#include <sstream>
#include "MCIPlayer.h"
#include "ZimuFile.h"

#define WM_GRAPHNOTIFY WM_APP + 1


///////////////////////////////////////////////////////////////////////////////////////////
class CZimuView;


/////////////////////////////////////////////////////////////////////////////
// CZimuEdit window
class CZimuDoc;
class CZimuView;
class CZimuEdit : public CEdit
{
// Construction
	std::list<CString> m_history;
	std::list<CString> m_future;
public:
	CZimuEdit();

// Attributes
public:
	Sentence *m_sentence;
	CZimuView *m_view;
	WNDPROC m_oldWndProc;
// Operations
public:
	void Initial();
	void Reset();
	void ReplaceSelection(LPTSTR text);
	LRESULT OnHotKey(WPARAM wParam, LPARAM lParam);
	afx_msg void OnChange();

	void MySubclassWindow(HWND hWnd);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CZimuEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CZimuEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CZimuEdit)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

class CCommand
{
public:
	CCommand(CZimuView &view, CZimuFile &data);

	virtual void Execute() = 0;
	virtual void Undo() = 0;
	virtual void Redo() = 0;
	virtual bool IsTimeModify() const { return FALSE;}
	int GetActiveLine() const { return m_activeLine; }
protected:
	CZimuView &m_view;
	CZimuFile &m_zimu;
	int m_activeLine;
};

class CAddRecordCommand : public CCommand
{
public:
	CAddRecordCommand(CZimuView &view, CZimuFile &data, unsigned long pos, unsigned long length);

	virtual void Execute();
	virtual void Undo();
	virtual void Redo();
protected:
	Sentence m_sentence;
	unsigned long m_pos;
	unsigned long m_length;
};

class CDelRecordCommand : public CCommand
{
public:
	CDelRecordCommand(CZimuView &view, CZimuFile &data, int seq);

	virtual void Execute();
	virtual void Undo();
	virtual void Redo();
protected:
	Sentence *m_sentence;
};

class CSplitCommand : public CCommand
{
public:
	CSplitCommand(CZimuView &view, CZimuFile &data, int seq, unsigned long pos);

	virtual void Execute();
	virtual void Undo();
	virtual void Redo();
protected:
	unsigned long m_pos;
	Sentence m_originSentence;
	Sentence m_splitSentence1;
	Sentence m_splitSentence2;
};

class CMergeCommand : public CCommand
{
public:
	CMergeCommand(CZimuView &view, CZimuFile &data, int seq1, int seq2);

	virtual void Execute();
	virtual void Undo();
	virtual void Redo();
protected:
	Sentence m_originSentence1;
	Sentence m_originSentence2;
	Sentence m_mergedSentence;
};

class CContentChange : public CCommand
{
public:
	CContentChange(CZimuView &view, CZimuFile &data, int seq, const string &contentOld);

	virtual void Execute();
	virtual void Undo();
	virtual void Redo();
private:
	int		 m_seq;
	string	 m_content;
	BOOL	 m_proof;
};

enum {TAM_START = 0x01, TAM_END = 0x02, TAM_BOTH = TAM_START | TAM_END };
class CTimeAdjust : public CCommand
{
public:
	CTimeAdjust(CZimuView &view, CZimuFile &data, int seq1, int seq2, DWORD mask1, DWORD mask2, int delta);

	virtual void Execute();
	virtual void Undo();
	virtual void Redo();
	virtual bool IsTimeModify() const { return TRUE;}
private:
	void AdjustSentence(int seq, DWORD mask, int delta);
protected:
	int m_seq1;
	int m_seq2;
	DWORD	m_mask1;
	DWORD	m_mask2;
	int		m_delta;
};
/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


class CZimuView : public CScrollView
				, public PlayerListener
				, public IRefreshable
{
protected: // create from serialization only
	CZimuView();
	DECLARE_DYNCREATE(CZimuView)

// Attributes
public:
	CZimuDoc* GetDocument();

// Operations
public:
	//file operations
	void OpenMP3File(const char* path);
	void CloseMP3File();
	void SaveSRT();

	//voice oper
	void PlayPause(bool play);
	void Stop();
	void SeekTo(long posMs);
	LRESULT OnMCINotify(WPARAM wParam, LPARAM lParam);
	LRESULT OnDSNotify(WPARAM wParam, LPARAM lParam);

	void AddItem();
	void ModifyTime(int seq, bool startTime, int delta);

	//Draw
	void DrawTitle(CDC &dc);
	void DisplayZimu(CDC &dc);
	void ShowReviseRecords(CDC &dc, CZimuFile::ContentsType::const_iterator start, CZimuFile::ContentsType::const_iterator end, int reviseItemCnt);
	void DrawProgressBar(CDC &dc);
	void RedrawItem(int seq, bool heighLight, CDC *dc = NULL);
	void DrawArrow(CDC &dc);
	void EraseArrow(CDC &dc);
	void ChangeScollSize();

	void SetActiveLine(int seq) { m_curItem = seq; }
	int GetActiveLine() const { return m_curItem; }
	//void SetEditState(bool val) { m_inEdit = val; }

	//help
	int PickItem(CPoint point, bool input);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CZimuView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct

	virtual void Refresh();
	//}}AFX_VIRTUAL
protected:
	void DoDraw(CDC &dc);
	void AdjustCurrentItem();
	void AdjustScrollPos();
	void OnAdjustTime(CPoint pt, int val, int item, bool left);
	LRESULT OnHotkey(WPARAM wParam, LPARAM lParam);
	void OnButtonConvertSl();
	void StartEdit(int line);
	BOOL ProcessKeyDown(UINT key);
	void OnCommand(CCommand *command, bool isNewTask = true);
	void ClearCommandHistory(bool redo, bool undo);

	void ResetStatus();
	virtual void OnPlay(int code);
// Implementation
public:
	bool OnCloseWindow();
	void OnInputContent(string &content, Sentence &st);
	void SetUpdated(bool update);
	//void OnPlayResult(int result);

	virtual void OnPlayResult(int code);
	virtual HWND GetNotifyHandle() { return m_hWnd; }
	virtual ~CZimuView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void ScrollToPos(int seq);
protected:
	//display info.
	int m_left;
	int m_top;
	int m_seqW;
	int m_startW;
	int m_endW;
	int m_contentW;
	int m_titleH;
	int m_itemH;
	int m_vscrollbarW;
	UINT m_scrollPos;
	CDC m_dcMemory;
	CBitmap m_bmpMemory;
	typedef std::vector<CBitmap*> BmpPageType;
	BmpPageType m_bmpPages;
	bool m_updated;
	bool m_sizeChanged;
	static const int m_bmpH;
	int m_orgOff;
	int m_lastVPos;
	bool m_thumbed;
	bool m_inEdit;
	bool m_dcMeminit;

	//context info.
	int m_curItem;
	int m_prevItem;
	int m_playingItem;
	int m_findLine;
	int m_mergeSecond;
	int m_borderPos;
	unsigned long m_lastPos;
	bool m_isPaused;
	bool m_changeStart;
	CZimuFile m_zimu;
	//CMCIPlayer m_player;
	//CDShowPlayer *m_player;
	DShowPlayer	*m_player;
	string m_searchText;

	CZimuEdit m_input;
	CRect m_upArrow;
	CRect m_downArrow;
	bool m_adjustBoth;
	static const int m_arrowBoxWidth;
	CFindReplaceDialog *m_frDlg;
	FindPos m_lastFindPos;
	CString m_lastSearch;
	CString m_lastReplace;
	UINT m_lastKey;

	BOOL m_proofReading;
	BOOL m_hasProofed;
	BOOL m_timeAdjusting;

	UINT m_timerAutoSave;
	int m_startPos; //sec
	int m_endPos; //sec

	typedef std::list<CCommand*> CommandList;
	CommandList m_undoList;
	CommandList m_redoList;
// Generated message map functions
protected:
	//{{AFX_MSG(CZimuView)
	afx_msg void OnButtonPlay();
	afx_msg void OnButtonPause();
	afx_msg void OnButtonStop();
	afx_msg void OnFileOpen();
	afx_msg void OnFileSave();
	afx_msg void OnButtonSkip();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonDel();
	afx_msg void OnButtonMerge();
	afx_msg void OnButtonMovedown();
	afx_msg void OnButtonMoveup();
	afx_msg void OnButtonSplit();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnFileImport();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnFileSaveAs();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnErrorCheck();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg LONG OnFindReplace(WPARAM wParam, LPARAM lParam);
	afx_msg void OnFind();
	afx_msg void OnReplace();
	afx_msg void OnFileNew();
	afx_msg void OnAppExit();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnButtonProof();
	afx_msg void OnUpdateButtonProof(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnButtonTimeAdjust();
	afx_msg void OnUpdateButtonTimeAdjust(CCmdUI* pCmdUI);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonAdd(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonDel(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonMerge(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonSplit(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonMovedown(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonMoveup(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonPause(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonPlay(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonSkip(CCmdUI* pCmdUI);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnEditTimegap();
	afx_msg void OnFileFinalVersion();
	afx_msg void OnFileClose();
	afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileOpen(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileImport(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	BOOL IsProofReading() const { return m_proofReading;}
	void SetProofed(BOOL proofed) { m_hasProofed = proofed; }
	int GetStartPos() const { return m_startPos;}
	afx_msg void OnTimeOffset();
};

#ifndef _DEBUG  // debug version in ZimuView.cpp
inline CZimuDoc* CZimuView::GetDocument()
   { return (CZimuDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(AFX_ZIMUVIEW_H__C6BC10F9_611C_4081_9D37_266625FAD0E3__INCLUDED_)
#pragma once


// CTimeOffsetDlg 对话框
#include "resource.h"
class CTimeOffsetDlg : public CDialog
{
	DECLARE_DYNAMIC(CTimeOffsetDlg)

	
public:
	CTimeOffsetDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CTimeOffsetDlg();

// 对话框数据
	enum { IDD = IDD_DLG_TIMEOFFSET };

	int GetOffset() const;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
	int m_direction;
	int m_min;
	int m_sec;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedRdDec();
	afx_msg void OnBnClickedRdInc();
};
#pragma once


// CStartPosDlg 对话框

class CStartPosDlg : public CDialog
{
	DECLARE_DYNAMIC(CStartPosDlg)

public:
	CStartPosDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CStartPosDlg();

// 对话框数据
	enum { IDD = IDD_DLG_STARTPOS };

	int GetOffset() const;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
	int m_min;
	int m_sec;
};
#pragma once

