#include "stdafx.h"
#include "ZimuView.h"


const string Sentence::m_initialText = "双击以编辑";

void Sentence::Update(const Sentence& other)
{
	m_endTime = other.m_endTime;
	m_content = other.m_content;
	if(m_content == "")
		m_content = m_initialText;
}

CZimuFile::CZimuFile()
:m_updated(false)
,m_withRevise(false)
,m_fileType(FT_REV)
{
}

CZimuFile::~CZimuFile()
{
	Close();
}

int CZimuFile::Open(const char *path, bool source)
{
	if(!path)
		return -1;

	m_contents.clear();

	if(source){
		m_sourceName = path;
		m_destName = m_sourceName.substr(0, m_sourceName.find_last_of("."));
		m_destName += ".srt";
	}else{ //import srt.
		m_destName = path;
		m_sourceName = m_destName.substr(0, m_destName.find_last_of("."));
		m_sourceName += ".mp3";
	}

	int rt = ReadSRT_W(m_destName.c_str());
	if(rt != 0)
		return rt;
	else if(!CheckContents(true, true, NULL))
		return 1;
	else
		return 0;
}

void CZimuFile::Close()
{
	m_sourceName.assign("");
	m_destName.assign("");
	m_updated = false;
	m_withRevise = false;
	m_contents.clear();
	m_fileType = FT_SRT;
}

void CZimuFile::InsertSentence(int pos, Sentence &date)
{
	if(pos < 0 || pos > m_contents.size())
		return;

	int i = 1;
	ContentsType::iterator it = m_contents.begin(), it2;
	while(i <= pos){
		i++;
		it++;
	}
	it2 = it;

	while(it != m_contents.end()){
		Sentence &st = *it;
		st.m_seq += 1;
		it++;
	}

	//	for(; it != m_contents.end(); ++i, ++it){
	//		if(i <= pos){
	//			it2 = it;
	//			continue;
	//		}
	//		
	//		Sentence &st = *it;
	//		st.m_seq += 1;
	// 	}

	date.m_seq = pos+1;
	m_contents.insert(it2, date);
	m_updated = true;
}

void CZimuFile::AppendSentence(Sentence &date)
{
	m_contents.push_back(date);
	m_updated = true;
}


void CZimuFile::UpdateSentence(Sentence &st)
{
	ContentsType::iterator it = m_contents.begin();
	for(; it != m_contents.end(); it++){
		Sentence &curSt = *it;
		if(curSt.m_seq == st.m_seq){
			curSt.Update(st);
			break;
		}
	}

	m_updated = true;
}

void CZimuFile::ModifyTime(int seq, bool startTime, int delta)
{
	Sentence *st = GetSentence(seq);
	if(!st)
		return;

	TimePoint &time = startTime ? st->m_startTime : st->m_endTime;
	long timeMs = time.m_msVal;
	timeMs += delta;
	if(timeMs < 0)
		timeMs = 0;
	if(startTime){
		Sentence *stPre = GetSentence(seq - 1);
		if(stPre && timeMs < stPre->m_endTime.m_msVal)
			timeMs = stPre->m_endTime.m_msVal;
		st->m_mark |= SM_TIME_MODIFIED1;
		st->m_mark &= ~SM_TIME_ERROR;
	}else{
		Sentence *nextSt = GetSentence(seq + 1);
		if(nextSt && timeMs > nextSt->m_startTime.m_msVal)
			timeMs = nextSt->m_startTime.m_msVal;
		st->m_mark |= SM_TIME_MODIFIED2;
		st->m_mark &= ~SM_TIME_ERROR;
	}
	time.SetTime(timeMs);

	m_updated = true;
}

void CZimuFile::ClearTimeModifyMark()
{
	ContentsType::iterator it = m_contents.begin();
	for(; it != m_contents.end(); it++){
		Sentence &curSt = *it;
		curSt.m_mark &= ~(SM_TIME_MODIFIED1 | SM_TIME_MODIFIED2);
	}
}

int CZimuFile::NextMarkSentence(DWORD mark, int startPos, Sentence *st)
{
	ContentsType::iterator it = m_contents.begin();
	for(; it != m_contents.end(); it++){
		Sentence &curSt = *it;
		if(curSt.m_seq < startPos)
			continue;
		if(curSt.m_mark & mark){
			if(st)
				st = &*it;
			return curSt.m_seq;
		}
	}
	return -1;
}

