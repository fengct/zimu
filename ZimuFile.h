#pragma once


struct TimePoint
{
	long m_msVal;
	string m_dispVal;
public:
	TimePoint(long tmMs = 0) : m_msVal(tmMs), m_dispVal(TimeMS2HMS(tmMs)) {}
	void SetTime(long tmMs) { m_msVal = tmMs; m_dispVal = TimeMS2HMS(tmMs); }
	bool operator <(const TimePoint& rhs) const { return m_msVal < rhs.m_msVal; }
	bool operator ==(const TimePoint& rhs) const { return m_msVal == rhs.m_msVal; }
	bool operator <=(const TimePoint& rhs) const { return m_msVal <= rhs.m_msVal; }
	bool Offset(long tmMs) { long ms = m_msVal + tmMs; if(ms < 0) return false; SetTime(ms); return true;}
};

struct FindPos
{
	int lineIdx; //from 0
	int charIdx; //from 0
public:
	FindPos() : lineIdx(0), charIdx(0) {}
};

struct ReviseRecord
{
	time_t m_time;
	string m_name;
	string m_contents;
};
typedef std::list<ReviseRecord> ReviseRecords;

enum { SM_TIME_ERROR = 0x01, 
	SM_TIME_MODIFIED1 = 0x02, 
	SM_TIME_MODIFIED2 = 0x04, 
	SM_TIME_LENGTH = 0x08, 
	SM_TIME_MARK = SM_TIME_ERROR | SM_TIME_LENGTH,
	SM_CONTENT_LENGTH = 0x10 
};


const int MAX_CHINESE_CHARACTERS = 12;
const int MAX_TOTAL_BYTES = 48+2; //one "/r/n"
const int TIME_DELTA = 200; //ms

struct Sentence{
	int m_seq;
	TimePoint m_startTime;
	TimePoint m_endTime;
	unsigned long m_mark;
	string m_content;
	ReviseRecords m_reviseRecords;
	static const string m_initialText;
public: 
	Sentence():m_seq(0), m_mark(0) {}
	void Update(const Sentence& other);
};
typedef std::list<Sentence> SentenceList;

class IRefreshable
{
public:
	virtual ~IRefreshable() {}

	virtual void Refresh() = 0;
};

class CZimuFile
{
	enum FileSaveType{
		FT_TEXT,
		FT_SRT,
		FT_REV,
		FT_LRC
	};

public:
	typedef std::list<Sentence> ContentsType;
public:
	CZimuFile();
	~CZimuFile();

	void InsertSentence(int pos, Sentence &date);
	void AppendSentence(Sentence &date);
	void UpdateSentence(Sentence &st);
	void ModifyTime(int seq, bool startTime, int delta);
	void RemoveSentence(int seq);

	int Open(const char *path, bool source);
	void Close();
	void Save(const char* path = NULL);
	void Save_w(const char* path = NULL, FileSaveType type = FT_SRT);
	void SaveToText(const char* path = NULL);
	void SaveToLrc(const char* path = NULL);
	void SaveAsReviseFile(const char* path = NULL);
	void SaveAsSrtFile(const char* path = NULL);

	const ContentsType& GetContents() const { return m_contents; }
	Sentence* GetSentence(int seq);
	int GetItemCount() const { return m_contents.size();}

	string GetSourcePath() const { return m_sourceName; }
	string GetDestPath() const { return m_destName; }
	bool IsUpdated() const { return m_updated; }
	void SetUpdated(bool updated) { m_updated = updated; }
	bool CheckContents(bool prompt, bool autoFix, IRefreshable *view);
	void ConvertToSingleLine();
	void ConvertToSingleLine(const string& src, string &dst);

	int Find(const char* target, const FindPos & startPos, FindPos &nextPos);
	int Replace(const FindPos &pos, const string &target, const string &replacer);
	int ReplaceAll(const string &target, const string &replacer);
	void ClearContents() { m_contents.clear(); m_updated = false;}

	int ClearReviseRecord();
	bool HasRevise() const { return m_withRevise; }
	void ClearTimeModifyMark();
	int NextMarkSentence(DWORD mark, int startPos = 0, Sentence *st = NULL);
	int NextTimeGap(int startPos = 0);
	//temp
	int TimeGapFill();
	int OffsetTime(int offset);
	bool CheckContentLength(Sentence &st);
	void Trim(string &text);
protected:
	int WriteBuf(const Sentence& st, char* buf, int size);
	int Streaming(const Sentence& st, std::string &out);
	int StreamingToLrc(const Sentence& st, std::string &out);
	int WriteLrcHeader(FILE *ofs);
	int ReadSRT(const char* path);
	int ReadSRT_W(const char* path);
	int GetNumer(const char* buf, int len);
	long GetTime(const char* buf, int len);
	int ParseBuffer(const char* buf, int len, int &left);
	int ParseContent(string &text, Sentence &st);
	int DoReadFile(FILE *file, char* buf, int bufLen);
private:
	string m_sourceName;
	string m_destName;
	ContentsType m_contents;
	bool m_updated;
	bool m_withRevise;

	FileSaveType m_fileType;
};


class CTempBuffer
{
	char *m_data;
	DWORD m_len;
	char m_buf[256];
public:
	CTempBuffer(DWORD sz = 256) 
		: m_data(sz <= sizeof(m_buf) ? &m_buf[0] : new char[sz])
		, m_len(sz)
	{
		//Reset();
	}

	~CTempBuffer()
	{
		if(m_data != &m_buf[0])
			delete [] m_data;
	}

	char* Data() { return m_data; }
	DWORD Size() { return m_len; }
	void Reset() { memset(m_data, 0, m_len); }
};
