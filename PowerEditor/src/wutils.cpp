
#ifndef WUTILS_H
#define WUTILS_H

#include "window.h"
#include "Common.h"
#include "wutils.h"
#include "Notepad_plus_msgs.h"
#include "InsituDebug.h"

LOGFONT logfont{};

FontStack HFontWraps = NULL;

void setFontStack(FontStack fontStack) {
	HFontWraps = fontStack;
}

void fetchFontStack(HWND h) {
	if(!HFontWraps) {
		HFontWraps = FontStack();
	} 
}

BOOL CALLBACK SetChildFont(HWND hwndChild, LPARAM lParam)
{
	SendMessage(hwndChild, WM_SETFONT, lParam, FALSE);
	return 1;
}

void setWindowFont(HWND hwnd, HFONT hFont) {
	EnumChildWindows(hwnd, SetChildFont, (LPARAM)hFont);
}

HFONT CreateFontIndirectly(int fontHeight, bool pushStack) {
	//ZeroMemory(&logfont, sizeof(LOGFONT));
	pushStack &= (bool)HFontWraps;
	if(pushStack) {
		for(auto hfI:*HFontWraps) {
			if(hfI.fontHeight==fontHeight) {
				//::MessageBox(NULL, TEXT("reuse!!!"), TEXT(""), MB_OK);
				return hfI.hfont;
			}
		}
	}
	logfont.lfCharSet = GB2312_CHARSET;
	logfont.lfHeight = fontHeight;
	//logfont.lfWeight = 550;
	//HFONT hFont = (HFONT)lParam;
	HFONT ret =  CreateFontIndirect(&logfont);
	//SendMessage(_hSelf,WM_SETFONT,(WPARAM)hFont,0);
	if(pushStack) {
		HFontWraps->push_back({fontHeight, ret});
	}
	return ret;
}

#endif