int CZimuFile::NextTimeGap(int startPos)
{
	ContentsType::iterator it = m_contents.begin();
	unsigned long lastTime = 0;
	for(; it != m_contents.end(); it++){
		Sentence &curSt = *it;
		if(curSt.m_seq < startPos){
			lastTime = curSt.m_endTime.m_msVal;
			continue;
		}

		if(curSt.m_startTime.m_msVal != lastTime){
			return curSt.m_seq;
		}
		lastTime = curSt.m_endTime.m_msVal;
	}
	return -1;
}

void CZimuFile::RemoveSentence(int seq)
{
	m_updated = true;

	if(seq == m_contents.size()){
		m_contents.pop_back();
		return;
	}

	int i = 1;
	ContentsType::iterator it = m_contents.begin(), it2;
	while(i < seq){
		i++;
		it++;
	}

	it2 = it;
	while(it != m_contents.end()){
		it->m_seq--;
		it++;
	}

	m_contents.erase(it2);
}

void CZimuFile::Save(const char* path)
{
	if(path == NULL)
		path = m_destName.c_str();
	if(strlen(path) == 0)
		return;

	m_updated = false;

	Save_w(path);
	return;

	ofstream ofs(path, std::ios::binary);
	if(!ofs){
		CString msg = "can't open file: ";
		msg += path;
		msg += " for write!";
		AfxMessageBox(msg);
		return;
	}

	const char* lb = "\r\n";
	ContentsType::iterator it = m_contents.begin();
	for(; it != m_contents.end(); it++){
		Sentence &s = *it;
		ofs << s.m_seq << lb;
		ofs << s.m_startTime.m_dispVal << " --> " << s.m_endTime.m_dispVal << lb;
		ofs << s.m_content << lb << lb;
	}

}

void CZimuFile::SaveToLrc(const char* path)
{
	Save_w(path, FT_LRC);
}

int Ascii2Unicode(const string& ascii, char *&unicode, int &utfLen)
{
	const int BUFLEN = 1024;
	char /*buf[BUFLEN], */buf2[BUFLEN*2], *utf;
	utf = buf2;
	int num1, num2, maxSize = BUFLEN*2;

	num1 = ascii.length();
	num2 = MultiByteToWideChar(CP_ACP, MB_COMPOSITE, ascii.c_str(), num1, (LPWSTR)utf, 0);
	if(num2*2 > maxSize){
		maxSize = num2*2;
		char *tmp = new char[maxSize];
		if(utf != &buf2[0]){
			delete [] utf;
		}
		utf = tmp;
	}
	MultiByteToWideChar(CP_ACP, MB_COMPOSITE, ascii.c_str(), num1, (LPWSTR)utf, num2);

	unicode = utf;
	utfLen = num2;
	return num2;
}

void CZimuFile::Save_w(const char* path, FileSaveType type)
{
	if(path == NULL){
		path = m_destName.c_str();
	}

	FILE *ofs = fopen(path, "wb");
	if(!ofs){
		CString msg = "can't open file: ";
		msg += path;
		msg += " for write!";
		AfxMessageBox(msg);
		return;
	}
	//fprintf(ofs, "%d", 65);
	unsigned short mark = 0xFEFF;
	fwrite(&mark, 2, 1, ofs);

	char *utf;
	int num1, utfLen;
	if(type == FT_LRC){
		WriteLrcHeader(ofs);
	}

	std::string line;
	ContentsType::iterator it = m_contents.begin();
	for(; it != m_contents.end(); it++){
		Sentence &st = *it;
		//num1 = WriteBuf(st, buf, BUFLEN);
		if(type == FT_LRC)
			num1 = StreamingToLrc(st, line);
		else
			num1 = Streaming(st, line);

		Ascii2Unicode(line, utf, utfLen);
		fwrite(utf, 2, utfLen, ofs);
	}

	fclose(ofs);
	m_updated = false;
}

