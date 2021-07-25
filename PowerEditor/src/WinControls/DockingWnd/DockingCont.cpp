//this file is part of docking functionality for Notepad++
//Copyright (C)2006 Jens Lorenz <jens.plugin.npp@gmx.de>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
// Note that the GPL places important restrictions on "derived works", yet
// it does not provide a detailed definition of that term.  To avoid      
// misunderstandings, we consider an application to constitute a          
// "derivative work" for the purpose of this license if it does any of the
// following:                                                             
// 1. Integrates source code from Notepad++.
// 2. Integrates/includes/aggregates Notepad++ into a proprietary executable
//    installer, such as those produced by InstallShield.
// 3. Links to a library or executes a program that does any of the above.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "dockingResource.h"
#include "DockingCont.h"

#include "SplitterContainer.h"
#include "ToolTip.h"
#include "Parameters.h"

#include "Gripper.h"

#include "DockingDebug.h"

#include "Notepad_plus.h"

extern Notepad_plus *nppApp;

Gripper* pGripper_;

using namespace std;

#ifndef WH_MOUSE_LL
#define WH_MOUSE_LL 14
#endif

// window styles
#define POPUP_STYLES		(WS_POPUP|WS_CLIPSIBLINGS|WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_MAXIMIZEBOX|WS_MINIMIZEBOX)
#define POPUP_EXSTYLES		(WS_EX_CONTROLPARENT|WS_EX_WINDOWEDGE)
#define CHILD_STYLES		(WS_CHILD)
#define CHILD_EXSTYLES		(0x00000000L)

static HWND		hWndServer		= NULL;
static HHOOK	hookMouse		= NULL;

extern bool ClosePanelRequested;

static LRESULT CALLBACK hookProcMouse(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		switch (wParam)
		{
		case WM_MOUSEMOVE:
		case WM_NCMOUSEMOVE:
			::PostMessage(hWndServer, UINT(wParam), 0, 0);
			break;
		case WM_LBUTTONUP:
		case WM_NCLBUTTONUP:
			::PostMessage(hWndServer, UINT(wParam), 0, 0);
			break;

		default:
			break;
		}
	}

	return ::CallNextHookEx(hookMouse, nCode, wParam, lParam);
}


int DockingCont::AllDockerLen=0;
int DockingCont::AllDockerCapacity=2;
DockingCont** DockingCont::AllDockers=new DockingCont*[8]; // 八块腹肌

extern HWND StatusBarHWND;

extern bool Terminating;

DockingCont::DockingCont()
{
	_isMouseOver		= FALSE;
	_isMouseClose		= FALSE;
	_isMouseDown		= FALSE;
	_isFloating			= false;
	_isTopCaption		= CAPTION_TOP;
	_dragFromTab		= FALSE;
	_hContTab			= NULL;
	_hDefaultTabProc	= NULL;
	_beginDrag			= FALSE;
	_prevItem			= 0;
	_hFont				= NULL;
	_bTabTTHover		= FALSE;
	_bCaptionTT			= FALSE;
	_bCapTTHover		= FALSE;
	_hoverMPos			= posClose;
	_bDrawOgLine		= TRUE;
	_vTbData.clear();

	_captionHeightDynamic = NppParameters::getInstance()._dpiManager.scaleY(_captionHeightDynamic);
	_captionGapDynamic = NppParameters::getInstance()._dpiManager.scaleY(_captionGapDynamic);
	_closeButtonPosLeftDynamic = NppParameters::getInstance()._dpiManager.scaleX(_closeButtonPosLeftDynamic);
	_closeButtonPosTopDynamic = NppParameters::getInstance()._dpiManager.scaleY(_closeButtonPosTopDynamic);

	_closeButtonWidth = NppParameters::getInstance()._dpiManager.scaleX(12); // bitmap image is 12x12
	_closeButtonHeight = NppParameters::getInstance()._dpiManager.scaleY(12);

	putDocker(this);
}

void DockingCont::putDocker(DockingCont* item) {
	if(!AllDockers) { // sanity check
		AllDockers = new DockingCont*[AllDockerCapacity=8];
		AllDockerLen=0;
	}
	if(AllDockerLen+1>AllDockerCapacity) {
		int newCap = 2*(AllDockerLen+1);
		DockingCont** buff = new DockingCont*[newCap];
		memset(buff, 0, newCap*sizeof(DockingCont*));
		memcpy(buff, AllDockers, AllDockerLen*sizeof(DockingCont*));
		AllDockers = buff;
		AllDockerCapacity = newCap;
	}
	AllDockers[AllDockerLen++]=item;

	//LogIs("dockercc=%d", AllDockerLen);
}

void DockingCont::removeDocker(DockingCont* item){
	for(int i=0; i<AllDockerLen; i++) {
		if(AllDockers[i]==item) {
			memcpy(AllDockers[i], AllDockers[i+1], (AllDockerLen-i)*sizeof(DockingCont*));
			AllDockers[AllDockerLen--]=0;
			break;
		}
	}
	TCHAR buffer[100]={0};
	wsprintf(buffer,TEXT("docker--=%d"), AllDockerLen);
	SetWindowText(StatusBarHWND, buffer);
}

DockingCont::~DockingCont()
{
	if(!Terminating) {
		removeDocker(this);
	}
	::DeleteObject(_hFont);
}


void DockingCont::doDialog(bool willBeShown, bool isFloating)
{
	if (!isCreated())
	{
		create(IDD_CONTAINER_DLG);

		_isFloating  = isFloating;

		if (_isFloating)
		{
			::SetWindowLongPtr(_hSelf, GWL_STYLE, POPUP_STYLES);
			::SetWindowLongPtr(_hSelf, GWL_EXSTYLE, POPUP_EXSTYLES);
			::ShowWindow(_hCaption, SW_HIDE);
		}
		else
		{
			::SetWindowLongPtr(_hSelf, GWL_STYLE, CHILD_STYLES);
			::SetWindowLongPtr(_hSelf, GWL_EXSTYLE, CHILD_EXSTYLES);
			::ShowWindow(_hCaption, SW_SHOW);
		}

		//If you want defualt GUI font
		_hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	}

	display(willBeShown);
}


tTbData* DockingCont::createToolbar(tTbData data, bool visible)
{
	tTbData *pTbData = new tTbData;

	*pTbData = data;

	// force window style of client window
	::SetWindowLongPtr(pTbData->hClient, GWL_STYLE, CHILD_STYLES);
	::SetWindowLongPtr(pTbData->hClient, GWL_EXSTYLE, CHILD_EXSTYLES);

	// restore position if plugin is in floating state
	if ((_isFloating) && (::SendMessage(_hContTab, TCM_GETITEMCOUNT, 0, 0) == 0))
	{
		reSizeToWH(pTbData->rcFloat);
	}

	// set attached child window
    ::SetParent(pTbData->hClient, ::GetDlgItem(_hSelf, IDC_CLIENT_TAB));

	if(visible)
	// set names for captions and view toolbar
	viewToolbar(pTbData);

	// attach to list
	_vTbData.push_back(pTbData);

	return pTbData;
}


void DockingCont::removeToolbar(tTbData TbData, bool activate)
{
	// remove from list
	// items in _vTbData are removed in the loop so _vTbData.size() should be checked in every iteration
	for (size_t iTb = 0 ; iTb < _vTbData.size(); ++iTb)
	{
		if (_vTbData[iTb]->hClient == TbData.hClient)
		{
			// remove tab
			//removeTab(_vTbData[iTb]);
			hideToolbar(_vTbData[iTb], FALSE, activate);

			// free resources
			delete _vTbData[iTb];
			vector<tTbData*>::iterator itr = _vTbData.begin() + iTb;
			_vTbData.erase(itr);
		}
	}
}


tTbData* DockingCont::findToolbarByWnd(HWND hClient)
{
	tTbData*	pTbData		= NULL;

	// find entry by handle
	for (size_t iTb = 0, len = _vTbData.size(); iTb < len; ++iTb)
	{
		if (hClient == _vTbData[iTb]->hClient)
		{
			pTbData = _vTbData[iTb];
		}
	}
	return pTbData;
}

