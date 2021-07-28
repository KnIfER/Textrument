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



#include <stdexcept>
#include "DockingManager.h"
#include "DockingSplitter.h"
#include "DockingCont.h"
#include "Gripper.h"
#include "Parameters.h"
#include "DirectGripper.h"

using namespace std;

BOOL DockingManager::_isRegistered = FALSE;

extern HWND StatusBarHWND;

extern DirectGripper * dGripper;

bool bSetAllPnlsByRatio = true;

//Window of event handling DockingManager (can only be one)
static	HWND			hWndServer	= NULL;
//Next hook in line
static	HHOOK			gWinCallHook = NULL;
LRESULT CALLBACK focusWndProc(int nCode, WPARAM wParam, LPARAM lParam);
int cccc;
// Callback function that handles messages (to test focus)
LRESULT CALLBACK focusWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION && hWndServer)
	{
		DockingManager *pDockingManager = (DockingManager *)::GetWindowLongPtr(hWndServer, GWLP_USERDATA);
		if (pDockingManager)
		{
			vector<DockingCont*> & vcontainer = pDockingManager->getContainerInfo();
			CWPSTRUCT * pCwp = (CWPSTRUCT*)lParam;
			if (pCwp->message == WM_KILLFOCUS)
			{
				if(IsWindow(pCwp->hwnd))
				for (int i = 0; i < DockingCont::AllDockerLen; ++i) //APP_LAYOUT_RNG_MAX
				{
					if(DockingCont::AllDockers[i]->_isActive&&IsChild(DockingCont::AllDockers[i]->getHSelf(), pCwp->hwnd))
						DockingCont::AllDockers[i]->SetActive(FALSE);
				}
			}
			else if (pCwp->message == WM_SETFOCUS)
			{
				for (int i = 0; i < DockingCont::AllDockerLen; ++i)
				{
					DockingCont::AllDockers[i]->SetActive(IsChild(DockingCont::AllDockers[i]->getHSelf(), pCwp->hwnd));	//activate the container that contains the window with focus, this can be none
				}
			}
		}
	}
	return CallNextHookEx(gWinCallHook, nCode, wParam, lParam);
}

#define SPLITTER_WIDTH			8

DockingManager::DockingManager()
{
	memset(_iContMap, -1, CONT_MAP_MAX * sizeof(int));

	_iContMap[0] = APP_LAYOUT_RNG_LEFT;
	_iContMap[1] = APP_LAYOUT_RNG_RIGHT;
	_iContMap[2] = APP_LAYOUT_RNG_TOP;
	_iContMap[3] = APP_LAYOUT_RNG_BOTTOM;
	_iContMap[4] = APP_LAYOUT_RNG_LEFT_SUB;
	_iContMap[5] = APP_LAYOUT_RNG_RIGHT_SUB;
	_iContMap[6] = APP_LAYOUT_RNG_TOP_SUB;
	_iContMap[7] = APP_LAYOUT_RNG_BOTTOM_SUB;

	// create four containers with splitters
	for (int i = 0; i < APP_LAYOUT_RNG_MAX; ++i)
	{
		DockingCont *_pDockCont = new DockingCont;
		_pDockCont->_rngIdx = i;
		_vPanels.push_back(_pDockCont);

		DockingSplitter *_pSplitter = new DockingSplitter;
		_vSplitter.push_back(_pSplitter);
	}
}

DockingManager::~DockingManager()
{
	// delete 4 splitters
	for (int i = 0; i < APP_LAYOUT_RNG_MAX; ++i)
	{
		delete _vSplitter[i];
	}
}

void DockingManager::init(HINSTANCE hInst, HWND hWnd, Window ** ppWin)
{
	Window::init(hInst, hWnd);

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
		clz.lpszClassName = DSPC_CLASS_NAME;

		if (!::RegisterClass(&clz))
		{
			throw std::runtime_error("DockingManager::init : RegisterClass() function failed");
		}
		_isRegistered = TRUE;
	}

	_hSelf = ::CreateWindowEx(
					0,
					DSPC_CLASS_NAME,
					TEXT(""),
					WS_CHILD | WS_CLIPCHILDREN,
					CW_USEDEFAULT, CW_USEDEFAULT,
					CW_USEDEFAULT, CW_USEDEFAULT,
					_hParent,
					NULL,
					_hInst,
					(LPVOID)this);

	if (!_hSelf)
	{
		throw std::runtime_error("DockingManager::init : CreateWindowEx() function return null");
	}

	setClientWnd(ppWin);


	//DockingManagerData & dockingData = (DockingManagerData &)(NppParameters::getInstance()).getNppGUI()._dockingData;

	// create docking container
	DockingCont* pCont;
	for (int iCont = 0; iCont < APP_LAYOUT_RNG_MAX; ++iCont)
	{
		pCont = _vPanels[iCont];
		pCont->init(_hInst, _hSelf);
		pCont->doDialog(false);
		::SetParent(pCont->getHSelf(), _hParent);

		if (iCont == APP_LAYOUT_RNG_TOP 
			|| iCont == APP_LAYOUT_RNG_BOTTOM
			|| iCont == APP_LAYOUT_RNG_LEFT_SUB
			|| iCont == APP_LAYOUT_RNG_RIGHT_SUB
			)
			_vSplitter[iCont]->init(_hInst, _hParent, _hSelf, DMS_HORIZONTAL);
		else
			_vSplitter[iCont]->init(_hInst, _hParent, _hSelf, DMS_VERTICAL);

		// initialize the saved float shape /\  Noway. the id is random.
		//dockingData.getFloatingRCFrom(iCont, pCont->_rcFloat);
	}
	// register window event hooking
	if (!hWndServer)
		hWndServer = _hSelf;
	CoInitialize(NULL);
	if (!gWinCallHook)	//only set if not already done
		gWinCallHook = ::SetWindowsHookEx(WH_CALLWNDPROC, focusWndProc, hInst, GetCurrentThreadId());

	if (!gWinCallHook)
	{
		throw std::runtime_error("DockingManager::init : SetWindowsHookEx() function return null");
	}

	dGripper = new DirectGripper(_hParent);
	dGripper->create();

	_dockData.hWnd = _hSelf;

	_isInitialized = TRUE;
}

