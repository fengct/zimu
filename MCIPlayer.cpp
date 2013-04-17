// MCIPlayer.cpp: implementation of the CMCIPlayer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <string>

using namespace std;

#include "MCIPlayer.h"
#include <mmsystem.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define USING_DSPLAYER 1

#include "ZimuView.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

string g_strNoError = "无错误。";

int CDShowPlayer::m_instance_count = 0;
CDShowPlayer::CDShowPlayer(CZimuView *playResult)
	:m_play_result(playResult)
	,m_graph_builder(NULL)
	,m_media_control(NULL)
	,m_media_seeking(NULL)
	,m_media_event(NULL)
	,m_start_pos(0)
	,m_end_pos(0)
	,m_rangeStart(0)
	,m_rangeEnd(0)
	,m_duration(0)
	,m_timer_id(0)
	,m_speedRatio(1.0)
{
	m_instance_count++;

	if(m_instance_count == 1){ //the first instance.
		// initialize COM
		//
		// initialize the COM library on the current thread and identifies the concurrency model as single-thread
		// apartment (STA).
		CoInitialize(0);
	}

}

long CDShowPlayer::init()
{
    // create the DirectMusic performance object
    //
    // creates a single uninitialized object of the class associated with a specified CLSID.
    if(FAILED(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, 
                               (void**)&m_graph_builder)))
    {
        MessageBox(NULL, "Unable to create DirectShow Graph Builder object.", "Error", MB_OK);
        return -1;
    }

	// Query for the media control and event objects
	ASSERT(m_graph_builder);
	if(m_graph_builder){
		m_graph_builder->QueryInterface(IID_IMediaControl, (void**)&m_media_control);
		m_graph_builder->QueryInterface(IID_IMediaSeeking , (void**)&m_media_seeking);
		m_graph_builder->QueryInterface(IID_IMediaEventEx, (void**)&m_media_event);

		if(m_media_control && m_media_seeking && m_media_event)
			;
		else
			return -1;
	}else{
		return -1;
	}
//	Play_MP3("F:\\Musics\\Music\\dxh.mp3");
	return 0;
}

void CDShowPlayer::uninit()
{
	if(m_media_control)
		m_media_control->Stop();
	if(m_media_event){
		m_media_event->Release();
		m_media_event = NULL;
	}
	
	if(m_media_control){
		m_media_control->Release();
		m_media_control = NULL;
	}
	if(m_media_seeking){
		m_media_seeking->Release();
		m_media_seeking = NULL;
	}
	if(m_graph_builder){
		m_graph_builder->Release();
		m_graph_builder = NULL;
	}
}

CDShowPlayer::~CDShowPlayer()
{
	uninit();

	m_instance_count--;
	if(m_instance_count == 0)
		CoUninitialize();
}

BOOL CDShowPlayer::Play_MP3(char* filename)
{
    // convert filename to wide-character string
    WCHAR w_filename[MAX_PATH] = {0};

    mbstowcs(w_filename, filename, MAX_PATH);

    // render the file
    m_graph_builder->RenderFile(w_filename, NULL);

    // play the file, switches the entire filter graph into a running state.
    m_media_control->Run();

    return TRUE;
}

long CDShowPlayer::Open(const char* filename)
{
//	filename = "F:\\Musics\\Music\\dxh.mp3";
	if(!m_graph_builder)
		init();

	m_media_file.Reset();
	m_media_file.m_filePath = filename;
	m_media_file.m_state = FS_OPEN;
	
    WCHAR w_filename[MAX_PATH] = {0};
	MultiByteToWideChar(CP_ACP, MB_COMPOSITE, filename, -1, w_filename, MAX_PATH);
    //mbstowcs(w_filename, filename, MAX_PATH);

    // render the file
    HRESULT rt = m_graph_builder->RenderFile(w_filename, NULL);
	if(rt != S_OK){
		return rt;
	}
	
	GUID timeFormat = TIME_FORMAT_MEDIA_TIME;
	rt = m_media_seeking->SetTimeFormat(&timeFormat);
	if(rt != S_OK){
		return rt;
	}

	rt = m_media_seeking->GetDuration(&m_duration);
	if(rt != S_OK){
		return rt;
	}
	m_media_file.m_lengthMS = m_duration / 10000;
	if(m_rangeEnd == 0)
		m_rangeEnd = m_media_file.m_lengthMS;

//	rt = m_media_event->SetNotifyWindow((OAHWND)m_view->m_hWnd, WM_GRAPHNOTIFY, 0);
//	if(rt != S_OK){
//		return rt;
//		//MessageBox(NULL, "Unable to set notify window.", "Error", MB_OK);
//	}
	return rt;
}

