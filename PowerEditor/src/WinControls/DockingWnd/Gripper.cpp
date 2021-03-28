// this file is part of docking functionality for Notepad++
// Copyright (C)2006 Jens Lorenz <jens.plugin.npp@gmx.de>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
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
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// Changed something around drawWindow() (for details see there) to enhance
// speed and consistency of the drag-rectangle - August 2010, Joern Gruel (jg)


#include <stdexcept>
#include "Gripper.h"
#include "DirectGripper.h"
#include "DockingManager.h"
#include "Parameters.h"

using namespace std;

#ifndef WH_KEYBOARD_LL
#define WH_KEYBOARD_LL 13
#endif

#ifndef WH_MOUSE_LL
#define WH_MOUSE_LL 14
#endif


#include "Notepad_plus.h"

extern Notepad_plus *nppApp;

BOOL Gripper::_isRegistered	= FALSE;

static HWND		hWndServer		= NULL;
static HHOOK	hookMouse		= NULL;
static HHOOK	hookKeyboard	= NULL;

extern list<DockingCont*> dockingContsZOrder;

static LRESULT CALLBACK hookProcMouse(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
		switch (wParam)
		{
			case WM_MOUSEMOVE:
			case WM_NCMOUSEMOVE:
				::SendMessage(hWndServer, static_cast<UINT>(wParam), 0, 0);
				break;

			case WM_LBUTTONUP:
			case WM_NCLBUTTONUP:
				::SendMessage(hWndServer, static_cast<UINT>(wParam), 0, 0);
				return TRUE;

			default:
				break;
		}
	}
	return ::CallNextHookEx(hookMouse, nCode, wParam, lParam);
}

static LRESULT CALLBACK hookProcKeyboard(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
		if (wParam == VK_ESCAPE)
		{
			::PostMessage(hWndServer, DMM_CANCEL_MOVE, 0, 0);
			return FALSE;
		}
	}

	return ::CallNextHookEx(hookKeyboard, nCode, wParam, lParam);
}

Gripper::Gripper()
{
	_hInst			= NULL;
	_hParent		= NULL;
	_hSelf			= NULL;
	_pDockMgr		= NULL;
	_pCont			= NULL;

	_ptOffset.x		= 0;
	_ptOffset.y		= 0;

	_ptOld.x		= 0;
	_ptOld.y		= 0;
	_bPtOldValid		= FALSE;

	_hTab			= NULL;
	_hTabSource		= NULL;
	_startMovingFromTab	= FALSE;
	_startMoveFromTab	= FALSE;
	_iItem			= 0;

	_hdc			= NULL;
	_hbm			= NULL;
	_hbrush			= NULL;

	memset(&_rcPrev, 0, sizeof(RECT));
	memset(&_rcItem, 0, sizeof(RECT));
	memset(&_tcItem, 0, sizeof(TCITEM));
	memset(&_dockData, 0, sizeof(tDockMgr));
}


DirectGripper * dGripper = NULL;

void Gripper::init(HINSTANCE hInst, HWND hParent) {
	_hInst   = hInst;	
	_hParent = hParent;
	if(!dGripper)
	{
		dGripper = new DirectGripper(_hParent);
	}
	else
	{
		MoveWindow(dGripper->_hSelf, 0, 0, 0, 0, false);
	}
};

void Gripper::startGrip(DockingCont* pCont, DockingManager* pDockMgr, bool create)
{
	_pDockMgr   = pDockMgr;
	_pCont		= pCont;

	_pDockMgr->getDockInfo(&_dockData);

	if(create)
	{
		if (!_isRegistered)
		{
			WNDCLASS clz;

			clz.style = 0;
			clz.lpfnWndProc = staticWinProc;
			clz.cbClsExtra = 0;
			clz.cbWndExtra = 0;
			clz.hInstance = _hInst;
			clz.hIcon = NULL;
			clz.hCursor = ::LoadCursor(NULL, IDC_ARROW);

			clz.hbrBackground = NULL;
			clz.lpszMenuName = NULL;
			clz.lpszClassName = MDLG_CLASS_NAME;

			if (!::RegisterClass(&clz))
			{
				throw std::runtime_error("Gripper::startGrip : RegisterClass() function failed");
			}
			_isRegistered = TRUE;
		}

		_hSelf = ::CreateWindowEx(
						0,
						MDLG_CLASS_NAME,
						TEXT(""), 0,
						CW_USEDEFAULT, CW_USEDEFAULT,
						CW_USEDEFAULT, CW_USEDEFAULT,
						NULL,
						NULL,
						_hInst,
						(LPVOID)this);
		hWndServer = _hSelf;

		if (!_hSelf)
		{
			throw std::runtime_error("Gripper::startGrip : CreateWindowEx() function return null");
			return;
		}
	}
	
	if(pCont->_rcFloat.right==0&&pCont->_rcFloat.left==0)
	{ // sanity check( not ). initialize the saved float shape.
		auto * data = pCont->getDataOfActiveTb();
		if (!data)
		{
			data = pCont->getDataOfAllTb()[0];
		}
		if (data)
		{
			pCont->_rcFloat = data->rcFloat;
		}
	}

	dGripper->show(true);
}