void DockingManager::destroy()
{
	::DestroyWindow(_hSelf);
}

LRESULT CALLBACK DockingManager::staticWinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	DockingManager *pDockingManager = NULL;
	switch (message)
	{
		case WM_NCCREATE :
			pDockingManager = reinterpret_cast<DockingManager *>(reinterpret_cast<LPCREATESTRUCT>(lParam)->lpCreateParams);
			pDockingManager->_hSelf = hwnd;
			::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pDockingManager));
			return TRUE;

		default :
			pDockingManager = reinterpret_cast<DockingManager *>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
			if (!pDockingManager)
				return ::DefWindowProc(hwnd, message, wParam, lParam);
			return pDockingManager->runProc(hwnd, message, wParam, lParam);
	}
}

void DockingManager::updateContainerInfo(HWND hClient)
{
	for (size_t iCont = 0, len = _vPanels.size(); iCont < len; ++iCont)
	{
		if (_vPanels[iCont]->updateInfo(hClient) == TRUE)
		{
			break;
		}
	}
}

void DockingManager::showContainer(HWND hCont, bool display)
{
	for (size_t iCont = 0, len = _vPanels.size(); iCont < len; ++iCont)
	{
		if (_vPanels[iCont]->getHSelf() == hCont)
			showContainer(iCont, display);
	}
}

void DockingManager::showFloatingContainers(bool show)
{
	for (size_t i=0; i < _vPanels.size(); i++)
	{
		// fix crash issue.
		// Note: close the floating panel, minify editor to tray and it'll evokes ghost window.
		if(!_vPanels[i]->getDataOfActiveTb()) continue; 
		size_t iElementCnt = _vPanels[i]->getElementCnt();
		if (iElementCnt > 0)
			_vPanels[i]->display(show);
	}
}

extern Gripper* pGripper_;

