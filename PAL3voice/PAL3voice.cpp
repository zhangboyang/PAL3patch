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
#include <d3dx9.h>
#include "voiceplugin.h"

#define PLUGIN_VERSION "v1.0 RC"
#define PLUGIN_BUILDDATE __DATE__ ", " __TIME__

#define VOICECONFIGFILE "VoiceConfig.txt"
#define LOGFILE "VoiceLog.txt"
#define VDATALOG "VoiceDataLog.txt"

#define AUDIO_SYNC_LIMIT 1000

#define TEXT_PREFIX "TXT"
#define CAPTION_PREFIX "CAP"
#define MOVIE_PREFIX "MOV"
#define CBDIALOG_PREFIX "CBD"

#define MAXLINE 4096
#define MAXLINEFMT "%4095"
#define eps 1e-6

static std::string path_prefix = "";
static double voicevol = 1.0;
static VoiceToolkit *tools;

// helper functions

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
	OutputDebugString(buf);
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

static void fail(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
	char buf[MAXLINE];
    _vsnprintf(buf, MAXLINE - 1, fmt, ap);
	buf[MAXLINE - 1] = '\0';
	plog("FATAL: %s", buf);
	if (tools) tools->misc->ReportFatalError(buf);
	else MessageBox(NULL, buf, NULL, MB_ICONERROR);
    exit(1);
    va_end(ap);
}

static std::string strtolower(const std::string &str)
{
	std::string ret(str);
	for (unsigned i = 0; i < ret.length(); i++) ret[i] = tolower(ret[i]);
	return ret;
}

static void ReadString(FILE *fp, char *buf, int buf_len)
{
	if (buf_len <= 0) return;
	fscanf(fp, " ");
	int ch;
	ch = fgetc(fp);
	if (ch == EOF) goto done;
	if (ch == '\"') {
		while (1) {
			ch = fgetc(fp);
			if (ch == EOF) break;
			if (ch == '\"') break;
			if (ch == '\\') {
				ch = fgetc(fp);
				if (ch == EOF) break;
				switch (ch) {
					case 'n': ch = '\n'; break;
				}
			}
			if (buf_len > 1) {
				*buf++ = ch;
				buf_len--;
			}
		}
	} else {
		ungetc(ch, fp);
		while (1) {
			ch = fgetc(fp);
			if (ch == EOF) break;
			if (ch == '\n') break;
			if (buf_len > 1) {
				*buf++ = ch;
				buf_len--;
			}
		}
	}
done:
	*buf = '\0';
}


// d3d state block
static IDirect3DStateBlock9 *pStateBlock;
static void CaptureStateBlock()
{
	pStateBlock->Capture();
}
static void RestoreStateBlock()
{
	pStateBlock->Apply();
}
static void InitStateBlock()
{
	tools->gfx->pd3dDevice->CreateStateBlock(D3DSBT_ALL, &pStateBlock);
}
static void StateBlockOnResetDevice()
{
	tools->gfx->pd3dDevice->CreateStateBlock(D3DSBT_ALL, &pStateBlock);
}
static void StateBlockOnLostDevice()
{
	pStateBlock->Release();
}




// pre and post script
static void RunScript(const char *filename);

static std::string script_pre = "", script_post = "";
static void RunScriptPre()
{
	RunScript(script_pre.c_str());
	script_pre = "";
}
static void RunScriptPost()
{
	RunScript(script_post.c_str());
	script_post = "";
}
static void SetScript(const char *pre, const char *post)
{
	script_pre = std::string(pre);
	script_post = std::string(post);
}