LRESULT CALLBACK Gripper::staticWinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Gripper *pDlgMoving = NULL;
	switch (message)
	{
		case WM_NCCREATE :
			pDlgMoving = reinterpret_cast<Gripper *>(reinterpret_cast<LPCREATESTRUCT>(lParam)->lpCreateParams);
			pDlgMoving->_hSelf = hwnd;
			::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pDlgMoving));
			return TRUE;

		default :
			pDlgMoving = reinterpret_cast<Gripper *>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
			if (!pDlgMoving)
				return ::DefWindowProc(hwnd, message, wParam, lParam);
			return pDlgMoving->runProc(message, wParam, lParam);
	}
}

LRESULT Gripper::runProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
		{
			create();
			break;
		}
		case WM_MOUSEMOVE:
		case WM_NCMOUSEMOVE:
		{
			if(IsZoomed(_pCont->getHSelf())) {
				ShowWindow(_pCont->getHSelf(), SW_NORMAL);
				_ptOffset.x=_pCont->getWidth()/2;
				_ptOffset.y=0;
				//::MoveWindow(_pCont->getHSelf(), -10000, -10000, _pCont->getWidth(), _pCont->getHeight(), FALSE);
			}
			onMove();

			//::MoveWindow(_pCont->getHSelf(), _ptOld.x, _ptOld.y, _ptOld.x+_pCont->getWidth(), _ptOld.y+_pCont->getHeight(), FALSE);
			return TRUE;
		}
		case WM_LBUTTONUP:
		case WM_NCLBUTTONUP:
		{
			/* end hooking */
			if (hookMouse)
			{
				::UnhookWindowsHookEx(hookMouse);
				::UnhookWindowsHookEx(hookKeyboard);
				hookMouse = NULL;
				hookKeyboard = NULL;
			}
			onButtonUp();
			::DestroyWindow(_hSelf);
			//::ShowWindow(_hSelf, SW_HIDE);
			return TRUE;
		}
		case DMM_CANCEL_MOVE:
		{
			POINT			pt			= {0,0};
			POINT			ptBuf		= {0,0};

			::GetCursorPos(&pt);
			getMousePoints(&pt, &ptBuf);

			/* erase last drawn rectangle */
			drawWindow(NULL);

			/* end hooking */
			::UnhookWindowsHookEx(hookMouse);
			::UnhookWindowsHookEx(hookKeyboard);

			::DestroyWindow(_hSelf);
			return FALSE;
		}
		case WM_DESTROY:
		{
			//mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			//::SetWindowPos(_hParent, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
			//_pCont->focusClient();
			delete this; // TODO: remove this line and delete this object outside of itself
			return TRUE;
		}
		default:
			break;
	}

	return ::DefWindowProc(_hSelf, message, wParam, lParam);
}


void Gripper::create()
{
	isCreated = true;
	RECT		rc		= {0};
	POINT		pt		= {0};

	// start hooking
	//::SetWindowPos(_pCont->getHSelf(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	::SetCapture(_hSelf);
	winVer ver = (NppParameters::getInstance()).getWinVersion();
	hookMouse = ::SetWindowsHookEx(WH_MOUSE_LL, hookProcMouse, _hInst, 0);

    if (!hookMouse)
    {
        DWORD dwError = ::GetLastError();
        TCHAR  str[128];
        ::wsprintf(str, TEXT("GetLastError() returned %lu"), dwError);
        ::MessageBox(NULL, str, TEXT("SetWindowsHookEx(MOUSE) failed on Gripper::create()"), MB_OK | MB_ICONERROR);
    }

	if (ver != WV_UNKNOWN && ver < WV_VISTA)
	{
		hookKeyboard = ::SetWindowsHookEx(WH_KEYBOARD_LL, hookProcKeyboard, _hInst, 0);
		if (!hookKeyboard)
		{
			DWORD dwError = ::GetLastError();
			TCHAR  str[128];
			::wsprintf(str, TEXT("GetLastError() returned %lu"), dwError);
			::MessageBox(NULL, str, TEXT("SetWindowsHookEx(KEYBOARD) failed on Gripper::create()"), MB_OK | MB_ICONERROR);
		}
	}
//  Removed regarding W9x systems
//	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

	// calculate the mouse pt within dialog
	::GetCursorPos(&pt);

	// get tab informations
	initTabInformation();

	if (_pCont->isFloating() == true)
	{
		::GetWindowRect(_pCont->getHSelf(), &rc);
	}
	else
	{
		_pCont->getClientRect(rc);
		::ScreenToClient(_pCont->getHSelf(), &pt);
	}

	_drawPat = ::IsZoomed(_hParent);

	_ptOffset.x	= pt.x - rc.left;
	_ptOffset.y	= pt.y - rc.top;
}