LRESULT DockingManager::runProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_NCACTIVATE:
		{
			// activate/deactivate titlebar of toolbars
			for (size_t iCont = APP_LAYOUT_RNG_MAX, len = _vPanels.size(); iCont < len; ++iCont)
			{
				::SendMessage(_vPanels[iCont]->getHSelf(), WM_NCACTIVATE, wParam, static_cast<LPARAM>(-1));
			}

			if (static_cast<int>(lParam) != -1)
			{
				::SendMessage(_hParent, WM_NCACTIVATE, wParam, static_cast<LPARAM>(-1));
			}
			break;
		}
		case WM_MOVE:
		case WM_SIZE:
		{
			resize();
			break;
		}
		case WM_DESTROY:
		{
			// unregister window event hooking BEFORE EVERYTHING ELSE
			if (hWndServer == hwnd)
			{
				UnhookWindowsHookEx(gWinCallHook);
				gWinCallHook = NULL;
				hWndServer = NULL;
			}

			// destroy imagelist if it exists
			if (_hImageList != NULL)
			{
				::ImageList_Destroy(_hImageList);
			}

			// destroy containers
			for (int32_t i = static_cast<int32_t>(_vPanels.size()); i > 0; i--)
			{
				_vPanels[i-1]->destroy();
				delete _vPanels[i-1];
			}
			break;
		}
		case DMM_LBUTTONUP:	//is this message still relevant?
		{
			if (::GetActiveWindow() != _hParent)
				break;

			// set respective activate state
			for (int i = 0; i < APP_LAYOUT_RNG_MAX; ++i)
			{
				_vPanels[i]->SetActive(IsChild(_vPanels[i]->getHSelf(), ::GetFocus()));
			}
			return TRUE;
		}

		case DMM_MOVE:
		{
			if(!pGripper_)
			{
				Gripper *pGripper = new Gripper;
				pGripper->init(_hInst, _hParent);
				pGripper->startGrip(reinterpret_cast<DockingCont*>(lParam), this);
			}
			break;
		}

		case DMM_MOVE_SPLITTER:
		{
			int offset = static_cast<int32_t>(wParam);

			for (int iCont = 0; iCont < APP_LAYOUT_RNG_MAX; ++iCont)
			{
				if (_vSplitter[iCont]->getHSelf() == reinterpret_cast<HWND>(lParam))
				{
					switch (iCont)
					{
						case APP_LAYOUT_RNG_TOP:
							_dockData.rcRegion[iCont].bottom -= offset;
							if (_dockData.rcRegion[iCont].bottom < 0)
							{
								_dockData.rcRegion[iCont].bottom = 0;
							}
							if ((_rcWork.bottom < (-SPLITTER_WIDTH)) && (offset < 0))
							{
								_dockData.rcRegion[iCont].bottom += offset;
							}
							if (_rect.right!=0)
							{
								_vPanels[iCont]->_ratio = _dockData.rcRegion[iCont].bottom*1.f/_rect.bottom;
							}
							break;
						case APP_LAYOUT_RNG_BOTTOM:
							_dockData.rcRegion[iCont].bottom   += offset;
							if (_dockData.rcRegion[iCont].bottom < 0)
							{
								_dockData.rcRegion[iCont].bottom   = 0;
							}
							if ((_rcWork.bottom < (-SPLITTER_WIDTH)) && (offset > 0))
							{
								_dockData.rcRegion[iCont].bottom -= offset;
							}
							if (_rect.right!=0)
							{
								_vPanels[iCont]->_ratio = _dockData.rcRegion[iCont].bottom*1.f/_rect.bottom;
							}
							break;
						case APP_LAYOUT_RNG_LEFT:
							_dockData.rcRegion[iCont].right    -= offset;
							if (_dockData.rcRegion[iCont].right < 0)
							{
								_dockData.rcRegion[iCont].right = 0;
							}
							if ((_rcWork.right < SPLITTER_WIDTH) && (offset < 0))
							{
								_dockData.rcRegion[iCont].right += offset;
							}
							if (_rect.right!=0)
							{
								_vPanels[iCont]->_ratio = _dockData.rcRegion[iCont].right*1.f/_rect.right;
							}
							break;
						case APP_LAYOUT_RNG_RIGHT:
							_dockData.rcRegion[iCont].right    += offset;
							if (_dockData.rcRegion[iCont].right < 0)
							{
								_dockData.rcRegion[iCont].right = 0;
							}
							if ((_rcWork.right < SPLITTER_WIDTH) && (offset > 0))
							{
								_dockData.rcRegion[iCont].right -= offset;
							}
							if (_rect.right!=0)
							{
								_vPanels[iCont]->_ratio = _dockData.rcRegion[iCont].right*1.f/_rect.right;
							}
							break;
						case APP_LAYOUT_RNG_TOP_SUB:
							_dockData.rcRegion[iCont].right    -= offset;
							if (_dockData.rcRegion[APP_LAYOUT_RNG_TOP].right!=0)
							{
								float value = _dockData.rcRegion[iCont].right*1.f/_dockData.rcRegion[APP_LAYOUT_RNG_TOP].right;
								if (value < 0) value = 0;
								_vPanels[iCont]->_ratio = value;
							}
							break;
						case APP_LAYOUT_RNG_BOTTOM_SUB:
							_dockData.rcRegion[iCont].right    -= offset;
							if (_dockData.rcRegion[APP_LAYOUT_RNG_BOTTOM].right!=0)
							{
								float value = _dockData.rcRegion[iCont].right*1.f/_dockData.rcRegion[APP_LAYOUT_RNG_BOTTOM].right;
								if (value < 0) value = 0;
								_vPanels[iCont]->_ratio = value;
							}
							break;
						case APP_LAYOUT_RNG_LEFT_SUB:
							_dockData.rcRegion[iCont].bottom    -= offset;
							if (_dockData.rcRegion[APP_LAYOUT_RNG_LEFT].bottom!=0)
							{
								float value = _dockData.rcRegion[iCont].bottom*1.f/_dockData.rcRegion[APP_LAYOUT_RNG_LEFT].bottom;
								if (value < 0) value = 0;
								_vPanels[iCont]->_ratio = value;
							}
							break;
						case APP_LAYOUT_RNG_RIGHT_SUB:
							_dockData.rcRegion[iCont].bottom    -= offset;
							if (_dockData.rcRegion[APP_LAYOUT_RNG_RIGHT].bottom!=0)
							{
								float value = _dockData.rcRegion[iCont].bottom*1.f/_dockData.rcRegion[APP_LAYOUT_RNG_RIGHT].bottom;
								if (value < 0) value = 0;
								_vPanels[iCont]->_ratio = value;
							}
							break;
					}
					resize();
					break;
				}
			}
			break;
		}
		case DMM_DOCK:
		case DMM_FLOAT:
		{
			toggleActiveTb(reinterpret_cast<DockingCont*>(lParam), message);
			return FALSE;
		}
		case DMM_CLOSE:
		{
			tTbData*	TbData	= (reinterpret_cast<DockingCont*>(lParam))->getDataOfActiveTb();
			LRESULT res = TbData?SendNotify(TbData->hClient, DMN_CLOSE)
				:1;	// Be sure the active item is OK with closing
			return res;
		}
		case DMM_FLOATALL:
		{
			toggleVisTb(reinterpret_cast<DockingCont*>(lParam), DMM_FLOAT);
			return FALSE;
		}
		case DMM_DOCKALL:
		{
			toggleVisTb(reinterpret_cast<DockingCont*>(lParam), DMM_DOCK);
			return FALSE;
		}
		case DMM_GETIMAGELIST:
		{
			return reinterpret_cast<LPARAM>(_hImageList);
		}
		case DMM_GETICONPOS:
		{
			for (size_t uImageCnt = 0, len = _vImageList.size(); uImageCnt < len; ++uImageCnt)
			{
				if (reinterpret_cast<HWND>(lParam) == _vImageList[uImageCnt])
				{
					return uImageCnt;
				}
			}
			return -1;
		}
		default :
			break;
	}

	return ::DefWindowProc(_hSelf, message, wParam, lParam);
}

void DockingManager::resize()
{
    reSizeTo(_rect);
}