int CZimuFile::WriteLrcHeader(FILE *ofs)
{
	const char* header[] =
	{"[ar:上师]\r\n",
	"[ti:上师瑜伽导修]\r\n",
	"[al:正法妙音]\r\n",
	"[by:道友]\r\n"
	};

	char *utf;
	int utfLen;
	for(int i = 0; i < sizeof(header)/sizeof(header[0]); i++){
		Ascii2Unicode(header[i], utf, utfLen);
		fwrite(utf, 2, utfLen, ofs);
	}

	return 0;
}

#define IsValidTime(start, end) ((start) < (end))


bool CZimuFile::CheckContents(bool prompt, bool autoFix, IRefreshable *view)
{
	if(m_contents.empty())
		return true;

	int cnt = 0;
	string content;
	Sentence *preSt, *st, *nextSt;
	preSt = nextSt = st = NULL;
	ContentsType::iterator it = m_contents.begin(), tmp;
	for(; it != m_contents.end(); it++){
		st = &*it;
		st->m_mark &= ~SM_TIME_ERROR; //clear mark first.
		tmp = it;
		tmp++;

		if(preSt && st->m_startTime < preSt->m_endTime){
			st->m_mark |= SM_TIME_ERROR;
			if(autoFix)
				st->m_startTime = preSt->m_endTime;
		}

		if(tmp != m_contents.end()){
			nextSt = &*tmp;
			if((nextSt->m_startTime < st->m_endTime )
				&& (st->m_startTime < nextSt->m_startTime)){
					st->m_mark |= SM_TIME_ERROR;
					if(autoFix)
						st->m_endTime = nextSt->m_startTime;
			}
		}

		if(!IsValidTime(st->m_startTime, st->m_endTime)){
			st->m_mark |= SM_TIME_ERROR;

			if(autoFix){
				if(preSt && (preSt->m_endTime <= st->m_startTime)
					&& (st->m_startTime.m_msVal - preSt->m_endTime.m_msVal <= 5*60*1000)){ //end time  is wrong.
						if(tmp != m_contents.end()){
							nextSt = &*tmp;
							st->m_endTime = nextSt->m_startTime; //对齐到下一条的开始时间。
						}else{
							content = "不会吧，最后一条记录的结束时间居然是错的！"; //can't happen
						}
				}else{ // start time wrong.
					if(preSt)
						st->m_startTime = preSt->m_endTime; //对齐到前一条的结束时间。
					else
						st->m_startTime = 0;
				}
			}
		}

		preSt = st;
		if(st->m_mark & SM_TIME_ERROR){
			cnt++;
		}
		/*
		if(st->m_startTime < preSt->m_endTime){
		st->m_mark |= SM_TIME_ERROR;
		preSt->m_mark |= SM_TIME_ERROR;
		err = true;
		if(autoFix){
		st->m_startTime = preSt->m_endTime;
		}
		}else if(st->m_startTime.m_msVal == preSt->m_endTime.m_msVal){
		ContentsType::iterator tmp = it;
		tmp++;
		if(tmp != m_contents.end()){
		Sentence &nextSt = *tmp;
		if(st->m_endTime <= st->m_startTime && autoFix){
		st->m_endTime = nextSt.m_startTime;
		}
		}
		}
		*/
		//	|| st.m_endTime.m_msVal <= st.m_startTime.m_msVal){
		//	st.m_startTime = last->m_endTime;
		//	err = true;
		//	content.assign(st.m_content);
		//	errSeq = st.m_seq;

		//	SetUpdated(true);
		//	if(prompt)
		//		break;
		//}
	}
	if(cnt && autoFix)
		m_updated = true;
	if(cnt && prompt){
		if(view){
			view->Refresh();
		}
		CString msg = "";
		msg.Format("有%d处发生时间错误，请自己修正！", cnt);
		int rt = AfxMessageBox(msg.GetBuffer(msg.GetLength()));
		//if(rt == IDYES){
		//}
		return false;
	}
	return (cnt == 0);
}

void CZimuFile::ConvertToSingleLine(const string& src, string &dst)
{
	dst.assign("");
	dst.reserve(src.length());
	int i, cnt = 0;
	for(i = 0; i < src.length(); i++){
		char c = src[i];
		if((c != '\r') && (c != '\n')){
			dst.append(1, c);
		}
	}
}