void Gripper::onMove()
{
	POINT		pt		= {0,0};
	POINT		ptBuf	= {0,0};

	::GetCursorPos(&pt);
	getMousePoints(&pt, &ptBuf);

	/* tab reordering only when tab was selected */
	if (_startMovingFromTab == TRUE)
	{
		doTabReordering(pt);
	}

	drawWindow(&pt);
}


bool Gripper::onButtonUp()
{
	POINT			pt			= {0,0};
	POINT			ptBuf		= {0,0};
	RECT			rc			= {0};
	RECT			rcCorr		= {0};

	::GetCursorPos(&pt);
	getMousePoints(&pt, &ptBuf);

	// do nothing, when old point is not valid
	if (_bPtOldValid == FALSE)
		return true;

	// erase last drawn rectangle
	drawWindow(NULL);

	// look if current position is within dockable area
	DockingCont*	pDockCont = dlgsHitTest(pt);

	if (pDockCont == NULL)
	{
		pDockCont = workHitTest(pt);
	}

	/* add dependency to other container class */
	if (pDockCont == NULL)
	{
		// 当鼠标位置检测不到其他可停靠容器，可能需要：
		//     将停靠窗口转换为浮动窗口；移动浮动口到新的位置。
		// 调用的是返回 |pContMove| 的 toggleActiveTb（移动一个子元素） / 或者 toggleVisTb（移动整个容器）
		// 返回值为需要移动的容器指针，空值用当前容器指针代替。
		if (!isCreated)
		{
			return true;
		}
		/* calculate new position */
		rc = _pCont->getDataOfActiveTb()->rcFloat;
		RECT rc1 = rc;
		//rc = _pCont->_rcFloat; // passing a copy of the float shape
		_pCont->getClientRect(rcCorr);

		CalcRectToScreen(_dockData.hWnd, &rc);
		CalcRectToScreen(_dockData.hWnd, &rcCorr);

		rc.left    = pt.x - _ptOffset.x;
		rc.top     = pt.y - _ptOffset.y;

		/* correct rectangle position when mouse is not within */
		DoCalcGripperRect(&rc, rcCorr, pt);

		DockingCont* pContMove	= NULL;

		bool fAlready = _pCont->isFloating();

		rc.right += rc.left;
		rc.bottom += rc.top;
		
		/* change location of toolbars */
		if (_startMovingFromTab == TRUE)
		{
			/* when tab is moved */
			// 若当前窗口已经是浮动窗口，且只有一元素，无需调用下面方法， 只需移动窗口本身即可。
			if ((!fAlready) || // 若不是浮动窗口，只能 {解除停靠}
				 // 若是浮动窗口，有多个成员，则需{将当前元素单独拎出，移到新的浮动窗口}。
				 // ( moving from tab )
				(fAlready && (::SendMessage(_hTabSource, TCM_GETITEMCOUNT, 0, 0) > 1)))
			{
				pContMove = _pDockMgr->toggleActiveTb(_pCont, DMM_FLOAT, TRUE, &rc);
				//ShowWindow(pContMove->getHSelf(), SW_NORMAL);
			}
		}
		else if (!fAlready)
		{
			// 若当前容器已经是浮动窗口，则不存在解除停靠的可能，自然无需调用下面的方法。
			//   否则，须 {转换容器中的全部元素到一个新的浮动容器窗口}。
			/* when all windows are moved */
			pContMove = _pDockMgr->toggleVisTb(_pCont, DMM_FLOAT, &rc);
		}

		/* set moving container */
		if (pContMove == NULL)
		{
			pContMove = _pCont;
		}

		rc.right  -= rc.left;
		rc.bottom -= rc.top;

		//if(pt.y>0||!fAlready) 
		{
			::MoveWindow(pContMove->getHSelf(), rc.left, rc.top, rc.right, rc.bottom, TRUE);
		}

		// Debug Docking Preview
		RECT rcPrint = pContMove->getDataOfActiveTb()->rcFloat;
		rcPrint = pContMove->_rcFloat;
		RECT rcPrint1 = rc1;
		TCHAR buffer[256]={};
		wsprintf(buffer,TEXT("onButtonUp floatSz =%d, %d, %d, %d    ---data.rc---    %d, %d, %d, %d ")
			, rcPrint.left, rcPrint.right, rcPrint.top, rcPrint.bottom
			, rcPrint1.left, rcPrint1.right, rcPrint1.top, rcPrint1.bottom
		);
		//::SendMessage(nppApp->_dockingManager.getHParent(), NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, (LPARAM)buffer);


		//pContMove->_rcFloat = rc;

		/* update window position */
		if(pt.y<=0) { // &&(!oldSchoolDraw||pContMove->isFloating())
			ShowWindow(pContMove->getHSelf(), SW_MAXIMIZE);
		} 
		::SendMessage(pContMove->getHSelf(), WM_SIZE, 0, 0);
		return true;
	}
	else if (_pCont != pDockCont)
	{
		// 当检测到的鼠标下窗口与当前窗口不一致时，
		//     移动元素到新的停靠窗口（已存在的、已关闭的） / 或者悬浮对话框（已存在的）
		// 调用的是无返回值的 toggleActiveTb（移动一个子元素） / 或者 toggleVisTb（移动整个容器）
		// passsing a copy of the float shape. 
		RECT rcPrint = _pCont->getDataOfActiveTb()->rcFloat;
		//rcPrint = pDockCont->getDataOfActiveTb()->rcFloat;
		if (!pDockCont->isVisible())
		{ 
			// the docking panel |pDockCont| now remembers it's new floating position.
			pDockCont->_rcFloat = _pCont->_rcFloat;
		}
		/* change location of toolbars */
		if ((_startMovingFromTab == TRUE) && (::SendMessage(_hTabSource, TCM_GETITEMCOUNT, 0, 0) != 1))
		{
			/* when tab is moved */
			_pDockMgr->toggleActiveTbWnd(_pCont, pDockCont);
		}
		else
		{
			/* when all windows are moved */
			_pDockMgr->toggleVisTbWnd(_pCont, pDockCont);
		}

		// Debug Docking Preview
		RECT rcPrint1 = pDockCont->getDataOfActiveTb()->rcFloat;
		TCHAR buffer[256]={};
		wsprintf(buffer,TEXT("onButtonUp1 floatSz =%d, %d, %d, %d    ---data.rc---    %d, %d, %d, %d ")
			, rcPrint.left, rcPrint.right, rcPrint.top, rcPrint.bottom
			, rcPrint1.left, rcPrint1.right, rcPrint1.top, rcPrint1.bottom
		);
		//::SendMessage(nppApp->_dockingManager.getHParent(), NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, (LPARAM)buffer);


		return false;
	}


}