void DockingManager::reSizeTo(RECT & rc)
{
	// store current size of client area
	_rect = rc;

	// prepare size of work area
	_rcWork	= rc;

	if (_isInitialized == FALSE)
		return;

	bool TopVis = _vPanels[APP_LAYOUT_RNG_TOP]->isVisible();
	bool TopVis1 = _vPanels[APP_LAYOUT_RNG_TOP_SUB]->isVisible();
	bool BotVis = _vPanels[APP_LAYOUT_RNG_BOTTOM]->isVisible();
	bool BotVis1 = _vPanels[APP_LAYOUT_RNG_BOTTOM_SUB]->isVisible();
	bool LeftVis = _vPanels[APP_LAYOUT_RNG_LEFT]->isVisible();
	bool LeftVis1 = _vPanels[APP_LAYOUT_RNG_LEFT_SUB]->isVisible();
	bool RightVis = _vPanels[APP_LAYOUT_RNG_RIGHT]->isVisible();
	bool RightVis1 = _vPanels[APP_LAYOUT_RNG_RIGHT_SUB]->isVisible();

	bool LeftVisible = LeftVis || LeftVis1;
	bool RightVisible = RightVis || RightVis1;
	bool TopVisible = TopVis || TopVis1;
	bool BotVisible = BotVis || BotVis1;

	bool TopExtrudeLeft =  TopVisible && true;
	bool TopExtrudeRight = TopVisible && true;
	bool BotExtrudeLeft =  BotVisible && true;
	bool BotExtrudeRight = BotVisible && true;

	int LeftW = _dockData.rcRegion[APP_LAYOUT_RNG_LEFT].right+SPLITTER_WIDTH;
	int RightW = _dockData.rcRegion[APP_LAYOUT_RNG_RIGHT].right+SPLITTER_WIDTH;
	int TopH = _dockData.rcRegion[APP_LAYOUT_RNG_TOP].bottom+SPLITTER_WIDTH;
	int BotH = _dockData.rcRegion[APP_LAYOUT_RNG_BOTTOM].bottom+SPLITTER_WIDTH;


	// 四方界 定形分
	// Sysmetry my friend, always brings elegancy to my code.


	// 左极界 
	RECT & rcAppRgn_Left = _dockData.rcRegion[APP_LAYOUT_RNG_LEFT];
	rcAppRgn_Left.left     = rc.left;
	rcAppRgn_Left.top      = _rcWork.top;
	rcAppRgn_Left.bottom   = _rcWork.bottom;
	if (bSetAllPnlsByRatio && rc.right)
	{
		rcAppRgn_Left.right = rc.right*_vPanels[APP_LAYOUT_RNG_LEFT]->_ratio;
	}
	if (TopExtrudeLeft)
	{
		rcAppRgn_Left.top     += TopH;
		rcAppRgn_Left.bottom     -= TopH;
	}
	if (BotExtrudeLeft)
	{
		rcAppRgn_Left.bottom     -= BotH;
	}
	if (LeftVisible)
	{
		_rcWork.left		+= rcAppRgn_Left.right + SPLITTER_WIDTH;
		_rcWork.right	-= rcAppRgn_Left.right + SPLITTER_WIDTH;
		// set size of splitter
		RECT	rc = {rcAppRgn_Left.right,
			rcAppRgn_Left.top,
			SPLITTER_WIDTH,
			rcAppRgn_Left.bottom};
		_vSplitter[APP_LAYOUT_RNG_LEFT]->reSizeTo(rc);
		int total = rcAppRgn_Left.bottom;			// 总计尺寸
		float main=0;								// 主要尺寸
		if (LeftVis)
		{
			main=total;
			if (LeftVis1 && (main*=_vPanels[APP_LAYOUT_RNG_LEFT_SUB]->_ratio)>total-SPLITTER_WIDTH)
			{
				main=total-SPLITTER_WIDTH;
			}
			::SetWindowPos(_vPanels[APP_LAYOUT_RNG_LEFT]->getHSelf(), NULL,
				rcAppRgn_Left.left  ,
				rcAppRgn_Left.top   ,
				rcAppRgn_Left.right ,
				main,
				SWP_NOZORDER);
		}
		if (LeftVis1)
		{
			_dockData.rcRegion[APP_LAYOUT_RNG_LEFT_SUB].bottom=main;
			if (LeftVis)
			{
				rc = {rcAppRgn_Left.left,
					(long)(rcAppRgn_Left.top+main),
					rcAppRgn_Left.right,
					SPLITTER_WIDTH};
				_vSplitter[APP_LAYOUT_RNG_LEFT_SUB]->reSizeTo(rc);
				_vSplitter[APP_LAYOUT_RNG_LEFT_SUB]->display();
				main+=SPLITTER_WIDTH;
			}
			::SetWindowPos(_vPanels[APP_LAYOUT_RNG_LEFT_SUB]->getHSelf(), NULL,
				rcAppRgn_Left.left  ,
				rcAppRgn_Left.top+main ,
				rcAppRgn_Left.right ,
				LeftVis?(total-main):total,
				SWP_NOZORDER);
		}
	}
	_vSplitter[APP_LAYOUT_RNG_LEFT]->display(LeftVisible);



	// 右极界 
	RECT & rcAppRgn_Right = _dockData.rcRegion[APP_LAYOUT_RNG_RIGHT];
	if (bSetAllPnlsByRatio && rc.right)
	{
		rcAppRgn_Right.right = rc.right*_vPanels[APP_LAYOUT_RNG_RIGHT]->_ratio;
	}
	rcAppRgn_Right.left    = rc.right - rcAppRgn_Right.right;
	rcAppRgn_Right.top     = _rcWork.top;
	rcAppRgn_Right.bottom  = _rcWork.bottom;
	if (TopExtrudeRight)
	{
		rcAppRgn_Right.top     += TopH;
		rcAppRgn_Right.bottom     -= TopH;
	}
	if (BotExtrudeRight)
	{
		rcAppRgn_Right.bottom     -= BotH;
	}
	RECT		rcRightTmp		= rcAppRgn_Right;
	if (RightVisible)
	{
		_rcWork.right	-= rcAppRgn_Right.right + SPLITTER_WIDTH;
		// correct the visibility of right container when width is NULL
		if (_rcWork.right < 15)
		{
			rcRightTmp.left    = _rcWork.left + 15 + SPLITTER_WIDTH;
			rcRightTmp.right  += _rcWork.right - 15;
			_rcWork.right	= 15;
		}
		RECT	rc = {rcRightTmp.left - SPLITTER_WIDTH,
			rcRightTmp.top,
			SPLITTER_WIDTH,
			rcRightTmp.bottom};
		_vSplitter[APP_LAYOUT_RNG_RIGHT]->reSizeTo(rc);
		int total = rcRightTmp.bottom;				  // 总计尺寸
		float main=0;								  // 主要尺寸
		if (RightVis)
		{
			main=total;
			if (RightVis1 && (main*=_vPanels[APP_LAYOUT_RNG_RIGHT_SUB]->_ratio)>total-SPLITTER_WIDTH)
			{
				main=total-SPLITTER_WIDTH;
			}
			::SetWindowPos(_vPanels[APP_LAYOUT_RNG_RIGHT]->getHSelf(), NULL,
				rcRightTmp.left  ,
				rcRightTmp.top   ,
				rcRightTmp.right ,
				main ,
				SWP_NOZORDER);
		}
		if (RightVis1)
		{
			_dockData.rcRegion[APP_LAYOUT_RNG_RIGHT_SUB].bottom=main;
			if (RightVis)
			{
				rc = {rcRightTmp.left,
					(long)(rcRightTmp.top+main),
					rcRightTmp.right,
					SPLITTER_WIDTH};
				_vSplitter[APP_LAYOUT_RNG_RIGHT_SUB]->reSizeTo(rc);
				_vSplitter[APP_LAYOUT_RNG_RIGHT_SUB]->display();
				main+=SPLITTER_WIDTH;
			}
			::SetWindowPos(_vPanels[APP_LAYOUT_RNG_RIGHT_SUB]->getHSelf(), NULL,
				rcRightTmp.left  ,
				rcRightTmp.top+main   ,
				rcRightTmp.right ,
				RightVis?(total-main):total,
				SWP_NOZORDER);
		}
	}
	_vSplitter[APP_LAYOUT_RNG_RIGHT]->display(RightVisible);


	// 顶界
	RECT & rcAppRgn_Top = _dockData.rcRegion[APP_LAYOUT_RNG_TOP];
	rcAppRgn_Top.left      = rc.left;
	rcAppRgn_Top.top       = rc.top;
	rcAppRgn_Top.right     = rc.right-rc.left;
	if (LeftVisible && !TopExtrudeLeft)
	{
		rcAppRgn_Top.left += LeftW;
		rcAppRgn_Top.right -= LeftW;
	}
	if (RightVisible && !TopExtrudeRight)
	{
		rcAppRgn_Top.right -= RightW;
	}
	if (bSetAllPnlsByRatio && rc.bottom)
	{
		rcAppRgn_Top.bottom = rc.bottom*_vPanels[APP_LAYOUT_RNG_TOP]->_ratio;
	}
	if (TopVisible)
	{
		_rcWork.top		+= rcAppRgn_Top.bottom + SPLITTER_WIDTH;
		_rcWork.bottom	-= rcAppRgn_Top.bottom + SPLITTER_WIDTH;

		// set size of splitter
		RECT	rc = {rcAppRgn_Top.left  ,
					  rcAppRgn_Top.top + rcAppRgn_Top.bottom,
					  rcAppRgn_Top.right ,
					  SPLITTER_WIDTH};
		_vSplitter[APP_LAYOUT_RNG_TOP]->reSizeTo(rc);
		int total = rcAppRgn_Top.right;				  // 总计尺寸
		float main=0;								  // 主要尺寸
		if (TopVis)
		{
			main=total;
			if (TopVis1 && (main*=_vPanels[APP_LAYOUT_RNG_TOP_SUB]->_ratio)>total-SPLITTER_WIDTH)
			{
				main=total-SPLITTER_WIDTH;
			}
			::SetWindowPos(_vPanels[APP_LAYOUT_RNG_TOP]->getHSelf(), NULL,
				rcAppRgn_Top.left  ,
				rcAppRgn_Top.top   ,
				main ,
				rcAppRgn_Top.bottom,
				SWP_NOZORDER);
		}
		if (TopVis1)
		{
			_dockData.rcRegion[APP_LAYOUT_RNG_TOP_SUB].right=main;
			if (TopVis)
			{
				rc = {(LONG)(rcAppRgn_Top.left+main),
					rcAppRgn_Top.top,
					SPLITTER_WIDTH,
					rcAppRgn_Top.bottom};
				_vSplitter[APP_LAYOUT_RNG_TOP_SUB]->reSizeTo(rc);
				_vSplitter[APP_LAYOUT_RNG_TOP_SUB]->display();
				main += SPLITTER_WIDTH;
			}
			::SetWindowPos(_vPanels[APP_LAYOUT_RNG_TOP_SUB]->getHSelf(), NULL,
				rcAppRgn_Top.left + main  ,
				rcAppRgn_Top.top   ,
				TopVis?(total-main):total ,
				rcAppRgn_Top.bottom,
				SWP_NOZORDER);
		}
	}
	_vSplitter[APP_LAYOUT_RNG_TOP]->display(TopVisible);


	// 底界
	RECT & rcAppRgn_Bot = _dockData.rcRegion[APP_LAYOUT_RNG_BOTTOM];
	rcAppRgn_Bot.left   = rc.left;
	rcAppRgn_Bot.top    = rc.top + rc.bottom - rcAppRgn_Bot.bottom;
	rcAppRgn_Bot.right  = rc.right-rc.left;
	if (LeftVisible && !BotExtrudeLeft)
	{
		rcAppRgn_Bot.left += LeftW;
		rcAppRgn_Bot.right -= LeftW;
	}
	if (RightVisible && !BotExtrudeRight)
	{
		rcAppRgn_Bot.right -= RightW;
	}
	if (bSetAllPnlsByRatio && rc.bottom)
	{
		rcAppRgn_Bot.bottom = rc.bottom*_vPanels[APP_LAYOUT_RNG_BOTTOM]->_ratio;
	}
	RECT		rcBottomTmp	= rcAppRgn_Bot;
	if (BotVisible)
	{
		_rcWork.bottom	-= rcAppRgn_Bot.bottom + SPLITTER_WIDTH;
		// correct the visibility of bottom container when height is NULL
		if (_rcWork.bottom < rc.top)
		{
			rcBottomTmp.top     = _rcWork.top + rc.top + SPLITTER_WIDTH;
			rcBottomTmp.bottom += _rcWork.bottom - rc.top;
			_rcWork.bottom = rc.top;
		}
		if ((rcBottomTmp.bottom + SPLITTER_WIDTH) < 0)
		{
			_rcWork.bottom = rc.bottom - rcAppRgn_Top.bottom;
		}
		// set size of splitter
		RECT	rc = {rcBottomTmp.left,
					  rcBottomTmp.top - SPLITTER_WIDTH,
					  rcBottomTmp.right,
					  SPLITTER_WIDTH};
		_vSplitter[APP_LAYOUT_RNG_BOTTOM]->reSizeTo(rc);
		int total = rcBottomTmp.right;					// 总计尺寸
		float main=0;									// 主要尺寸
		if (BotVis)
		{
			main=total;
			if (BotVis1 && (main*=_vPanels[APP_LAYOUT_RNG_BOTTOM_SUB]->_ratio)>total-SPLITTER_WIDTH)
			{
				main=total-SPLITTER_WIDTH;
			}
			::SetWindowPos(_vPanels[APP_LAYOUT_RNG_BOTTOM]->getHSelf(), NULL,
				rcBottomTmp.left  ,
				rcBottomTmp.top   ,
				main ,
				rcBottomTmp.bottom,
				SWP_NOZORDER);
		}
		//_vPanels[APP_LAYOUT_RNG_BOTTOM_SUB]->display();
		if (BotVis1)
		{
			_dockData.rcRegion[APP_LAYOUT_RNG_BOTTOM_SUB].right=main;
			if (BotVis)
			{
				rc = {(LONG)(rcBottomTmp.left+main),
					rcBottomTmp.top,
					SPLITTER_WIDTH,
					rcBottomTmp.bottom};
				_vSplitter[APP_LAYOUT_RNG_BOTTOM_SUB]->reSizeTo(rc);
				_vSplitter[APP_LAYOUT_RNG_BOTTOM_SUB]->display();
				main += SPLITTER_WIDTH;
			}
			::SetWindowPos(_vPanels[APP_LAYOUT_RNG_BOTTOM_SUB]->getHSelf(), NULL,
				rcBottomTmp.left + main  ,
				rcBottomTmp.top   ,
				BotVis?(total-main):total ,
				rcBottomTmp.bottom,
				SWP_NOZORDER);
		}
	}
	_vSplitter[APP_LAYOUT_RNG_BOTTOM]->display(BotVisible);


	// 中心界
	(*_ppMainWindow)->reSizeTo(_rcWork);
}