void CZimuFile::ConvertToSingleLine()
{
	string content;
	ContentsType::iterator it = m_contents.begin();
	for(; it != m_contents.end(); it++){
		Sentence &st = *it;
		ConvertToSingleLine(st.m_content, content);
		st.m_content.assign(content);
	}
}

int CZimuFile::Find(const char* target, const FindPos & startPos, FindPos &nextPos)
{
	if(startPos.lineIdx >= m_contents.size())
		return -1;

	int lineIdx = 0, pos = 0;
	ContentsType::iterator it = m_contents.begin();
	for(; it != m_contents.end(); it++, lineIdx++){
		if(lineIdx < startPos.lineIdx)
			continue;

		Sentence &st = *it;
		pos = st.m_content.find(target, lineIdx == startPos.lineIdx ? (startPos.charIdx+1) : 0);
		if(pos != string::npos){
			nextPos.lineIdx = lineIdx;
			nextPos.charIdx = pos;
			return lineIdx;
		}
	}
	return -1;
}

int CZimuFile::Replace(const FindPos &pos, const string &target, const string &replacer)
{
	if(pos.lineIdx >= m_contents.size())
		return -1;

	int lineIdx = 0;
	ContentsType::iterator it = m_contents.begin();
	for(; it != m_contents.end(); it++, lineIdx++){
		if(lineIdx < pos.lineIdx)
			continue;

		Sentence &st = *it;
		string &words = st.m_content;
		string tmp;
		tmp = words.substr(0, pos.charIdx);
		tmp.append(replacer);
		tmp.append(words.substr(pos.charIdx + target.length(), words.length()));
		std::swap(tmp, words);
		break;
	}

	m_updated = true;
	return 0;
}

int CZimuFile::ReplaceAll(const string &target, const string &replacer)
{
	if(target == replacer)
		return -1;

	int cnt = 0;
	ContentsType::iterator it = m_contents.begin();
	for(; it != m_contents.end(); it++){
		Sentence &st = *it;
		string &words = st.m_content;

		int pos = words.find(target);
		while(pos != string::npos){
			string tmp;
			tmp = words.substr(0, pos);
			tmp.append(replacer);
			tmp.append(words.substr(pos + target.length(), words.length()));
			std::swap(tmp, words);
			cnt++;
			pos = words.find(target);
		}
	}

	m_updated = true;
	return cnt;
}

int CZimuFile::ClearReviseRecord()
{
	ContentsType final;
	int cnt = 0;
	ContentsType::iterator it = m_contents.begin();
	for(; it != m_contents.end(); it++){
		Sentence &st = *it;
		if(st.m_reviseRecords.size())
			cnt++;
		st.m_reviseRecords.clear();

		if(st.m_content != Sentence::m_initialText){
			Sentence st2 = st;
			st2.m_seq = final.size() + 1;
			final.push_back(st2);
		}
	}

	std::swap(m_contents, final);
	if(cnt)
		m_updated = true;
	return cnt;
}

int CZimuFile::TimeGapFill()
{
	int cnt = 0;
	ContentsType::iterator it = m_contents.begin(), it2;
	for(; it != m_contents.end();){
		Sentence &st = *it;
		it2 = ++it;
		if(it2 == m_contents.end())
			break;

		Sentence &st2 = *it2;
		if(st.m_endTime.m_msVal != st2.m_startTime.m_msVal){
			st.m_endTime = st2.m_startTime;
			cnt++;
			st.m_mark &= ~SM_TIME_ERROR;
			st2.m_mark &= ~SM_TIME_ERROR;
		}
	}

	if(cnt)
		m_updated = true;
	return cnt;
}

int CZimuFile::OffsetTime(int offset)
{
	ContentsType::iterator it = m_contents.begin();
	if(it != m_contents.end()){
		Sentence &st = *it;
		if(st.m_startTime.m_msVal + offset < 0)
			return -1;
	}
	for(; it != m_contents.end(); it++){
		Sentence &st = *it;
		st.m_startTime.Offset(offset);
		st.m_endTime.Offset(offset);
	}
	return 0;
}

void regularLines(const string& input, string &output)
{
	int i = 0, sz;
	sz = input.size();
	char ch;
	for(; i < sz; i++){
		ch = input[i];
		if(ch != '\r')
			output.append(1, ch);
	}
}