tTbData* DockingCont::findToolbarByName(TCHAR* pszName)
{
	tTbData*	pTbData		= NULL;

	// find entry by handle
	for (size_t iTb = 0, len = _vTbData.size(); iTb < len; ++iTb)
	{
		if (lstrcmp(pszName, _vTbData[iTb]->pszName) == 0)
		{
			pTbData = _vTbData[iTb];
		}
	}
	return pTbData;
}

void DockingCont::setActiveTb(tTbData* pTbData)
{
	int iItem = searchPosInTab(pTbData);
	setActiveTb(iItem);
}

void DockingCont::setActiveTb(int iItem)
{
	if (iItem < ::SendMessage(_hContTab, TCM_GETITEMCOUNT, 0, 0))
	{
		selectTab(iItem);
	}
}

int DockingCont::getActiveTb()
{
	return static_cast<int32_t>(::SendMessage(_hContTab, TCM_GETCURSEL, 0, 0));
}

tTbData* DockingCont::getDataOfActiveTb()
{
	tTbData*	pTbData	= NULL;
	int			iItem	= getActiveTb();

	if (iItem != -1)
	{
		TCITEM	tcItem	= {0};

		tcItem.mask		= TCIF_PARAM;
		::SendMessage(_hContTab, TCM_GETITEM, iItem, reinterpret_cast<LPARAM>(&tcItem));
		pTbData = (tTbData*)tcItem.lParam;
	}

	return pTbData;
}

vector<tTbData*> DockingCont::getDataOfVisTb()
{
	vector<tTbData*> vTbData;
	TCITEM tcItem = {0};
	int iItemCnt = static_cast<int32_t>(::SendMessage(_hContTab, TCM_GETITEMCOUNT, 0, 0));

	tcItem.mask	= TCIF_PARAM;

	for (int iItem = 0; iItem < iItemCnt; ++iItem)
	{
		::SendMessage(_hContTab, TCM_GETITEM, iItem, reinterpret_cast<LPARAM>(&tcItem));
		vTbData.push_back((tTbData*)tcItem.lParam);
	}
	return vTbData;
}

bool DockingCont::isTbVis(tTbData* data)
{
	TCITEM tcItem = {0};
	int iItemCnt = static_cast<int32_t>(::SendMessage(_hContTab, TCM_GETITEMCOUNT, 0, 0));

	tcItem.mask	= TCIF_PARAM;

	for (int iItem = 0; iItem < iItemCnt; ++iItem)
	{
		::SendMessage(_hContTab, TCM_GETITEM, iItem, reinterpret_cast<LPARAM>(&tcItem));
		if (!tcItem.lParam)
			return false;
		if (((tTbData*)tcItem.lParam) == data)
			return true;
	}
	return false;
}


//
//    Process function of caption bar
//
LRESULT DockingCont::runProcCaption(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	static ToolTip	toolTip;

	switch (Message)
	{
		case WM_LBUTTONDOWN:
		{
			_isMouseDown = TRUE;

			if (isInRect(hwnd, LOWORD(lParam), HIWORD(lParam)) == posClose)
			{
				_isMouseClose	= TRUE;
				_isMouseOver	= TRUE;

				// start hooking
				hWndServer		= _hCaption;
				hookMouse = ::SetWindowsHookEx(WH_MOUSE_LL, hookProcMouse, _hInst, 0);

				if (!hookMouse)
				{
					DWORD dwError = ::GetLastError();
					TCHAR  str[128];
					::wsprintf(str, TEXT("GetLastError() returned %lu"), dwError);
					::MessageBox(NULL, str, TEXT("SetWindowsHookEx(MOUSE) failed on runProcCaption"), MB_OK | MB_ICONERROR);
				}
				::RedrawWindow(hwnd, NULL, NULL, TRUE);
			}

			focusClient();
			return TRUE;
		}
		case WM_LBUTTONUP:
		{
			_isMouseDown = FALSE;
			if (_isMouseClose == TRUE)
			{
				ClosePanelRequested=1;
				// end hooking
				::UnhookWindowsHookEx(hookMouse);

				if (_isMouseOver == TRUE)
				{
					doClose();
				}
				_isMouseClose	= FALSE;
				_isMouseOver	= FALSE;
				ClosePanelRequested=0;
			}
			
			focusClient();
			return TRUE;
		}
		case WM_LBUTTONDBLCLK:
		{
			// 双击标题栏

			if (isInRect(hwnd, LOWORD(lParam), HIWORD(lParam)) == posCaption)
				::SendMessage(_hParent, DMM_FLOATALL, 0, reinterpret_cast<LPARAM>(this));

			focusClient();

			return TRUE;
		}
		case WM_MOUSEMOVE:
		{
			POINT	pt			= {0};

			// get correct cursor position
			::GetCursorPos(&pt);
			::ScreenToClient(_hCaption, &pt);

			if (_isMouseDown == TRUE)
			{
				if (_isMouseClose == FALSE)
				{
                    // keep sure that button is still down and within caption
                    if ((wParam == MK_LBUTTON) && (isInRect(hwnd, pt.x, pt.y) == posCaption))
                    {
    					_dragFromTab = FALSE;
    					NotifyParent(DMM_MOVE);
    					_isMouseDown = FALSE;
                    }
                    else
                    {
                        _isMouseDown = FALSE;
                    }
				}
				else
				{
					BOOL    isMouseOver	= _isMouseOver;
					_isMouseOver = (isInRect(hwnd, pt.x, pt.y) == posClose ? TRUE : FALSE);

					// if state is changed draw new
					if (_isMouseOver != isMouseOver)
					{
						::SetFocus(NULL);
						::RedrawWindow(hwnd, NULL, NULL, TRUE);
					}
				}
			}
			else if (_bCapTTHover == FALSE)
			{
				_hoverMPos = isInRect(hwnd, LOWORD(lParam), HIWORD(lParam));

				if ((_bCaptionTT == TRUE) || (_hoverMPos == posClose))
				{
					TRACKMOUSEEVENT tme;
					tme.cbSize = sizeof(tme);
					tme.hwndTrack = hwnd;
					tme.dwFlags = TME_LEAVE | TME_HOVER;
					tme.dwHoverTime = 1000;
					_bCapTTHover = _TrackMouseEvent(&tme);
				}
			}
			else if ((_bCapTTHover == TRUE) &&
				(_hoverMPos != isInRect(hwnd, LOWORD(lParam), HIWORD(lParam))))
			{
				toolTip.destroy();
				_bCapTTHover = FALSE;
			}
			//return TRUE;
			break;
		}
		case WM_MOUSEHOVER:
		{
			RECT	rc	= {0};
			POINT	pt	= {0};


			// get mouse position
			::GetCursorPos(&pt);

			toolTip.init(_hInst, hwnd);
			if (_hoverMPos == posCaption)
			{
				toolTip.Show(rc, _pszCaption.c_str(), pt.x, pt.y + 20);
			}
			else
			{
				toolTip.Show(rc, TEXT("关闭"), pt.x, pt.y + 20);
			}
			return TRUE;
		}
		case WM_MOUSELEAVE:
		{
			toolTip.destroy();
			_bCapTTHover = FALSE;
			return TRUE;
		}
		case WM_SIZE:
		{
			::GetWindowRect(hwnd, &_rcCaption);
			ScreenRectToClientRect(hwnd, &_rcCaption);
			break;
		}
		case WM_SETTEXT:
		{
			::RedrawWindow(hwnd, NULL, NULL, TRUE);
			return TRUE;
		}
		default:
			break;
	}

	return ::CallWindowProc(_hDefaultCaptionProc, hwnd, Message, wParam, lParam);
}

