#include "stdafx.h"
#include "dxstdafx.h"

static IDirect3D9 *pD3D = NULL;
static CD3DEnumeration *pD3DEnum = NULL;

typedef IDirect3D9 * (WINAPI *typeofDirect3DCreate9)(UINT SDKVersion);

int CheckDX90SDKVersion()
{
	if (D3D_SDK_VERSION != 31) {
        MessageBox(NULL, _T("wrong D3D_SDK_VERSION."), NULL, MB_ICONERROR);
		return 0;
    }
    /* // uncomment this if linked with d3dx9.lib
	if (D3DX_SDK_VERSION != 21) {
        MessageBox(NULL, _T("wrong D3DX_SDK_VERSION."), NULL, MB_ICONERROR);
		return 0;
    }
	if (!D3DXCheckVersion(D3D_SDK_VERSION, D3DX_SDK_VERSION)) {
        return 0;
    }*/
	return 1;
}

int InitD3DEnumeration()
{
	HMODULE hD3D9;
	typeofDirect3DCreate9 pDirect3DCreate9;

	hD3D9 = LoadLibrary(_T("d3d9.dll"));
	if (!hD3D9) {
		goto fail;
	}
	pDirect3DCreate9 = (typeofDirect3DCreate9) GetProcAddress(hD3D9, "Direct3DCreate9");
	if (!pDirect3DCreate9) {
		goto fail;
	}
	pD3D = pDirect3DCreate9(D3D_SDK_VERSION);
	if (!pD3D) {
		goto fail;
	}
	pD3DEnum = new CD3DEnumeration;
	pD3DEnum->ConfirmDeviceCallback = NULL;
	pD3DEnum->AppUsesDepthBuffer = TRUE;
	pD3DEnum->SetD3D(pD3D);
	pD3DEnum->Enumerate();
	return 1;

fail:
	GetPleaseWaitDlg()->MessageBox(STRTABLE(IDS_NOD3D9), STRTABLE(IDS_NOD3D9_TITLE), MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
	return 0;
}

void CleanupD3DEnumeration()
{
	if (pD3DEnum) {
		delete pD3DEnum;
		pD3DEnum = NULL;
	}
	if (pD3D) {
		pD3D->Release();
		pD3D = NULL;
	}
}


EnumDisplayMode EnumDisplayModeInstance;
void EnumDisplayMode::EnumConfigValues(std::vector<CString> &result)
{
	std::vector<std::pair<int, int> > dlist;
	std::vector<std::pair<int, int> >::iterator it;

	if (pD3DEnum->m_pAdapterInfoList->Count() > 0) {
		unsigned i;
		D3DAdapterInfo *pD3DAdapterInfo = (D3DAdapterInfo *) pD3DEnum->m_pAdapterInfoList->GetPtr(0); // First Adapter
		for (i = 0; i < pD3DAdapterInfo->pDisplayModeList->Count(); i++) {
			D3DDISPLAYMODE *pDisplayMode = (D3DDISPLAYMODE *) pD3DAdapterInfo->pDisplayModeList->GetPtr(i);
			dlist.push_back(std::make_pair(pDisplayMode->Width, pDisplayMode->Height));
		}
		std::sort(dlist.begin(), dlist.end(), std::greater<std::pair<int, int> > ());
		dlist.resize(std::unique(dlist.begin(), dlist.end()) - dlist.begin());
	}

	result.clear();
	result.push_back(CString(_T("current")));
	for (it = dlist.begin(); it != dlist.end(); it++) {
		if (it->first >= 800 && it->second >= 600) {
			CString str;
			str.Format(_T("%dx%d"), it->first, it->second);
			result.push_back(str);
		}
	}
}
CString EnumDisplayMode::GetValueTitle(const CString &value)
{
	if (value == CString(_T("current"))) {
		return STRTABLE(IDS_AUTORESOLUTION);
	}
	return ConfigDescOptionListEnum::GetValueTitle(value);
}
CString EnumDisplayMode::GetValueDescription(const CString &value)
{
	if (value == CString(_T("current"))) {
		return STRTABLE(IDS_AUTORESOLUTION_DESC);
	}
	CString desc;
	desc.Format(IDS_RESOLUTION_DESC, (LPCTSTR)value);
	return desc;
}


EnumDepthBuffer EnumDepthBufferInstance;
void EnumDepthBuffer::EnumConfigValues(std::vector<CString> &result)
{
	result.clear();
	result.push_back(CString(_T("auto")));
	result.push_back(CString(_T("16")));
	result.push_back(CString(_T("24")));

	if (pD3DEnum->m_pAdapterInfoList->Count() > 0) {
		unsigned i, j;
		D3DFORMAT fmt;
		std::set<D3DFORMAT> fmtset;
		D3DAdapterInfo *pD3DAdapterInfo = (D3DAdapterInfo *) pD3DEnum->m_pAdapterInfoList->GetPtr(0); // First Adapter
		D3DDeviceInfo *pD3DDeviceInfo = (D3DDeviceInfo *) pD3DAdapterInfo->pDeviceInfoList->GetPtr(0); // HAL
		if (pD3DDeviceInfo->DevType == D3DDEVTYPE_HAL) {
			D3DDeviceCombo *pDeviceComboList;
			for (j = 0; j < pD3DDeviceInfo->pDeviceComboList->Count(); j++) {
				pDeviceComboList = (D3DDeviceCombo *) pD3DDeviceInfo->pDeviceComboList->GetPtr(j);
				for (i = 0; i < pDeviceComboList->pDepthStencilFormatList->Count(); i++) {
					fmt = *(D3DFORMAT *) pDeviceComboList->pDepthStencilFormatList->GetPtr(i);
					if (fmtset.insert(fmt).second) {
						CString str;
						str.Format(_T("%d"), -fmt);
						result.push_back(str);
					}
				}
			}
		}
	}
}
CString EnumDepthBuffer::GetValueTitle(const CString &value)
{
	if (value == CString(_T("auto"))) {
		return STRTABLE(IDS_AUTOSELECT);
	}
	if (value == CString(_T("16"))) {
		return STRTABLE(IDS_AUTOZBUF16);
	}
	if (value == CString(_T("24"))) {
		return STRTABLE(IDS_AUTOZBUF24);
	}
	int fmt;
	if (_stscanf(value, _T("-%d"), &fmt) == 1) {
		switch (fmt) {
		case D3DFMT_D16:
			return STRTABLE(IDS_ZBUF_D3DFMT_D16);
		case D3DFMT_D15S1:
			return STRTABLE(IDS_ZBUF_D3DFMT_D15S1);
		case D3DFMT_D24X8:
			return STRTABLE(IDS_ZBUF_D3DFMT_D24X8);
		case D3DFMT_D24S8:
			return STRTABLE(IDS_ZBUF_D3DFMT_D24S8);
		case D3DFMT_D24X4S4:
			return STRTABLE(IDS_ZBUF_D3DFMT_D24X4S4);
		case D3DFMT_D32:
			return STRTABLE(IDS_ZBUF_D3DFMT_D32);
		}
	}
	return ConfigDescOptionListEnum::GetValueTitle(value);
}
CString EnumDepthBuffer::GetValueDescription(const CString &value)
{
	if (value == CString(_T("auto"))) {
		return STRTABLE(IDS_AUTOSELECT_DESC);
	}
	return ConfigDescOptionListEnum::GetValueDescription(value);
}


EnumMultisample EnumMultisampleInstance;
void EnumMultisample::EnumConfigValues(std::vector<CString> &result)
{
	descmap.clear();
	result.clear();
	result.push_back(CString(_T("auto,auto")));
	result.push_back(CString(_T("0,0")));

	if (pD3DEnum->m_pAdapterInfoList->Count() > 0) {
		unsigned i, j;
		D3DMULTISAMPLE_TYPE mtype;
		int q, maxq;
		std::map<D3DMULTISAMPLE_TYPE, int> m;
		D3DAdapterInfo *pD3DAdapterInfo = (D3DAdapterInfo *) pD3DEnum->m_pAdapterInfoList->GetPtr(0); // First Adapter
		D3DDeviceInfo *pD3DDeviceInfo = (D3DDeviceInfo *) pD3DAdapterInfo->pDeviceInfoList->GetPtr(0); // HAL
		if (pD3DDeviceInfo->DevType == D3DDEVTYPE_HAL) {
			D3DDeviceCombo *pDeviceComboList;
			for (j = 0; j < pD3DDeviceInfo->pDeviceComboList->Count(); j++) {
				pDeviceComboList = (D3DDeviceCombo *) pD3DDeviceInfo->pDeviceComboList->GetPtr(j);
				for (i = 0; i < pDeviceComboList->pMultiSampleTypeList->Count(); i++) {
					mtype = *(D3DMULTISAMPLE_TYPE *) pDeviceComboList->pMultiSampleTypeList->GetPtr(i);
					maxq = *(DWORD *) pDeviceComboList->pMultiSampleQualityList->GetPtr(i);
					if (maxq > m[mtype]) m[mtype] = maxq;
				}
			}
			std::map<D3DMULTISAMPLE_TYPE, int>::reverse_iterator it;
			for (it = m.rbegin(); it != m.rend(); it++) {
				mtype = it->first;
				maxq = it->second;
				if (mtype == D3DMULTISAMPLE_NONE) continue;
				for (q = 0; q < maxq; q++) {
					CString key, val;
					if (mtype == D3DMULTISAMPLE_NONMASKABLE) {
						key.Format(IDS_MSAA_NONMASKABLE_FORMAT, q);
					} else {
						key.Format(IDS_MSAA_FORMAT, mtype, q);
					}
					val.Format(_T("%d,%d"), mtype, q);
					result.push_back(val);
					descmap.insert(std::make_pair(val, key));
				}
			}
		}
	}
}
CString EnumMultisample::GetValueTitle(const CString &value)
{
	if (value == CString(_T("auto,auto"))) {
		return STRTABLE(IDS_AUTOSELECT);
	}
	if (value == CString(_T("0,0"))) {
		return STRTABLE(IDS_MSAA_NONE);
	}
	std::map<CString, CString>::iterator it;
	if ((it = descmap.find(value)) != descmap.end()) {
		return it->second;
	}
	return ConfigDescOptionListEnum::GetValueTitle(value);
}
CString EnumMultisample::GetValueDescription(const CString &value)
{
	if (value == CString(_T("auto,auto"))) {
		return STRTABLE(IDS_AUTOSELECT_DESC);
	}
	if (value == CString(_T("0,0"))) {
		return STRTABLE(IDS_MSAA_NONE_DESC);
	}
	return ConfigDescOptionListEnum::GetValueDescription(value);
}
