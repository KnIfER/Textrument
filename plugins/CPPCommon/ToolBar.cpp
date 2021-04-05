//this file is part of notepad++
//Copyright (C)2003 Don HO ( donho@altern.org )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

//#include "..\..\resource.h"
#include "ToolBar.h"
#include "SysMsg.h"

const int WS_TOOLBARSTYLE = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TBSTYLE_TOOLTIPS |TBSTYLE_FLAT | CCS_TOP | BTNS_AUTOSIZE | CCS_NOPARENTALIGN | CCS_NORESIZE | CCS_NODIVIDER;

bool ToolBar::init( HINSTANCE hInst, HWND hPere, toolBarStatusType type, 
					ToolBarButtonUnit *buttonUnitArray, int arraySize)
{
	Window::init(hInst, hPere);
	_state = type;
	int iconSize = (_state == TB_LARGE?32:16);
	//int iconSize = NppParameters::getInstance()._dpiManager.scaleX(_state == TB_LARGE?32:16);

	_toolBarIcons.init(buttonUnitArray, arraySize);
	_toolBarIcons.create(_hInst, iconSize);

	_nbDynButtons = _vDynBtnReg.size();

	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC  = ICC_WIN95_CLASSES|ICC_COOL_CLASSES|ICC_BAR_CLASSES|ICC_USEREX_CLASSES;
	InitCommonControlsEx(&icex);

	//Create the list of buttons
	_nbButtons    = arraySize;
	_nbTotalButtons = _nbButtons + (_nbDynButtons ? _nbDynButtons + 1 : 0);
	_pTBB = new TBBUTTON[_nbTotalButtons];	//add one for the extra separator

	int cmd = 0;
	int bmpIndex = -1, style;
	size_t i = 0;
	for (; i < _nbButtons ; ++i)
	{
		cmd = buttonUnitArray[i]._cmdID;
		if (cmd != 0)
		{
			++bmpIndex;
			style = BTNS_BUTTON;
		}
		else
		{
			style = BTNS_SEP;
		}

		_pTBB[i].iBitmap = (cmd != 0?bmpIndex:0);
		_pTBB[i].idCommand = cmd;
		_pTBB[i].fsState = TBSTATE_ENABLED;
		_pTBB[i].fsStyle = (BYTE)style; 
		_pTBB[i].dwData = 0; 
		_pTBB[i].iString = 0;
	}

	if (_nbDynButtons > 0)
	{
		//add separator
		_pTBB[i].iBitmap = 0;
		_pTBB[i].idCommand = 0;
		_pTBB[i].fsState = TBSTATE_ENABLED;
		_pTBB[i].fsStyle = BTNS_SEP;
		_pTBB[i].dwData = 0; 
		_pTBB[i].iString = 0;
		++i;
		//add plugin buttons
		for (size_t j = 0; j < _nbDynButtons ; ++j, ++i)
		{
			cmd = _vDynBtnReg[j].message;
			++bmpIndex;

			_pTBB[i].iBitmap = bmpIndex;
			_pTBB[i].idCommand = cmd;
			_pTBB[i].fsState = TBSTATE_ENABLED;
			_pTBB[i].fsStyle = BTNS_BUTTON; 
			_pTBB[i].dwData = 0; 
			_pTBB[i].iString = 0;
		}
	}

	reset(true);	//load icons etc

	return true;
}

void ToolBar::destroy() {
	if (_pRebar) {
		_pRebar->removeBand(_rbBand.wID);
		_pRebar = NULL;
	}
	delete [] _pTBB;
	::DestroyWindow(_hSelf);
	_hSelf = NULL;
	_toolBarIcons.destroy();
};

int ToolBar::getWidth() const {
	RECT btnRect;
	int totalWidth = 0;
	for (size_t i = 0; i < _nbCurrentButtons; ++i)
	{
		::SendMessage(_hSelf, TB_GETITEMRECT, i, reinterpret_cast<LPARAM>(&btnRect));
		totalWidth += btnRect.right - btnRect.left;
	}
	return totalWidth;
}

int ToolBar::getHeight() const {
	DWORD size = (DWORD)SendMessage(_hSelf, TB_GETBUTTONSIZE, 0, 0);
    DWORD padding = (DWORD)SendMessage(_hSelf, TB_GETPADDING, 0,0);
	int totalHeight = HIWORD(size) + HIWORD(padding);

	return totalHeight;
}