long CDShowPlayer::Close()
{
	timeKillEvent(m_timer_id);

	uninit();

	m_media_file.Reset();
	return 0;
}

long CDShowPlayer::Stop()
{
	LONGLONG cur = 0;
	HRESULT rt = m_media_seeking->SetPositions(&cur, AM_SEEKING_AbsolutePositioning, &m_duration, AM_SEEKING_AbsolutePositioning);
	if(rt != S_OK){
		TRACE0("DShowPlayer seek to begin failed!.");
	}
	rt = m_media_control->Stop();
	if(rt != S_OK){
		DebugOut("Stop failed." << std::endl);
		TRACE0("DShowPlayer, Stop failed!");
	}
// 	m_graph_builder->Abort();
// 	
// 	m_start_pos = m_end_pos = 0;
// 	m_duration = 0;
// 	m_media_file.Reset();

	timeKillEvent(m_timer_id);

// 	uninit();
// 	rt = init();
	DebugOut("Stop " << std::endl);
	return rt;
}

long CDShowPlayer::GetCurPos(unsigned long &pos)
{
	LONGLONG cur = 0;
	HRESULT rt = m_media_seeking->GetCurrentPosition(&cur);
	if(rt != S_OK){
		DebugOut("GetCurPos failed: rt=" << rt << std::endl);
	}
	pos = cur /10000;
	DebugOut("GetCurPos pos=" << pos << std::endl);
	return rt;
}

void PASCAL CDShowPlayer::s_TimerProc(UINT timerID, UINT msg, DWORD DSPlayer, DWORD para1, DWORD para2)
{
	CDShowPlayer *player = (CDShowPlayer*)DSPlayer;
	if(player)
		player->OnTimer(timerID);
}

void CDShowPlayer::OnTimer(UINT timerID)
{
	if(timerID != m_timer_id)
		return;
	
	m_timer_id = 0;

	//if(m_play_result)
	//	m_play_result->OnPlayResult(0);

	//for fix a assert prompt, for timeSetEvent will callback in different thread.
	::SendMessage(m_play_result->m_hWnd, MM_MCINOTIFY, MCI_NOTIFY_SUCCESSFUL, 0);
}

long CDShowPlayer::Play(unsigned long fromMS, unsigned long toMs, bool forceSeeking)
{
	HRESULT rt = 0;

	if(toMs == 0)
		toMs = m_media_file.m_lengthMS;
	if(fromMS < m_rangeStart)
		fromMS = m_rangeStart;
	if(toMs > m_rangeEnd)
		toMs = m_rangeEnd;

	m_start_pos = fromMS;
	m_end_pos = toMs;

	if(forceSeeking){
		LONGLONG cur = fromMS, stop = toMs;
		cur *= 10000;
		//stop *= 10000;
		stop = m_rangeEnd * 1000;
		rt = m_media_seeking->SetPositions(&cur, AM_SEEKING_AbsolutePositioning, &stop, AM_SEEKING_AbsolutePositioning);
		if(rt != S_OK){
			DebugOut("SetPos failed: cur " << cur << ", stop " << stop << "rt=" << rt << std::endl);
			return rt;
		}
	}

	if(m_timer_id != 0){
		timeKillEvent(m_timer_id);
		m_timer_id = 0;
	}

	m_timer_id = timeSetEvent((toMs-fromMS)/m_speedRatio, 1, CDShowPlayer::s_TimerProc, (DWORD)this, TIME_ONESHOT);
	
	rt = m_media_control->Run();
	if(rt != S_OK){
		DebugOut("Run failed: " << "rt=" << rt << std::endl);
	}
	m_media_file.m_state = FS_PLAYING;
	
	DebugOut("Play from " << fromMS << ", to " << toMs << " force=" << forceSeeking << std::endl);
	return rt;
}