// 返回true：代表非窗口间移动
bool Gripper::stopGrip()
{
	bool ret = onButtonUp();
	::DestroyWindow(_hSelf);
	return ret;
}

void Gripper::doTabReordering(POINT pt)
{
	vector<DockingCont*>	vCont		= _pDockMgr->getContainerInfo();
	BOOL					inTab		= FALSE;
	HWND					hTab		= NULL;
	HWND					hTabOld		= _hTab;
	int						iItemOld	= _iItem;

	/* search for every tab entry */
	for (size_t iCont = 0, len = vCont.size(); iCont < len; ++iCont)
	{
		hTab = vCont[iCont]->getTabWnd();

		/* search only if container is visible */
		if (::IsWindowVisible(hTab) == TRUE)
		{
			RECT	rc		= {0};

			::GetWindowRect(hTab, &rc);

			/* test if cursor points in tab window */
			if (::PtInRect(&rc, pt) == TRUE)
			{
				TCHITTESTINFO	info	= {0};

				if (_hTab == NULL)
				{
					initTabInformation();
					hTabOld  = _hTab;
					iItemOld = _iItem;
				}

				// get pointed tab item
				info.pt	= pt;
				::ScreenToClient(hTab, &info.pt);
				auto iItem = ::SendMessage(hTab, TCM_HITTEST, 0, reinterpret_cast<LPARAM>(&info));

				if (iItem != -1)
				{
					// prevent flickering of tabs with different sizes
					::SendMessage(hTab, TCM_GETITEMRECT, iItem, reinterpret_cast<LPARAM>(&rc));
					ClientRectToScreenRect(hTab, &rc);

					if ((rc.left + (_rcItem.right  - _rcItem.left)) < pt.x)
					{
						return;
					}

					_iItem = static_cast<int32_t>(iItem);
				}
				else if (_hTab && ((hTab != _hTab) || (_iItem == -1)))
				{
					// test if cusor points after last tab
					auto iLastItem = ::SendMessage(hTab, TCM_GETITEMCOUNT, 0, 0) - 1;

					::SendMessage(hTab, TCM_GETITEMRECT, iLastItem, reinterpret_cast<LPARAM>(&rc));
					if ((rc.left + rc.right) < pt.x)
					{
						_iItem = static_cast<int32_t>(iLastItem) + 1;
					}
				}

				_hTab = hTab;
				inTab = TRUE;
				break;
			}
		}
	}

	// set and remove tabs correct
	if ((inTab == TRUE) && (iItemOld != _iItem))
	{
		if (_hTab == _hTabSource)
		{
			// delete item if switching back to source tab
			auto iSel = ::SendMessage(_hTab, TCM_GETCURSEL, 0, 0);
			::SendMessage(_hTab, TCM_DELETEITEM, iSel, 0);
		}
		else if (_hTab == hTabOld)
		{
			// delete item on switch between tabs
			::SendMessage(_hTab, TCM_DELETEITEM, iItemOld, 0);
		}
	}
	else if (inTab == FALSE)
	{
		if (hTabOld != _hTabSource)
		{
			::SendMessage(hTabOld, TCM_DELETEITEM, iItemOld, 0);
		}
		_iItem = -1;
	}

	// insert new entry when mouse doesn't point to current hovered tab
	if (_hTab && ((_hTab != hTabOld) || (_iItem != iItemOld)))
	{
		_tcItem.mask	= TCIF_PARAM | (_hTab == _hTabSource ? TCIF_TEXT : 0);
		::SendMessage(_hTab, TCM_INSERTITEM, _iItem, reinterpret_cast<LPARAM>(&_tcItem));
	}

	// select the tab only in source tab window
	if ((_hTab == _hTabSource) && (_iItem != -1))
	{
		::SendMessage(_hTab, TCM_SETCURSEL, _iItem, 0);
	}

#if 0
	extern HWND g_hMainWnd;
	TCHAR str[128];
	wsprintf(str, TEXT("Size: %i"), vCont.size());
	::SetWindowText(g_hMainWnd, str);
#endif

	::UpdateWindow(_hParent);
}