void ToolBar::reset(bool create) 
{


	if (create && _hSelf)
	{
		//Store current button state information
		TBBUTTON tempBtn;
		for (size_t i = 0; i < _nbTotalButtons; ++i)
		{
			::SendMessage(_hSelf, TB_GETBUTTON, i, reinterpret_cast<LPARAM>(&tempBtn));
			_pTBB[i].fsState = tempBtn.fsState;
		}
		::DestroyWindow(_hSelf);
		_hSelf = NULL;
	}

	if (!_hSelf)
	{
		_hSelf = ::CreateWindowEx(
			WS_EX_PALETTEWINDOW,
			TOOLBARCLASSNAME,
			TEXT(""),
			WS_TOOLBARSTYLE,
			0, 0,
			0, 0,
			_hParent,
			NULL,
			_hInst,
			0);
		// Send the TB_BUTTONSTRUCTSIZE message, which is required for 
		// backward compatibility.
		::SendMessage(_hSelf, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
		::SendMessage(_hSelf, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_HIDECLIPPEDBUTTONS);
	}

	if (!_hSelf)
	{
		throw std::runtime_error("ToolBar::reset : CreateWindowEx() function return null");
	}

	if (create) {
		if(_nbDynButtons > 0 && _toolBarIcons.size()<_nbTotalButtons) {
			for (size_t j = 0; j < _nbDynButtons; ++j)
			{
				auto dynIconI = _vDynBtnReg.at(j);
				if(dynIconI.hIcon==0 && dynIconI.ORH->magicNum!=0x666) {
					if(PLUGIN_ICO==0) PLUGIN_ICO = ::LoadIcon(_hInst, MAKEINTRESOURCE(1542));
					dynIconI.hIcon = PLUGIN_ICO;
				}
				_toolBarIcons.addIcon(dynIconI);
			}
		}
	}

	if (_state != TB_STANDARD)
	{
		//If non standard icons, use custom imagelists

		setDefaultImageList();
		setHotImageList();
		setDisableImageList();
	}
	else
	{
		//Else set the internal imagelist with standard bitmaps
		int iconDpiDynamicalSize = 16;
		::SendMessage(_hSelf, TB_SETBITMAPSIZE, 0, MAKELPARAM(iconDpiDynamicalSize, iconDpiDynamicalSize));

		//TBADDBITMAP addbmp = {_hInst, 0};
		TBADDBITMAP addbmp = {0, 0};
		TBADDBITMAP addbmpdyn = {0, 0};
		for (size_t i = 0 ; i < _nbButtons ; ++i)
		{
			int icoID = _toolBarIcons.getStdIconAt(static_cast<int32_t>(i));
			HBITMAP hBmp = static_cast<HBITMAP>(::LoadImage(_hInst, MAKEINTRESOURCE(icoID), IMAGE_BITMAP, iconDpiDynamicalSize, iconDpiDynamicalSize, LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT));
			addbmp.nID = reinterpret_cast<UINT_PTR>(hBmp);

			//addbmp.nID = _toolBarIcons.getStdIconAt(i);
			::SendMessage(_hSelf, TB_ADDBITMAP, 1, reinterpret_cast<LPARAM>(&addbmp));
		}
		if (_nbDynButtons > 0)
		{
			for (size_t j = 0; j < _nbDynButtons; ++j)
			{
				auto atat = _vDynBtnReg.at(j);
				HBITMAP hBmp = atat.hBmp;
				if(!hBmp && atat.ORH->magicNum==0x666) {
					hBmp = static_cast<HBITMAP>(::LoadImage((HINSTANCE)(atat.ORH->HRO), MAKEINTRESOURCE(atat.ORH->resBmp), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADMAP3DCOLORS));
				}
				if(hBmp) {
					addbmpdyn.nID = reinterpret_cast<UINT_PTR>(hBmp);
					::SendMessage(_hSelf, TB_ADDBITMAP, 1, reinterpret_cast<LPARAM>(&addbmpdyn));
				}
			}
		}
	}

	if (create)
	{	//if the toolbar has been recreated, readd the buttons
		size_t nbBtnToAdd = _nbTotalButtons;//(_state == TB_STANDARD?_nbTotalButtons:_nbButtons);
		_nbCurrentButtons = nbBtnToAdd;
		WORD btnSize = (_state == TB_LARGE?32:16);
		::SendMessage(_hSelf, TB_SETBUTTONSIZE , 0, MAKELONG(btnSize, btnSize));
		::SendMessage(_hSelf, TB_ADDBUTTONS, nbBtnToAdd, reinterpret_cast<LPARAM>(_pTBB));
	}
	::SendMessage(_hSelf, TB_AUTOSIZE, 0, 0);

	if (_pRebar)
	{
		_rbBand.hwndChild	= getHSelf();
		_rbBand.cxMinChild	= 0;
		_rbBand.cyIntegral	= 1;
		_rbBand.cyMinChild	= _rbBand.cyMaxChild = getHeight();
		_rbBand.cxIdeal		= getWidth();

		_pRebar->reNew(REBAR_BAR_TOOLBAR, &_rbBand);
	}
}

void ToolBar::registerDynBtn(UINT messageID, toolbarIcons* tIcon)
{
	// Note: Register of buttons only possible before init!
	if ((_hSelf == NULL) && (messageID != 0) && (tIcon->hToolbarBmp != NULL))
	{
		tDynamicList		dynList;
		dynList.message		= messageID;
		dynList.hBmp		= tIcon->hToolbarBmp;
		dynList.hIcon		= tIcon->hToolbarIcon;
		_vDynBtnReg.push_back(dynList);
	}
}

UINT ToolBar::doPopop(POINT chevPoint) {
	//first find hidden buttons
	int width = Window::getWidth();

	size_t start = 0;
	RECT btnRect = {0,0,0,0};
	while(start < _nbCurrentButtons) {
		::SendMessage(_hSelf, TB_GETITEMRECT, start, (LPARAM)&btnRect);
		if(btnRect.right > width)
			break;
		start++;
	}

	if (start < _nbCurrentButtons) {	//some buttons are hidden
		UINT elements = 0;
		TOOLTIPTEXT ttt = {0};
		HMENU menu = ::CreatePopupMenu();
		while (start < _nbCurrentButtons) {

			INT	cmd = _pTBB[start].idCommand;

			/* get text over tooltip function */
			ttt.hdr.code	= TTN_GETDISPINFO;
			ttt.hdr.idFrom	= cmd;
			if (cmd != 0)
			{
				::SendMessage(_hParent, WM_NOTIFY, cmd, (LPARAM)&ttt);
				if (::SendMessage(_hSelf, TB_ISBUTTONENABLED, cmd, 0) != 0) {
					AppendMenu(menu, MF_ENABLED, cmd, ttt.lpszText);
				} else {
					AppendMenu(menu, MF_DISABLED|MF_GRAYED, cmd, ttt.lpszText);
				}
			} else if (elements != 0) {
				AppendMenu(menu, MF_SEPARATOR, 0, TEXT(""));
			}
			elements++;
			start++;
		}
		return (UINT)TrackPopupMenu(menu, TPM_RETURNCMD, chevPoint.x, chevPoint.y, 0, _hParent, NULL);
	}
	return (UINT)-1;
}

void ToolBar::addToRebar(ReBar * rebar) {
	if (_pRebar)
		return;
	_pRebar = rebar;

	ZeroMemory(&_rbBand, sizeof(REBARBANDINFO));
	_rbBand.cbSize  = sizeof(REBARBANDINFO);
	_rbBand.fMask   = RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE |
					  RBBIM_SIZE | RBBIM_IDEALSIZE | RBBIM_ID;

	_rbBand.fStyle		= RBBS_VARIABLEHEIGHT | RBBS_USECHEVRON;
	_rbBand.hwndChild	= getHSelf();
	_rbBand.wID			= REBAR_BAR_TOOLBAR;	//ID REBAR_BAR_TOOLBAR for toolbar
	_rbBand.cxMinChild	= 0;
	_rbBand.cyIntegral	= 1;
	_rbBand.cyMinChild	= _rbBand.cyMaxChild	= getHeight();
	_rbBand.cxIdeal		= _rbBand.cx			= getWidth();

	_pRebar->addBand(&_rbBand, true);

	_rbBand.fMask   = RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_IDEALSIZE | RBBIM_SIZE;
}

void ReBar::init(HINSTANCE hInst, HWND hPere)
{
	Window::init(hInst, hPere);
	
	_hSelf = CreateWindowEx(WS_EX_TOOLWINDOW,
							REBARCLASSNAME,
							NULL,
							WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|RBS_VARHEIGHT|
							RBS_BANDBORDERS | CCS_NODIVIDER | CCS_NOPARENTALIGN,
							0,0,0,0, _hParent, NULL, _hInst, NULL);

	REBARINFO rbi;
	ZeroMemory(&rbi, sizeof(REBARINFO));
	rbi.cbSize = sizeof(REBARINFO);
	rbi.fMask  = 0;
	rbi.himl   = (HIMAGELIST)NULL;
	::SendMessage(_hSelf, RB_SETBARINFO, 0, (LPARAM)&rbi);
}

bool ReBar::addBand(REBARBANDINFO * rBand, bool useID) {
	if (rBand->fMask & RBBIM_STYLE)
		rBand->fStyle |= RBBS_GRIPPERALWAYS;
	else
		rBand->fStyle = RBBS_GRIPPERALWAYS;
	rBand->fMask |= RBBIM_ID | RBBIM_STYLE;
	if (useID) {
		if (isIDTaken(rBand->wID))
			return false;

	} else {
		rBand->wID = getNewID();
	}
	::SendMessage(_hSelf, RB_INSERTBAND, (WPARAM)-1, (LPARAM)rBand);	//add to end of list
	return true;
}

void ReBar::reNew(int id, REBARBANDINFO * rBand) {
	int index = (int)SendMessage(_hSelf, RB_IDTOINDEX, (WPARAM)id, 0);
	::SendMessage(_hSelf, RB_SETBANDINFO, (WPARAM)index, (LPARAM)rBand);
};

void ReBar::removeBand(int id) {
	int index = (int)SendMessage(_hSelf, RB_IDTOINDEX, (WPARAM)id, 0);
	if (id >= REBAR_BAR_EXTERNAL)
		releaseID(id);
	::SendMessage(_hSelf, RB_DELETEBAND, (WPARAM)index, (LPARAM)0);
}

void ReBar::setIDVisible(int id, bool show) {
	int index = (int)SendMessage(_hSelf, RB_IDTOINDEX, (WPARAM)id, 0);
	if (index == -1 )
		return;	//error
	REBARBANDINFO rbBand;
	rbBand.cbSize = sizeof(rbBand);
	rbBand.fMask = RBBIM_STYLE;
	::SendMessage(_hSelf, RB_GETBANDINFO, (WPARAM)index, (LPARAM)&rbBand);
	if (show)
		rbBand.fStyle &= (RBBS_HIDDEN ^ -1);
	else
		rbBand.fStyle |= RBBS_HIDDEN;
	::SendMessage(_hSelf, RB_SETBANDINFO, (WPARAM)index, (LPARAM)&rbBand);
}

bool ReBar::getIDVisible(int id) {
	int index = (int)SendMessage(_hSelf, RB_IDTOINDEX, (WPARAM)id, 0);
	if (index == -1 )
		return false;	//error
	REBARBANDINFO rbBand;
	rbBand.cbSize = sizeof(rbBand);
	rbBand.fMask = RBBIM_STYLE;
	::SendMessage(_hSelf, RB_GETBANDINFO, (WPARAM)index, (LPARAM)&rbBand);
	return ((rbBand.fStyle & RBBS_HIDDEN) == 0);
}

int ReBar::getNewID() {
	int idToUse = REBAR_BAR_EXTERNAL;
	int curVal = 0;
	size_t size = usedIDs.size();
	for(size_t i = 0; i < size; i++) {
		curVal = usedIDs.at(i);
		if (curVal < idToUse) {
			continue;
		} else if (curVal == idToUse) {
			idToUse++;
		} else {
			break;		//found gap
		}
	}

	usedIDs.push_back(idToUse);
	return idToUse;
}

void ReBar::releaseID(int id) {
	size_t size = usedIDs.size();
	for(size_t i = 0; i < size; i++) {
		if (usedIDs.at(i) == id) {
			usedIDs.erase(usedIDs.begin()+i);
			break;
		}
	}
}

bool ReBar::isIDTaken(int id) {
	size_t size = usedIDs.size();
	for(size_t i = 0; i < size; i++) {
		if (usedIDs.at(i) == id) {
			return true;
		}
	}
	return false;
}

void ToolBar::setCheck(int ID2Check, bool willBeChecked) const
{
	auto tbItem = const_cast<ToolBar*>(this)->getRecordedDataFromCMDID(ID2Check);
	if(tbItem) {
		tbItem->fsState&=~TBSTATE_CHECKED;
		if(willBeChecked) {
			tbItem->fsState|=TBSTATE_CHECKED;
		}
	}
	::SendMessage(_hSelf, TB_CHECKBUTTON, ID2Check, MAKELONG(willBeChecked, 0));
};

void ToolBar::setEnable(int ID2Check, bool willBeChecked) const
{
	auto tbItem = const_cast<ToolBar*>(this)->getRecordedDataFromCMDID(ID2Check);
	if(tbItem) {
		tbItem->fsState&=~TBSTATE_ENABLED;
		if(willBeChecked) {
			tbItem->fsState|=TBSTATE_ENABLED;
		}
	}
	::SendMessage(_hSelf, TB_ENABLEBUTTON, ID2Check, MAKELONG(willBeChecked, 0));
};

