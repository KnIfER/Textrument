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


#pragma once

#include <windows.h>
#include <commctrl.h>
#include "Common.h"
#include "Docking.h"
#include "dockingResource.h"

class DockingCont;
class DockingManager;


#define MDLG_CLASS_NAME TEXT("moveDlg")

class DirectGripper;

class Gripper
{
public:
	Gripper();

	void init(HINSTANCE hInst, HWND hParent);

	void startGrip(DockingCont* pCont, DockingManager* pDockMgr, bool create=true);

	bool stopGrip();

	~Gripper();

//protected :

	void create();

	static LRESULT CALLBACK staticWinProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
	LRESULT runProc(UINT Message, WPARAM wParam, LPARAM lParam);

	void onMove();
	bool onButtonUp();

	void doTabReordering(POINT pt);
	void drawRectangle(const POINT* pPt);
	void drawWindow(const POINT* pPt);
	void getMousePoints(POINT* pt, POINT* ptPrev);
	void getMovingRect(POINT pt, RECT *rc);
	DockingCont * contHitTest(POINT pt);
	DockingCont * dlgsHitTest(POINT pt);
	DockingCont * workHitTest(POINT pt, RECT *rcCont = NULL);

	void initTabInformation();

	void CalcRectToScreen(HWND hWnd, RECT *rc) {
		ClientRectToScreenRect(hWnd, rc);
		ShrinkRcToSize(rc);
	};
	void CalcRectToClient(HWND hWnd, RECT *rc) {
		ScreenRectToClientRect(hWnd, rc);
		ShrinkRcToSize(rc);
	};
	void ShrinkRcToSize(RECT *rc) {
		rc->right	-= rc->left;
		rc->bottom	-= rc->top;
	};
	void DoCalcGripperRect(RECT* rc, RECT rcCorr, POINT pt) {
		if ((rc->left + rc->right) < pt.x)
			rc->left = pt.x - 20;
		if ((rc->top + rc->bottom) < pt.y)
			rc->top  += rcCorr.bottom - rc->bottom;
	};

//private:
	// Handle
	HINSTANCE _hInst;
	HWND _hParent;
	HWND _hSelf;

	// data of container
	tDockMgr _dockData;
	DockingManager *_pDockMgr;
	DockingCont *_pCont;

	// mouse offset in moving rectangle
	POINT _ptOffset;

	// remembers old mouse point
	POINT _ptOld;
	BOOL _bPtOldValid;

	// remember last drawn rectangle (jg)
	RECT _rcPrev;

	// for sorting tabs
	HWND _hTab;
	HWND _hTabSource;

	//  Move From Tab / 或者当前容器只有一个成员
	BOOL _startMovingFromTab;
	BOOL _startMoveFromTab;
	int	_iItem;
	RECT _rcItem;
	TCITEM _tcItem;

	HDC _hdc;
	HBITMAP _hbm;
	HBRUSH _hbrush;


	// is class registered
	static BOOL _isRegistered;
	bool oldSchoolDraw = false;
	bool isCreated=false;
};