// Draw a transparent window to represent the docking target.
// Inspaired by duilib.
void Gripper::drawWindow(const POINT* pPt)
{
	if(oldSchoolDraw)
	{
		drawRectangle(pPt);
		return;
	}
	RECT   rcNew	 = {0};
	RECT   rcOld	 = _rcPrev;
	bool lock=0;

	if (pPt != NULL)
	{
		getMovingRect(*pPt, &rcNew);
		if(rcNew.right==rcNew.left&&rcNew.left==0) 
		{
			return;
		}
		_rcPrev= rcNew;		// save the new drawn rcNew
		if (_bPtOldValid)
		{
			//if (rcOld.left==rcNew.left && rcOld.right==rcNew.right && rcOld.top== rcNew.top && rcOld.bottom==rcNew.bottom)
			//	return;
		}
	}

	if(IsWindow(dGripper->_hSelf))
	{
		if (pPt != NULL)
		{
			MoveWindow(dGripper->_hSelf, rcNew.left, rcNew.top, rcNew.right, rcNew.bottom, TRUE);
		}
		else
		{
			dGripper->show(false);
		}
	}

	if (pPt == NULL)
	{
		if (lock)
			::LockWindowUpdate(NULL);
		else
			_bPtOldValid= FALSE;
		if (_hdc)
		{
			::ReleaseDC(0, _hdc);
			_hdc= NULL;
		}
	}
	else	_bPtOldValid= TRUE;
}