//very low efficietly implimention.
void splitToLines(const string& text, std::list<string>& lines)
{
	string regStr;
	regularLines(text, regStr);

	int i = 0, sz, lineBegin;
	sz = regStr.size();
	lineBegin = i;
	unsigned char ch;
	while(i < sz){
		ch = regStr[i];
		if(ch == '\n'){
			string line(regStr, lineBegin, i-lineBegin);
			lines.push_back(line);
			lineBegin = i+1;
		}
		i++;
	}
	string line(regStr, lineBegin, i);
	lines.push_back(line);
}

bool CZimuFile::CheckContentLength(Sentence &st)
{
	st.m_mark &= ~SM_CONTENT_LENGTH;

	const string &text = st.m_content;
	if(text.size() > MAX_TOTAL_BYTES){
		st.m_mark |= SM_CONTENT_LENGTH;
		return true;
	}

	std::list<string> lines;
	splitToLines(text, lines);
	if(lines.size() > 2){
		st.m_mark |= SM_CONTENT_LENGTH;
		return true;
	}

	list<string>::iterator it = lines.begin();
	for(; it != lines.end(); it++){
		string &line = *it;
		if(line.size() > MAX_CHINESE_CHARACTERS*2){
			st.m_mark |= SM_CONTENT_LENGTH;
			return true;
		}
	}
	return false;
	// 	st.m_mark = (text.size() > MAX_TOTAL_BYTES) ? SM_CONTENT_LENGTH : 0;
	// 	int len1 = text.find("\n");
	// 	if(len1 == string::npos)
	// 		len1 = 0;
	// 	if((len1 > 0) && (text[len1-1] == '\r'))
	// 		len1--;
	// 	if(len1 > MAX_CHINESE_CHARACTERS*2 || (text.size() - len1) > MAX_CHINESE_CHARACTERS*2)
	// 		st.m_mark |= SM_CONTENT_LENGTH;
	//	return (st.m_mark & SM_CONTENT_LENGTH) == 0;
}

void CZimuFile::Trim(string &text)
{
	int p1, p2, len;
	p1 = 0;
	len = text.size();
	p2 = len;
	unsigned char ch;
	while(p1 < len){
		ch = text.at(p1);
		if((ch < 0x80) && isspace(ch))
			p1++;
		else
			break;
	}

	while(p2-- > p1){
		ch = text.at(p2);
		if((ch >= 0x80) || !isspace(ch))
			break;
	}

	string tmp = text.substr(p1, p2-p1+1);
	std::swap(tmp, text);
}

void CZimuFile::SaveToText(const char* path)
{
	if(path == NULL){
		string tmp = m_destName.substr(0, m_destName.size()-3);
		tmp.append(".txt");
	}

	FILE *ofs = fopen(path, "wb");
	if(!ofs){
		CString msg = "can't open file: ";
		msg += path;
		msg += " for write!";
		AfxMessageBox(msg);
		return;
	}
	//fprintf(ofs, "%d", 65);
	unsigned short mark = 0xFEFF;
	fwrite(&mark, 2, 1, ofs);

	const int BUFLEN = 1024;
	char buf[BUFLEN], buf2[BUFLEN*2], *utf;
	utf = buf2;
	int num1, num2, maxSize = BUFLEN*2;
	ContentsType::iterator it = m_contents.begin();
	for(; it != m_contents.end(); it++){
		Sentence &st = *it;
		num1 = sprintf(buf, "%s。", st.m_content.c_str());
		num2 = MultiByteToWideChar(CP_ACP, MB_COMPOSITE, buf, num1, (LPWSTR)utf, 0);
		if(num2*2 > maxSize){
			maxSize = num2*2;
			char *tmp = new char[maxSize];
			if(utf != &buf2[0]){
				delete [] utf;
			}
			utf = tmp;
		}
		MultiByteToWideChar(CP_ACP, MB_COMPOSITE, buf, num1, (LPWSTR)utf, num2);
		fwrite(utf, 2, num2, ofs);
	}

	fclose(ofs);

	m_updated = false;
}

void CZimuFile::SaveAsReviseFile(const char* path)
{
	m_fileType = FT_REV;
	Save_w(path);
}

