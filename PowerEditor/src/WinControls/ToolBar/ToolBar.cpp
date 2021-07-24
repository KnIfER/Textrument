// This file is part of Notepad++ project
// Copyright (C)2020 Don HO <don.h@free.fr>
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
#include "ToolBar.h"
#include "shortcut.h"

//strange, what happened?

#ifndef PluginToolbar
#include "FindReplaceDlg_rc.h"
#include "Notepad_plus.h"
#endif

const int WS_TOOLBARSTYLE = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TBSTYLE_TOOLTIPS |TBSTYLE_FLAT | CCS_TOP | BTNS_AUTOSIZE | CCS_NOPARENTALIGN | CCS_NORESIZE | CCS_NODIVIDER;

ToolBar::ToolBar(DPIManager* _dpiManager)
{
	dpiManager=_dpiManager;
}

ToolBar::ToolBar()
{
#ifdef PluginToolbar
	dpiManager = new DPIManager();
#endif
}

void ToolBar::initTheme(TiXmlDocument *toolIconsDocRoot)
{
#ifdef ToolBarTheme
    _toolIcons =  toolIconsDocRoot->FirstChild(TEXT("NotepadPlus"));
	if (_toolIcons)
	{
		_toolIcons = _toolIcons->FirstChild(TEXT("ToolBarIcons"));
		if (_toolIcons)
		{
			_toolIcons = _toolIcons->FirstChild(TEXT("Theme"));
			if (_toolIcons)
			{
				const TCHAR *themeDir = (_toolIcons->ToElement())->Attribute(TEXT("pathPrefix"));

				for (TiXmlNode *childNode = _toolIcons->FirstChildElement(TEXT("Icon"));
					 childNode ;
					 childNode = childNode->NextSibling(TEXT("Icon")))
				{
					int iIcon;
					const TCHAR *res = (childNode->ToElement())->Attribute(TEXT("id"), &iIcon);
					if (res)
					{
						TiXmlNode *grandChildNode = childNode->FirstChildElement(TEXT("normal"));
						if (grandChildNode)
						{
							TiXmlNode *valueNode = grandChildNode->FirstChild();
							//putain, enfin!!!
							if (valueNode)
							{
								generic_string locator = themeDir?themeDir:TEXT("");
								
								locator += valueNode->Value();
								_customIconVect.push_back(iconLocator(0, iIcon, locator));
							}
						}

						grandChildNode = childNode->FirstChildElement(TEXT("hover"));
						if (grandChildNode)
						{
							TiXmlNode *valueNode = grandChildNode->FirstChild();
							//putain, enfin!!!
							if (valueNode)
							{
								generic_string locator = themeDir?themeDir:TEXT("");
								
								locator += valueNode->Value();
								_customIconVect.push_back(iconLocator(1, iIcon, locator));
							}
						}

						grandChildNode = childNode->FirstChildElement(TEXT("disabled"));
						if (grandChildNode)
						{
							TiXmlNode *valueNode = grandChildNode->FirstChild();
							//putain, enfin!!!
							if (valueNode)
							{
								generic_string locator = themeDir?themeDir:TEXT("");
								
								locator += valueNode->Value();
								_customIconVect.push_back(iconLocator(2, iIcon, locator));
							}
						}
					}
				}
			}
		}
	}
#endif
}

bool ToolBar::init( HINSTANCE hInst, HWND hPere, toolBarStatusType type, ToolBarButtonUnit *buttonUnitArray, int arraySize)
{
	Window::init(hInst, hPere);
	_state = type;
	int iconDpiDynamicalSize = dpiManager->scaleX(_state == TB_LARGE?32:16);

	_toolBarIcons.init(buttonUnitArray, arraySize);
	_toolBarIcons.create(_hInst, iconDpiDynamicalSize);

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
		_pTBB[i].dwData = i; //store standard icon idx here.
		_pTBB[i].iString = 0;
		_pTBB_CMDID_table[cmd]=i;
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
			_pTBB_CMDID_table[cmd]=i;
		}
	}

	reset(true);	//load icons etc

	return true;
}