long CDShowPlayer::Speed(double ratio)
{
	if(ratio == 0)
		return -1;

	HRESULT rt = 0;

	rt = m_media_seeking->SetRate(ratio);
	if(rt != S_OK){
		DebugOut("SetRate failed: ratio=" << ratio << "rt=" << rt << std::endl);
	}
	m_speedRatio = ratio;

	unsigned long pos;
	GetCurPos(pos);
	Play(pos, 0, true);

	DebugOut("Speed ratio= " << ratio << std::endl);

	return rt;
}

long CDShowPlayer::Pause()
{
	HRESULT rt = m_media_control->Pause();
	if(rt != S_OK){
		DebugOut("Pause failed: " << "rt=" << rt << std::endl);
	}

	LONGLONG cur = 0;
	rt = m_media_seeking->GetCurrentPosition(&cur);
	if(rt != S_OK){
		MessageBox(NULL, "DShow获取媒体位置失败！", "菩提字幕", MB_OK);
	}
	m_start_pos = cur / 10000;
	//ASSERT(m_start_pos <= m_end_pos);
	
	if(m_timer_id){
		timeKillEvent(m_timer_id);
		m_timer_id = 0;
	}
	
	m_media_file.m_state = FS_PAUSED;
	DebugOut("Paused. " << std::endl);
	return rt;
}

long CDShowPlayer::Resume()
{
	if(m_timer_id != 0){
		timeKillEvent(m_timer_id);
		m_timer_id = 0;
	}

	m_timer_id = timeSetEvent(m_end_pos-m_start_pos, 1, CDShowPlayer::s_TimerProc, (DWORD)this, TIME_ONESHOT);

	HRESULT rt = m_media_control->Run();
	if(rt != S_OK){
		DebugOut("Resume. " << std::endl);
	}

	m_media_file.m_state = FS_PLAYING;
	DebugOut("Resume. " << std::endl);
	return rt;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// MCI PLAYER
//////////////////////////////////////////////////////////////////////////////////////////////////////
void PASCAL MCIPlayer_TimerProc(UINT timerID, UINT msg, DWORD mciPlayer, DWORD para1, DWORD para2)
{
	CMCIPlayer *player = (CMCIPlayer*)mciPlayer;
	if(player->m_retry){
		player->Play2();
	}else{
		; //player->GetView()->OnPlayResult(0);
	}
}

CMCIPlayer::CMCIPlayer(CZimuView* view)
:m_timerID(0)
,m_view(view)
,m_retry(false)
{

}

CMCIPlayer::~CMCIPlayer()
{

}

int CMCIPlayer::Open(const string &file, HWND callbackHandle)
{
	Close();

	MCIERROR rt;
	char buf[128] = {0};
	MCI_OPEN_PARMS param;
	param.dwCallback = m_curFile.m_callback;
	param.lpstrDeviceType = "mpegvideo";
	param.lpstrElementName = (char*)file.c_str();

	rt = mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD)&param);
	if(rt){
		mciGetErrorString(rt, buf, sizeof(buf));
		m_curFile.m_lastError.assign(buf);
		return rt;
	}else{
		m_curFile.m_lastError.assign(g_strNoError);
	}

	m_curFile.m_deviceId = param.wDeviceID;
	m_curFile.m_filePath = file;
	m_curFile.m_state = FS_OPEN;
	m_curFile.m_callback = (DWORD)callbackHandle;
	
	QueryStatus(MCI_STATUS_LENGTH, m_curFile.m_lengthMS);

	return 0;
}

int CMCIPlayer::Close()
{
	MCIERROR rt;
	char buf[128] = {0};
	rt = mciSendCommand(m_curFile.m_deviceId, MCI_CLOSE, 0, 0);
	if(rt){
		mciGetErrorString(rt, buf, sizeof(buf));
		m_curFile.m_lastError.assign(buf);
	}else{
		m_curFile.m_lastError.assign(g_strNoError);
	}
	m_curFile.m_state = FS_CLOSED;
	return 0;
}

int CMCIPlayer::Play2()
{
	return Play(m_from, m_to);
}

