#include "stdafx.h"

void CheckForUpdates(CPatchConfigDlg *dlg)
{
#define MAXRESPONSE (32 * 1024)

#if defined(_MBCS)
	static char *msg_a = NULL;
#endif
	static wchar_t *response = NULL;

	bool retry;
	HINTERNET hInternet, hFile;
	CString url;
	CString msg;
	CString descloc;
	CString urlparam;
	CString website;
	char databuf[MAXRESPONSE];
	DWORD datalen, recvlen;
	DWORD httpcode, httpcodesz;
	DWORD r;
	unsigned acp;

retry:

	retry = false;
	hInternet = NULL;
	hFile = NULL;
	
	ShowPleaseWaitDlg(dlg, STRTABLE(IDS_WAITCHECKFORUPDATES));

	// format url param 
	acp = GetACP();
	descloc = STRTABLE(IDS_CONFIGDESCLIST);
	urlparam.Format(_T("?appname=%s&version=%hs&builton=%hs&compiler=%hs&locale=%s&codepage=%u"), PATCH_APPNAME, pVersionStr, pBuildDate, pCompiler, (LPCTSTR) descloc, acp);

	// open internet handle
	hInternet = InternetOpen(PATCH_UPDATEUA, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (!hInternet) goto fail;

	// make url and open it
	url = CString(PATCH_UPDATEURL) + urlparam;
	hFile = InternetOpenUrl(hInternet, url, NULL, 0, INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_UI | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 0);
	if (!hFile) goto fail;

	// query http status code
	httpcodesz = sizeof(httpcode);
	if (!HttpQueryInfo(hFile, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &httpcode, &httpcodesz, NULL)) goto fail;
	if (httpcode < 200 || httpcode >= 300) goto fail;

	// receive data
	datalen = 0;
	while (1) {
		if (datalen >= sizeof(databuf)) goto fail;
		if (InternetReadFile(hFile, databuf + datalen, sizeof(databuf) - datalen, &recvlen) == FALSE) goto fail;
		if (recvlen == 0) break;
		datalen += recvlen;
	}
	databuf[datalen] = 0;

	// decode response
	cs2wcs_managed((datalen >= 3 && strncmp(databuf, "\xEF\xBB\xBF", 3) == 0 ? databuf + 3 : databuf), CP_UTF8, &response);

	// parse response
#if defined(_UNICODE)
#define GET_RESPONSE_DATA(data, header) ((data) + wcslen(header))
#elif defined(_MBCS)
#define GET_RESPONSE_DATA(data, header) wcs2cs_managed((data) + wcslen(header), CP_ACP, &msg_a)
#endif

#define HEADER_NOUPDATES L"[NOUPDATES]"
	if (wcsncmp(response, HEADER_NOUPDATES, wcslen(HEADER_NOUPDATES)) == 0) {
		msg.Format(IDS_CHECKFORUPDATES_NONEWVERSION, GET_RESPONSE_DATA(response, HEADER_NOUPDATES));
		GetPleaseWaitDlg()->MessageBox(msg, STRTABLE(IDS_CHECKFORUPDATES_TITLE), MB_ICONINFORMATION);
		goto done;
	}

#define HEADER_NEWVERSION L"[NEWVERSION]"
	if (wcsncmp(response, HEADER_NEWVERSION, wcslen(HEADER_NEWVERSION)) == 0) {
		msg.Format(IDS_CHECKFORUPDATES_NEWVERSIONFOUND, GET_RESPONSE_DATA(response, HEADER_NEWVERSION));
		if (GetPleaseWaitDlg()->MessageBox(msg, STRTABLE(IDS_CHECKFORUPDATES_TITLE), MB_ICONQUESTION | MB_YESNO) == IDYES) {
			ShowPleaseWaitDlg(dlg, STRTABLE(IDS_WAITOPENWEBSITE));
			website = CString(PATCH_WEBSITEURL) + urlparam;
			r = (DWORD) ShellExecute(NULL, _T("open"), (LPCTSTR) website, NULL, NULL, SW_SHOWNORMAL);
			if (r <= 32) {
				GetPleaseWaitDlg()->MessageBox(STRTABLE(IDS_CANTOPENWEBSITE), STRTABLE(IDS_CHECKFORUPDATES_TITLE), MB_ICONERROR);
			}
		}
		goto done;
	}

#define HEADER_ERROR L"[ERROR]"
	if (wcsncmp(response, HEADER_ERROR, wcslen(HEADER_ERROR)) == 0) {
		msg.Format(IDS_CHECKFORUPDATES_ERROR, GET_RESPONSE_DATA(response, HEADER_ERROR));
		GetPleaseWaitDlg()->MessageBox(msg, STRTABLE(IDS_CHECKFORUPDATES_TITLE), MB_ICONERROR);
		goto done;
	}

#define HEADER_INFO L"[INFO]"
	if (wcsncmp(response, HEADER_INFO, wcslen(HEADER_INFO)) == 0) {
		msg.Format(IDS_CHECKFORUPDATES_INFO, GET_RESPONSE_DATA(response, HEADER_INFO));
		GetPleaseWaitDlg()->MessageBox(msg, STRTABLE(IDS_CHECKFORUPDATES_TITLE), MB_ICONINFORMATION);
		goto done;
	}

	goto fail;

done:
	if (hFile) InternetCloseHandle(hFile);
	if (hInternet) InternetCloseHandle(hInternet);

	if (retry) goto retry;

	DestroyPleaseWaitDlg(dlg);
	return;

fail:
	retry = (GetPleaseWaitDlg()->MessageBox(STRTABLE(IDS_CHECKFORUPDATES_RETRY), STRTABLE(IDS_CHECKFORUPDATES_TITLE), MB_RETRYCANCEL | MB_ICONWARNING) == IDRETRY);
	goto done;
}