void DockingCont::drawCaptionItem(DRAWITEMSTRUCT *pDrawItemStruct)
{
	HBRUSH		bgbrush		= NULL;
	HFONT		hOldFont	= NULL;
	RECT		rc			= pDrawItemStruct->rcItem;
	HDC			hDc			= pDrawItemStruct->hDC;
	HPEN		hPen		= ::CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_BTNSHADOW));
	BITMAP		bmp			= {0};
	HBITMAP		hBmpCur		= NULL;
	HBITMAP		hBmpOld 	= NULL;
	HBITMAP		hBmpNew		= NULL;
	int length = static_cast<int32_t>(_pszCaption.length());

	int nSavedDC			= ::SaveDC(hDc);

	// begin with paint
	::SetBkMode(hDc, TRANSPARENT);

	auto holdPen = static_cast<HPEN>(::SelectObject(hDc, NppDarkMode::isEnabled() ? NppDarkMode::getEdgePen() : hPen));

	if (NppDarkMode::isEnabled())
	{
		bgbrush = ::CreateSolidBrush(_isActive ? NppDarkMode::getSofterBackgroundColor() : NppDarkMode::getBackgroundColor());
		SetTextColor(hDc, NppDarkMode::getTextColor());
	}
	else
	{
		if (_isActive == TRUE)
		{
			bgbrush = ::CreateSolidBrush(::GetSysColor(COLOR_ACTIVECAPTION));
			::SetTextColor(hDc, ::GetSysColor(COLOR_CAPTIONTEXT));
		}
		else
		{
			bgbrush = ::CreateSolidBrush(::GetSysColor(COLOR_BTNFACE));
		}
	}

	// set text and/or caption grid
	if (_isTopCaption == TRUE)
	{
		if (_isActive == TRUE)
		{
			// fill background
			::FillRect(hDc, &rc, bgbrush);
			rc.right	-= 1;
			rc.bottom	-= 1;
		}
		else
		{
			// fill background
			rc.right	-= 1;
			rc.bottom	-= 1;
			::FillRect(hDc, &rc, bgbrush);

			// draw grid lines

			MoveToEx(hDc, rc.left , rc.top , NULL);
			LineTo  (hDc, rc.right, rc.top );
			LineTo  (hDc, rc.right, rc.bottom );
			LineTo  (hDc, rc.left , rc.bottom );
			LineTo  (hDc, rc.left , rc.top);
		}

		// draw text
		rc.left		+= 2;
		rc.top		+= 1;
		rc.right	-= 16;
		hOldFont = (HFONT)::SelectObject(hDc, _hFont);
		::DrawText(hDc, _pszCaption.c_str(), length, &rc, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

		// calculate text size and if its trankated...
		SIZE	size	= {0};
		GetTextExtentPoint32(hDc, _pszCaption.c_str(), length, &size);
		_bCaptionTT = (((rc.right - rc.left) < size.cx) ? TRUE : FALSE);

		::SelectObject(hDc, hOldFont);
	}
	else
	{
		// create local font for vertical draw
		HFONT	hFont;

		if (_isActive == TRUE)
		{
			// fill background
			::FillRect(hDc, &rc, bgbrush);
			rc.right	-= 1;
			rc.bottom	-= 1;
		}
		else
		{
			// fill background
			rc.right	-= 1;
			rc.bottom	-= 1;
			::FillRect(hDc, &rc, bgbrush);

			// draw grid lines
			MoveToEx(hDc, rc.left , rc.top , NULL);
			LineTo  (hDc, rc.right, rc.top );
			LineTo  (hDc, rc.right, rc.bottom );
			LineTo  (hDc, rc.left , rc.bottom );
			LineTo  (hDc, rc.left , rc.top);
		}

		// draw text
		rc.left		+= 1;
		rc.top += _captionHeightDynamic;
		// to make ellipsis working
		rc.right	= rc.bottom - rc.top;
		rc.bottom	+= 14;

		hFont = ::CreateFont(12, 0, 90 * 10, 0,
			 FW_NORMAL, FALSE, FALSE, FALSE,
			 ANSI_CHARSET, OUT_DEFAULT_PRECIS,
			 CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			 DEFAULT_PITCH | FF_ROMAN,
			 TEXT("MS Shell Dlg"));

		hOldFont = (HFONT)::SelectObject(hDc, hFont);
		::DrawText(hDc, _pszCaption.c_str(), length, &rc, DT_BOTTOM | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

		// calculate text size and if its trankated...
		SIZE	size	= {0};
		GetTextExtentPoint32(hDc, _pszCaption.c_str(), length, &size);
		_bCaptionTT = (((rc.bottom - rc.top) < size.cy) ? TRUE : FALSE);

		::SelectObject(hDc, hOldFont);
		::DeleteObject(hFont);
	}
	::SelectObject(hDc, holdPen);
	::DeleteObject(hPen);
	::DeleteObject(bgbrush);

	// draw button

	if (NppDarkMode::isEnabled())
	{
		::SelectObject(hDc, NppParameters::getInstance().getDefaultUIFont());

		rc = pDrawItemStruct->rcItem;
		if (_isTopCaption == TRUE)
		{
			rc.left = rc.right - _closeButtonWidth - _closeButtonPosLeftDynamic;
		}
		else
		{
			rc.bottom = rc.top + _closeButtonWidth + _closeButtonPosLeftDynamic; // non-dark uses Left instead of Top for the button pos so being consistent
		}

		if ((_isMouseOver == TRUE) && (_isMouseDown == TRUE))
		{
			::SetTextColor(hDc, RGB(0xFF, 0xFF, 0xFF));
		}

		::DrawText(hDc, L"x", 1, &rc, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
	}
	else
	{

		HDC dcMem = ::CreateCompatibleDC(NULL);

		// select correct bitmap
		if ((_isMouseOver == TRUE) && (_isMouseDown == TRUE))
			hBmpCur = (HBITMAP)::LoadImage(_hInst, MAKEINTRESOURCE(IDB_CLOSE_DOWN), IMAGE_BITMAP, _closeButtonWidth, _closeButtonHeight, 0);
		else
			hBmpCur = (HBITMAP)::LoadImage(_hInst, MAKEINTRESOURCE(IDB_CLOSE_UP), IMAGE_BITMAP, _closeButtonWidth, _closeButtonHeight, 0);

		// blit bitmap into the destination
		::GetObject(hBmpCur, sizeof(bmp), &bmp);
		hBmpOld = (HBITMAP)::SelectObject(dcMem, hBmpCur);
		hBmpNew = ::CreateCompatibleBitmap(dcMem, bmp.bmWidth, bmp.bmHeight);

		rc = pDrawItemStruct->rcItem;
		::SelectObject(hDc, hBmpNew);

		if (_isTopCaption == TRUE)
			::BitBlt(hDc, rc.right - bmp.bmWidth - _closeButtonPosLeftDynamic, _closeButtonPosTopDynamic, bmp.bmWidth, bmp.bmHeight, dcMem, 0, 0, SRCCOPY);
		else
			::BitBlt(hDc, _closeButtonPosLeftDynamic, _closeButtonPosLeftDynamic, bmp.bmWidth, bmp.bmHeight, dcMem, 0, 0, SRCCOPY);

		::SelectObject(dcMem, hBmpOld);
		::DeleteObject(hBmpCur);
		::DeleteObject(hBmpNew);
		::DeleteDC(dcMem);
	}

	::RestoreDC(hDc, nSavedDC);
}

eMousePos DockingCont::isInRect(HWND hwnd, int x, int y)
{
	RECT		rc;
	eMousePos	ret	= posOutside;

	::GetWindowRect(hwnd, &rc);
	ScreenRectToClientRect(hwnd, &rc);

	if (_isTopCaption == TRUE)
	{
		if ((x > rc.left) && (x < rc.right - _captionHeightDynamic) && (y > rc.top) && (y < rc.bottom))
		{
			ret = posCaption;
		}
		else if ((x > rc.right - (_closeButtonWidth + _closeButtonPosLeftDynamic)) && (x < (rc.right - _closeButtonPosLeftDynamic)) &&
			(y >(rc.top + _closeButtonPosTopDynamic)) && (y < (rc.bottom - _closeButtonPosTopDynamic)))
		{
			ret = posClose;
		}
	}
	else
	{
		if ((x > rc.left) && (x < rc.right) && (y > rc.top + _captionHeightDynamic) && (y < rc.bottom))
		{
			ret = posCaption;
		}
		else if ((x > rc.left + _closeButtonPosLeftDynamic) && (x < rc.right - _closeButtonPosLeftDynamic) &&
			(y >(rc.top + _closeButtonPosTopDynamic)) && (y < (rc.top + (_closeButtonHeight + _closeButtonPosLeftDynamic))))
		{
			ret = posClose;
		}
	}

	return ret;
}


//----------------------------------------------------------
//    Process function of tab
//
LRESULT DockingCont::runProcTab(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	static	ToolTip	toolTip;

	switch (Message)
	{
				case WM_ERASEBKGND:
		{
			if (!NppDarkMode::isEnabled())
			{
				break;
			}

			return TRUE;
		}

		case WM_PAINT:
		{
			if (!NppDarkMode::isEnabled())
			{
				break;
			}

			LONG_PTR dwStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
			if (!(dwStyle & TCS_OWNERDRAWFIXED))
			{
				break;
			}

			PAINTSTRUCT ps;
			HDC hdc = ::BeginPaint(hwnd, &ps);
			::FillRect(hdc, &ps.rcPaint, NppDarkMode::getDarkerBackgroundBrush());

			UINT id = ::GetDlgCtrlID(hwnd);

			auto holdPen = static_cast<HPEN>(::SelectObject(hdc, NppDarkMode::getEdgePen()));

			HRGN holdClip = CreateRectRgn(0, 0, 0, 0);
			if (1 != GetClipRgn(hdc, holdClip))
			{
				DeleteObject(holdClip);
				holdClip = nullptr;
			}

			int nTabs = TabCtrl_GetItemCount(hwnd);
			int nFocusTab = TabCtrl_GetCurFocus(hwnd);
			int nSelTab = TabCtrl_GetCurSel(hwnd);
			for (int i = 0; i < nTabs; ++i)
			{
				DRAWITEMSTRUCT dis = { ODT_TAB, id, (UINT)i, ODA_DRAWENTIRE, ODS_DEFAULT, hwnd, hdc };
				TabCtrl_GetItemRect(hwnd, i, &dis.rcItem);

				if (i == nFocusTab)
				{
					dis.itemState |= ODS_FOCUS;
				}
				if (i == nSelTab)
				{
					dis.itemState |= ODS_SELECTED;
				}

				dis.itemState |= ODS_NOFOCUSRECT; // maybe, does it handle it already?

				RECT rcIntersect = { 0 };
				if (IntersectRect(&rcIntersect, &ps.rcPaint, &dis.rcItem))
				{
					dis.rcItem.top += NppParameters::getInstance()._dpiManager.scaleY(1);
					dis.rcItem.right -= 1;
					dis.rcItem.bottom += 2;

					if (i == 0)
					{
						POINT edges[] = {
							{dis.rcItem.left - 1, dis.rcItem.top},
							{dis.rcItem.left - 1, dis.rcItem.bottom}
						};
						Polyline(hdc, edges, _countof(edges));
					}
					
					{
						POINT edges[] = {
							{dis.rcItem.right, dis.rcItem.top},
							{dis.rcItem.right, dis.rcItem.bottom}
						};
						Polyline(hdc, edges, _countof(edges));
					}

					HRGN hClip = CreateRectRgnIndirect(&dis.rcItem);

					SelectClipRgn(hdc, hClip);

					drawTabItem(&dis);

					DeleteObject(hClip);

					SelectClipRgn(hdc, holdClip);
				}
			}

			SelectClipRgn(hdc, holdClip);
			if (holdClip)
			{
				DeleteObject(holdClip);
				holdClip = nullptr;
			}

			SelectObject(hdc, holdPen);

			EndPaint(hwnd, &ps);
			return 0;
		}

		case WM_LBUTTONDOWN:
		{
			_beginDrag	= TRUE;
			return TRUE;
		}
		case WM_LBUTTONUP:
		{
			int				iItem	= 0;
			TCHITTESTINFO	info	= {0};

			// get selected sub item
			info.pt.x = LOWORD(lParam);
			info.pt.y = HIWORD(lParam);
			iItem = static_cast<int32_t>(::SendMessage(hwnd, TCM_HITTEST, 0, reinterpret_cast<LPARAM>(&info)));

			selectTab(iItem);
			_beginDrag = FALSE;
			return TRUE;
		}
		case WM_LBUTTONDBLCLK:
		{
			NotifyParent((_isFloating == true)?DMM_DOCK:DMM_FLOAT);
			return TRUE;
		}
		case WM_MBUTTONUP:
		{
			int				iItem	= 0;
			TCITEM			tcItem	= {0};
			TCHITTESTINFO	info	= {0};

			// get selected sub item
			info.pt.x = LOWORD(lParam);
			info.pt.y = HIWORD(lParam);
			iItem = static_cast<int32_t>(::SendMessage(hwnd, TCM_HITTEST, 0, reinterpret_cast<LPARAM>(&info)));

			selectTab(iItem);

			// get data and hide toolbar
			tcItem.mask		= TCIF_PARAM;
			::SendMessage(hwnd, TCM_GETITEM, iItem, reinterpret_cast<LPARAM>(&tcItem));

			if (!tcItem.lParam)
				return FALSE;

			// notify child windows
			if (NotifyParent(DMM_CLOSE) == 0)
			{
				hideToolbar((tTbData*)tcItem.lParam);
			}
			return TRUE;
		}

		case WM_MOUSEMOVE:
		{
			int				iItem	= 0;
			TCHITTESTINFO	info	= {0};

			// get selected sub item
			info.pt.x = LOWORD(lParam);
			info.pt.y = HIWORD(lParam);
			iItem = static_cast<int32_t>(::SendMessage(hwnd, TCM_HITTEST, 0, reinterpret_cast<LPARAM>(&info)));

			if ((_beginDrag == TRUE) && (wParam == MK_LBUTTON))
			{
				selectTab(iItem);

				// send moving message to parent window
				_dragFromTab = TRUE;
				NotifyParent(DMM_MOVE);
				_beginDrag = FALSE;
			}
            else
            {
				int	iItemSel = static_cast<int32_t>(::SendMessage(hwnd, TCM_GETCURSEL, 0, 0));

				if ((_bTabTTHover == FALSE) && (iItem != iItemSel))
				{
					TRACKMOUSEEVENT tme;
					tme.cbSize = sizeof(tme);
					tme.hwndTrack = hwnd;
					tme.dwFlags = TME_LEAVE | TME_HOVER;
					tme.dwHoverTime = 1000;
					_bTabTTHover = _TrackMouseEvent(&tme);
				}
				else
				{
					if (iItem == iItemSel)
					{
						toolTip.destroy();
						_bTabTTHover = FALSE;
					}
					else if (iItem != _iLastHovered)
					{
						TCITEM	tcItem	= {0};
						RECT	rc		= {0};

						// destroy old tooltip
						toolTip.destroy();

						// recalc mouse position
						::ClientToScreen(hwnd, &info.pt);

						// get text of toolbar
						tcItem.mask		= TCIF_PARAM;
						::SendMessage(hwnd, TCM_GETITEM, iItem, reinterpret_cast<LPARAM>(&tcItem));
						if (!tcItem.lParam)
							return FALSE;

						toolTip.init(_hInst, hwnd);
						toolTip.Show(rc, (reinterpret_cast<tTbData*>(tcItem.lParam))->pszName, info.pt.x, info.pt.y + 20);
					}
				}

				// save last hovered item
				_iLastHovered = iItem;

				_beginDrag = FALSE;
			}
			return TRUE;
			break;
		}

		case WM_MOUSEHOVER:
		{
			int				iItem	= 0;
			TCITEM			tcItem	= {0};
			RECT			rc		= {0};
			TCHITTESTINFO	info	= {0};

			// get selected sub item
			info.pt.x = LOWORD(lParam);
			info.pt.y = HIWORD(lParam);
			iItem = static_cast<int32_t>(::SendMessage(hwnd, TCM_HITTEST, 0, reinterpret_cast<LPARAM>(&info)));

			// recalc mouse position
			::ClientToScreen(hwnd, &info.pt);

			// get text of toolbar
			tcItem.mask		= TCIF_PARAM;
			::SendMessage(hwnd, TCM_GETITEM, iItem, reinterpret_cast<LPARAM>(&tcItem));
			if (!tcItem.lParam)
				return FALSE;

			toolTip.init(_hInst, hwnd);
			toolTip.Show(rc, ((tTbData*)tcItem.lParam)->pszName, info.pt.x, info.pt.y + 20);
			return TRUE;
		}

		case WM_MOUSELEAVE:
		{
			toolTip.destroy();
			_bTabTTHover = FALSE;
			return TRUE;
		}

		case WM_NOTIFY:
		{
			LPNMHDR	lpnmhdr = (LPNMHDR)lParam;

			if ((lpnmhdr->hwndFrom == _hContTab) && (lpnmhdr->code == TCN_GETOBJECT))
			{
				int				iItem	= 0;
				TCHITTESTINFO	info	= {0};

				// get selected sub item
				info.pt.x = LOWORD(lParam);
				info.pt.y = HIWORD(lParam);
				iItem = static_cast<int32_t>(::SendMessage(hwnd, TCM_HITTEST, 0, reinterpret_cast<LPARAM>(&info)));

				selectTab(iItem);
			}
			break;
		}
		default:
			break;
	}

	return ::CallWindowProc(_hDefaultTabProc, hwnd, Message, wParam, lParam);
}


LRESULT DockingCont::runProc1(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{

	switch (Message) 
	{
		case WM_NCHITTEST:
		{
			return HTTRANSPARENT;
			break;
		}
	}
	//return run_dlgProc(Message, wParam, lParam);

	return ::CallWindowProc(_hDefaultProc1, hwnd, Message, wParam, lParam);
}

void DockingCont::drawTabItem(DRAWITEMSTRUCT *pDrawItemStruct)
{
	TCITEM	tcItem		= {0};
	RECT	rc			= pDrawItemStruct->rcItem;
	
	int		nTab		= pDrawItemStruct->itemID;
	bool	isSelected	= (nTab == getActiveTb());

	// get current selected item
	tcItem.mask = TCIF_PARAM;
	::SendMessage(_hContTab, TCM_GETITEM, nTab, reinterpret_cast<LPARAM>(&tcItem));
	if (!tcItem.lParam)
		return;

	const TCHAR *text = reinterpret_cast<tTbData*>(tcItem.lParam)->pszName;
	int		length = lstrlen(reinterpret_cast<tTbData*>(tcItem.lParam)->pszName);


	// get drawing context
	HDC hDc = pDrawItemStruct->hDC;

	int nSavedDC = ::SaveDC(hDc);

	// For some bizarre reason the rcItem you get extends above the actual
	// drawing area. We have to workaround this "feature".
	rc.top += ::GetSystemMetrics(SM_CYEDGE);

	::SetBkMode(hDc, TRANSPARENT);
	//HBRUSH hBrush = ::CreateSolidBrush(::GetSysColor(COLOR_BTNFACE));
	//::FillRect(hDc, &rc, hBrush);
	//::DeleteObject((HGDIOBJ)hBrush);

	if (NppDarkMode::isEnabled())
	{
		RECT selectedRect = rc;
		selectedRect.top -= 2;
		selectedRect.bottom += 2;
		if (isSelected)
		{
			::FillRect(hDc, &selectedRect, NppDarkMode::getSofterBackgroundBrush());
		}
		else
		{
			::FillRect(hDc, &selectedRect, NppDarkMode::getBackgroundBrush());
		}
	}

	// draw orange bar
	if (!NppDarkMode::isEnabled() && _bDrawOgLine && isSelected)
	{
		RECT barRect  = rc;
		barRect.top  += rc.bottom - 4;

		HBRUSH hBrush = ::CreateSolidBrush(RGB(250, 170, 60));
		::FillRect(hDc, &barRect, hBrush);
		::DeleteObject(hBrush);
	}

	// draw icon if enabled
	if (((tTbData*)tcItem.lParam)->uMask & DWS_ICONTAB)
	{
		HIMAGELIST	hImageList	= (HIMAGELIST)::SendMessage(_hParent, DMM_GETIMAGELIST, 0, 0);
		int iPosImage = static_cast<int32_t>(::SendMessage(_hParent, DMM_GETICONPOS, 0, reinterpret_cast<LPARAM>(reinterpret_cast<tTbData*>(tcItem.lParam)->hClient)));

		if ((hImageList != NULL) && (iPosImage >= 0))
		{
			// Get height of image so we
			IMAGEINFO	info		= {0};
			RECT &		imageRect	= info.rcImage;
			
			ImageList_GetImageInfo(hImageList, iPosImage, &info);

			int darkPaddingX = NppDarkMode::isEnabled() ? 1 : 0;
			int darkPaddingY = NppDarkMode::isEnabled() ? 2 : (isSelected ? 1 : 0);

			int iconDpiDynamicalX = isSelected ? rc.left + 3
				: rc.left + (rc.right - rc.left - imageRect.right + imageRect.left) / 2 + darkPaddingX;
			int iconDpiDynamicalY = NppParameters::getInstance()._dpiManager.scaleY(5) + darkPaddingY;

			ImageList_Draw(hImageList, iPosImage, hDc, iconDpiDynamicalX, iconDpiDynamicalY, ILD_NORMAL);

			if (isSelected)
			{
				rc.left += imageRect.right - imageRect.left + 5;
			}
		}
	}

	if (isSelected)
	{
		COLORREF _unselectedColor = RGB(0, 0, 0);
		::SetTextColor(hDc, NppDarkMode::isEnabled() ? NppDarkMode::getTextColor() : _unselectedColor);

		// draw text
		rc.top -= ::GetSystemMetrics(SM_CYEDGE);
		::SelectObject(hDc, _hFont);
		::DrawText(hDc, text, length, &rc, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
	}

	::RestoreDC(hDc, nSavedDC);
}

list<DockingCont*> dockingContsZOrder;

bool rcDebug=false;

//----------------------------------------------
//    Process function of dialog
//
INT_PTR CALLBACK DockingCont::run_dlgProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message) 
	{
		case WM_NCACTIVATE:
		{
			// Note: lParam to identify the trigger window
			if (static_cast<int>(lParam) != -1)
			{
				::SendMessage(_hParent, WM_NCACTIVATE, wParam, 0);
			}
			if(LOWORD(wParam)==WA_ACTIVE)
			{
				dockingContsZOrder.remove(this);
				dockingContsZOrder.push_front(this);
			}
			break;
		}
		case WM_INITDIALOG:
		{
			_hContTab = ::GetDlgItem(_hSelf, IDC_TAB_CONT);
			_hCaption = ::GetDlgItem(_hSelf, IDC_BTN_CAPTION);

			// intial subclassing of caption
			::SetWindowLongPtr(_hCaption, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
			_hDefaultCaptionProc = reinterpret_cast<WNDPROC>(::SetWindowLongPtr(_hCaption, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(wndCaptionProc)));

			// intial subclassing of tab
			::SetWindowLongPtr(_hContTab, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
			_hDefaultTabProc = reinterpret_cast<WNDPROC>(::SetWindowLongPtr(_hContTab, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(wndTabProc)));

			//_hDefaultProc1 = reinterpret_cast<WNDPROC>(::SetWindowLongPtr(_hSelf, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(wndProc1)));


			// set min tab width
			int tabDpiDynamicalMinWidth = NppParameters::getInstance()._dpiManager.scaleX(24);
			::SendMessage(_hContTab, TCM_SETMINTABWIDTH, 0, tabDpiDynamicalMinWidth);

			break;
		}
		case WM_ERASEBKGND:
		{
			if (!NppDarkMode::isEnabled())
			{
				break;
			}
			RECT rc = { 0 };
			getClientRect(rc);
			::FillRect(reinterpret_cast<HDC>(wParam), &rc, NppDarkMode::getDarkerBackgroundBrush());
			return TRUE;
		}

		case WM_DRAWITEM :
		{
			// draw tab or caption
			if (reinterpret_cast<DRAWITEMSTRUCT *>(lParam)->CtlID == IDC_TAB_CONT)
			{
				if (!NppDarkMode::isEnabled())
				{
					drawTabItem(reinterpret_cast<DRAWITEMSTRUCT*>(lParam));
					return TRUE;
				}
				return TRUE;
			}
			else
			{
				drawCaptionItem((DRAWITEMSTRUCT *)lParam);
				return TRUE;
			}
			break;
		}

		case WM_CONTEXTMENU:
		{
			rcDebug=true;

			#if DEBUG_DOCKING_PREVIEW
			RECT rcPrint = getDataOfActiveTb()->rcFloat;
			RECT rc1 = _rcFloat;
			LogIs(true, (HWND)-1 , TEXT("WM_CONTEXTMENU data.floatSz =%d, %d, %d, %d  ---rcFloat--- %d, %d, %d, %d  ---name---  %s")
				, rcPrint.left, rcPrint.right, rcPrint.top, rcPrint.bottom
				, rc1.left, rc1.right, rc1.top, rc1.bottom
				//, rc.left, rc.right, rc.top, rc.bottom
				, getDataOfActiveTb()->pszName
			);
			#endif

			if (true)
			{
				break;
			}
			//if (wParam==VK_ESCAPE)
			{
				RECT rc;
				if (!isFloating())
				{
					auto data = getDataOfActiveTb();
					//nppApp->_dockingManager.toggleActiveTb(this, DMM_FLOAT, TRUE, NULL);
					RECT rcFloat;
					if(DMM_FLOATALL) rcFloat = data->rcFloat;
					rc = rcFloat;
					nppApp->_dockingManager.toggleVisTb(this, DMM_FLOAT, &data->rcFloat);
				}
				else
				{
					::SendMessage(_hParent, DMM_FLOATALL, 0, reinterpret_cast<LPARAM>(this));
					GetWindowRect(getHSelf(), &rc);

				}

				::MoveWindow(getHSelf(), rc.left, rc.top, rc.right, rc.bottom, TRUE);

				::SendMessage(getHSelf(), WM_SIZE, 0, 0);

				focusClient();
			}
			break;
		}
		case WM_NCLBUTTONDBLCLK:
		{
			RECT	rcWnd		= {0};
			RECT	rcClient	= {0};
			POINT	pt			= {HIWORD(lParam), LOWORD(lParam)};

			getWindowRect(rcWnd);
			getClientRect(rcClient);
			ClientRectToScreenRect(_hSelf, &rcClient);
			rcWnd.bottom = rcClient.top;

			// if in caption
			if ((rcWnd.top  < pt.x) && (rcWnd.bottom > pt.x) &&
				(rcWnd.left < pt.y) && (rcWnd.right  > pt.y))
			{
				_toDock = _isFloating;
				//resize();
				NotifyParent(DMM_DOCKALL);

				if(pGripper_)
				{
					pGripper_->drawWindow(NULL);
					delete pGripper_;
					pGripper_ = NULL;
				}

				return TRUE;
			}
			break;
		}
		case WM_NCCALCSIZE:
		case WM_SIZE:
		{
			onSize();

			if (_isFloating)
			{
				// todo .. save when the user resize the Aero Snap Mode.
			}

			#if DEBUG_DOCKING_PREVIEW
			RECT rcPrint = _rcFloat;
			LogIs(false, (HWND)-1 , TEXT("onSize floatSz =%d, %d, %d, %d :: ")
				, rcPrint.left, rcPrint.right, rcPrint.top, rcPrint.bottom);
			#endif

			break;
		}
		case WM_MOVE:
			if(pGripper_)
			{
				pGripper_->onMove();
			}
		break;
		//case DMM_CANCEL_MOVE:
		case WM_EXITSIZEMOVE:
			// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-exitsizemove
			//lastMovedTm = clock();
			_toDock = false;
			if(DockingPreviewMethod==2)
			if(pGripper_)
			{
				RECT rc=_rcFloat;

				releasePt = pGripper_->_ptOld;

				if (pGripper_->stopGrip()&&_isFloating)
				{
					RECT rc;
					GetWindowRect(_hSelf, &rc); // rcplace
					// 此处，抓起缩放完毕，…… 的大小。

					WINDOWPLACEMENT placement = {};
					GetWindowPlacement(_hSelf, &placement);

					RECT & rc1 = placement.rcNormalPosition; 
					bool AeroSnap = rc1.left-rc1.right!=rc.left-rc.right
						||rc1.top-rc1.bottom!=rc.top-rc.bottom;
					if (!AeroSnap)
					{
						// todo , move should not affect the floating sz of all children.
						_rcFloat = rc;
						for (size_t iTb = 0, len = _vTbData.size(); iTb < len; ++iTb)
						{
							_vTbData[iTb]->rcFloat = rc;
						}
					}

					#if DEBUG_DOCKING_PREVIEW
					RECT rcPrint = _rcFloat;
					LogIs(true, (HWND)-1 , TEXT("WM_EXITSIZEMOVE floatSz =%d, %d, %d, %d :: ")
						, rcPrint.left, rcPrint.right, rcPrint.top, rcPrint.bottom
					);
					#endif
				}

				delete pGripper_;
				pGripper_ = NULL;
			}
			else if (_isFloating)
			{
				// relase mouse, Aero snap, Sz changed.
				// doule click, Sz changed.
				RECT rc;
				GetWindowRect(_hSelf, &rc); // rcplace
											//  _rcFloat = _rcFloatPlace;
				bool updateCurrentAsNormalRect = true;
				WINDOWPLACEMENT placement = {};
				GetWindowPlacement(_hSelf, &placement);

				RECT & rc1 = placement.rcNormalPosition; 
				bool AeroSnap = rc1.left-rc1.right!=rc.left-rc.right
					||rc1.top-rc1.bottom!=rc.top-rc.bottom;
				if (!AeroSnap)
				{
					// todo , move should not affect the floating sz of all children.
					_rcFloat = rc;
					for (size_t iTb = 0, len = _vTbData.size(); iTb < len; ++iTb)
					{
						_vTbData[iTb]->rcFloat = rc;
					}
				}

			}


			{
				#if DEBUG_DOCKING_PREVIEW
				RECT rcPrint = _rcFloat;
				LogIs(false, (HWND)-1 , TEXT("WM_EXITSIZEMOVE 2 floatSz =%d, %d, %d, %d :: ")
					, rcPrint.left, rcPrint.right, rcPrint.top, rcPrint.bottom
				);
				#endif
			}


		break;
		case WM_SYSCOMMAND :
		{
			switch (wParam & 0xfff0)
			{
					case SC_MOVE:
					//if(true) break;
					if(_isFloating && DockingPreviewMethod==2) 
					{
						_toDock = true;
						if (!IsZoomed(_hSelf))
						{
							RECT rc;
							GetWindowRect(_hSelf, &rc); // rcplace
															  //  WINDOWPLACEMENT you ***** ！
							WINDOWPLACEMENT placement = {};
							GetWindowPlacement(_hSelf, &placement);

							RECT & rc1 = placement.rcNormalPosition;

							// 此处，抓起窗口标题，不应记录 Aero Snap状态下的大小。

							bool AeroSnap = rc1.left-rc1.right!=rc.left-rc.right
								||rc1.top-rc1.bottom!=rc.top-rc.bottom;

							if (!AeroSnap)
							{
								_rcFloat = rc;
								GetWindowPlacement(_hSelf, &placement);
								placement.rcNormalPosition = rc; // rcplace
								SetWindowPlacement(_hSelf, &placement);
							}


							#if DEBUG_DOCKING_PREVIEW
							RECT rcPrint = _rcFloat;
							LogIs(true, (HWND)-1 , TEXT("SC_MOVE floatSz =%d, %d, %d, %d    ---plc---    %d, %d, %d, %d    ---now---    %d, %d, %d, %d")
								, rcPrint.left, rcPrint.right, rcPrint.top, rcPrint.bottom
								, rc1.left, rc1.right, rc1.top, rc1.bottom
								, rc.left, rc.right, rc.top, rc.bottom
								);
							#endif
						}

						if(!pGripper_)
						{
							pGripper_=new Gripper();
							pGripper_->init(_hInst, nppApp->_dockingManager.getHParent());
						}
						pGripper_->startGrip(this, &nppApp->_dockingManager, false);
						_pGripper = pGripper_;
						break;
					}
					NotifyParent(DMM_MOVE);
					// SC_MOVE
					return TRUE;
				default: 
					break;
			}
			return FALSE;
		}
		case WM_COMMAND : 
		{
			switch (LOWORD(wParam))
			{   
				case IDCANCEL:
					doClose();
					return TRUE;
				default :
					break;
			}
			break;
		}
		default:
			break;
	}

	return FALSE;
}

void DockingCont::onSize()
{
	//if(1) return;
	TCITEM tcItem = {0};
	RECT rc = {0};
	RECT rcTemp = {0};
	UINT iItemCnt = static_cast<int32_t>(::SendMessage(_hContTab, TCM_GETITEMCOUNT, 0, 0));
	UINT iTabOff = 0;

	getClientRect(rc);

	if (iItemCnt >= 1)
	{
		// resize to docked window
		int tabDpiDynamicalHeight = NppParameters::getInstance()._dpiManager.scaleY(16) + 8;
		if (_isFloating == false) // 停靠窗口改变大小
		{ 
			// draw caption
			if (_isTopCaption == TRUE)
			{
				::SetWindowPos(_hCaption, NULL, rc.left, rc.top, rc.right, _captionHeightDynamic, SWP_NOZORDER | SWP_NOACTIVATE);
				rc.top += _captionHeightDynamic;
				rc.bottom -= _captionHeightDynamic;
			}
			else
			{
				::SetWindowPos(_hCaption, NULL, rc.left, rc.top, _captionHeightDynamic, rc.bottom, SWP_NOZORDER | SWP_NOACTIVATE);
				rc.left += _captionHeightDynamic;
				rc.right -= _captionHeightDynamic;
			}

			if (iItemCnt >= 2)
			{
				// resize tab and plugin control if tabs exceeds one
				// resize tab
				rcTemp = rc;
				rcTemp.top = (rcTemp.bottom + rcTemp.top) - (tabDpiDynamicalHeight + _captionGapDynamic);
				rcTemp.bottom	= tabDpiDynamicalHeight;
				iTabOff			= tabDpiDynamicalHeight;

				::SetWindowPos(_hContTab, NULL,
								rcTemp.left, rcTemp.top, rcTemp.right, rcTemp.bottom, 
								SWP_NOZORDER | SWP_SHOWWINDOW |  SWP_NOACTIVATE);
			}

			// resize client area for plugin
			rcTemp = rc;
			if (_isTopCaption == TRUE)
			{
				rcTemp.top += _captionGapDynamic;
				rcTemp.bottom -= (iTabOff + _captionGapDynamic);
			}
			else
			{
				rcTemp.left += _captionGapDynamic;
				rcTemp.right -= _captionGapDynamic;
				rcTemp.bottom -= iTabOff;
			}

			// set position of client area
			::SetWindowPos(::GetDlgItem(_hSelf, IDC_CLIENT_TAB), NULL,
							rcTemp.left, rcTemp.top, rcTemp.right, rcTemp.bottom, 
							SWP_NOZORDER | SWP_NOACTIVATE);
		}
		// resize to float window
		else // 浮动窗口改变大小
		{ 
			if (!IsZoomed(_hSelf))
			{
				RECT rcw{0,0,0,0};


				if(_toDock)
				{
					if(memcmp(&_rcFloat, &rcw, sizeof(RECT))==0) // rcplace
					{
						GetWindowRect(_hSelf, &rcw);
						//_rcFloat = rcw;  // rcplace
					}
					rcw = _rcFloat; // rcplace
				}
				else
				{
					rcw = _rcFloat; // rcplace
					//GetWindowRect(_hSelf, &rcw);
				}

				if(DockingPreviewMethod<=0)
				{
					GetWindowRect(_hSelf, &rcw);
					_rcFloat = rcw;
					// update floating size
					for (size_t iTb = 0, len = _vTbData.size(); iTb < len; ++iTb)
					{
						_vTbData[iTb]->rcFloat = rcw;
					}
				}



			}

			if(1)
			{
				// draw caption
				if (iItemCnt >= 2)
				{
					// resize tab if size of elements exceeds one
					rcTemp = rc;
					rcTemp.top = rcTemp.bottom - (tabDpiDynamicalHeight + _captionGapDynamic);
					rcTemp.bottom = tabDpiDynamicalHeight;

					::SetWindowPos(_hContTab, NULL,
						rcTemp.left, rcTemp.top, rcTemp.right, rcTemp.bottom, 
						SWP_NOZORDER | SWP_SHOWWINDOW);
				}

				// resize client area for plugin
				rcTemp = rc;
				rcTemp.bottom -= ((iItemCnt == 1)?0:tabDpiDynamicalHeight);

				::SetWindowPos(::GetDlgItem(_hSelf, IDC_CLIENT_TAB), NULL,
					rcTemp.left, rcTemp.top, rcTemp.right, rcTemp.bottom, 
					SWP_NOZORDER | SWP_NOACTIVATE);
			}
		}
		

		// get active item data
		size_t iItemCnt2 = static_cast<size_t>(::SendMessage(_hContTab, TCM_GETITEMCOUNT, 0, 0));

		// resize visible plugin windows
		for (size_t iItem = 0; iItem < iItemCnt2; ++iItem)
		{
			tcItem.mask		= TCIF_PARAM;
			::SendMessage(_hContTab, TCM_GETITEM, iItem, reinterpret_cast<LPARAM>(&tcItem));
			if (!tcItem.lParam)
				continue;

			::SetWindowPos(((tTbData*)tcItem.lParam)->hClient, NULL,
							0, 0, rcTemp.right, rcTemp.bottom, 
							SWP_NOZORDER);

			// Notify switch in
			NMHDR nmhdr;
			nmhdr.code		= DMN_FLOATDROPPED;
			nmhdr.hwndFrom	= _hSelf;
			nmhdr.idFrom	= 0;
			::SendMessage(((tTbData*)tcItem.lParam)->hClient, WM_NOTIFY, nmhdr.idFrom, reinterpret_cast<LPARAM>(&nmhdr));
			
		}
	}
}

void DockingCont::doCloseOneTab()
{
	int	iItemCnt = static_cast<int32_t>(::SendMessage(_hContTab, TCM_GETITEMCOUNT, 0, 0));
	int iItemCur = getActiveTb();

	TCITEM		tcItem		= {0};

	tcItem.mask	= TCIF_PARAM;
	::SendMessage(_hContTab, TCM_GETITEM, iItemCur, reinterpret_cast<LPARAM>(&tcItem));
	if (tcItem.lParam)
	{
		// notify child windows
		if (NotifyParent(DMM_CLOSE) == 0)
		{
			// delete tab
			hideToolbar((tTbData*)tcItem.lParam);
		}
	}

	iItemCnt = static_cast<int32_t>(::SendMessage(_hContTab, TCM_GETITEMCOUNT, 0, 0));
	if (iItemCnt == 0)
	{
		// hide dialog first
		this->doDialog(false);
		::SendMessage(_hParent, WM_SIZE, 0, 0);
	}
}

void DockingCont::doClose()
{
	int	iItemOff = 0;
	int	iItemCnt = static_cast<int32_t>(::SendMessage(_hContTab, TCM_GETITEMCOUNT, 0, 0));

	for (int iItem = 0; iItem < iItemCnt; ++iItem)
	{
		TCITEM		tcItem		= {0};

		// get item data
		selectTab(iItemOff);
		tcItem.mask	= TCIF_PARAM;
		::SendMessage(_hContTab, TCM_GETITEM, iItemOff, reinterpret_cast<LPARAM>(&tcItem));
		if (!tcItem.lParam)
			continue;

		// notify child windows
		if (NotifyParent(DMM_CLOSE) == 0)
		{
			// delete tab
			hideToolbar((tTbData*)tcItem.lParam);
		}
		else
		{
			++iItemOff;
		}
	}

	if (iItemOff == 0)
	{
		// hide dialog first
		this->doDialog(false);
		::SendMessage(_hParent, WM_SIZE, 0, 0);
	}
}

void DockingCont::showToolbar(tTbData* pTbData, BOOL state)
{
	if (state == SW_SHOW)
	{
		viewToolbar(pTbData);
	}
	else
	{
		hideToolbar(pTbData);
	}
}

int DockingCont::hideToolbar(tTbData *pTbData, BOOL hideClient, bool activate)
{
	int iItem = searchPosInTab(pTbData);
	BOOL hadFocus = ::IsChild (pTbData->hClient, ::GetFocus());

	// delete item
	if (TRUE == ::SendMessage(_hContTab, TCM_DELETEITEM, iItem, 0))
	{
		auto iItemCnt = ::SendMessage(_hContTab, TCM_GETITEMCOUNT, 0, 0);

		if (iItemCnt != 0)
		{
			if (iItem == iItemCnt)
			{
				iItem--;
			}

			// activate new selected item and view plugin dialog
			_prevItem = iItem;
			
			if(activate)
			selectTab(iItem);

			// hide tabs if only one element
			if (iItemCnt == 1)
			{
				::ShowWindow(_hContTab, SW_HIDE);
			}
		}
		else 
		{
			// hide dialog
			this->doDialog(false);

			// send message to docking manager for resize
			if (!_isFloating)
			{
				::SendMessage(_hParent, WM_SIZE, 0, 0);
			}
			// set focus to current edit window if the docking window had focus
			if (hadFocus)
				::PostMessage(::GetParent(_hParent), WM_ACTIVATE, WA_ACTIVE, 0);
		}

		// keep sure, that client is hide!!!
		if (hideClient == TRUE)
		{
			::ShowWindow(pTbData->hClient, SW_HIDE);
		}
	}
	if(activate)
	onSize();

	return iItem;
}

bool bIsTabSelSupressed=false;

void DockingCont::viewToolbar(tTbData *pTbData)
{
	TCITEM tcItem = {0};
	int iItemCnt = static_cast<int32_t>(::SendMessage(_hContTab, TCM_GETITEMCOUNT, 0, 0));

	if (iItemCnt > 0)
	{
		UINT	iItem	= getActiveTb();

		tcItem.mask		= TCIF_PARAM;
		::SendMessage(_hContTab, TCM_GETITEM, iItem, reinterpret_cast<LPARAM>(&tcItem));
		if (!tcItem.lParam)
			return;
		
		// hide active dialog
		::ShowWindow(((tTbData*)tcItem.lParam)->hClient, SW_HIDE);
	}

	// create new tab if it not exists
	int iTabPos = searchPosInTab(pTbData);
	tcItem.mask			= TCIF_PARAM;
	tcItem.lParam = reinterpret_cast<LPARAM>(pTbData);

	if (iTabPos == -1)
	{
		// set only params and text even if icon available
		::SendMessage(_hContTab, TCM_INSERTITEM, iItemCnt, reinterpret_cast<LPARAM>(&tcItem));
		selectTab(iItemCnt);
	}
	// if exists select it and update data
	else
	{
		::SendMessage(_hContTab, TCM_SETITEM, iTabPos, reinterpret_cast<LPARAM>(&tcItem));
		selectTab(iTabPos);
	}

	// show dialog and notify parent to update dialog view
	if (isVisible() == false)
	{
		this->doDialog();
		::SendMessage(_hParent, WM_SIZE, 0, 0);
	}

	// set position of client
	onSize();
}

int DockingCont::searchPosInTab(tTbData* pTbData)
{
	TCITEM tcItem = {0};
	int iItemCnt = static_cast<int32_t>(::SendMessage(_hContTab, TCM_GETITEMCOUNT, 0, 0));

	tcItem.mask	= TCIF_PARAM;

	for (int iItem = 0; iItem < iItemCnt; ++iItem)
	{
		::SendMessage(_hContTab, TCM_GETITEM, iItem, reinterpret_cast<LPARAM>(&tcItem));
		if (!tcItem.lParam)
			continue;

		if (((tTbData*)tcItem.lParam)->hClient == pTbData->hClient)
			return iItem;
	}
	return -1;
}

void DockingCont::selectTab(int iTab)
{
	if (!bIsTabSelSupressed)
	if (iTab != -1)
	{
		const TCHAR	*pszMaxTxt	= NULL;
		TCITEM tcItem = {0};
		SIZE size = {0};
		int maxWidth = 0;
		int iItemCnt = static_cast<int32_t>(::SendMessage(_hContTab, TCM_GETITEMCOUNT, 0, 0));

		// get data of new active dialog
		tcItem.mask		= TCIF_PARAM;
		::SendMessage(_hContTab, TCM_GETITEM, iTab, reinterpret_cast<LPARAM>(&tcItem));
		// show active dialog
		if (!tcItem.lParam)
			return;

		::ShowWindow(((tTbData*)tcItem.lParam)->hClient, SW_SHOW);
		::SetFocus(((tTbData*)tcItem.lParam)->hClient);

		// Notify switch in
		NMHDR nmhdr;
		nmhdr.code		= DMN_SWITCHIN;
		nmhdr.hwndFrom	= _hSelf;
		nmhdr.idFrom	= 0;
		::SendMessage(reinterpret_cast<tTbData*>(tcItem.lParam)->hClient, WM_NOTIFY, nmhdr.idFrom, reinterpret_cast<LPARAM>(&nmhdr));

		if (static_cast<unsigned int>(iTab) != _prevItem)
		{
			// hide previous dialog
			::SendMessage(_hContTab, TCM_GETITEM, _prevItem, reinterpret_cast<LPARAM>(&tcItem));

			if (!tcItem.lParam)
				return;
			::ShowWindow(((tTbData*)tcItem.lParam)->hClient, SW_HIDE);
		
			// Notify switch off
			NMHDR nmhdr;
			nmhdr.code		= DMN_SWITCHOFF;
			nmhdr.hwndFrom	= _hSelf;
			nmhdr.idFrom	= 0;
			::SendMessage(((tTbData*)tcItem.lParam)->hClient, WM_NOTIFY, nmhdr.idFrom, reinterpret_cast<LPARAM>(&nmhdr));
		}

		// resize tab item

		// get at first largest item ...
		HDC		hDc	= ::GetDC(_hContTab);
		SelectObject(hDc, _hFont);

		for (int iItem = 0; iItem < iItemCnt; ++iItem)
		{
			const TCHAR *pszTabTxt = NULL;

			::SendMessage(_hContTab, TCM_GETITEM, iItem, reinterpret_cast<LPARAM>(&tcItem));
			if (!tcItem.lParam)
				continue;
			pszTabTxt = reinterpret_cast<tTbData*>(tcItem.lParam)->pszName;

			// get current font width
			GetTextExtentPoint32(hDc, pszTabTxt, lstrlen(pszTabTxt), &size);

			if (maxWidth < size.cx) 
			{
				maxWidth	= size.cx;
				pszMaxTxt	= pszTabTxt;
			}
		}
		::ReleaseDC(_hSelf, hDc);

		tcItem.mask	= TCIF_TEXT;

		for (int iItem = 0; iItem < iItemCnt; ++iItem)
		{
			generic_string szText;
			if (iItem == iTab && pszMaxTxt)
			{
				// fake here an icon before text ...
				szText = TEXT("    ");
				szText += pszMaxTxt;
			}
			tcItem.pszText = (TCHAR *)szText.c_str();
			::SendMessage(_hContTab, TCM_SETITEM, iItem, reinterpret_cast<LPARAM>(&tcItem));
		}

		// selects the pressed tab and store previous tab
		::SendMessage(_hContTab, TCM_SETCURSEL, iTab, 0);
		_prevItem = iTab;

		// update caption text
		updateCaption();

		onSize();
	}
}

bool DockingCont::updateCaption()
{
	if (!_hContTab)
		return false;

	TCITEM			tcItem	= {0};
	int				iItem	= getActiveTb();

	if (iItem < 0)
		return false;

	// get data of new active dialog
	tcItem.mask		= TCIF_PARAM;
	::SendMessage(_hContTab, TCM_GETITEM, iItem, reinterpret_cast<LPARAM>(&tcItem));

	if (!tcItem.lParam) return false;

	// update caption text
	_pszCaption = ((tTbData*)tcItem.lParam)->pszName;

	// test if additional information are available
	if ((((tTbData*)tcItem.lParam)->uMask & DWS_ADDINFO) && 
		(lstrlen(((tTbData*)tcItem.lParam)->pszAddInfo) != 0))
	{
		_pszCaption += TEXT(" - ");
		_pszCaption += ((tTbData*)tcItem.lParam)->pszAddInfo; 
	}

	if (_isFloating == true)
	{
		::SetWindowText(_hSelf, _pszCaption.c_str());
	}
	else
	{
		::SetWindowText(_hCaption, _pszCaption.c_str());
	}
	return true;
}

void DockingCont::focusClient()
{
	TCITEM		tcItem	= {0};
	int			iItem	= getActiveTb();	

	if (iItem != -1)
	{
		// get data of new active dialog
		tcItem.mask		= TCIF_PARAM;
		::SendMessage(_hContTab, TCM_GETITEM, iItem, reinterpret_cast<LPARAM>(&tcItem));

		// set focus
		if (!tcItem.lParam)
			return;

		tTbData *tbData = (tTbData *)tcItem.lParam;
		if (tbData->pszAddInfo && lstrcmp(tbData->pszAddInfo, DM_NOFOCUSWHILECLICKINGCAPTION) == 0)
			return;
		
		::SetFocus(tbData->hClient);
	}
}

void DockingCont::SetActive(BOOL bState)
{
	_isActive = bState;
	updateCaption();
};

LPARAM DockingCont::NotifyParent(UINT message)
{
	return ::SendMessage(_hParent, message, 0, reinterpret_cast<LPARAM>(this));
}