int CMCIPlayer::Play(int from, int to, bool repeat)
{
//	if(m_curFile.m_state != FS_OPEN || m_curFile.m_state != FS_PLAYING)
//		return -1;
	
	MCIERROR rt = 0;
	char buf[128] = {0};
	MCI_PLAY_PARMS param;
	param.dwCallback = m_curFile.m_callback;
	param.dwFrom = from;
	param.dwTo = to;
	DWORD flag = MCI_NOTIFY | MCI_FROM;
	if(to && repeat)
		flag |= MCI_TO;
	if(FS_PLAYING != m_curFile.m_state)
		rt = mciSendCommand(m_curFile.m_deviceId, MCI_PLAY, flag, (DWORD)&param);
	if(rt){
//		m_retry = (rt == 303);
//		if(m_retry){
//			m_from = from;
//			m_to = to;
//			m_timerID = timeSetEvent(5, 1, MCIPlayer_TimerProc, (DWORD)this, TIME_ONESHOT); //try 5 milliseconds later
//		}
		mciGetErrorString(rt, buf, sizeof(buf));
		m_curFile.m_lastError.assign(buf);
		return rt;
	}else{
		m_curFile.m_lastError.assign(g_strNoError);

		if(!repeat)
			m_timerID = timeSetEvent(to - from, 1, MCIPlayer_TimerProc, (DWORD)this, TIME_ONESHOT);
	}
	m_curFile.m_state = FS_PLAYING;
	return rt;
}

int CMCIPlayer::Pause()
{
	MCIERROR rt;
	char buf[128] = {0};
	rt = mciSendCommand(m_curFile.m_deviceId, MCI_PAUSE, 0, 0);
	if(rt){
		mciGetErrorString(rt, buf, sizeof(buf));
		m_curFile.m_lastError.assign(buf);
		return rt;
	}else{
		m_curFile.m_lastError.assign(g_strNoError);
	}
	m_curFile.m_state = FS_PAUSED;
//	timeKillEvent(m_timerID);
	return rt;
}

int CMCIPlayer::Resume()
{
	MCIERROR rt;
	char buf[128] = {0};
	rt = mciSendCommand(m_curFile.m_deviceId, MCI_RESUME, 0, 0);
	if(rt){
		mciGetErrorString(rt, buf, sizeof(buf));
		m_curFile.m_lastError.assign(buf);
		return rt;
	}else{
		m_curFile.m_lastError.assign(g_strNoError);
	}
	m_curFile.m_state = FS_PLAYING;
	return rt;
}

int CMCIPlayer::Seek(int posMS)
{

//	return Play(posMS);

	MCIERROR rt;
	MCI_SEEK_PARMS param;
	param.dwTo = posMS;
	char buf[128] = {0};
	rt = mciSendCommand(m_curFile.m_deviceId, MCI_SEEK, MCI_TO, (DWORD)&param);
	if(rt){
		mciGetErrorString(rt, buf, sizeof(buf));
		m_curFile.m_lastError.assign(buf);
		return rt;
	}
	m_curFile.m_state = FS_PLAYING;
	return rt;
}

int CMCIPlayer::Stop()
{
	MCIERROR rt;
	char buf[128] = {0};
	rt = mciSendCommand(m_curFile.m_deviceId, MCI_STOP, 0, 0);
	if(rt){
		mciGetErrorString(rt, buf, sizeof(buf));
		m_curFile.m_lastError.assign(buf);
		return rt;
	}else{
		m_curFile.m_lastError.assign(g_strNoError);
	}
	m_curFile.m_state = FS_OPEN;
//	timeKillEvent(m_timerID);
	
	Seek(0);
	return rt;
}

int CMCIPlayer::GetCurPos(long &pos)
{
	if(m_curFile.m_state == FS_CLOSED)
		return -1;
	return QueryStatus(MCI_STATUS_POSITION, pos);
}

int CMCIPlayer::QueryStatus(DWORD item, LONG &value)
{
	MCIERROR rt;
	char buf[128] = {0};
	MCI_STATUS_PARMS param;
	param.dwItem = item;
	rt = mciSendCommand(m_curFile.m_deviceId, MCI_STATUS, MCI_WAIT | MCI_STATUS_ITEM, (DWORD)&param);
	if(rt){
		mciGetErrorString(rt, buf, sizeof(buf));
		m_curFile.m_lastError.assign(buf);
	}else{
		m_curFile.m_lastError.assign(g_strNoError);
	}
	value = param.dwReturn;
	return rt;
}

string BeatyDisplay(int num, int width, bool lt)
{
	stringstream ss;
	ss << num;
	string s = ss.str();
	while(s.length() < width)
		s = "0" + s;
	if(!lt)
		return s.substr(0, width);
	else
		return s;
}