// audio player
static HSTREAM hstream = NULL;
static void AudioStart()
{
	if (hstream) {
		tools->mss->AIL_start_stream(hstream);
	}
}
static void AudioStop()
{
	if (hstream) {
		tools->mss->AIL_close_stream(hstream);
		hstream = NULL;
	}
}
static void AudioPrepare(const char *filename, double volume)
{
	std::string audiopath = path_prefix + std::string(filename);

	if (hstream) AudioStop();
	if (stricmp(filename, "x") != 0) {
		hstream = tools->mss->AIL_open_stream(tools->mss->h2DDriver, audiopath.c_str(), 0);
		if (hstream) {
			tools->mss->AIL_set_stream_volume(hstream, floor(voicevol * volume * 127.0 + eps));
		} else {
			plog("unable to open stream '%s'", audiopath.c_str());
		}
	}
}
static void AudioPause()
{
	if (hstream) tools->mss->AIL_pause_stream(hstream, 1);
}
static void AudioResume()
{
	if (hstream) tools->mss->AIL_pause_stream(hstream, 0);
}
static void AudioSync(int ms)
{
	if (hstream) {
		int tot, cur;
		tools->mss->AIL_stream_ms_position(hstream, &tot, &cur);
		if (ms < 0 || ms >= tot) ms = tot;
		if (abs(cur - ms) >= AUDIO_SYNC_LIMIT) {
			tools->mss->AIL_set_stream_ms_position(hstream, ms);
		}
	}
}



// sleep
static void SleepDoModal(unsigned ms, bool canskip)
{
	DWORD tick;

	tick = timeGetTime();
	while (1) {
		MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                PostQuitMessage(msg.wParam);
                return;
            }
			if (canskip && msg.message == WM_CHAR && msg.wParam == VK_ESCAPE) {
				return;
			}
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

		if (timeGetTime() - tick >= ms) return;
		Sleep(1);
	}
}


// Texture Quad
struct tq_vertex_t {
    float x, y, z, rhw;
    DWORD color;
    float u, v;
};
#define TQ_VERTEX_FVF (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)
#define TQ_VERTEX_SIZE (sizeof(struct tq_vertex_t))
#define TQ_VBUF_TRANGLE_COUNT 2
#define TQ_VBUF_SIZE (TQ_VBUF_TRANGLE_COUNT * 3)
#define TQ_VBUF_SIZE_BYTES (TQ_VERTEX_SIZE * TQ_VBUF_SIZE)
static void tq_fillvbuf_rect(struct tq_vertex_t *vbuf, const RECT *rect)
{
    float left = rect->left - 0.5f;
    float top = rect->top - 0.5f;
    float right = rect->right - 0.5f;
    float bottom = rect->bottom - 0.5f;
	float u1 = 0.0f, v1 = 0.0f;
	float u2 = 1.0f, v2 = 1.0f;
    vbuf[0].x = vbuf[5].x = left;
    vbuf[0].y = vbuf[5].y = top;
    vbuf[0].u = vbuf[5].u = u1;
    vbuf[0].v = vbuf[5].v = v1;
    vbuf[1].x = left;
    vbuf[1].y = bottom;
    vbuf[1].u = u1;
    vbuf[1].v = v2;
    vbuf[2].x = vbuf[3].x = right;
    vbuf[2].y = vbuf[3].y = bottom;
    vbuf[2].u = vbuf[3].u = u2;
    vbuf[2].v = vbuf[3].v = v2;
    vbuf[4].x = right;
    vbuf[4].y = top;
    vbuf[4].u = u2;
    vbuf[4].v = v1;
    vbuf[0].color = vbuf[1].color = vbuf[2].color = vbuf[3].color = vbuf[4].color = vbuf[5].color = 0x00FFFFFF;
    vbuf[0].rhw = vbuf[1].rhw = vbuf[2].rhw = vbuf[3].rhw = vbuf[4].rhw = vbuf[5].rhw = 1.0f;
    vbuf[0].z = vbuf[1].z = vbuf[2].z = vbuf[3].z = vbuf[4].z = vbuf[5].z = 0.0f;
}
static IDirect3DVertexBuffer9 *tqvbuf = NULL;

static void TexQuadInit()
{
	if (FAILED(tools->gfx->pd3dDevice->CreateVertexBuffer(TQ_VBUF_SIZE_BYTES, 0, TQ_VERTEX_FVF, D3DPOOL_MANAGED, &tqvbuf, NULL))) {
		fail("can't create vertex buffer for texquad.");
	}
}