void DockingManager::createDockableDlg(tTbData data, int iCont, bool isVisible)
{
	// add icons
	if ((data.uMask & DWS_ICONTAB) && data.hIconTab != NULL)
	{
		// create image list if not exist
		if (_hImageList == NULL)
		{
			int iconDpiDynamicalSize = NppParameters::getInstance()._dpiManager.scaleY(12) + 2;
			_hImageList = ::ImageList_Create(iconDpiDynamicalSize, iconDpiDynamicalSize, ILC_COLOR32 | ILC_MASK, 0, 0);
		}

		// add icon
		::ImageList_AddIcon(_hImageList, data.hIconTab);

		// do the reference here to find later the correct position
		_vImageList.push_back(data.hClient);
	}

	// create additional containers if necessary
	RECT				rc			= {0,0,0,0};
	DockingCont*		pCont		= NULL;

	// if floated rect not set
	if (memcmp(&data.rcFloat, &rc, sizeof(RECT)) == 0)
	{
		// set default rect state
		::GetWindowRect(data.hClient, &data.rcFloat);

		// test if dialog is first time created
		if (iCont == -1)
		{
			// set default visible state
			isVisible = (::IsWindowVisible(data.hClient) == TRUE);

			if (data.uMask & DWS_DF_FLOATING)
			{
				// create new container
				pCont = new DockingCont;
				_vPanels.push_back(pCont);

				// initialize
				pCont->init(_hInst, _hSelf);
				pCont->doDialog(isVisible, true);

				// get previous position and set container id
				data.iPrevCont = (data.uMask & 0x30000000) >> 28;
				iCont = static_cast<int32_t>(_vPanels.size()) - 1;
			}
			else
			{
				// set position
				iCont = (data.uMask & 0x30000000) >> 28;

				// previous container is not selected
				data.iPrevCont = -1;
			}
		}
	}
	// if one of the container was not created before
	else if ((iCont >= APP_LAYOUT_RNG_MAX) || (data.iPrevCont >= APP_LAYOUT_RNG_MAX))
	{
        // test if current container is in floating state
		if (iCont >= APP_LAYOUT_RNG_MAX)
		{
			// no mapping for available store mapping
			if (_iContMap[iCont] == -1)
			{
				// create new container
				pCont = new DockingCont;
				_vPanels.push_back(pCont);

				// initialize and map container id
				pCont->init(_hInst, _hSelf);
				pCont->doDialog(isVisible, true);
				_iContMap[iCont] = static_cast<int32_t>(_vPanels.size()) - 1;
			}

			// get current container from map
			iCont = _iContMap[iCont];
		}
		// previous container is in floating state
		else
		{
			// no mapping for available store mapping
			if (_iContMap[data.iPrevCont] == -1)
			{
				// create new container
				pCont = new DockingCont;
				_vPanels.push_back(pCont);

				// initialize and map container id
				pCont->init(_hInst, _hSelf);
				pCont->doDialog(false, true);
				pCont->reSizeToWH(data.rcFloat);
				_iContMap[data.iPrevCont] = static_cast<int32_t>(_vPanels.size()) - 1;
			}
			data.iPrevCont = _iContMap[data.iPrevCont];
		}
	}

	// attach toolbar
	if (_vPanels.size() > (size_t)iCont && _vPanels[iCont] != NULL)
	{
		_vPanels[iCont]->createToolbar(data, isVisible);
	}

	// notify client app
	if (iCont < APP_LAYOUT_RNG_MAX)
		SendNotify(data.hClient, MAKELONG(DMN_DOCK, iCont));
	else
		SendNotify(data.hClient, MAKELONG(DMN_FLOAT, iCont));
}