string TimeMS2HMS(int timeMS)
{
	stringstream ss;
	int sec = timeMS / 1000;
	int minute = (sec % 3600)/60;
	//if (sec > 3600)
		ss << BeatyDisplay(sec/3600, 2) << ":";
	ss << BeatyDisplay(minute, 2) << ":" << BeatyDisplay(sec%60, 2);
	ss << "," << BeatyDisplay(timeMS%1000, 3);
	return ss.str();
}


string TimeMS2Str(int timeMS)
{
	stringstream ss;
	int sec = timeMS / 1000;
	int minute = (sec % 3600)/60;
	if (sec > 3600)
		ss << sec/3600 << ":";
	ss << BeatyDisplay(minute, 2) << ":" << BeatyDisplay(sec%60, 2);
	ss << "," << BeatyDisplay(timeMS%1000, 3);
	return ss.str();
}

string TimeMS2HMS2(int timeMS)
{
	stringstream ss;
	int sec = timeMS/1000;
	int minute = sec /60;
	ss << BeatyDisplay(minute, 2, true) << ":" << BeatyDisplay(sec%60, 2);
	ss << "." << BeatyDisplay(timeMS%1000, 2);
	return ss.str();
}


#include "ZimuView.h"

//////////////////////////////////////////////////////////////////////
// DShowPlayer
//////////////////////////////////////////////////////////////////////
MediaPlayer::MediaPlayer(PlayerListener *listener)
	: m_listener(listener)
	, m_duration(0)
	, m_state(MPS_CLOSED)
{
}

MediaPlayer::~MediaPlayer()
{

}

int DShowPlayer::m_instance_count = 0;

DShowPlayer* DShowPlayer::CreateInstance(PlayerListener *listener)
{
	DShowPlayer *inst = new DShowPlayer(listener);
	if(inst->Init() != 0)
	{
		delete inst;
		return NULL;
	}
	return inst;
}

DShowPlayer::DShowPlayer(PlayerListener *listener)
	:MediaPlayer(listener)
	,m_graph_builder(NULL)
	,m_media_control(NULL)
	,m_media_seeking(NULL)
	,m_media_event(NULL)
	,m_seg_start(0)
	,m_seg_end(0)
	,m_timer_id(0)
{
	m_instance_count++;

	if(m_instance_count == 1){ //the first instance.
		// initialize COM
		//
		// initialize the COM library on the current thread and identifies the concurrency model 
		// as single-thread apartment (STA).
		CoInitialize(0);
	}
}

DShowPlayer::~DShowPlayer()
{
	Uninit();

	m_instance_count--;
	if(m_instance_count == 0)
		CoUninitialize();
}


long DShowPlayer::Init()
{
	// create the DirectMusic performance object
	//
	// creates a single uninitialized object of the class associated with a specified CLSID.
	if(FAILED(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, 
		(void**)&m_graph_builder)))
	{
		MessageBox(NULL, "Unable to create DirectShow Graph Builder object.", "Error", MB_OK);
		return -1;
	}

	// Query for the media control and event objects
	ASSERT(m_graph_builder);
	if(m_graph_builder){
		m_graph_builder->QueryInterface(IID_IMediaControl, (void**)&m_media_control);
		m_graph_builder->QueryInterface(IID_IMediaSeeking, (void**)&m_media_seeking);
		m_graph_builder->QueryInterface(IID_IMediaEventEx, (void**)&m_media_event);

		if(m_media_control && m_media_seeking && m_media_event)
			;
		else
			return -1;
	}else{
		return -1;
	}
	//	Play_MP3("F:\\Musics\\Music\\dxh.mp3");
	return 0;
}

void DShowPlayer::Uninit()
{
	if(m_media_control)
		m_media_control->Stop();

	if(m_media_event){
		m_media_event->Release();
		m_media_event = NULL;
	}

	if(m_media_control){
		m_media_control->Release();
		m_media_control = NULL;
	}
	if(m_media_seeking){
		m_media_seeking->Release();
		m_media_seeking = NULL;
	}
	if(m_graph_builder){
		m_graph_builder->Release();
		m_graph_builder = NULL;
	}
}