void CZimuFile::SaveAsSrtFile(const char* path)
{
	m_fileType = FT_SRT;
	Save_w(path);
}

int CZimuFile::WriteBuf(const Sentence& st, char* buf, int size)
{
	string s = st.m_content;
	if(s.size() > 200)
		s = s.substr(0, 200);
	return sprintf(buf, "%d\r\n%s --> %s\r\n%s\r\n\r\n", st.m_seq, 
		st.m_startTime.m_dispVal.c_str(), st.m_endTime.m_dispVal.c_str(), s.c_str());
}

int CZimuFile::Streaming(const Sentence& st, std::string &out)
{
	stringstream ss;
	//std::ios::resetiosflag(ios::dec);
	ss << st.m_seq << "\r\n"
		<< st.m_startTime.m_dispVal	<< " --> " << st.m_endTime.m_dispVal << "\r\n"
		<< st.m_content	<< "\r\n";

	if(m_fileType == FT_REV){
		ReviseRecords::const_iterator it = st.m_reviseRecords.begin();
		for(; it != st.m_reviseRecords.end(); it++){
			const ReviseRecord &revise = *it;
			ss << "<" << revise.m_contents << ">\r\n";
		}
	}

	ss << "\r\n";
	out.assign(ss.str());
	return out.size();
}

void EraseLF(const string &str, string &out)
{
	char c;
	for(int i = 0; i < str.length(); i++){
		c = str[i];
		if(c == '\r' || c == '\n')
			continue;
		out.append(1, c);
	}
}

int CZimuFile::StreamingToLrc(const Sentence& st, std::string &out)
{
	stringstream ss;
	//std::ios::resetiosflag(ios::dec);
	string content;
	EraseLF(st.m_content, content);
	ss << "["
		<< TimeMS2HMS2(st.m_startTime.m_msVal) << "]"
		<< content	<< "\r\n";

	out.assign(ss.str());
	return out.size();
}

int CZimuFile::DoReadFile(FILE *file, char* buf, int bufLen)
{
	int rd = fread(buf, 1, bufLen, file);
	if(ferror(file)){
		return -1;
	}
	return rd;
}

int CZimuFile::ReadSRT_W(const char* path)
{
	int rt = 0;
	const int BUFLEN = 1024;
	char buf[BUFLEN], buf2[BUFLEN*2], *ascii;
	ascii = &buf2[0];
	int maxSize = BUFLEN*2;
	int left = 0;
	int off = 0;
	int num = 0;

	FILE *fin = fopen(path, "rb");
	if(!fin){
		//AfxMessageBox("字幕文件不存在!");
		return -1;
	}

	int rd = DoReadFile(fin, buf, BUFLEN);
	if(rd < 0){
		goto failure;
	}

	if(rd > 2){
		unsigned short *mark = (unsigned short*)buf;
		if(*mark == 0xfffe || *mark == 0xfeff)
			off = 2;
		else{
			fclose(fin);
			return ReadSRT(path);
		}
	}else{
		goto failure; //not invalid file.
	}
	ASSERT(rd % 2 == 0);

	num = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, (LPCWSTR)(buf+off), (rd-off)/2, ascii, 0, NULL, NULL);
	if(num > maxSize){
		maxSize = num;
		ascii = new char[maxSize+1];
	}
	WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, (LPCWSTR)(buf+off), (rd-off)/2, ascii, num+1, NULL, NULL);
	ascii[num] = '\0';

	rt = ParseBuffer(ascii, num, left);
	if(rt < 0){
		goto failure;
	}

	while(!feof(fin)){
		if(left > 0)
			memmove(ascii, ascii+num-left, left); //if overlap ??

		rd = DoReadFile(fin, buf, BUFLEN);
		if(rd < 0)
			goto failure;
		ASSERT(rt % 2 == 0);

		num = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, (LPCWSTR)buf, rd/2, ascii+left, 0, NULL, NULL);
		if(num+left > maxSize){
			maxSize = num+left;
			char *tmp = new char[maxSize+1];
			if(left > 0)
				memcpy(tmp, ascii, left);
			if(ascii != &buf2[0]){
				delete [] ascii;
			}
			ascii = tmp;
		}
		WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, (LPCWSTR)buf, rd/2, ascii+left, num, NULL, NULL);
		num += left;
		ascii[num] = '\0';

		rt = ParseBuffer(ascii, num, left);
		if(rt < 0)
			goto failure;
	}
	if(left > 0){
		memmove(ascii, ascii+num-left, left);
		ASSERT(left + 4 < maxSize);
		memcpy(ascii+left, "\r\n\r\n", 4);
		ascii[left+4] = '\0';
		rt = ParseBuffer(ascii, left+4, left);
	}