void DockingManager::setActiveTab(int iCont, int iItem, bool bMustSet)
{
	if ((iCont == -1) || (_iContMap[iCont] == -1))
		return;
	if (bMustSet && iItem>0)
	{
		int elCnt = _vPanels[_iContMap[iCont]]->getElementCnt();
		if (iItem>=elCnt)
		{
			iItem = elCnt-1;
		}
	}
	_vPanels[_iContMap[iCont]]->setActiveTb(iItem);
}

void DockingManager::showDockableDlg(HWND hDlg, BOOL view)
{
	for (size_t i = 0, len = _vPanels.size(); i < len; ++i)
	{
		tTbData *pTbData = _vPanels[i]->findToolbarByWnd(hDlg);
		if (pTbData != NULL)
		{
			_vPanels[i]->showToolbar(pTbData, view);
			return;
		}
	}
}

void DockingManager::showDockableDlg(TCHAR* pszName, BOOL view)
{
	for (size_t i = 0, len = _vPanels.size(); i < len; ++i)
	{
		tTbData *pTbData = _vPanels[i]->findToolbarByName(pszName);
		if (pTbData != NULL)
		{
			_vPanels[i]->showToolbar(pTbData, view);
			return;
		}
	}
}

LRESULT DockingManager::SendNotify(HWND hWnd, UINT message)
{
	NMHDR	nmhdr;
	nmhdr.code		= message;
	nmhdr.hwndFrom	= _hParent;
	nmhdr.idFrom	= ::GetDlgCtrlID(_hParent);
	::SendMessage(hWnd, WM_NOTIFY, nmhdr.idFrom, reinterpret_cast<LPARAM>(&nmhdr));
	return ::GetWindowLongPtr(hWnd, DWLP_MSGRESULT);
}