static void TexQuadPrepare(RECT *rect)
{
	struct tq_vertex_t *pbData;
    IDirect3DVertexBuffer9_Lock(tqvbuf, 0, 0, (void **) &pbData, 0);
    tq_fillvbuf_rect(pbData, rect);
    IDirect3DVertexBuffer9_Unlock(tqvbuf);
}

static void TexQuadRender(IDirect3DTexture9 *tex, unsigned bgcolor)
{
	tools->gfx->pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	tools->gfx->pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	tools->gfx->pd3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
	tools->gfx->pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	tools->gfx->pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	tools->gfx->pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	tools->gfx->pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	tools->gfx->pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	tools->gfx->pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	tools->gfx->pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	tools->gfx->pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	tools->gfx->pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	tools->gfx->pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, bgcolor, 1.0f, 0);
	tools->gfx->pd3dDevice->BeginScene();
	if (tex) {
		tools->gfx->pd3dDevice->SetFVF(TQ_VERTEX_FVF);
		tools->gfx->pd3dDevice->SetTexture(0, tex);
		tools->gfx->pd3dDevice->SetStreamSource(0, tqvbuf, 0, TQ_VERTEX_SIZE);
		tools->gfx->pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, TQ_VBUF_TRANGLE_COUNT);
	}
	tools->gfx->pd3dDevice->EndScene();
	tools->gfx->pd3dDevice->Present(NULL, NULL, NULL, NULL);
}


// clear screen
static void ClearScreen(unsigned bgcolor)
{
	CaptureStateBlock();
	TexQuadRender(NULL, bgcolor);
	RestoreStateBlock();
}


// picture viewer
static void PictureDoModal(const char *filename, unsigned bgcolor, unsigned ms)
{
	std::string picpath = path_prefix + std::string(filename);
	DWORD tick;
	IDirect3DTexture9 *tex = NULL;

	D3DXIMAGE_INFO imginfo;
	if (FAILED(D3DXGetImageInfoFromFile(picpath.c_str(), &imginfo))) {
		plog("unable to open picture '%s'", picpath.c_str());
	}

	if (FAILED(D3DXCreateTexture(tools->gfx->pd3dDevice, imginfo.Width, imginfo.Height, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &tex))) {
		tex = NULL;
		goto done;
	} else {
		IDirect3DSurface9 *texsuf;
		if (SUCCEEDED(tex->GetSurfaceLevel(0, &texsuf))) {
			D3DXLoadSurfaceFromFile(texsuf, NULL, NULL, picpath.c_str(), NULL, D3DX_FILTER_LINEAR, 0, NULL);
			texsuf->Release();
		}
	}

	RECT rc;
	SetRect(&rc, 0, 0, tools->gfx->Width, tools->gfx->Height);
	tools->misc->GetRatioRect(&rc, &rc, (double) imginfo.Width / imginfo.Height);
	TexQuadPrepare(&rc);

	CaptureStateBlock();

	tick = timeGetTime();
	while (1) {
		MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                PostQuitMessage(msg.wParam);
                goto done;
            }
			if (msg.message == WM_CHAR && msg.wParam == VK_ESCAPE) {
				goto done;
			}
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

		tools->gfx->EnsureCooperativeLevel();
		TexQuadRender(tex, bgcolor);
		if (timeGetTime() - tick >= ms) break;
		Sleep(1);
	}

done:
	if (tex) tex->Release();
	RestoreStateBlock();
}