failure:
	if(rt < 0)
		MessageBox(NULL, "字幕文件读取失败，请确定文件格式符合要求。", "菩提字幕", MB_OK);
	fclose(fin);
	if(ascii != &buf2[0])
		delete [] ascii;

	return rt;
}

const char* find_last_of(const char* haystack, int len, const char* needle)
{
	int len2 = strlen(needle);
	const char *pos = strstr(haystack, needle), *posLast;
	posLast = pos;
	while(pos){
		posLast = pos;
		haystack = pos+len2;
		pos = strstr(haystack, needle);
	}
	return posLast ? (posLast+len2) : NULL;
}

const char* get_line_end(const char* buf, const char* border, const char*& nextLine)
{
	if(!buf || !border || buf >= border)
		return NULL;

	const char *pos = strstr(buf, "\n");
	if(pos){
		nextLine = pos+1;
		return (*--pos == '\r') ? pos : (pos+1);
	}else if(pos = strstr(buf, "\r")){
		nextLine = pos+1;
		return pos;
	}else
		return NULL;
}

const char* find_last_record_end(const char *buf, int len)
{
	const char *pos = find_last_of(buf, len, "\n\n");
	if(pos)
		return pos;
	pos = find_last_of(buf, len, "\r\n\r\n");
	if(pos)
		return pos;
	pos = find_last_of(buf, len, "\r\r");
	if(pos)
		return pos;
	return NULL;
}

int CZimuFile::ParseBuffer(const char* buf, int len, int &left)
{
	long tm = 0;
	const char* arrow = "-->";
	//const char* lb = "\r\n", *sep = "\r\n\r\n";
	const int /*LEN1 = strlen(lb), */LEN2 = strlen(arrow); //, LEN3=strlen(sep); //"\r\n"
	const char *start = buf, *end = buf, *itemStart = buf, *border = buf+len;
	const char *nextLine = NULL;
	itemStart = find_last_record_end(buf, len);
	//ASSERT(itemStart);
	if(!itemStart){
		itemStart = buf + len;
	}

	while(1){
		string text;
		Sentence st;

		end = get_line_end(start, border, nextLine);
		if(end == NULL || nextLine > border){
			return -1;
		}
		st.m_seq = GetNumer(start, end-start);
		start = nextLine;
		if(st.m_seq == 0)
			return -1;

		end = strstr(start, arrow);
		if(end == NULL){
			return -1;
		}
		tm = GetTime(start, end-start);
		st.m_startTime.SetTime(tm);
		start = end+LEN2;

		end = get_line_end(start, border, nextLine);
		if(end == NULL || nextLine > border){
			return -1;
		}
		tm = GetTime(start, end-start);
		st.m_endTime.SetTime(tm);
		start = nextLine;

		end = get_line_end(start, border, nextLine);
		if(end == NULL){
			return -1;
		}
		//text.assign(start, (nextLine ? nextLine : end)-start);
		const char *contentStart = start;
		start = nextLine;

		//the rest line of contents.
		while(end = get_line_end(start, border, nextLine)){
			if(end == start){ //an empty line
				start = nextLine;
				unsigned char ch = *nextLine;
				if((border > nextLine) && ((ch > 0x80) || !(isdigit(*nextLine)))){
					continue;
				}
				break;
			}
			//text.append(start, (nextLine ? nextLine : end)-start);
			start = nextLine;
		}
		if(!end)
			end = border;
		text.assign(contentStart, end - contentStart);
		if(ParseContent(text, st) > 0)
			m_withRevise = true;
		if(text.at(text.size()-1) == '\n')
			text.erase(text.size()-1);
		if(text.at(text.size()-1) == '\r')
			text.erase(text.size()-1);

		Trim(text);
		st.m_content.assign(text);
		// 		st.m_mark = (text.size() > MAX_TOTAL_BYTES) ? SM_CONTENT_LENGTH : 0;
		// 		int len1 = text.find("\n");
		// 		if(len1 > MAX_CHINESE_CHARACTERS*2 || (text.size() - len1) > MAX_CHINESE_CHARACTERS*2)
		// 			st.m_mark |= SM_CONTENT_LENGTH;

		CheckContentLength(st);
		m_contents.push_back(st);

		if(!start || (start == itemStart)) //the broken record pos.
			break;

		if(start == border)
			break;
	}

	left = buf + len - itemStart;
	return 0;
}