void ToolBar::destroy()
{
	if (_pRebar)
	{
		_pRebar->removeBand(_rbBand.wID);
		_pRebar = NULL;
	}
	delete [] _pTBB;
	if(_pTBB_CUSTOM) {
		delete _pTBB_CUSTOM;
	}
	::DestroyWindow(_hSelf);
	_hSelf = NULL;
	_toolBarIcons.destroy();
};

int ToolBar::getWidth() const
{
	RECT btnRect;
	int totalWidth = 0;
	for (size_t i = 0; i < _nbCurrentButtons; ++i)
	{
		::SendMessage(_hSelf, TB_GETITEMRECT, i, reinterpret_cast<LPARAM>(&btnRect));
		totalWidth += btnRect.right - btnRect.left;
	}
	return totalWidth;
}

int ToolBar::getHeight() const
{
	DWORD size = static_cast<DWORD>(SendMessage(_hSelf, TB_GETBUTTONSIZE, 0, 0));
	DWORD padding = static_cast<DWORD>(SendMessage(_hSelf, TB_GETPADDING, 0, 0));
	int totalHeight = HIWORD(size) + HIWORD(padding) - (rows==1?3:0);
	return totalHeight;
}

void ToolBar::reduce() 
{
	if (_state == TB_SMALL)
		return;

	int iconDpiDynamicalSize = dpiManager->scaleX(16);
	_toolBarIcons.resizeIcon(iconDpiDynamicalSize);
	bool recreate = (_state == TB_STANDARD || _state == TB_LARGE);
	setState(TB_SMALL);
	reset(recreate);	//recreate toolbar if previous state was Std icons or Big icons
	Window::redraw();
}

void ToolBar::enlarge()
{
	if (_state == TB_LARGE)
		return;

	int iconDpiDynamicalSize = dpiManager->scaleX(32);
	_toolBarIcons.resizeIcon(iconDpiDynamicalSize);
	bool recreate = (_state == TB_STANDARD || _state == TB_SMALL);
	setState(TB_LARGE);
	reset(recreate);	//recreate toolbar if previous state was Std icons or Small icons
	Window::redraw();
}