// video player
static HBINK curbink = NULL;
static void VideoPause()
{
	if (curbink) tools->bik->BinkPause(curbink, 1);
}
static void VideoResume()
{
	if (curbink) tools->bik->BinkPause(curbink, 0);
}
static void VideoDoModal(const char *filename, unsigned bgcolor, double volume)
{
	RECT rc;
	std::string videopath = path_prefix + std::string(filename);
	IDirect3DSurface9 *suf = NULL;
	IDirect3DTexture9 *tex = NULL;

	CaptureStateBlock();

	HBINK bink = tools->bik->BinkOpen(videopath.c_str(), 0);
	if (!bink) {
		plog("unable to open bink video '%s'", videopath.c_str());
		goto done;
	}
	curbink = bink;

	tools->bik->BinkSetVolume(bink, tools->vol->GetMusicMasterVolume() * volume * 32768.0);

	if (FAILED(tools->gfx->pd3dDevice->CreateOffscreenPlainSurface(bink->Width, bink->Height, D3DFMT_X8R8G8B8, D3DPOOL_SCRATCH, &suf, NULL))) {
		suf = NULL;
		goto done;
	}

	if (FAILED(D3DXCreateTexture(tools->gfx->pd3dDevice, bink->Width, bink->Height, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &tex))) {
		tex = NULL;
		goto done;
	}

	SetRect(&rc, 0, 0, tools->gfx->Width, tools->gfx->Height);
	tools->misc->GetRatioRect(&rc, &rc, (double) bink->Width / bink->Height);
	TexQuadPrepare(&rc);

	while (1) {
		MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                PostQuitMessage(msg.wParam);
                goto done;
            }
			if (msg.message == WM_CHAR && msg.wParam == VK_ESCAPE) {
				goto done;
			}
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
		tools->gfx->EnsureCooperativeLevel();
		if (tools->bik->BinkWait(bink)) {
			Sleep(1);
		} else {
			if (tools->bik->BinkDoFrame(bink) == 0) {
				// draw frame
				D3DLOCKED_RECT lrc;
				if (SUCCEEDED(suf->LockRect(&lrc, NULL, 0))) {
					int ret = tools->bik->BinkCopyToBuffer(bink, lrc.pBits, lrc.Pitch, bink->Height, 0, 0, BINKSURFACE32 | BINKCOPYALL);
					suf->UnlockRect();
					if (ret == 0) {
						IDirect3DSurface9 *texsuf;
						if (SUCCEEDED(tex->GetSurfaceLevel(0, &texsuf))) {
							if (SUCCEEDED(D3DXLoadSurfaceFromSurface(texsuf, NULL, NULL, suf, NULL, NULL, D3DX_FILTER_LINEAR, 0))) {
								TexQuadRender(tex, bgcolor);
							}
							texsuf->Release();
						}
					}
				}
			}

			if (bink->Frames == bink->FrameNum) break;
			tools->bik->BinkNextFrame(bink);
		}
	}

done:
	curbink = NULL;
	if (bink) tools->bik->BinkClose(bink);
	if (suf) suf->Release();
	if (tex) tex->Release();
	RestoreStateBlock();
}