long DShowPlayer::ConnectFilter(IBaseFilter *upper, IBaseFilter *down, LPCWSTR pinNameOut, LPCWSTR pinNameIn)
{
	HRESULT rt = S_OK;
	IPin *inPin = NULL, *outPin = NULL;
	rt = upper->FindPin(pinNameOut, &outPin);
	if (rt != S_OK)
		goto FAILED;
	rt = down->FindPin(pinNameIn, &inPin);
	if (rt != S_OK)
		goto FAILED;
	rt = m_graph_builder->Connect(outPin, inPin);
	if (rt != S_OK)
		goto FAILED;
FAILED:
	if (inPin)
		inPin->Release();
	if (outPin)
		outPin->Release();
	return rt;
}

long DShowPlayer::CreateFilter(CLSID &clsid, IBaseFilter **filter)
{
	HRESULT rt = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<void**>(filter));
	if (rt != S_OK)
		return rt;
	return m_graph_builder->AddFilter(*filter, NULL);
}

long DShowPlayer::BuildGraph(const WCHAR *filename)
{
	LPCWSTR filterName = L"File Source (Async.)";
	IPin *inPin, *outPin;
	IBaseFilter *srcFilter = NULL;
	HRESULT rt = m_graph_builder->AddSourceFilter(filename, filterName, &srcFilter);
	if (rt != S_OK)
		return rt;
	rt = srcFilter->FindPin(L"Output", &outPin);
	if (rt != S_OK)
		return rt;

	//IBaseFilter *splitFilter = NULL;
	//CLSID CLSID_AVSplitter = {0x529A00DB, 0x0C43, 0x4F5B, {0x8E,0xF2,0x05,0x00,0x4C,0xBE,0x0C,0x6F}}; 
	//rt = CreateFilter(CLSID_AVSplitter, &splitFilter);
	//if (rt != S_OK)
	//	return rt;
	//rt = ConnectFilter(srcFilter, splitFilter, L"Output", L"Input");
	//if (rt != S_OK)
	//	return rt;

	IBaseFilter *decodeFilter = NULL;
	CLSID CLSID_AudioDecoder = {0x0F40E1E5, 0x4F79, 0x4988, {0xB1,0xA9,0xCC,0x98,0x79,0x4E,0x6B,0x55}};
	rt = CreateFilter(CLSID_AudioDecoder, &decodeFilter);
	if (rt != S_OK)
		return rt;
	rt = ConnectFilter(srcFilter, decodeFilter, L"Output", L"In");
	if (rt != S_OK)
		return rt;

	//IBaseFilter *renderFilter = NULL;
	//CLSID CLSID_AudioRender = {0x79376820, 0x07D0, 0x11CF, {0xA2,0x4D,0x00,0x20,0xAF,0xD7,0x97,0x67}};
	//rt = CreateFilter(CLSID_AudioRender, &renderFilter);
	//if (rt != S_OK)
	//	return rt;
	//rt = ConnectFilter(decodeFilter, renderFilter, L"Out", L"Audio Input pin (rendered)");
	//if (rt != S_OK)
	//	return rt;
	rt = decodeFilter->FindPin(L"Out", &outPin);
	if (rt != S_OK)
		return rt;

	rt = m_graph_builder->Render(outPin);
	outPin->Release();
	return rt;
}

long DShowPlayer::Open(const TString &filename)
{
	if(!m_graph_builder && Init() != 0)
		return -1;

	m_media_file.Reset();
	m_media_file.m_filePath = filename;
	m_media_file.m_state = FS_OPEN;
	m_state = MPS_OPEN;

	WCHAR w_filename[MAX_PATH] = {0};
	MultiByteToWideChar(CP_ACP, MB_COMPOSITE, filename.c_str(), -1, w_filename, MAX_PATH);
	//mbstowcs(w_filename, filename, MAX_PATH);

	// render the file
	//HRESULT rt = m_graph_builder->RenderFile(w_filename, NULL);
	HRESULT rt = BuildGraph(w_filename);
	if(rt != S_OK){
		return rt;
	}

	GUID timeFormat = TIME_FORMAT_MEDIA_TIME;
	rt = m_media_seeking->SetTimeFormat(&timeFormat);
	if(rt != S_OK){
		return rt;
	}

	LONGLONG duration;
	rt = m_media_seeking->GetDuration(&duration);
	if(rt != S_OK){
		return rt;
	}
	m_duration = 
	m_media_file.m_lengthMS = duration / 10000;

	return rt;
}

void DShowPlayer::Close()
{
	Uninit();

	m_state = MPS_CLOSED;
	m_media_file.Reset();
}