// deprecated, painting on the desktop HDC is not smooth and dirty when the rect is not small.
void Gripper::drawRectangle(const POINT* pPt)
{
	HBRUSH hbrushOrig= NULL;
	HBITMAP hbmOrig  = NULL;
	RECT   rc	 = {0};
	RECT   rcNew	 = {0};
	RECT   rcOld	 = _rcPrev;
	bool lock=0;
#if defined (USE_LOCKWINDOWUPDATE)
	lock=1;
#endif
	lock=0;
	// Get a screen device context with backstage redrawing disabled - to have a consistently
	// and stable drawn rectangle while floating - keep in mind, that we must ensure, that
	// finally ::LockWindowUpdate(NULL) will be called, to enable drawing for others again.
	if (!_hdc)
	{
		HWND hWnd=_drawPat?_hParent:GetDesktopWindow();
		if (lock)
		_hdc= ::GetDCEx(hWnd, NULL, ::LockWindowUpdate(hWnd) ? DCX_WINDOW|DCX_CACHE|DCX_LOCKWINDOWUPDATE : DCX_WINDOW|DCX_CACHE);
		else
		_hdc= ::GetDCEx(hWnd, NULL, DCX_WINDOW|DCX_CACHE);
		
	}

	// Create a brush with the appropriate bitmap pattern to draw our drag rectangle
	if (!_hbm)
		_hbm = ::CreateBitmap(8, 8, 1, 1, DotPattern);
	if (!_hbrush)
		_hbrush = ::CreatePatternBrush(_hbm);

	if (pPt != NULL)
	{
		// Determine whether to draw a solid drag rectangle or checkered
		// ???(jg) solid or checked ??? - must have been an old comment, I didn't
		// find here this difference, but at least it's a question of drag-rects size
		//
		getMovingRect(*pPt, &rcNew);
		_rcPrev= rcNew;		// save the new drawn rcNew

		// note that from here for handling purposes the right and bottom values of the rects
		// contain width and height - its handsome, but i find it dangerous, but didn't want to
		// change that already this time.

		if (_bPtOldValid)
		{
			// okay, there already a drag-rect has been drawn - and its position
			// had been saved within the rectangle _rectPrev, wich already had been
			// copied into rcOld in the beginning, and a new drag position
			// is available, too.
			// If now rcOld and rcNew are the same, just stop further handling to not
			// draw the same drag-rectangle twice (this really happens, it should be
			// better avoided anywhere earlier)
			//
			if (rcOld.left==rcNew.left && rcOld.right==rcNew.right && rcOld.top== rcNew.top && rcOld.bottom==rcNew.bottom)
				return;

			rc.left   = min(rcOld.left, rcNew.left);
			rc.top    = min(rcOld.top,  rcNew.top);
			rc.right  = max(rcOld.left + rcOld.right,  rcNew.left + rcNew.right);
			rc.bottom = max(rcOld.top  + rcOld.bottom, rcNew.top  + rcNew.bottom);
			rc.right -= rc.left;
			rc.bottom-= rc.top;
		}
		else	rc= rcNew;	// only new rect will be drawn
	}
	else	rc= rcOld;	// only old rect will be drawn - to erase it

	// now rc contains the rectangle wich encloses all needed, new and/or previous rectangle
	// because in the following we drive within a memory device context wich is limited to rc,
	// we have to localize rcNew and rcOld within rc...
	//
	rcOld.left= rcOld.left - rc.left;
	rcOld.top = rcOld.top  - rc.top;
	rcNew.left= rcNew.left - rc.left;
	rcNew.top = rcNew.top  - rc.top;

	HDC hdcMem= ::CreateCompatibleDC(_hdc);
	HBITMAP hBm= ::CreateCompatibleBitmap(_hdc, rc.right, rc.bottom);
	hbrushOrig= (HBRUSH)::SelectObject(hdcMem, hBm);

	::SetBrushOrgEx(hdcMem, rc.left%8, rc.top%8, 0);
	hbmOrig= (HBITMAP)::SelectObject(hdcMem, _hbrush);

	::BitBlt(hdcMem, 0, 0, rc.right, rc.bottom, _hdc, rc.left, rc.top, SRCCOPY);
	if (_bPtOldValid)
	{	// erase the old drag-rectangle
		::PatBlt(hdcMem, rcOld.left  , rcOld.top  , rcOld.right  , rcOld.bottom  , PATINVERT);
		//::PatBlt(hdcMem, rcOld.left+3, rcOld.top+3, rcOld.right-6, rcOld.bottom-6, PATINVERT);
	}
	if (pPt != NULL)
	{	// draw the new drag-rectangle
		::PatBlt(hdcMem, rcNew.left  , rcNew.top  , rcNew.right  , rcNew.bottom  , PATINVERT);
		//::PatBlt(hdcMem, rcNew.left+3, rcNew.top+3, rcNew.right-6, rcNew.bottom-6, PATINVERT);
	}
	::BitBlt(_hdc, rc.left, rc.top, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);

	SelectObject(hdcMem, hbrushOrig);
	SelectObject(hdcMem, hbmOrig);
	DeleteObject(hBm);
	DeleteDC(hdcMem);

	if (pPt == NULL)
	{
		if (lock)
		::LockWindowUpdate(NULL);
		else
		_bPtOldValid= FALSE;
		if (_hdc)
		{
			::ReleaseDC(0, _hdc);
			_hdc= NULL;
		}
	}
	else	_bPtOldValid= TRUE;
}

void Gripper::getMousePoints(POINT* pt, POINT* ptPrev)
{
	*ptPrev	= _ptOld;
	_ptOld	= *pt;
}

