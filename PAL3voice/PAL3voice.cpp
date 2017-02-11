#pragma warning(disable: 4786)

#include <map>
#include <utility>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <climits>
#include <cmath>

#include <windows.h>
#include "voiceplugin.h"

#define VOICECONFIGFILE "VoiceConfig.txt"
#define LOGFILE "VoiceLog.txt"
#define SPLASHFILE "VoiceSplash.txt"
#define TEXTLOG "TextLog.txt"

#define AUDIO_SYNC_LIMIT 1000

#define TEXT_PREFIX "TXT"
#define CAPTION_PREFIX "CAP"
#define MOVIE_PREFIX "MOV"

#define MAXLINE 4096
#define MAXLINEFMT "%4095"
#define eps 1e-6

static VoiceToolkit *tools;

// helper functions

static void fail(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
	char buf[MAXLINE];
    _vsnprintf(buf, MAXLINE - 1, fmt, ap);
	buf[MAXLINE - 1] = '\0';
	tools->ReportFatalError(buf);
    exit(1);
    va_end(ap);
}

static void cleanup_log()
{
	FILE *fp = fopen(LOGFILE, "w");
	if (fp) {
		fclose(fp);
	}
}
static void plog(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
	char buf[MAXLINE];
    _vsnprintf(buf, MAXLINE - 1, fmt, ap);
	buf[MAXLINE - 1] = '\0';
	FILE *fp = fopen(LOGFILE, "a");
	if (fp) {
		SYSTEMTIME SystemTime;
        GetLocalTime(&SystemTime);
        fprintf(fp, "%04hu-%02hu-%02hu %02hu:%02hu:%02hu.%03hu | ", SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, SystemTime.wMilliseconds);
		fprintf(fp, "%s\n", buf);
		fclose(fp);
	}
    va_end(ap);
}

static std::string strtolower(const std::string &str)
{
	std::string ret(str);
	for (unsigned i = 0; i < ret.length(); i++) ret[i] = tolower(ret[i]);
	return ret;
}




// audio functions
static double voicevol = 1.0;
static HSTREAM hstream = NULL;
static void AudioStart()
{
	if (hstream) tools->AIL_start_stream(hstream);
}
static void AudioStop()
{
	if (hstream) {
		tools->AIL_close_stream(hstream);
		hstream = NULL;
	}
}
static void AudioPrepare(const char *filename, double volume)
{
	if (hstream) AudioStop();
	hstream = tools->AIL_open_stream(tools->dig, filename, 0);
	if (hstream) {
		tools->AIL_set_stream_volume(hstream, floor(voicevol * volume * 127.0 + eps));
	} else {
		plog("unable to open stream '%s'", filename);
	}
}
static void AudioPause()
{
	if (hstream) tools->AIL_pause_stream(hstream, 1);
}
static void AudioResume()
{
	if (hstream) tools->AIL_pause_stream(hstream, 0);
}
static void AudioSync(int ms)
{
	if (hstream) {
		int tot, cur;
		tools->AIL_stream_ms_position(hstream, &tot, &cur);
		if (ms < 0 || ms >= tot) ms = tot;
		if (abs(cur - ms) >= AUDIO_SYNC_LIMIT) {
			tools->AIL_set_stream_ms_position(hstream, ms);
		}
	}
}



// text logger
static int log_counter = 0;
static void MakeTextLogHeader()
{
	FILE *fp = fopen(TEXTLOG, "a");
	if (!fp) return;
	SYSTEMTIME SystemTime;
    GetLocalTime(&SystemTime);
    fprintf(fp, "\n\n; %04hu-%02hu-%02hu %02hu:%02hu:%02hu.%03hu\n\n", SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, SystemTime.wMilliseconds);
	fclose(fp);
}
static void LogText(const char *text)
{
	FILE *fp = fopen(TEXTLOG, "a");
	if (!fp) return;
	log_counter++;
	fprintf(fp, "FIXME%04d.WAV|1.000|%08X|%s\n", log_counter, tools->CalcStringCRC32(text), text);
	fclose(fp);
}