void DockingManager::setDockedContSize(int iCont, int iSize)
{
	float ratio = 0.22;
	if (_rect.right<=0||_rect.bottom<=0)
	{
		::GetClientRect(getHParent(), &_rect);
	}
	//LogIs(2, "%d, %d", _rect.right, _rect.bottom);
	if ((iCont == APP_LAYOUT_RNG_TOP) || (iCont == APP_LAYOUT_RNG_BOTTOM)) {
		_dockData.rcRegion[iCont].bottom = iSize;
		ratio = iSize*1.f/_rect.bottom;
	}
	else if ((iCont == APP_LAYOUT_RNG_LEFT) || (iCont == APP_LAYOUT_RNG_RIGHT)){
		_dockData.rcRegion[iCont].right = iSize;
		ratio = iSize*1.f/_rect.right;
	}
	else {
		if (iCont>=APP_LAYOUT_RNG_MAX/2&&iCont<APP_LAYOUT_RNG_MAX)
		{
			ratio = iSize==0?0.5f:(iSize*1.f/0xfff);
		} 
		else 
		{
			(iCont==-1?_rect.right:_rect.bottom) = iSize;
			return;
		}
	}
	if (ratio<0)
	{
		ratio = 0;
	}
	if (ratio>=1)
	{
		ratio = 0.99;
	}
	_vPanels[iCont]->_ratio = ratio;
	resize();
}

int DockingManager::getDockedContSize(int iCont)
{
	if ((iCont == APP_LAYOUT_RNG_TOP) || (iCont == APP_LAYOUT_RNG_BOTTOM))
		return _dockData.rcRegion[iCont].bottom;
	else if ((iCont == APP_LAYOUT_RNG_LEFT) || (iCont == APP_LAYOUT_RNG_RIGHT))
		return _dockData.rcRegion[iCont].right;
	else  {
		if (iCont>=APP_LAYOUT_RNG_MAX/2&&iCont<APP_LAYOUT_RNG_MAX)
		{
			return _vPanels[iCont]->_ratio*0xfff;
		} 
		return iCont==-1?_rect.right:_rect.bottom;
	}
}

DockingCont* DockingManager::toggleActiveTb(DockingCont* pContSrc, UINT message, BOOL bNew, LPRECT prcFloat)
{
	//if (true) return 0;
	tTbData			TbData		= *pContSrc->getDataOfActiveTb();
	int				iContSrc	= GetContainer(pContSrc);
	int				iContPrev	= TbData.iPrevCont;
	BOOL			isCont		= ContExists(iContPrev);
	DockingCont*	pContTgt	= NULL;

	RECT rc = TbData.rcFloat;
	// if new float position is given
	if (prcFloat != NULL)
	{
	//	pContSrc->_rcFloat = *prcFloat;
		rc = *prcFloat;
		TbData.rcFloat = rc;
	}


	if ((isCont == FALSE) || (bNew == TRUE))
	{
		// find an empty container
		int	iContNew = FindEmptyContainer();

		//iContNew = -1;

		if (iContNew == -1)
		{
			// if no free container is available create a new one
			pContTgt = new DockingCont;

			pContTgt->_rcFloat = rc;

			pContTgt->init(_hInst, _hSelf);
			pContTgt->doDialog(true, true);

			// change only on toggling
			if ((bNew == FALSE) || (!pContSrc->isFloating()))
				TbData.iPrevCont = iContSrc;

			pContTgt->createToolbar(TbData);
			_vPanels.push_back(pContTgt);
		}
		else
		{
			// set new target
			pContTgt = _vPanels[iContNew];

			pContTgt->_rcFloat = rc;

			//WINDOWPLACEMENT placement = {sizeof(WINDOWPLACEMENT)};
			//GetWindowPlacement(pContTgt->getHSelf(), &placement);
			//placement.rcNormalPosition = TbData.rcFloat;
			//SetWindowPlacement(pContTgt->getHSelf(), &placement);

			// change only on toggling
			if ((pContSrc->isFloating()) != (pContTgt->isFloating()))
                TbData.iPrevCont = iContSrc;

			//ShowWindow(pContTgt->getHSelf(), SW_SHOWNORMAL);

			pContTgt->createToolbar(TbData);
		}
	}
	else
	{
		// set new target
		pContTgt = _vPanels[iContPrev];

		pContTgt->_rcFloat = rc;

		// change data normaly
		TbData.iPrevCont = iContSrc;
		pContTgt->createToolbar(TbData);
	}
	pContTgt->_rcFloat = rc;
	if (prcFloat != NULL)
	{
		//pContTgt->_rcFloatPlace = *prcFloat;
	}

	// notify client app
	SendNotify(TbData.hClient, MAKELONG(message==DMM_DOCK?DMN_DOCK:DMN_FLOAT, GetContainer(pContTgt)));

	// remove toolbar from source
	_vPanels[iContSrc]->removeToolbar(TbData);

	return pContTgt;
}

