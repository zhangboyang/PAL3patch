#include <cstdio>
#include <utility>
#include <functional>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include "dxstdafx.h"

extern "C" int d3denum(FILE *fp, IDirect3D9 *pD3D);

static void EnumDisplayMode(FILE *fp, CD3DEnumeration *pD3DEnum)
{
	fprintf(fp, "displaymode");
	if (pD3DEnum->m_pAdapterInfoList->Count() > 0) {
        std::vector<std::pair<int, int> > dlist;
        std::vector<std::pair<int, int> >::iterator it;
		unsigned i;
		D3DAdapterInfo *pD3DAdapterInfo = (D3DAdapterInfo *) pD3DEnum->m_pAdapterInfoList->GetPtr(0); // First Adapter
		for (i = 0; i < pD3DAdapterInfo->pDisplayModeList->Count(); i++) {
			D3DDISPLAYMODE *pDisplayMode = (D3DDISPLAYMODE *) pD3DAdapterInfo->pDisplayModeList->GetPtr(i);
			dlist.push_back(std::make_pair(pDisplayMode->Width, pDisplayMode->Height));
		}
		std::sort(dlist.begin(), dlist.end(), std::greater<std::pair<int, int> > ());
		dlist.resize(std::unique(dlist.begin(), dlist.end()) - dlist.begin());
    	for (it = dlist.begin(); it != dlist.end(); it++) {
    		if (it->first >= 800 && it->second >= 600 && (it->first != 800 || it->second != 600)) {
    			fprintf(fp, " %dx%d", it->first, it->second);
    		}
    	}
	}
	fprintf(fp, "\n");
}

static void EnumDepthStencil(FILE *fp, CD3DEnumeration *pD3DEnum)
{
	fprintf(fp, "depthstencil");
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
						fprintf(fp, " %d", -fmt);
					}
				}
			}
		}
	}
	fprintf(fp, "\n");
}

static void EnumMultisample(FILE *fp, CD3DEnumeration *pD3DEnum)
{
	fprintf(fp, "multisample");
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
					fprintf(fp, " %d,%d", mtype, q);
				}
			}
		}
	}
	fprintf(fp, "\n");
}

int d3denum(FILE *fp, IDirect3D9 *pD3D)
{
    int ret;
    try {
        CD3DEnumeration D3DEnum;
        D3DEnum.ConfirmDeviceCallback = NULL;
        D3DEnum.AppUsesDepthBuffer = TRUE;
        D3DEnum.SetD3D(pD3D);
        D3DEnum.Enumerate();	
        EnumDisplayMode(fp, &D3DEnum);
        EnumDepthStencil(fp, &D3DEnum);
        EnumMultisample(fp, &D3DEnum);
        ret = 1;
    } catch (...) {
        ret = 0;
    }
    return ret;
}