// script interpreter
static std::map<std::string, std::string> scriptvars; // script global vars
static void ReadScriptString(FILE *fp, char *buf, int buf_len)
{
	std::map<std::string, std::string>::iterator it;
	if (buf_len <= 0) return;
	fscanf(fp, " ");
	int ch = fgetc(fp);
	if (ch == EOF) {
		goto fail;
	}
	if (ch != '$') {
		ungetc(ch, fp);
		ReadString(fp, buf, buf_len);
		return;
	}
	char varname[MAXLINE];
	ch = fgetc(fp);
	if (ch == EOF) {
		goto fail;
	}
	ungetc(ch, fp);
	if (ch == '\"') {
		ReadString(fp, varname, sizeof(varname));
	} else {
		if (fscanf(fp, MAXLINEFMT "s", varname) != 1) {
			goto fail;
		}
	}
	if ((it = scriptvars.find(std::string(varname))) != scriptvars.end()) {
		strncpy(buf, it->second.c_str(), buf_len - 1);
		buf[buf_len - 1] = '\0';
	} else {
		goto fail;
	}
	return;
fail:
	*buf = '\0';
}
static void RunScript(const char *filename)
{
	if (!filename || !*filename) return;

	std::string scriptpath = path_prefix + std::string(filename);
	FILE *fp = fopen(scriptpath.c_str(), "r");
	if (!fp) {
		plog("can't open script file '%s'", scriptpath.c_str());
		goto done;
	}
	
	char cmd[MAXLINE];
	while (fscanf(fp, MAXLINEFMT "s", cmd) == 1) {
		if (cmd[0] == ';') {
			fscanf(fp, "%*[^\n]");
		} else if (_stricmp(cmd, "CALL") == 0) {
			char str[MAXLINE];
			ReadScriptString(fp, str, sizeof(str));
			RunScript(str);
		} else if (_stricmp(cmd, "RETN") == 0) {
			goto done;
		} else if (_stricmp(cmd, "SETVAR") == 0) {
			char varname[MAXLINE];
			char varvalue[MAXLINE];
			ReadScriptString(fp, varname, sizeof(varname));
			ReadScriptString(fp, varvalue, sizeof(varvalue));
			scriptvars[std::string(varname)] = std::string(varvalue);
		} else if (_stricmp(cmd, "LOG") == 0) {
			char str[MAXLINE];
			ReadScriptString(fp, str, sizeof(str));
			plog("script log: %s", str);
		} else if (_stricmp(cmd, "SLEEP") == 0) {
			unsigned ms;
			int canskip;
			if (fscanf(fp, "%u%d", &ms, &canskip) == 2) {
				SleepDoModal(ms, !!canskip);
			}
		} else if (_stricmp(cmd, "CLS") == 0) {
			unsigned bgcolor;
			if (fscanf(fp, "%x", &bgcolor) == 1) {
				ClearScreen(bgcolor);
			}
		} else if (_stricmp(cmd, "PLAYVIDEO") == 0) {
			unsigned bgcolor;
			double vol;
			if (fscanf(fp, "%x%lf", &bgcolor, &vol) == 2) {
				char str[MAXLINE];
				ReadScriptString(fp, str, sizeof(str));
				VideoDoModal(str, bgcolor, vol);
			}
		} else if (_stricmp(cmd, "SHOWPICTURE") == 0) {
			unsigned bgcolor;
			unsigned ms;
			if (fscanf(fp, "%x%u", &bgcolor, &ms) == 2) {
				char str[MAXLINE];
				ReadScriptString(fp, str, sizeof(str));
				PictureDoModal(str, bgcolor, ms);
			}
		} else if (_stricmp(cmd, "STARTAUDIO") == 0) {
			double vol;
			if (fscanf(fp, "%lf", &vol) == 1) {
				char str[MAXLINE];
				ReadScriptString(fp, str, sizeof(str));
				AudioPrepare(str, vol);
				AudioStart();
			}
		} else if (_stricmp(cmd, "STOPAUDIO") == 0) {
			AudioStop();
		} else {
			fail("unknown command '%s' in script file '%s'.", cmd, scriptpath.c_str());
		}
	}
done:
	if (fp)	fclose(fp);
}



// text logger
static int log_counter = 0;
static void MakeTextLogHeader()
{
	FILE *fp = fopen(VDATALOG, "a");
	if (!fp) return;
	SYSTEMTIME SystemTime;
    GetLocalTime(&SystemTime);
    fprintf(fp, "\n\n; %04hu-%02hu-%02hu %02hu:%02hu:%02hu.%03hu\n\n", SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, SystemTime.wMilliseconds);
	fclose(fp);
}
static void LogVoiceData(const char *text)
{
	FILE *fp = fopen(VDATALOG, "a");
	if (!fp) return;
	fprintf(fp, "VOICE%04d.WAV|1.000|:|%08X|%s\n", log_counter, tools->misc->CalcStringGBCRC32(text), text);
	log_counter++;
	fclose(fp);
}



// config loader

struct VoiceItem {
	int file_num;
	int line_num;
	std::string audiofile;
	std::string scriptfile_pre;
	std::string scriptfile_post;
	double volume;
	unsigned texthash;
	std::string text;
};

static std::map<std::pair<unsigned, std::string>, std::vector<VoiceItem> > vdata;
static int text_enable = 1, caption_enable = 1, movie_enable = 1;
static int log_unknown = 0;