// config loader

struct VoiceItem {
	int file_num;
	int line_num;
	std::string audiofile;
	double volume;
	unsigned texthash;
	std::string text;
};

static std::map<std::pair<unsigned, std::string>, std::vector<VoiceItem> > vdata;
static int log_known = 0, log_unknown = 0;

static int LoadSingleVoiceData(const char *filename, int file_num)
{
	FILE *fp = fopen(filename, "r");
	if (!fp) {
		fail("unable to open voice data file '%s'.", filename);
		return 0;
	}
	
	int vitem_count = 0;
	int line_num = 0;
	char buf[MAXLINE];
	while (fgets(buf, sizeof(buf), fp)) {
		line_num++;

		if (!buf[0] || buf[0] == ';' || buf[0] == '\n') continue;

		char audiofile[MAXLINE];
		double volume;
		unsigned texthash;
		char text[MAXLINE];

		if (sscanf(buf, "%[^|]|%lf|%x|%[^\n]", audiofile, &volume, &texthash, text) == 4) {
			if (tools->CalcStringCRC32(text) != texthash) {
				plog("WARN: unmatched text and hash at %s:%d", filename, line_num);
			}
			struct VoiceItem vitem;
			vitem.file_num = file_num;
			vitem.line_num = line_num;
			vitem.audiofile = std::string(audiofile);
			vitem.volume = volume;
			vitem.texthash = texthash;
			vitem.text = std::string(text);
			vdata[std::make_pair(texthash, std::string(text))].push_back(vitem);
			vitem_count++;
		}
	}

	plog("loaded %d voice items from '%s'", vitem_count, filename);
	return vitem_count;
}

static void LoadVoiceConfig()
{
	vdata.clear();

	FILE *fp = fopen(VOICECONFIGFILE, "r");
	if (!fp) fail("unable to open voice config file '%s'.", VOICECONFIGFILE);

	int vitem_count = 0;
	int file_num = 0;
	char cmd[MAXLINE];
	while (fscanf(fp, MAXLINEFMT "s", cmd) == 1) {
		if (cmd[0] == ';') {
			fscanf(fp, "%*[^\n]");
		} else if (_stricmp(cmd, "LOAD") == 0) {
			char buf[MAXLINE];
			if (fscanf(fp, MAXLINEFMT "s", buf) == 1) {
				file_num++;
				vitem_count += LoadSingleVoiceData(buf, file_num);
			}
		} else if (_stricmp(cmd, "MUSICVOL") == 0) {
			double vol;
			if (fscanf(fp, "%lf", &vol) == 1) {
				tools->SetMusicMasterVolume(vol);
			}
		} else if (_stricmp(cmd, "2DVOL") == 0) {
			double vol;
			if (fscanf(fp, "%lf", &vol) == 1) {
				tools->Set2DMasterVolume(vol);
			}
		} else if (_stricmp(cmd, "3DVOL") == 0) {
			double vol;
			if (fscanf(fp, "%lf", &vol) == 1) {
				tools->Set3DMasterVolume(vol);
			}
		} else if (_stricmp(cmd, "VOICEVOL") == 0) {
			fscanf(fp, "%lf", &voicevol);
		} else if (_stricmp(cmd, "LOGKNOWN") == 0) {
			fscanf(fp, "%d", &log_known);
		} else if (_stricmp(cmd, "LOGUNKNOWN") == 0) {
			fscanf(fp, "%d", &log_unknown);
		} else if (_stricmp(cmd, "LOGCOUNTER") == 0) {
			fscanf(fp, "%d", &log_counter);
		} else {
			fail("unknown command '%s' in index file.", cmd);
		}
	}
	fclose(fp);

	if (log_known || log_unknown) {
		MakeTextLogHeader();
	}

	plog("total %d voice items", vitem_count);
}




// statistics
static int text_total = 0, text_known = 0, text_unknown = 0;