void Gripper::getMovingRect(POINT pt, RECT *rc)
{
	RECT			rcCorr			= {0};
	DockingCont*	pContHit		= NULL;

	/* test if mouse hits a container */
	pContHit = dlgsHitTest(pt);
	//pContHit = contHitTest(pt);

	if (pContHit != NULL)
	{
		/* get rect of client */
		::GetWindowRect(pContHit->getHSelf(), rc);

		/* get rect for correction */
		if (_pCont->isFloating() == TRUE)
			//rcCorr = _pCont->getDataOfActiveTb()->rcFloat;
			rcCorr = _pCont->_rcFloat;
		else
			_pCont->getClientRect(rcCorr);

		ShrinkRcToSize(rc);
		ShrinkRcToSize(&rcCorr);

		/* correct rectangle position when mouse is not within */
		DoCalcGripperRect(rc, rcCorr, pt);
	}
	else
	{
		/* test if mouse is within work area */
		pContHit = workHitTest(pt, rc);

		/* calcutlates the rect and its position */
		if (pContHit == NULL)
		{
			//if(false) // 无事不绘制
			if(!isCreated&&!oldSchoolDraw)
			{
				rc->left = rc->right = -1;
				return;
			}
			/* calcutlates the rect and draws it */
			if (!_pCont->isFloating())
			{
				if (_startMovingFromTab)
				{
					*rc = _pCont->getDataOfActiveTb()->rcFloat;
				}
				else
				{
					*rc = _pCont->_rcFloat;
				}
			}
			else
			{
				if (_startMoveFromTab)
				{
					*rc = _pCont->getDataOfActiveTb()->rcFloat;
				}
				else
				{
					_pCont->getWindowRect(*rc);
				}
			}
			_pCont->getClientRect(rcCorr);

			CalcRectToScreen(_dockData.hWnd, rc);
			CalcRectToScreen(_dockData.hWnd, &rcCorr);

			rc->left    = pt.x - _ptOffset.x;
			rc->top     = pt.y - _ptOffset.y;

			/* correct rectangle position when mouse is not within */
			DoCalcGripperRect(rc, rcCorr, pt);
		}
	}
}

// fix: return |HTTRANSPARENT| int the |dialog| callback |case WM_NCHITTEST| is invalid! 
//   So It's impossible to exclude the dragging dialog from being returned by |WindowFromPoint|.
//   Don't be panic, we can loop the |dockingContsZOrder| to figure out where to place our floating dialog.
DockingCont* Gripper::dlgsHitTest(POINT pt)
{ 
	if(!_pCont->isFloating())
	{
		return contHitTest(pt);
	}

	RECT	rc	= {0};

	auto iter = dockingContsZOrder.begin();
	//if(0)
	while(iter!=dockingContsZOrder.end())
	{ 
		// first loop, check for floating dialogs
		auto cI = iter._Ptr->_Myval;
		if((_startMoveFromTab||cI!=_pCont)&&cI->isFloating()&&cI->isVisible())
		{
			cI->getWindowRect(rc);
			if(rc.left<=pt.x&&rc.right>=pt.x&&rc.top<=pt.y&&rc.bottom>=pt.y)
			{
				// todo is 24px too small?
				if (pt.y<(rc.top + 24))
				{
					return cI;
				}
				if(_startMoveFromTab)
				{
					::GetWindowRect(cI->getTabWnd(), &rc);
					if (::PtInRect(&rc, pt))
					{
						return cI;
					}
				}
				return NULL;
			}
		}
		iter++;
	} 
	iter = dockingContsZOrder.begin();
	while(iter!=dockingContsZOrder.end())
	{ 
		// second loop, check for docked dialogs
		auto cI = iter._Ptr->_Myval;
		if(!cI->isFloating()&&cI->isVisible())
		{
			//cI->getWindowRect(rc);
			GetWindowRect(cI->getCaptionWnd(), &rc);
			if(rc.left<=pt.x&&rc.right>=pt.x&&rc.top<=pt.y&&rc.bottom>=pt.y)
			{
				return cI;
			}
		}
		iter++;
	}

	/* doesn't hit a container */
	return NULL;
}