DockingCont* DockingManager::toggleVisTb(DockingCont* pContSrc, UINT message, LPRECT prcFloat)
{
	//if (true) return 0;
	vector<tTbData*>	vTbData		= pContSrc->getDataOfVisTb();
	tTbData*			pTbData		= pContSrc->getDataOfActiveTb();

	int activeTb = pContSrc->getActiveTb();

	int					iContSrc	= GetContainer(pContSrc);
	int					iContPrev	= pTbData->iPrevCont;
	BOOL				isCont		= ContExists(iContPrev);
	DockingCont*		pContTgt	= NULL;

	// at first hide container and resize
	pContSrc->doDialog(false);
	resize();

	if (prcFloat != NULL)
	{
		pContSrc->_rcFloat = *prcFloat;
	}

	for (size_t iTb = 0, len = vTbData.size(); iTb < len; ++iTb)
	{
		// get data one by another
		tTbData		TbData = *vTbData[iTb];

		// if new float position is given
		if (prcFloat != NULL)
		{
			TbData.rcFloat = *prcFloat;
		}

		if (isCont == FALSE)
		{
            // create new container
			pContTgt = new DockingCont;
			pContTgt->_rcFloat = pContSrc->_rcFloat;

			pContTgt->init(_hInst, _hSelf);
			pContTgt->doDialog(true, true);

			TbData.iPrevCont = iContSrc;
			pContTgt->createToolbar(TbData);
			_vPanels.push_back(pContTgt);

			// now container exists
			isCont	= TRUE;
			iContPrev = GetContainer(pContTgt);
		}
		else
		{
			// set new target
			pContTgt = _vPanels[iContPrev];

			TbData.iPrevCont = iContSrc;
			pContTgt->createToolbar(TbData);
		}

		pContTgt->_rcFloat = pContSrc->_rcFloat;
		if (prcFloat != NULL)
		{
			//pContTgt->_rcFloatPlace = *prcFloat;
		}

		SendNotify(TbData.hClient, MAKELONG(message==DMM_DOCK?DMN_DOCK:DMN_FLOAT, GetContainer(pContTgt)));

		// remove toolbar from anywhere
		_vPanels[iContSrc]->removeToolbar(TbData, iTb==len-1);
	}

	_vPanels[iContPrev]->setActiveTb(activeTb);

	return pContTgt;
}

void DockingManager::toggleActiveTbWnd(DockingCont* pContSrc, DockingCont* pContTgt)
{
	//if (true) return;
	tTbData		TbData		= *pContSrc->getDataOfActiveTb();

	toggleTb(pContSrc, pContTgt, TbData);
}

void DockingManager::toggleVisTbWnd(DockingCont* pContSrc, DockingCont* pContTgt)
{
	//if (true) return;
	vector<tTbData*>	vTbData		= pContSrc->getDataOfVisTb();
	tTbData*			pTbData		= pContSrc->getDataOfActiveTb();
	int activeTb = pContTgt->getElementCnt()+pContSrc->getActiveTb();

	// at first hide container and resize
	pContSrc->doDialog(false);
	resize();

	for (size_t iTb = 0, len = vTbData.size(); iTb < len; ++iTb)
	{
		// get data one by another
		tTbData		TbData = *vTbData[iTb];
		toggleTb(pContSrc, pContTgt, TbData, iTb==len-1);
	}
	pContTgt->setActiveTb(activeTb);
}

void DockingManager::toggleTb(DockingCont* pContSrc, DockingCont* pContTgt, tTbData TbData, bool activate)
{
	//if (true) return;
	int					iContSrc	= GetContainer(pContSrc);
	int					iContTgt	= GetContainer(pContTgt);

	// test if container state changes from docking to floating or vice versa
	if (((iContSrc <  APP_LAYOUT_RNG_MAX) && (iContTgt >= APP_LAYOUT_RNG_MAX)) ||
		((iContSrc >= APP_LAYOUT_RNG_MAX) && (iContTgt <  APP_LAYOUT_RNG_MAX)))
	{
		// change states
		TbData.iPrevCont = iContSrc;
	}

	// notify client app
	if (iContTgt < APP_LAYOUT_RNG_MAX)
		SendNotify(TbData.hClient, MAKELONG(DMN_DOCK, iContTgt));
	else
		SendNotify(TbData.hClient, MAKELONG(DMN_FLOAT, iContTgt));

	// create new toolbar
	pContTgt->createToolbar(TbData);

	//pContTgt->_rcFloat = pContSrc->_rcFloat;

	// remove toolbar from source
	_vPanels[iContSrc]->removeToolbar(TbData, activate);
}

BOOL DockingManager::ContExists(size_t iCont)
{
	BOOL	bRet = FALSE;

	if (iCont < _vPanels.size())
	{
		bRet = TRUE;
	}

	return bRet;
}

int DockingManager::GetContainer(DockingCont* pCont)
{
	int iRet = -1;
	for (size_t iCont = 0, len = _vPanels.size(); iCont < len; ++iCont)
	{
		if (_vPanels[iCont] == pCont)
		{
			iRet = static_cast<int32_t>(iCont);
			break;
		}
	}

	return iRet;
}

int DockingManager::FindEmptyContainer()
{
    int      iRetCont       = -1;
    BOOL*    pPrevDockList  = (BOOL*) new BOOL[_vPanels.size()+1];
    BOOL*    pArrayPos      = &pPrevDockList[1];

    // delete all entries
    for (size_t iCont = 0, len = _vPanels.size()+1; iCont < len; ++iCont)
    {
        pPrevDockList[iCont] = FALSE;
    }

    // search for used floated containers
    for (size_t iCont = 0; iCont < APP_LAYOUT_RNG_MAX; ++iCont)
    {
        vector<tTbData*>    vTbData = _vPanels[iCont]->getDataOfAllTb();

        for (size_t iTb = 0, len = vTbData.size(); iTb < len; ++iTb)
        {
            pArrayPos[vTbData[iTb]->iPrevCont] = TRUE;
        }
    }

    // find free container
    for (size_t iCont = APP_LAYOUT_RNG_MAX, len = _vPanels.size(); iCont < len; ++iCont)
    {
        if (pArrayPos[iCont] == FALSE)
        {
            // and test if container is hidden
            if (!_vPanels[iCont]->isVisible())
            {
				iRetCont = static_cast<int32_t>(iCont);
                break;
            }
        }
    }

	pPrevDockList[_vPanels.size()] = 0;

    delete [] pPrevDockList; // todo may crash

    // search for empty arrays
    return iRetCont;
}