static int LoadSingleVoiceData(const char *filename, int file_num)
{
	std::string datapath = path_prefix + std::string(filename);

	FILE *fp = fopen(datapath.c_str(), "r");
	if (!fp) {
		fail("unable to open voice data file '%s'.", datapath.c_str());
		return 0;
	}
	
	int vitem_count = 0;
	int line_num = 0;
	char buf[MAXLINE];
	while (fgets(buf, sizeof(buf), fp)) {
		line_num++;

		if (!buf[0] || buf[0] == ';' || buf[0] == '\n') continue;

		char scriptbuf[MAXLINE];
		char audiofile[MAXLINE];
		double volume;
		unsigned texthash;
		char text[MAXLINE];

		if (sscanf(buf, "%[^|]|%lf|%[^|]|%x|%[^\n]", audiofile, &volume, scriptbuf, &texthash, text) == 5) {
			if (tools->misc->CalcStringGBCRC32(text) != texthash) {
				plog("WARN: unmatched text and hash at %s:%d", filename, line_num);
			}
			char *scriptfile_pre = scriptbuf;
			char *scriptfile_post = strchr(scriptbuf, ':');
			if (!scriptfile_post) {
				goto invalid_line;
			} else {
				*scriptfile_post++ = '\0';
			}

			struct VoiceItem vitem;
			vitem.file_num = file_num;
			vitem.line_num = line_num;
			vitem.audiofile = std::string(audiofile);
			vitem.scriptfile_pre = std::string(scriptfile_pre);
			vitem.scriptfile_post = std::string(scriptfile_post);
			vitem.volume = volume;
			vitem.texthash = texthash;
			vitem.text = std::string(text);
			vdata[std::make_pair(texthash, std::string(text))].push_back(vitem);
			vitem_count++;
		} else {
			goto invalid_line;
		}
		continue;
invalid_line:
		plog("invalid line at %s:%d", filename, line_num);
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
			ReadString(fp, buf, sizeof(buf));
			file_num++;
			vitem_count += LoadSingleVoiceData(buf, file_num);
		} else if (_stricmp(cmd, "PREFIX") == 0) {
			char buf[MAXLINE];
			ReadString(fp, buf, sizeof(buf));
			path_prefix = std::string(buf);
		} else if (_stricmp(cmd, "MUSICVOL") == 0) {
			double vol;
			if (fscanf(fp, "%lf", &vol) == 1) {
				tools->vol->SetMusicMasterVolume(vol);
			}
		} else if (_stricmp(cmd, "SOUNDVOL") == 0) {
			double vol;
			if (fscanf(fp, "%lf", &vol) == 1) {
				tools->vol->Set2DMasterVolume(vol);
				tools->vol->Set3DMasterVolume(vol);
			}
		} else if (_stricmp(cmd, "VOICEVOL") == 0) {
			fscanf(fp, "%lf", &voicevol);
		} else if (_stricmp(cmd, "LOGUNKNOWN") == 0) {
			fscanf(fp, "%d", &log_unknown);
		} else if (_stricmp(cmd, "LOGCOUNTFROM") == 0) {
			fscanf(fp, "%d", &log_counter);
		} else if (_stricmp(cmd, "TEXT") == 0) {
			fscanf(fp, "%d", &text_enable);
		} else if (_stricmp(cmd, "CAPTION") == 0) {
			fscanf(fp, "%d", &caption_enable);
		} else if (_stricmp(cmd, "MOVIE") == 0) {
			fscanf(fp, "%d", &movie_enable);
		} else {
			fail("unknown command '%s' in config file.", cmd);
		}
	}
	fclose(fp);

	if (log_unknown) {
		MakeTextLogHeader();
	}

	plog("total %d voice items", vitem_count);
}





// D3D lost and reset device
static void WINAPI OnLostDevice()
{
	StateBlockOnLostDevice();
}
static void WINAPI OnResetDevice()
{
	StateBlockOnResetDevice();
}

// statistics
static int text_total = 0, text_known = 0, text_unknown = 0;

// item selector
static int last_file_num = 0, last_line_num = 0, text_disabled = 0;
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

