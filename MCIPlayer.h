// MCIPlayer.h: interface for the CMCIPlayer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MCIPLAYER_H__D7434DF6_4705_483F_BFE4_ED85916C97CF__INCLUDED_)
#define AFX_MCIPLAYER_H__D7434DF6_4705_483F_BFE4_ED85916C97CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "StdAfx.h"

#include <list>
#include <string>
#include <sstream>
#include <fstream>
#include <dshow.h>
using namespace std;

class MyLoger
{
	HWND m_targetWnd;
	ofstream m_ofs;
private:
	MyLoger()
		: m_targetWnd(NULL)
		, m_ofs(ofstream("d:\\zimu.log"))
	{
		ASSERT(m_ofs);
	}
public:
	static MyLoger& Instance(){
		static MyLoger _inst;
		return _inst;
	}

	void LogMessage(const string& msg)
	{//WM_HOTKEY
		m_ofs << msg;
		m_ofs.flush();
	}

};
#define _LOG(m) MyLoger::Instance().LogMessage(m)
#define DebugOut(d) \
{ ostringstream os; os << d; _LOG(os.str()); }

enum FileState{ FS_CLOSED, FS_OPEN, FS_PLAYING, FS_PAUSED };

string TimeMS2HMS(int timeMS);
string TimeMS2Str(int timeMS);
string BeatyDisplay(int num, int width, bool lt = false);

struct MP3File
{
	string m_filePath;
	UINT m_deviceId;
	FileState m_state;
	long m_lengthMS;
	DWORD m_callback;
	string m_lastError;
public:
	MP3File() : m_deviceId(0), m_state(FS_CLOSED), m_lengthMS(0), m_callback(0) {}
};

class CZimuView;
class IPlayResultListner
{
public:
	virtual ~IPlayResultListner() {}
	virtual void OnPlayResult(int code) = 0;
	virtual HWND GetNotifyHandle() = 0;
};

struct MediaFile
{
	string m_filePath;
	FileState m_state;
	unsigned long m_lengthMS;
	string m_lastError;
public:
	MediaFile()
	{
		Reset();
	}
	void Reset() { m_filePath.assign(""); m_state = FS_CLOSED; m_lengthMS = 0; m_lastError.assign(""); }
};

class CDShowPlayer
{
	IGraphBuilder *m_graph_builder;
	IMediaControl *m_media_control;
	IMediaSeeking *m_media_seeking;
	IMediaEventEx *m_media_event;

	CZimuView *m_play_result;
	UINT m_timer_id;

	int m_start_pos;
	int m_end_pos;
	unsigned long m_rangeStart;
	unsigned long m_rangeEnd;
	LONGLONG m_duration;
	MediaFile m_media_file;
	static int m_instance_count;
	
	double m_speedRatio;
private:
	CDShowPlayer(CZimuView *playResult);
	long init();
	void uninit();
public:
	static CDShowPlayer* CreateInstance(CZimuView *playResult)
	{
		CDShowPlayer *inst = new CDShowPlayer(playResult);
		if(inst->init() != 0){
			delete inst;
			return NULL;
		}
		return inst;
	}
	virtual ~CDShowPlayer();

	long Open(const char* filename);
	long Close();
	long Pause();
	long Resume();
	void SetRange(unsigned long startMs, unsigned long endMs) { m_rangeStart = startMs; m_rangeEnd = endMs; }
	long Play(unsigned long fromMS, unsigned long toMs, bool forceSeeking);
	long Stop();
	long Speed(double ratio);
	long GetCurPos(unsigned long &pos);

	const MediaFile& GetCurFile() const { return m_media_file; }
	void GetLastError(string &err) const { err.assign(m_media_file.m_lastError); }

	static void PASCAL s_TimerProc(UINT timerID, UINT msg, DWORD player, DWORD para1, DWORD para2);
	void OnTimer(UINT timerID);
private:
	//just for test.
	BOOL Play_MP3(char* filename);
};

class CMCIPlayer  
{
public:
	MP3File m_curFile;
	UINT m_timerID;
	CZimuView *m_view;
	bool m_retry;
	int m_from;
	int m_to;

//	CDShowPlayer m_dsPlayer;

public:
	CMCIPlayer(CZimuView* view);
	virtual ~CMCIPlayer();

	const MP3File& GetCurFile() const { return m_curFile; }
	int Open(const string &file, HWND callbackHandle);
	int Close();
	int Play(int fromMs = 0, int toMs = 0, bool repeat = false);
	int Play2();
	int Pause();
	int Resume();
	int Stop();
	int Seek(int posMS);
	int GetCurPos(long &posMs);
	int QueryStatus(DWORD item, LONG &value);
	void GetLastError(string &message) { message.assign(m_curFile.m_lastError);}

	CZimuView* GetView() const { return m_view;}
	friend void PASCAL MCIPlayer_TimerProc(UINT timerID, UINT msg, DWORD mciPlayer, DWORD para1, DWORD para2);
};





/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::string TString;

class PlayerListener
{
public:
	virtual void OnPlay(int code) = 0;
};

class MediaPlayer
{
public:
	enum State{
		MPS_CLOSED,
		MPS_OPEN,
		MPS_PAUSED,
		MPS_PLAYING,
		MPS_STOPPED
	};

	MediaPlayer(PlayerListener *listener);
	virtual ~MediaPlayer();
public:
	virtual long Open(const TString &file) = 0;
	virtual void Close() = 0;
	virtual long Play(long seglen) = 0;
	virtual long Pause() = 0;
	virtual long Stop() = 0;
	virtual long Resume() = 0;
	virtual long Seek(long pos) = 0;

	virtual bool IsPlaying() const { return m_state == MPS_PLAYING; }
	virtual bool IsPaused() const { return m_state == MPS_PAUSED; }
	virtual bool IsOpen() const { return m_state != MPS_CLOSED; }

	virtual long GetCurrentPos() const = 0;
	virtual long GetMediaDuration() const { return m_duration; }
	virtual void GetLastError(string &err) const = 0;
protected:
	PlayerListener *m_listener;
	long m_duration;
	State m_state;
};

class DShowPlayer : public MediaPlayer
{
	DShowPlayer(PlayerListener *listener);
public:
	static DShowPlayer* CreateInstance(PlayerListener *listener);
	virtual ~DShowPlayer();

	virtual long Open(const TString &file);
	virtual void Close();
	virtual long Play(long seglen);
	virtual long Pause();
	virtual long Stop();
	virtual long Resume();
	virtual long Seek(long pos);

	virtual long GetCurrentPos() const;
	virtual long GetMediaDuration() const;
	virtual void GetLastError(string &err) const;
protected:
	long Init();
	void Uninit();
	long BuildGraph(const WCHAR *filename);
	long ConnectFilter(IBaseFilter *upper, IBaseFilter *down, LPCWSTR pinNameOut, LPCWSTR pinNameIn);
	long CreateFilter(CLSID &clsid, IBaseFilter **filter);
	void OnTimer(UINT timerID);
protected:
	IGraphBuilder *m_graph_builder;
	IMediaControl *m_media_control;
	IMediaSeeking *m_media_seeking;
	IMediaEventEx *m_media_event;

	MediaFile m_media_file;
	long m_seg_start;
	long m_seg_end;
	UINT m_timer_id;

	static void PASCAL s_TimerProc(UINT timerID, UINT msg, DWORD player, DWORD para1, DWORD para2);
	static int m_instance_count;
};


#endif // !defined(AFX_MCIPLAYER_H__D7434DF6_4705_483F_BFE4_ED85916C97CF__INCLUDED_)