long DShowPlayer::Play(long seglen)
{
	m_state = MPS_PLAYING;
	m_media_file.m_state = FS_PLAYING;

	HRESULT rt = m_media_control->Run();
	if(rt != S_OK)
	{
		DebugOut("IMediaControl::Run failed: " << "rt=" << rt << std::endl);
		//return rt;
	}

	m_seg_start = GetCurrentPos();
	m_seg_end = m_seg_start + seglen;

	if(m_timer_id){
		timeKillEvent(m_timer_id);
		m_timer_id = 0;
	}
	m_timer_id = timeSetEvent(seglen, 0, DShowPlayer::s_TimerProc, (DWORD)this, TIME_ONESHOT);

	return rt;
}

long DShowPlayer::Pause()
{
	if(IsPaused())
		return 0;

	m_state = MPS_PAUSED;
	m_media_file.m_state = FS_PAUSED;

	HRESULT rt = m_media_control->Pause();
	if(rt != S_OK){
		DebugOut("IMediaControl::Pause failed: " << "rt=" << rt << std::endl);
		//return rt;
	}

	m_seg_start = GetCurrentPos();
	//ASSERT(m_seg_start <= m_seg_end);

	if(m_timer_id){
		timeKillEvent(m_timer_id);
		m_timer_id = 0;
	}

	DebugOut("Paused. " << std::endl);
	return rt;
}

long DShowPlayer::Resume()
{
	ASSERT(IsPaused());
	ASSERT(m_timer_id == 0);

	HRESULT rt = m_media_control->Run();
	if(rt != S_OK){
		DebugOut("IMediaControl::Run failed: " << "rt=" << rt << std::endl);
		//return rt;
	}

	if(m_timer_id != 0){
		timeKillEvent(m_timer_id);
		m_timer_id = 0;
	}
	m_timer_id = timeSetEvent(m_seg_end-m_seg_start, 0, DShowPlayer::s_TimerProc, (DWORD)this, TIME_ONESHOT);

	m_media_file.m_state = FS_PLAYING;
	m_state = MPS_PLAYING;

	DebugOut("Resume. " << std::endl);
	return rt;
}

long DShowPlayer::Stop()
{
	HRESULT rt;
	rt = m_media_control->Stop();
	if(rt != S_OK){
		DebugOut("IMediaControl::Stop failed." << std::endl);
		TRACE0("DShowPlayer, Stop failed!");
	}

	timeKillEvent(m_timer_id);
	m_timer_id = 0;

	m_state = MPS_STOPPED;

	DebugOut("Stop " << std::endl);
	return rt;
}

long DShowPlayer::Seek(long pos)
{
	//ASSERT(m_timer_id == 0);
	if(m_timer_id)
	{
		timeKillEvent(m_timer_id);
		m_timer_id = 0;
	}

	HRESULT rt;
	LONGLONG start = pos,
		stop = m_duration;
	start *= 10000;
	stop *= 10000;
	rt = m_media_seeking->SetPositions(&start, AM_SEEKING_AbsolutePositioning, &stop, AM_SEEKING_AbsolutePositioning);
	if(rt != S_OK){
		DebugOut("IMediaSeek::SetPositions failed: start=" << start << ", stop=" << stop << ", rt=" << rt << std::endl);
	}

	return rt;
}

long DShowPlayer::GetCurrentPos() const
{
	LONGLONG cur = 0;
	HRESULT rt = m_media_seeking->GetCurrentPosition(&cur);
	if(rt != S_OK){
		DebugOut("IMediaSeek::GetCurrentPosition failed rt=" << rt << std::endl);
		MessageBox(NULL, "DShow获取媒体位置失败！", "菩提字幕", MB_OK);
	}

	return cur / 10000;
}

long DShowPlayer::GetMediaDuration() const
{
	return m_duration;
}

void DShowPlayer::GetLastError(string &err) const
{
	err.assign(m_media_file.m_lastError);
}

void PASCAL DShowPlayer::s_TimerProc(UINT timerID, UINT msg, DWORD DSPlayer, DWORD para1, DWORD para2)
{
	DShowPlayer *player = (DShowPlayer*)DSPlayer;
	if(player)
		player->OnTimer(timerID);
}

void DShowPlayer::OnTimer(UINT timerID)
{
	if(m_listener)
		m_listener->OnPlay(0);
}