DockingCont* Gripper::contHitTest(POINT pt)
{
	vector<DockingCont*>	vCont	= _pDockMgr->getContainerInfo();
	HWND					hWnd	= ::WindowFromPoint(pt);
	//HWND					hWnd	= ::ChildWindowFromPointEx(_pCont->getHParent(), pt, CWP_SKIPDISABLED|CWP_SKIPINVISIBLE);

	for (size_t iCont = 0, len = vCont.size(); iCont < len; ++iCont)
	{
		//if(vCont[iCont]==_pCont&&!isCreated) continue;
		/* test if within caption */
		if (hWnd == vCont[iCont]->getCaptionWnd())
		{
			if (vCont[iCont]->isFloating())
			{
				RECT	rc	= {0};

				vCont[iCont]->getWindowRect(rc);
				if ((rc.top < pt.y) && (pt.y < (rc.top + 24)))
				{
					/* when it is the same container start moving immediately */
					if (vCont[iCont] == _pCont)
					{
						return NULL;
					}
					else
					{
						return vCont[iCont];
					}
				}
			}
			else
			{
				return vCont[iCont];
			}
		}

		/* test only tabs that are visible */
		if (::IsWindowVisible(vCont[iCont]->getTabWnd()))
		{
			/* test if within tab (rect test is used, because of drag and drop behaviour) */
			RECT		rc	= {0};

			::GetWindowRect(vCont[iCont]->getTabWnd(), &rc);
			if (::PtInRect(&rc, pt))
			{
				return vCont[iCont];
			}
		}
	}

	/* doesn't hit a container */
	return NULL;
}


DockingCont* Gripper::workHitTest(POINT pt, RECT *rc)
{
	//if(1) return NULL;
	RECT					rcCont	= {0};
	vector<DockingCont*>	vCont	= _pDockMgr->getContainerInfo();

	/* at first test if cursor points into a visible container */
	for (size_t iCont = 0, len = vCont.size(); iCont < len; ++iCont)
	{
		if (vCont[iCont]->isVisible())
		{
			vCont[iCont]->getWindowRect(rcCont);

			if (::PtInRect(&rcCont, pt) == TRUE)
			{
				/* when it does, return with non found docking area */
				//return NULL;
			}
		}
	}

	/* now search if cusor hits a possible docking area */
	for (int iWork = 0; iWork < DOCKCONT_MAX; ++iWork)
	{
		if (vCont[iWork]!=_pCont&&!vCont[iWork]->isVisible())
		{
			rcCont = _dockData.rcRegion[iWork];
			rcCont.right  += rcCont.left;
			rcCont.bottom += rcCont.top;

			if (rc != NULL)
			{
				*rc = rcCont;
			}

			/* set fix hit test with */
			switch(iWork)
			{
				case CONT_LEFT:
					rcCont.right   = rcCont.left + HIT_TEST_THICKNESS;
					rcCont.left   -= HIT_TEST_THICKNESS;
					break;
				case CONT_RIGHT:
					rcCont.left    = rcCont.right - HIT_TEST_THICKNESS;
					rcCont.right  += HIT_TEST_THICKNESS;
					break;
				case CONT_TOP:
					rcCont.bottom  = rcCont.top + HIT_TEST_THICKNESS;
					rcCont.top    -= HIT_TEST_THICKNESS;
					break;
				case CONT_BOTTOM:
					rcCont.top     = rcCont.bottom - HIT_TEST_THICKNESS;
					rcCont.bottom += HIT_TEST_THICKNESS;
					break;
				default:
					break;
			}
			ClientRectToScreenRect(_dockData.hWnd, &rcCont);

			if (::PtInRect(&rcCont, pt) == TRUE)
			{
				if (rc != NULL)
				{
					ClientRectToScreenRect(_dockData.hWnd, rc);
					rc->right  -= rc->left;
					rc->bottom -= rc->top;
				}
				return vCont[iWork];
			}
		}
	}

	/* no docking area found */
	return NULL;
}


void Gripper::initTabInformation()
{
	/* for tab reordering */

	/* remember handle */
	_hTabSource = _pCont->getTabWnd();
	_startMovingFromTab	= _pCont->startMovingFromTab();
	_startMoveFromTab	= _startMovingFromTab;
	if ((_startMovingFromTab == FALSE) && (::SendMessage(_hTabSource, TCM_GETITEMCOUNT, 0, 0) == 1))
	{
		_startMovingFromTab = TRUE;
		_iItem				= 0;
	}
	else
	{
		// get active tab item
		_iItem = static_cast<int32_t>(::SendMessage(_hTabSource, TCM_GETCURSEL, 0, 0));
	}

	/* get size of item */
	_hTab = _hTabSource;
	::SendMessage(_hTabSource, TCM_GETITEMRECT, _iItem, reinterpret_cast<LPARAM>(&_rcItem));

	/* store item data */
	static TCHAR	szText[64];
	_tcItem.mask		= TCIF_PARAM | TCIF_TEXT;
	_tcItem.pszText		= szText;
	_tcItem.cchTextMax	= 64;
	::SendMessage(_hTabSource, TCM_GETITEM, _iItem, reinterpret_cast<LPARAM>(&_tcItem));
}

