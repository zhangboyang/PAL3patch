#include "stdafx.h"
#include "D3DEnum.h"
#include "dxstdafx.h"
#include "Resource.h"

#include <set>
#include <functional>
#include <algorithm>


static IDirect3D9 *pD3D = NULL;
static CD3DEnumeration *pD3DEnum = NULL;

void InitD3DEnumeration()
{
	pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	pD3DEnum = new CD3DEnumeration;
	pD3DEnum->ConfirmDeviceCallback = NULL;
	pD3DEnum->AppUsesDepthBuffer = TRUE;
	pD3DEnum->SetD3D(pD3D);
	pD3DEnum->Enumerate();
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



void EnumDisplayMode(std::vector<std::pair<CString, CString> > &result)
{
	int i;
	std::vector<std::pair<int, int> > dlist;
	std::vector<std::pair<int, int> >::iterator it;
	D3DAdapterInfo *pD3DAdapterInfo = (D3DAdapterInfo *) pD3DEnum->m_pAdapterInfoList->GetPtr(0); // First Adapter
	for (i = 0; i < pD3DAdapterInfo->pDisplayModeList->Count(); i++) {
		D3DDISPLAYMODE *pDisplayMode = (D3DDISPLAYMODE *) pD3DAdapterInfo->pDisplayModeList->GetPtr(i);
		dlist.push_back(std::make_pair(pDisplayMode->Width, pDisplayMode->Height));
	}
	std::sort(dlist.begin(), dlist.end(), std::greater<std::pair<int, int> > ());
	dlist.resize(std::unique(dlist.begin(), dlist.end()) - dlist.begin());

	result.clear();
	result.push_back(std::make_pair(STRTABLE(IDS_AUTORESOLUTION), CString(_T("current"))));
	for (it = dlist.begin(); it != dlist.end(); it++) {
		if (it->first >= 800 && it->second >= 600) {
			CString str;
			str.Format(_T("%dx%d"), it->first, it->second);
			result.push_back(std::make_pair(str, str));
		}
	}
}

void EnumDepthBuffer(std::vector<std::pair<CString, CString> > &result)
{
	int i, j;
	D3DFORMAT fmt;
	std::set<D3DFORMAT> fmtset;
	D3DAdapterInfo *pD3DAdapterInfo = (D3DAdapterInfo *) pD3DEnum->m_pAdapterInfoList->GetPtr(0); // First Adapter
	D3DDeviceInfo *pD3DDeviceInfo = (D3DDeviceInfo *) pD3DAdapterInfo->pDeviceInfoList->GetPtr(0); // HAL
	D3DDeviceCombo *pDeviceComboList;
	result.clear();
	result.push_back(std::make_pair(STRTABLE(IDS_AUTOMAXIMUM), CString(_T("max"))));
	result.push_back(std::make_pair(STRTABLE(IDS_AUTOZBUF16), CString(_T("16"))));
	result.push_back(std::make_pair(STRTABLE(IDS_AUTOZBUF24), CString(_T("24"))));
	for (j = 0; j < pD3DDeviceInfo->pDeviceComboList->Count(); j++) {
		pDeviceComboList = (D3DDeviceCombo *) pD3DDeviceInfo->pDeviceComboList->GetPtr(j);
		for (i = 0; i < pDeviceComboList->pDepthStencilFormatList->Count(); i++) {
			fmt = *(D3DFORMAT *) pDeviceComboList->pDepthStencilFormatList->GetPtr(i);
			if (fmtset.insert(fmt).second) {
				CString str;
				str.Format(_T("%d"), -fmt);
				switch (fmt) {
					case D3DFMT_D16:
						result.push_back(std::make_pair(STRTABLE(IDS_ZBUF_D3DFMT_D16), str));
						break;
					case D3DFMT_D15S1:
						result.push_back(std::make_pair(STRTABLE(IDS_ZBUF_D3DFMT_D15S1), str));
						break;
					case D3DFMT_D24X8:
						result.push_back(std::make_pair(STRTABLE(IDS_ZBUF_D3DFMT_D24X8), str));
						break;
					case D3DFMT_D24S8:
						result.push_back(std::make_pair(STRTABLE(IDS_ZBUF_D3DFMT_D24S8), str));
						break;
					case D3DFMT_D24X4S4:
						result.push_back(std::make_pair(STRTABLE(IDS_ZBUF_D3DFMT_D24X4S4), str));
						break;
					case D3DFMT_D32:
						result.push_back(std::make_pair(STRTABLE(IDS_ZBUF_D3DFMT_D32), str));
						break;
					default:
						break;
				}
			}
		}
	}
}

void EnumMultisample(std::vector<std::pair<CString, CString> > &result)
{
	int i, j;
	int q, maxq;
	D3DMULTISAMPLE_TYPE mtype;
	std::set<D3DMULTISAMPLE_TYPE> mtypeset;
	D3DAdapterInfo *pD3DAdapterInfo = (D3DAdapterInfo *) pD3DEnum->m_pAdapterInfoList->GetPtr(0); // First Adapter
	D3DDeviceInfo *pD3DDeviceInfo = (D3DDeviceInfo *) pD3DAdapterInfo->pDeviceInfoList->GetPtr(0); // HAL
	D3DDeviceCombo *pDeviceComboList;
	result.clear();
	result.push_back(std::make_pair(STRTABLE(IDS_AUTOMAXIMUM), CString(_T("max,max"))));
	for (j = 0; j < pD3DDeviceInfo->pDeviceComboList->Count(); j++) {
		pDeviceComboList = (D3DDeviceCombo *) pD3DDeviceInfo->pDeviceComboList->GetPtr(j);
		for (i = 0; i < pDeviceComboList->pMultiSampleTypeList->Count(); i++) {
			mtype = *(D3DMULTISAMPLE_TYPE *) pDeviceComboList->pMultiSampleTypeList->GetPtr(i);
			maxq = *(DWORD *) pDeviceComboList->pMultiSampleQualityList->GetPtr(i);
			if (mtype != D3DMULTISAMPLE_NONMASKABLE && mtypeset.insert(mtype).second) {
				if (mtype == D3DMULTISAMPLE_NONE) {
					result.push_back(std::make_pair(STRTABLE(IDS_MSAA_NONE), CString(_T("0,0"))));
				} else {
					for (q = 0; q < maxq; q++) {
						CString key, val;
						key.Format(STRTABLE(IDS_MSAA_FORMAT), mtype, q);
						val.Format(_T("%d,%d"), mtype, q);
						result.push_back(std::make_pair(key, val));
					}
				}
			}
		}
	}
}