int CZimuFile::ParseContent(string &text, Sentence &st)
{
	string::size_type pos = text.find("<"), pos2, posFirst;
	posFirst = pos;
	while(pos != string::npos){
		pos2 = text.find(">", pos+1);
		if(pos2 != string::npos){
			string s;
			s.append(text, pos+1, pos2-pos-1);
			ReviseRecord revise;
			revise.m_contents.assign(s);
			st.m_reviseRecords.push_back(revise);
		}else{
			break;
		}
		pos = text.find("<", pos2+1);
	}
	if(posFirst != string::npos){
		string content = text.substr(0, posFirst);
		text.assign(content);
		return 1;
	}
	return 0;
}

int CZimuFile::ReadSRT(const char* path)
{
	ifstream ifs(path);
	if(!ifs){
		CString msg = "can't open file: ";
		msg += path;
		msg += " for read!";
		AfxMessageBox(msg);
		return -1;
	}

	//	unsigned short mark = 0;
	//	ifs >> mark;
	//	if(mark == 0xfffe || mark == 0xfeff)
	//		;
	//	else
	//		AfxMessageBox("not unicode!");

	const char* lb = "\r\n";
	const char* arrow = "-->";
	const int LEN1 = strlen(arrow);
	Sentence st;
	const int BUFLEN = 256;
	char buf[BUFLEN] = {0};
	m_contents.clear();
	while(ifs){
		ifs.getline(buf, BUFLEN);
		st.m_seq = GetNumer(buf, BUFLEN);

		ifs.getline(buf, BUFLEN);
		string tmp(buf);
		unsigned pos = tmp.find(arrow);
		if(pos == string::npos)
			return -1;

		string stime = tmp.substr(0, pos);
		long tm = GetTime(stime.c_str(), stime.length());
		if(tm < 0)
			return -1;
		st.m_startTime.SetTime(tm);
		stime = tmp.substr(pos + LEN1);
		tm = GetTime(stime.c_str(), stime.length());
		if(tm < 0)
			return -1;
		st.m_endTime.SetTime(tm);

		ifs.getline(buf, BUFLEN);
		string text(buf);
		while(ifs){
			ifs.getline(buf, BUFLEN);
			if(strlen(buf) == 0)
				break;
			text += buf;
		}
		st.m_content.assign(text);
		m_contents.push_back(st);
	}

	return 0;
}

int CZimuFile::GetNumer(const char* buf, int len)
{
	int rt = atoi(buf);
	if(rt <= 0)
		rt = 0; //just for debug.
	return rt;
}

long CZimuFile::GetTime(const char* buf, int len)
{
	int h, m, s, ms;
	const char* ptr = buf;
	while(isspace(*ptr)) //skip blank.
		ptr++;
	while(len>0 && isspace(buf[len-1])){
		len--;
	}
	if(len <= 0)
		return -1;

	int rt = sscanf(ptr, "%d:%d:%d,%d", &h, &m, &s, &ms);
	if(rt == 0)
		return -1;
	long tm = (3600*h + 60*m + s)*1000 + ms;
	if(tm <= 0)
		tm = 0; //just for debug.
	return tm;
}

Sentence* CZimuFile::GetSentence(int seq)
{
	if(seq <= 0 || seq > m_contents.size()) //start from 1.
		return NULL;

	ContentsType::iterator it = m_contents.begin();
	for(; it != m_contents.end(); it++){
		if(it->m_seq == seq)
			return &(*it);
	}

	return NULL;
}
