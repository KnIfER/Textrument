#pragma once

#include "window.h"

struct HFontWrap {
	int fontHeight;
	HFONT hfont; 
};

typedef std::vector<HFontWrap>* FontStack;

void setFontStack(FontStack fontStack);

void fetchFontStack(HWND h);

BOOL CALLBACK SetChildFont(HWND hwndChild, LPARAM lParam);

void setWindowFont(HWND hwnd, HFONT hFont);

HFONT CreateFontIndirectly(int fontHeight, bool pushStack=false);