void ToolBar::setToUglyIcons()
{
	if (_state == TB_STANDARD) 
		return;
	bool recreate = true;
	setState(TB_STANDARD);
	reset(recreate);	//must recreate toolbar if setting to internal bitmaps
	Window::redraw();
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

void ToolBar::setCustomSizeAndEnssureCapasity(int size) {
	if(!_pTBB_CUSTOM||_pTBB_CUSTOM->size()<size) {
		if(_pTBB_CUSTOM)
			delete _pTBB_CUSTOM;
		_pTBB_CUSTOM=new std::vector<TBBUTTON>(size*1.2);
	}
	_pTBB_CUSTOM_LEN=size;
}

TBBUTTON* ToolBar::getRecordedDataFromCMDID(int cmdId) 
{
	auto pdx = _pTBB_CMDID_table.find(cmdId);
	if(pdx!=_pTBB_CMDID_table.end()) {
		auto idx = (*pdx).second;
		if(idx>=0&&idx<_pTBB_CUSTOM_LEN) {
			return &_pTBB[idx];
		}
	}
	return nullptr;
}

void ToolBar::reset(bool create) 
{
	if(dirty) {
		retrieveCustomBtns();
	}

	if (create && _hSelf)
	{
		::DestroyWindow(_hSelf);
		_hSelf = NULL;
	}

	if (!_hSelf)
	{
		auto style=WS_TOOLBARSTYLE|CCS_ADJUSTABLE;//|TBSTYLE_ALTDRAG;
		if(wrap) {
			style|=TBSTYLE_WRAPABLE;
		}
		if (NppDarkMode::isEnabled())
		{
			style |= TBSTYLE_CUSTOMERASE;
		}
		_hSelf = ::CreateWindowEx(
					WS_EX_PALETTEWINDOW,
					TOOLBARCLASSNAME,
					TEXT(""),
					style,
					0, 0,
					0, 0,
					_hParent,
					NULL,
					_hInst,
					0);
		// Send the TB_BUTTONSTRUCTSIZE message, which is required for 
		// backward compatibility.

		NppDarkMode::setDarkTooltips(_hSelf, NppDarkMode::ToolTipsType::toolbar);

		::SendMessage(_hSelf, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
		::SendMessage(_hSelf, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_HIDECLIPPEDBUTTONS);
	}

	if (!_hSelf)
	{
		throw std::runtime_error("ToolBar::reset : CreateWindowEx() nullptr");
	}

	if (_state != TB_STANDARD) 
	{
		//load dynamic icons (load only once)
		if(_nbDynButtons > 0 && _toolBarIcons.pureWithoutDynamics) {
#if 0
			static int cc=0;
			TCHAR buffer[256]={0};
			wsprintf(buffer,TEXT("dynamic=%d=%d=%d"), _toolBarIcons.length(), _nbTotalButtons, (LPARAM)cc++);
			::SendMessage(_hParent, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, (LPARAM)buffer);
#endif
			for (size_t j = 0; j < _nbDynButtons; ++j)
			{
				auto dynIconI = _vDynBtnReg.at(j);
				if(dynIconI.hIcon==0 && dynIconI.ORH->magicNum!=0x666) {
#ifndef PluginToolbar
					if(PLUGIN_ICO==0) PLUGIN_ICO = ::LoadIcon(_hInst, MAKEINTRESOURCE(IDR_PLUGIN_ICO));
#endif
					dynIconI.hIcon = PLUGIN_ICO;
				}
				_toolBarIcons.addIcon(dynIconI);
			}
			_toolBarIcons.pureWithoutDynamics=false;
		}

		//If non standard icons, use custom imagelists
		setDefaultImageList();
		setHotImageList();
		setDisableImageList();
	}
	else
	{
		//Else set the internal imagelist with standard bitmaps
		int iconDpiDynamicalSize = dpiManager->scaleX(16);
		::SendMessage(_hSelf, TB_SETBITMAPSIZE, 0, MAKELPARAM(iconDpiDynamicalSize, iconDpiDynamicalSize));

		//TBADDBITMAP addbmp = {_hInst, 0};
		TBADDBITMAP addbmp = {0, 0};
		TBADDBITMAP addbmpdyn = {0, 0};
		for (size_t i = 0 ; i < _nbButtons ; ++i)
		{
			int icoID = _toolBarIcons.getStdIconAt(static_cast<int32_t>(i));
			//todo load once. but standard icons small, so leave it.
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
				HBITMAP & hBmp = atat.hBmp;
				if(!hBmp && atat.ORH->magicNum==0x666) {
					// dynamic standard icons also load only once.
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
		if(_pTBB_CUSTOM && _pTBB_CUSTOM->size()) {
			applyCustomization();
		} else {
			::SendMessage(_hSelf, TB_ADDBUTTONS, nbBtnToAdd, reinterpret_cast<LPARAM>(_pTBB));
			//lazysync seps for the wrap mode.
		}
	}
	::SendMessage(_hSelf, TB_AUTOSIZE, 0, 0);

	reNewRBar();
}

void ToolBar::applyCustomization(bool reset){
	// apply customization.
	if(reset) {
		for(int i = ::SendMessage(_hSelf, TB_BUTTONCOUNT, 0, 0) - 1; i >= 0; i--)
		{
			SendMessage(_hSelf, TB_DELETEBUTTON, i, 0);
		}
	}
	_pTBB_SEP.clear();
	//MENUITEMINFO mii; mii.cbSize = sizeof(MENUITEMINFO);
	for(int i = 0; i < _pTBB_CUSTOM_LEN; i++) { //sync seps and state
		auto & customItem = _pTBB_CUSTOM->at(i);
		if(customItem.fsStyle&BTNS_SEP) {
			_pTBB_SEP.push_back(i);
		} else {
			//if(::GetMenuItemInfo(_mainMenuHandle, _pTBB_CUSTOM->at(i).idCommand, MF_BYCOMMAND, &mii)) {
			//	//MF_BYCOMMAND perfomance?
			//	_pTBB_CUSTOM->at(i).fsState = mii.fState;
			//}
			auto tbItem = getRecordedDataFromCMDID(customItem.idCommand);
			if(tbItem && customItem.iBitmap==tbItem->iBitmap) { //sanity check
				customItem = *tbItem;
			}
		}

	}
	SendMessage(_hSelf, TB_ADDBUTTONS, _pTBB_CUSTOM_LEN, (LPARAM)_pTBB_CUSTOM->data());
}

void ToolBar::syncToolbarRows() {
	int IdealRows = 1;
	if(wrap) {
		//IdealRows = SendMessage(_hSelf, TB_GETROWS , 0, 0);
		TBBUTTON tempBtn;
		bool bOnlyWrapOnSep=0;
		//for(int i=0, len=_pTBB_SEP.size();i<len;i++) {
		for(int i=0, len=bOnlyWrapOnSep?_pTBB_SEP.size():SendMessage(_hSelf, TB_BUTTONCOUNT , 0, 0);i<len;i++) {
			::SendMessage(_hSelf, TB_GETBUTTON, bOnlyWrapOnSep?_pTBB_SEP[i]:i, reinterpret_cast<LPARAM>(&tempBtn));
			if(tempBtn.fsState&TBSTATE_WRAP) {
				IdealRows++;
			}
		}
	}

	if(1||rows!=IdealRows) {
		rows=IdealRows;
		RECT rc;
		GetClientRect(_hSelf, &rc);

		DWORD padding = static_cast<DWORD>(SendMessage(_hSelf, TB_GETPADDING, 0, 0));

		rc.bottom=rc.top+getHeight()*IdealRows+HIWORD(padding)*(IdealRows-1);

		reNewRBar();

		reSizeTo(rc);

		reNewRBar();
	}
	SendMessage(_hSelf, TB_AUTOSIZE, 0, 0);
}

void ToolBar::retrieveCustomBtns() {
	if(_hSelf) {
		int size = ::SendMessage(_hSelf, TB_BUTTONCOUNT, 0, 0);
		setCustomSizeAndEnssureCapasity(size);

		for(int i=0;i<size;i++) {
			TBBUTTON & tempBtn = _pTBB_CUSTOM->at(i);
			::SendMessage(_hSelf, TB_GETBUTTON, i, reinterpret_cast<LPARAM>(&tempBtn));
		}
	}
	dirty=false;
}

void ToolBar::toggleToolbarWrap(){
	if(wrap ^= TRUE) {
		SendMessage(_hSelf, TB_SETSTYLE,0,
			SendMessage(_hSelf,TB_GETSTYLE,0,0)|TBSTYLE_WRAPABLE);	
	} else {
		SendMessage(_hSelf, TB_SETSTYLE,0,
			SendMessage(_hSelf,TB_GETSTYLE,0,0)&(~TBSTYLE_WRAPABLE));
	}
	//int size = ::SendMessage(getHSelf(), TB_BUTTONCOUNT, 0, 0);
	//SendMessage(getHSelf(), TB_INSERTBUTTON, size, (LPARAM)&_pTBB[7]);
	//SendMessage(getHSelf(), TB_DELETEBUTTON, size, 0);
	//::SendMessage(_pPublicInterface->getHSelf(), WM_SIZE, 0, 0);
	//syncToolbarRows();
	//

	if (_state != TB_STANDARD) {
		int size = _state==TB_LARGE?32:16;
		int iconDpiDynamicalSize = dpiManager->scaleX(size-1);
		_toolBarIcons.resizeIcon(iconDpiDynamicalSize);
		reset(0);
		iconDpiDynamicalSize = dpiManager->scaleX(size);
		_toolBarIcons.resizeIcon(iconDpiDynamicalSize);
		reset(0);
	} else {
		//一闪一闪亮晶晶
		reset(1);
	}
#if 0
	_toolBarIcons.resizeIcon(iconDpiDynamicalSize);
	reset(0);
	iconDpiDynamicalSize = nppParms->_dpiManager.scaleX(32);
	_toolBarIcons.resizeIcon(iconDpiDynamicalSize);
	reset(1);
	//#endif
	iconDpiDynamicalSize = nppParms->_dpiManager.scaleX(31);
	_toolBarIcons.resizeIcon(iconDpiDynamicalSize);

	::SendMessage(getHSelf(), TB_SETBITMAPSIZE, 0, MAKELPARAM(iconDpiDynamicalSize, iconDpiDynamicalSize));
	iconDpiDynamicalSize = nppParms->_dpiManager.scaleX(32);
	_toolBarIcons.resizeIcon(iconDpiDynamicalSize);

	::SendMessage(getHSelf(), TB_SETBITMAPSIZE, 0, MAKELPARAM(iconDpiDynamicalSize, iconDpiDynamicalSize));
	reset(0);
	::SendMessage(getHSelf(), TB_SETBUTTONSIZE , 0, MAKELONG(31, 31));
	::SendMessage(getHSelf(), TB_SETBUTTONSIZE , 0, MAKELONG(32, 32));

	::SendMessage(getHSelf(), TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
	::SendMessage(getHSelf(), TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_HIDECLIPPEDBUTTONS);
#endif
	syncToolbarRows();


	//int iconDpiDynamicalSize = nppParms->_dpiManager.scaleX(_state==TB_LARGE?32:16);
	//_toolBarIcons.resizeIcon(iconDpiDynamicalSize);
	//syncToolbarRows();

	//::SendMessage(_pPublicInterface->getHSelf(), WM_SIZE, 0, 0);

	//reset(1);
}

void ToolBar::reNewRBar()
{
	if (_pRebar)
	{
#if 1
		_rbBand.fMask   |= RBBIM_STYLE;
		if(wrap) {
			_rbBand.fStyle &= ~RBBS_USECHEVRON;
		} else {
			_rbBand.fStyle |= RBBS_USECHEVRON;
		}
#endif
		_rbBand.hwndChild	= getHSelf();
		_rbBand.cxMinChild	= 0;
		_rbBand.cyIntegral	= 1;
		_rbBand.cyMinChild	= _rbBand.cyMaxChild = getHeight()*rows;
		_rbBand.cxIdeal		= getWidth();

		_pRebar->reNew(REBAR_BAR_TOOLBAR, &_rbBand);
		//SendMessage(_rebarTop.getHSelf(), RB_SETBARINFO, 0, &_rbBand);
	}
}

void ToolBar::registerDynBtn(UINT messageID, toolbarIcons* tIcon)
{
	// Note: Register of buttons only possible before init!
	if ((_hSelf == NULL) && (messageID != 0) && (tIcon->hToolbarBmp != NULL || tIcon->magicNum==0x666))
	{
		tDynamicList dynList;
		dynList.message = messageID;
		dynList.hBmp = tIcon->hToolbarBmp;
		dynList.hIcon = tIcon->hToolbarIcon;
		//dynList.hIconHot = tIcon->hToolbarIconHot;
		//dynList.hIconGray = tIcon->hToolbarIconGray;
		dynList.ORH = tIcon;
		_vDynBtnReg.push_back(dynList);
	}
}

UINT ToolBar::doPopop(POINT chevPoint)
{
	//first find hidden buttons
	int width = Window::getWidth();

	size_t start = 0;
	RECT btnRect = {0,0,0,0};
	while (start < _nbCurrentButtons)
	{
		::SendMessage(_hSelf, TB_GETITEMRECT, start, reinterpret_cast<LPARAM>(&btnRect));
		if (btnRect.right > width)
			break;
		++start;
	}

	if (start < _nbCurrentButtons)
	{	//some buttons are hidden
		UINT elements = 0;
		HMENU menu = ::CreatePopupMenu();
#ifdef PluginToolbar
		TOOLTIPTEXT ttt = {0};
#else
		generic_string text;
#endif
		while (start < _nbCurrentButtons)
		{
			int cmd = _pTBB[start].idCommand;
#ifdef PluginToolbar
			/* get text over tooltip function */
			ttt.hdr.code	= TTN_GETDISPINFO;
			ttt.hdr.idFrom	= cmd;
#else
			getNameStrFromCmd(cmd, text);
#endif
			if (cmd != 0)
			{
#ifdef PluginToolbar
				::SendMessage(_hParent, WM_NOTIFY, cmd, (LPARAM)&ttt);
				if (::SendMessage(_hSelf, TB_ISBUTTONENABLED, cmd, 0) != 0) {
					AppendMenu(menu, MF_ENABLED, cmd, ttt.lpszText);
				} else {
					AppendMenu(menu, MF_DISABLED|MF_GRAYED, cmd, ttt.lpszText);
				}
#else
				if (::SendMessage(_hSelf, TB_ISBUTTONENABLED, cmd, 0) != 0)
					AppendMenu(menu, MF_ENABLED, cmd, text.c_str());
				else
					AppendMenu(menu, MF_DISABLED|MF_GRAYED, cmd, text.c_str());
#endif
			} else if (elements != 0) {
				AppendMenu(menu, MF_SEPARATOR, 0, TEXT(""));
			}

			++elements;
			++start;
		}
		TrackPopupMenu(menu, 0, chevPoint.x, chevPoint.y, 0, _hSelf, NULL);
	}
	return 0;
}

void ToolBar::addToRebar(ReBar * rebar) 
{
	if (_pRebar)
		return;
	_pRebar = rebar;
	ZeroMemory(&_rbBand, REBARBAND_SIZE);
	_rbBand.cbSize  = REBARBAND_SIZE;


	_rbBand.fMask   = RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE |
					  RBBIM_SIZE | RBBIM_IDEALSIZE | RBBIM_ID;

	_rbBand.fStyle		= RBBS_VARIABLEHEIGHT | RBBS_NOGRIPPER;
	if(!wrap) {
		_rbBand.fStyle|=RBBS_USECHEVRON;
	}
	_rbBand.hwndChild	= getHSelf();
	_rbBand.wID			= REBAR_BAR_TOOLBAR;	//ID REBAR_BAR_TOOLBAR for toolbar
	_rbBand.cxMinChild	= 0;
	_rbBand.cyIntegral	= 1;
	_rbBand.cyMinChild	= _rbBand.cyMaxChild	= getHeight();
	_rbBand.cxIdeal		= _rbBand.cx			= getWidth();

	_pRebar->addBand(&_rbBand, true);

	_rbBand.fMask   = RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_IDEALSIZE | RBBIM_SIZE;
}

constexpr UINT_PTR g_rebarSubclassID = 42;

LRESULT CALLBACK RebarSubclass(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	UINT_PTR uIdSubclass,
	DWORD_PTR dwRefData
)
{
	UNREFERENCED_PARAMETER(dwRefData);
	UNREFERENCED_PARAMETER(uIdSubclass);

	switch (uMsg)
	{
	case WM_ERASEBKGND:
		if (NppDarkMode::isEnabled())
		{
			RECT rc;
			GetClientRect(hWnd, &rc);
			FillRect((HDC)wParam, &rc, NppDarkMode::getDarkerBackgroundBrush());
			return TRUE;
		}
		else
		{
			break;
		}

	case WM_NCDESTROY:
		RemoveWindowSubclass(hWnd, RebarSubclass, g_rebarSubclassID);
		break;
	}
	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

void ReBar::init(HINSTANCE hInst, HWND hPere)
{
	Window::init(hInst, hPere);
	
	_hSelf = CreateWindowEx(WS_EX_TOOLWINDOW,
							REBARCLASSNAME,
							NULL,
							WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|RBS_VARHEIGHT|
							CCS_NODIVIDER | CCS_NOPARENTALIGN,
							0,0,0,0, _hParent, NULL, _hInst, NULL);

	SetWindowSubclass(_hSelf, RebarSubclass, g_rebarSubclassID, 0);

	REBARINFO rbi;
	ZeroMemory(&rbi, sizeof(REBARINFO));
	rbi.cbSize = sizeof(REBARINFO);
	rbi.fMask  = 0;
	rbi.himl   = (HIMAGELIST)NULL;
	::SendMessage(_hSelf, RB_SETBARINFO, 0, reinterpret_cast<LPARAM>(&rbi));
}

bool ReBar::addBand(REBARBANDINFO * rBand, bool useID) 
{
	if (rBand->fMask & RBBIM_STYLE)
	{
		if (!(rBand->fStyle & RBBS_NOGRIPPER))
			rBand->fStyle |= RBBS_GRIPPERALWAYS;
	}
	else
		rBand->fStyle = RBBS_GRIPPERALWAYS;

	rBand->fMask |= RBBIM_ID | RBBIM_STYLE;
	if (useID)
	{
		if (isIDTaken(rBand->wID))
			return false;
	}
	else
	{
		rBand->wID = getNewID();
	}
	::SendMessage(_hSelf, RB_INSERTBAND, static_cast<WPARAM>(-1), reinterpret_cast<LPARAM>(rBand));	//add to end of list
	return true;
}

void ReBar::reNew(int id, REBARBANDINFO * rBand) 
{
	auto index = SendMessage(_hSelf, RB_IDTOINDEX, id, 0);
	::SendMessage(_hSelf, RB_SETBANDINFO, index, reinterpret_cast<LPARAM>(rBand));
}

void ReBar::removeBand(int id) 
{
	auto index = SendMessage(_hSelf, RB_IDTOINDEX, id, 0);
	if (id >= REBAR_BAR_EXTERNAL)
		releaseID(id);
	::SendMessage(_hSelf, RB_DELETEBAND, index, 0);
}

void ReBar::setIDVisible(int id, bool show) 
{
	auto index = SendMessage(_hSelf, RB_IDTOINDEX, id, 0);
	if (index == -1 )
		return;	//error

	REBARBANDINFO rbBand;
	ZeroMemory(&rbBand, REBARBAND_SIZE);
	rbBand.cbSize  = REBARBAND_SIZE;


	rbBand.fMask = RBBIM_STYLE;
	::SendMessage(_hSelf, RB_GETBANDINFO, index, reinterpret_cast<LPARAM>(&rbBand));
	if (show)
		rbBand.fStyle &= (RBBS_HIDDEN ^ -1);
	else
		rbBand.fStyle |= RBBS_HIDDEN;
	::SendMessage(_hSelf, RB_SETBANDINFO, index, reinterpret_cast<LPARAM>(&rbBand));
}

bool ReBar::getIDVisible(int id)
{
	auto index = SendMessage(_hSelf, RB_IDTOINDEX, id, 0);
	if (index == -1 )
		return false;	//error
	REBARBANDINFO rbBand;
	ZeroMemory(&rbBand, REBARBAND_SIZE);
	rbBand.cbSize  = REBARBAND_SIZE;

	rbBand.fMask = RBBIM_STYLE;
	::SendMessage(_hSelf, RB_GETBANDINFO, index, reinterpret_cast<LPARAM>(&rbBand));
	return ((rbBand.fStyle & RBBS_HIDDEN) == 0);
}


void ReBar::setGrayBackground(int id) 
{
#ifndef PluginToolbar
	auto index = SendMessage(_hSelf, RB_IDTOINDEX, id, 0);
	if (index == -1 )
		return;	//error
	REBARBANDINFO rbBand;
	ZeroMemory(&rbBand, REBARBAND_SIZE);
	rbBand.cbSize  = REBARBAND_SIZE;
	rbBand.fMask = RBBIM_BACKGROUND;
	rbBand.hbmBack = LoadBitmap((HINSTANCE)::GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_INCREMENTAL_BG));
	::SendMessage(_hSelf, RB_SETBANDINFO, index, reinterpret_cast<LPARAM>(&rbBand));
#endif
}

int ReBar::getNewID()
{
	int idToUse = REBAR_BAR_EXTERNAL;
	size_t size = usedIDs.size();
	for (size_t i = 0; i < size; ++i)
	{
		int curVal = usedIDs.at(i);
		if (curVal < idToUse)
		{
			continue;
		}
		else if (curVal == idToUse)
		{
			++idToUse;
		}
		else
		{
			break;		//found gap
		}
	}

	usedIDs.push_back(idToUse);
	return idToUse;
}

void ReBar::releaseID(int id)
{
	size_t size = usedIDs.size();
	for (size_t i = 0; i < size; ++i)
	{
		if (usedIDs.at(i) == id)
		{
			usedIDs.erase(usedIDs.begin()+i);
			break;
		}
	}
}

bool ReBar::isIDTaken(int id)
{
	size_t size = usedIDs.size();
	for (size_t i = 0; i < size; ++i)
	{
		if (usedIDs.at(i) == id)
		{
			return true;
		}
	}
	return false;
}