static void PrepareVoiceItem(const char *data, const char *type)
{
	if ((!text_enable && strcmp(type, TEXT_PREFIX) == 0) ||
		(!caption_enable && strcmp(type, CAPTION_PREFIX) == 0) ||
		(!movie_enable && strcmp(type, MOVIE_PREFIX) == 0)) {
		AudioStop();
		text_disabled++;
	} else {
		std::string text = std::string(type) + "|" + std::string(data);
		std::pair<unsigned, std::string> key(tools->misc->CalcStringGBCRC32(text.c_str()), text);
		std::map<std::pair<unsigned, std::string>, std::vector<VoiceItem> >::iterator it;
		if ((it = vdata.find(key)) != vdata.end()) {
			VoiceItem *pitem = ChooseBestVoiceItem(&it->second);
			AudioPrepare(pitem->audiofile.c_str(), pitem->volume);
			SetScript(pitem->scriptfile_pre.c_str(), pitem->scriptfile_post.c_str());
			text_known++;
		} else {
			if (log_unknown) LogVoiceData(text.c_str());
			text_unknown++;
		}
	}

	text_total++;
}



// plugin management functions
int WINAPI VoiceDLLAttached()
{
	return VOICEPLUGIN_ABI_VERSION;
}
void WINAPI VoiceInit(VoiceToolkit *toolkit)
{
	tools = toolkit;
	cleanup_log();
	plog("plugin init, plugin version: %s (build on %s)", PLUGIN_VERSION, PLUGIN_BUILDDATE);
	plog("host version: %s (built on %s)", tools->patch_version, tools->build_date);
	LoadVoiceConfig();
	InitStateBlock();
	TexQuadInit();
	tools->gfx->SetOnLostDeviceCallback(OnLostDevice);
	tools->gfx->SetOnResetDeviceCallback(OnResetDevice);
}
void WINAPI VoiceCleanup()
{
	AudioStop();
	plog("plugin unload, statistics: total %d, known %d (%.2f%%), unknown %d (%.2f%%), disabled %d (%.2f%%)", text_total, text_known, 100.0 * text_known / text_total, text_unknown, 100.0 * text_unknown / text_total, text_disabled, 100.0 * text_disabled / text_total);
}
void WINAPI GamePause()
{
	AudioPause();
	VideoPause();
}
void WINAPI GameResume()
{
	AudioResume();
	VideoResume();
}



// dialog text functions
void WINAPI TextIdle(int state)
{
}
void WINAPI TextPrepare(const char *text, int mode)
{
	if (mode != 0) return; // ignore timed dialog
	PrepareVoiceItem(text, TEXT_PREFIX);
}
void WINAPI TextStart()
{
	RunScriptPre();
	AudioStart();
}
void WINAPI TextStop()
{
	AudioStop();
	RunScriptPost();
}



// caption functions
void WINAPI CaptionIdle(int state)
{
}
void WINAPI CaptionPrepare(const char *tex)
{
	PrepareVoiceItem(strtolower(std::string(tex)).c_str(), CAPTION_PREFIX);
}
void WINAPI CaptionStart(void)
{
	RunScriptPre();
	AudioStart();
}
void WINAPI CaptionStop(void)
{
	// no AudioStop() here
	// because the voice may longer than caption display time
	RunScriptPost();
}



// movie functions
void WINAPI MovieIdle(int state)
{
	if (state) {
		HBINK hbink = *tools->bik->pMovieHandle;
		if (hbink) {
			AudioSync(hbink->FrameNum * 1000.0 / hbink->FrameRate * hbink->FrameRateDiv);
		}
	}
}
void WINAPI MoviePrepare(const char *movie)
{
	PrepareVoiceItem(strtolower(std::string(movie)).c_str(), MOVIE_PREFIX);
}
void WINAPI MovieStart(void)
{
	RunScriptPre();
	AudioStart();
}
void WINAPI MovieStop(void)
{
	AudioStop();
	RunScriptPost();
}



// combat dialog functions
void WINAPI CBDialogIdle(int state)
{
}
void WINAPI CBDialogPrepare(const char *text)
{
	PrepareVoiceItem(strtolower(std::string(text)).c_str(), CBDIALOG_PREFIX);
}
void WINAPI CBDialogStart(void)
{
	RunScriptPre();
	AudioStart();
}
void WINAPI CBDialogStop(void)
{
	AudioStop();
	RunScriptPost();
}