// item selector
static int last_file_num = 0, last_line_num = 0;
static VoiceItem *ChooseBestVoiceItem(std::vector<VoiceItem> *itemlist)
{
	std::pair<int, int> mindist(INT_MAX, INT_MAX);

	std::vector<VoiceItem>::iterator it;
	VoiceItem *ret = NULL;
	for (it = itemlist->begin(); it != itemlist->end(); it++) {
		std::pair<int, int> curdist = std::make_pair(abs(it->file_num - last_file_num), abs(it->line_num - (last_line_num + 1)));
		if (curdist < mindist) {
			ret = &*it;
			mindist = curdist;
		}
	}
	
	last_file_num = ret->file_num;
	last_line_num = ret->line_num;
	return ret;
}

static void AudioPrepareVoiceItem(const char *data, const char *type)
{
	std::string text = std::string(type) + "|" + std::string(data);
	std::pair<unsigned, std::string> key(tools->CalcStringCRC32(text.c_str()), text);
	std::map<std::pair<unsigned, std::string>, std::vector<VoiceItem> >::iterator it;
	if ((it = vdata.find(key)) != vdata.end()) {
		VoiceItem *pitem = ChooseBestVoiceItem(&it->second);
		AudioPrepare(pitem->audiofile.c_str(), pitem->volume);
		if (log_known) LogText(text.c_str());
		text_known++;
	} else {
		if (log_unknown) LogText(text.c_str());
		text_unknown++;
	}
	text_total++;
}



// plugin management functions
void WINAPI VoiceDLLAttached()
{
	FILE *fp = fopen(SPLASHFILE, "r");
	if (fp) {
		char title[MAXLINE];
		char text[MAXLINE];
		memset(title, 0, sizeof(title));
		fscanf(fp, MAXLINEFMT "[^\n] ", title);
		memset(text, 0, sizeof(text));
		fread(text, 1, sizeof(text) - 1, fp);
		MessageBox(NULL, text, title, MB_ICONINFORMATION);
		fclose(fp);
	}
}
void WINAPI VoiceInit(VoiceToolkit *toolkit)
{
	tools = toolkit;
	cleanup_log();
	plog("plugin init, host version: %s (built on %s)", tools->patch_version, tools->build_date);
	LoadVoiceConfig();
}
void WINAPI VoiceCleanup()
{
	AudioStop();
	plog("statistics: total %d, known %d (%.2f%%), unknown %d (%.2f%%)", text_total, text_known, 100.0 * text_known / text_total, text_unknown, 100.0 * text_unknown / text_total);
}
void WINAPI GamePause()
{
	AudioPause();
}
void WINAPI GameResume()
{
	AudioResume();
}



// dialog text functions
void WINAPI TextIdle(int state)
{
}
void WINAPI TextPrepare(const char *text, int mode)
{
	if (mode != 0) return; // ignore timed dialog
	AudioPrepareVoiceItem(text, TEXT_PREFIX);
}
void WINAPI TextStart()
{
	AudioStart();
}
void WINAPI TextStop()
{
	AudioStop();
}



// caption functions
void WINAPI CaptionIdle(int state)
{
}
void WINAPI CaptionPrepare(const char *tex)
{
	AudioPrepareVoiceItem(strtolower(std::string(tex)).c_str(), CAPTION_PREFIX);
}
void WINAPI CaptionStart(void)
{
	AudioStart();
}
void WINAPI CaptionStop(void)
{
	// no AudioStop() here
	// because the voice may longer than caption display time
}



// movie functions
void WINAPI MovieIdle(int state)
{
	if (state) {
		HBINK hbink = *tools->phbink;
		if (hbink) {
			AudioSync(hbink->FrameNum * 1000.0 / hbink->FrameRate * hbink->FrameRateDiv);
		}
	}
}
void WINAPI MoviePrepare(const char *movie)
{
	AudioPrepareVoiceItem(strtolower(std::string(movie)).c_str(), MOVIE_PREFIX);
}
void WINAPI MovieStart(void)
{
	AudioStart();
}
void WINAPI MovieStop(void)
{
	AudioStop();
}