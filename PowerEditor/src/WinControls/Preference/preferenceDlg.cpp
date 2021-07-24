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

#include <shlwapi.h>
#include <shlobj.h>
#include <uxtheme.h>
#include "preferenceDlg.h"
#include "../../resource.h"
#include "lesDlgs.h"
#include "EncodingMapper.h"
#include "localization.h"
#include "wutils.h"
#include "Notepad_plus.h"
#include <windowsx.h>

struct DOREGSTRUCT {
	HKEY	hRootKey;
	LPCTSTR	szSubKey;
	LPCTSTR	lpszValueName;
	DWORD	type;
	LPCTSTR	szData;
};

TCHAR szGUID[] = TEXT("{B298D29A-A6ED-11DE-BA8C-A68E55D89593}");

#define MyGetGValue(rgb)      (LOBYTE((rgb)>>8))

using namespace std;

const int BLINKRATE_FASTEST = 50;
const int BLINKRATE_SLOWEST = 2500;
const int BLINKRATE_INTERVAL = 50;

const int BORDERWIDTH_SMALLEST = 0;
const int BORDERWIDTH_LARGEST = 30;
const int BORDERWIDTH_INTERVAL = 1;

const int PADDING_SMALLEST = 0;
const int PADDING_LARGEST = 30;
const int PADDING_INTERVAL = 1;

const int DISTRACTIONFREE_SMALLEST = 3;
const int DISTRACTIONFREE_LARGEST = 9;
const int DISTRACTIONFREE_INTERVAL = 1;

HFONT hFontSubPanel;

HFONT hFontCategory;

RECT rSubPanel;

int currentSettingsIndex;

bool fontReUsable=1;

extern bool ClosePanelRequested;

PreferenceDlg* _preferenceDlg;

extern void addText2Combo(const TCHAR * txt2add, HWND hCombo);

extern void delLeftWordInEdit(HWND hEdit, bool tabSep);

extern generic_string getTextFromCombo(HWND hCombo);

extern NppParameters* nppParms;

extern NppGUI* nppUIParms;

extern Notepad_plus* nppApp;

bool CreateFonts(bool init) {
	if(hFontSubPanel==0) {
		bool bigFonts = nppUIParms->_useBigFonts;
		if(init && !bigFonts) 
		{
			return false;
		}
		hFontSubPanel = CreateFontIndirectly(bigFonts?-19:-16, fontReUsable);
		hFontCategory = CreateFontIndirectly(bigFonts?-21:-15, fontReUsable);
		return true;
	}
}

// This int encoding array is built from "EncodingUnit encodings[]" (see EncodingMapper.cpp)
// And DefaultNewDocDlg will use "int encoding array" to get more info from "EncodingUnit encodings[]"
static int encodings[] = {
	1250, 
	1251, 
	1252, 
	1253, 
	1254, 
	1255, 
	1256, 
	1257, 
	1258, 
	28591,
	28592,
	28593,
	28594,
	28595,
	28596,
	28597,
	28598,
	28599,
	28603,
	28604,
	28605,
	437,  
	720,  
	737,  
	775,  
	850,  
	852,  
	855,  
	857,  
	858,  
	860,  
	861,  
	862,  
	863,  
	865,  
	866,  
	869,  
	950,  
	936,  
	932,  
	949,  
	51949,
	874,
	10007,
	21866,
	20866
};

INT_PTR CALLBACK PreferenceDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
		case WM_INITDIALOG :
		{
			_preferenceDlg = this;
			_wVector.push_back(DlgInfo(&_barsDlg, TEXT("常用"), TEXT("Global"), IDD_PREFERENCE_BAR_BOX));
			_wVector.push_back(DlgInfo(&_marginsDlg, TEXT("编辑"), TEXT("Scintillas"), IDD_PREFERENCE_MARGEIN_BOX));
			_wVector.push_back(DlgInfo(&_darkModeSubDlg, TEXT("黑暗模式"), TEXT("Scintillas"), IDD_PREFERENCE_SUB_DARKMODE));
			_wVector.push_back(DlgInfo(&_defaultNewDocDlg, TEXT("新建"), TEXT("NewDoc"), IDD_PREFERENCE_NEWDOCSETTING_BOX));
			_wVector.push_back(DlgInfo(&_defaultDirectoryDlg, TEXT("目录"), TEXT("DefaultDir"), IDD_PREFERENCE_DEFAULTDIRECTORY_BOX));
			_wVector.push_back(DlgInfo(&_recentFilesHistoryDlg, TEXT("最近文件记录"), TEXT("RecentFilesHistory"), IDD_PREFERENCE_RECENTFILESHISTORY_BOX));
			_wVector.push_back(DlgInfo(&_fileAssocDlg, TEXT("文件关联"), TEXT("FileAssoc"), IDD_REGEXT_BOX));
			_wVector.push_back(DlgInfo(&_langMenuDlg, TEXT("代码语言"), TEXT("Language"), IDD_PREFERENCE_LANG_BOX));
			_wVector.push_back(DlgInfo(&_highlighting, TEXT("高亮"), TEXT("Highlighting"), IDD_PREFERENCE_HILITE_BOX));
			_wVector.push_back(DlgInfo(&_printSettingsDlg, TEXT("打印"), TEXT("Print"), IDD_PREFERENCE_PRINT_BOX));
			_wVector.push_back(DlgInfo(&_searchingSettingsDlg, TEXT("搜索"), TEXT("Searching"), IDD_PREFERENCE_SEARCHINGSETTINGS_BOX));
			_wVector.push_back(DlgInfo(&_backupDlg, TEXT("备份"), TEXT("Backup"), IDD_PREFERENCE_BACKUP_BOX));
			_wVector.push_back(DlgInfo(&_autoCompletionDlg, TEXT("自动完成"), TEXT("AutoCompletion"), IDD_PREFERENCE_AUTOCOMPLETION_BOX));
			_wVector.push_back(DlgInfo(&_multiInstDlg, TEXT("多实例"), TEXT("MultiInstance"), IDD_PREFERENCE_MULTIINSTANCE_BOX));
			_wVector.push_back(DlgInfo(&_delimiterSettingsDlg, TEXT("定界符"), TEXT("Delimiter"), IDD_PREFERENCE_DELIMITERSETTINGS_BOX));
			_wVector.push_back(DlgInfo(&_settingsOnCloudDlg, TEXT("云同步"), TEXT("Cloud"), IDD_PREFERENCE_SETTINGSONCLOUD_BOX));
			_wVector.push_back(DlgInfo(&_searchEngineDlg, TEXT("搜索引擎"), TEXT("SearchEngine"), IDD_PREFERENCE_SEARCHENGINE_BOX));
			_wVector.push_back(DlgInfo(&_miscSettingsDlg, TEXT("其他"), TEXT("MISC"), IDD_PREFERENCE_SETTING_BOX));

			//if(CreateFonts(true)) {
			//	setWindowFont(_hSelf, hFontCategory);
			//}

			getClientRect(rSubPanel);
			rSubPanel.top += nppParms->_dpiManager.scaleY(10);
			rSubPanel.bottom -= nppParms->_dpiManager.scaleY(43);
			rSubPanel.left += nppParms->_dpiManager.scaleX(180);
			rSubPanel.right -= nppParms->_dpiManager.scaleX(30);

			showDialogByIndex(nppUIParms->currentSettingsIndex, true);

			makeCategoryList();

			if(currentSettingsIndex) {
				setListSelection(currentSettingsIndex);
			}

			initAllPanels();
#if 1//lazyInit
#else
			initAllPanels();
			_wVector[0]._dlg->display();
#endif

			ETDTProc enableDlgTheme = (ETDTProc)nppParms->getEnableThemeDlgTexture();
			if (enableDlgTheme)
				enableDlgTheme(_hSelf, ETDT_ENABLETAB);

			return TRUE;
		}

		case WM_COMMAND :
		{
			if (LOWORD(wParam) == IDC_LIST_DLGTITLE)
			{
				if (HIWORD(wParam) == CBN_SELCHANGE)
				{
					auto i = ::SendDlgItemMessage(_hSelf, IDC_LIST_DLGTITLE, LB_GETCURSEL, 0, 0);
					if (i != LB_ERR)
					{
						showDialogByIndex(i);
					}
				}
			}
			else 
			{
				switch (wParam)
				{
					case IDC_BUTTON_CLOSE :
					case IDCANCEL :
						display(false);
						return TRUE;

					default :
						::SendMessage(_hParent, WM_COMMAND, wParam, lParam);
						return TRUE;
				}
			}
		}
	}
	return FALSE;
}

void PreferenceDlg::makeCategoryList()
{
	for (size_t i = 0, len = _wVector.size(); i < len; ++i)
	{
		::SendDlgItemMessage(_hSelf, IDC_LIST_DLGTITLE, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(_wVector[i]._name.c_str()));
	}
	setListSelection(0);
}

int32_t PreferenceDlg::getIndexFromName(const TCHAR *name) const
{
	if (not name)
		return -1;

	int32_t i = 0;
	for (auto it = _wVector.begin() ; it != _wVector.end(); ++it, ++i)
	{
		if (it->_internalName == name)
			return i;
	}
	return -1;
}

bool PreferenceDlg::setListSelection(size_t currentSel) const
{
	// Stupid LB API doesn't allow LB_SETSEL to be used on single select listbox, so we do it in a hard way
	const size_t selStrLenMax = 255;
	TCHAR selStr[selStrLenMax + 1];
	auto lbTextLen = ::SendMessage(_hSelf, LB_GETTEXTLEN, currentSel, 0);

	if (lbTextLen > selStrLenMax)
		return false;

	::SendDlgItemMessage(_hSelf, IDC_LIST_DLGTITLE, LB_GETTEXT, currentSel, reinterpret_cast<LPARAM>(selStr));
	::SendDlgItemMessage(_hSelf, IDC_LIST_DLGTITLE, LB_SELECTSTRING, currentSel, reinterpret_cast<LPARAM>(selStr));
	return true;
}

bool PreferenceDlg::renameDialogTitle(const TCHAR *internalName, const TCHAR *newName)
{
	bool foundIt = false;
	size_t i = 0;
	for (size_t len = _wVector.size(); i < len; ++i)
	{
		if (_wVector[i]._internalName == internalName)
		{
			foundIt = true;
			break;
		}
	}
	if (!foundIt)
		return false;

	const size_t lenMax = 256;
	TCHAR oldName[lenMax] = {0};
	size_t txtLen = ::SendDlgItemMessage(_hSelf, IDC_LIST_DLGTITLE, LB_GETTEXTLEN, i, 0);
	if (txtLen >= lenMax)
		return false;

	::SendDlgItemMessage(_hSelf, IDC_LIST_DLGTITLE, LB_GETTEXT, i, reinterpret_cast<LPARAM>(oldName));

	// Same name, no need to change, but operation is considered success
	if (lstrcmp(newName, oldName) == 0)
		return true;

	::SendDlgItemMessage(_hSelf, IDC_LIST_DLGTITLE, LB_DELETESTRING, i, 0);
	::SendDlgItemMessage(_hSelf, IDC_LIST_DLGTITLE, LB_INSERTSTRING, i, reinterpret_cast<LPARAM>(newName));

	return true;
}

void PreferenceDlg::initAllPanels() const
{
	StaticDialog* dlg;
	for (auto vI : _wVector) {
		dlg = (StaticDialog*) vI._dlg;
		int* flag = &dlg->flag;
		if(!(*flag&0x1))
		{ 
			dlg->init(_hInst, _hSelf);
			dlg->create(vI._res, false, false);
			dlg->reSizeTo(rSubPanel);
			*flag=0x1;
		} 
	}
}

void PreferenceDlg::refresh() const
{
	if(hFontCategory!=0) {
		if(!fontReUsable) {
			DeleteObject(hFontCategory);
			DeleteObject(hFontSubPanel);
		}
		hFontCategory=hFontSubPanel=0;
	}

	CreateFonts(false);

	setWindowFont(_hSelf, hFontCategory);

	auto len=_wVector.size();
	for (size_t i = 0; i < len ; ++i)
	{
		if(i!=currentSettingsIndex) {
			//int* flag=&((StaticDialog*)_wVector[i]._dlg)->flag;
			//*flag = (*flag&~0x2)|0x4;
			((StaticDialog*)_wVector[i]._dlg)->flag&=~0x2;
		}
	}

	_wVector[currentSettingsIndex]._dlg->redraw();
}

void PreferenceDlg::showDialogByName(const TCHAR *name) const
{
	int32_t i = getIndexFromName(name);
	if (i >= 0)
	{
		showDialogByIndex(i);
		setListSelection(i);
	}
}


void PreferenceDlg::showDialogByIndex(size_t index, bool init) const
{
	currentSettingsIndex = index;
	size_t len = _wVector.size();
	DlgInfo dlgWrap = _wVector[index];
	StaticDialog* dlg = (StaticDialog*) dlgWrap._dlg;
	int* flag = &dlg->flag;
	if(!(*flag&0x1))
	{ 
		//::MessageBox(NULL, TEXT("CREATE"), TEXT(""), MB_OK);
		dlg->init(_hInst, _hSelf);
		dlg->create(dlgWrap._res, false, false);
		dlg->reSizeTo(rSubPanel);
		if(rSubPanel.right>0) *flag=0x1;
	} 
	if(!(*flag&0x2) && hFontSubPanel!=0) {
		//::MessageBox(NULL, TEXT("SETFONT"), TEXT(""), MB_OK);
		setWindowFont(dlg->getHSelf(), hFontSubPanel);
		*flag|=0x2;
	}
	if(init) {
		_wVector[index]._dlg->display();
	} else {
		::SendMessage(_hParent, NPPM_SETTTINGSTO, 0, index);
		for (size_t i = 0; i < len; ++i)
		{
			_wVector[i]._dlg->display(i==index);
		}
	}
	//if(*flag&0x4) {
	//	::MessageBox(NULL, TEXT("REDRAW"), TEXT(""), MB_OK);
	//	_wVector[index]._dlg->redraw();
	//	*flag&=~0x4;
	//}
}

void PreferenceDlg::refreshCurrentDlg(int index) const
{
	if(isCreated()&&_wVector[index]._dlg)
	if(index==0) {
		bool showDocSwitcher = ::SendMessage(_hParent, NPPM_ISDOCSWITCHERSHOWN, 0, 0) == TRUE;
		::SendDlgItemMessage(_wVector[index]._dlg->getHSelf(), IDC_CHECK_DOCSWITCH, BM_SETCHECK, showDocSwitcher, 0);
	}
}

void PreferenceDlg::destroy()
{
	_barsDlg.destroy();
	_marginsDlg.destroy();
	_miscSettingsDlg.destroy();
	_fileAssocDlg.destroy();
	_langMenuDlg.destroy();
	_highlighting.destroy();
	_printSettingsDlg.destroy();
	_searchingSettingsDlg.destroy();
	_defaultNewDocDlg.destroy();
	_defaultDirectoryDlg.destroy();
	_recentFilesHistoryDlg.destroy();
	_backupDlg.destroy();
	_autoCompletionDlg.destroy();
	_multiInstDlg.destroy();
	_delimiterSettingsDlg.destroy();
	_darkModeSubDlg.destroy();
}

void PreferenceDlg::invalidateRadioBtns(bool forPrefDLg)
{
	if(_barsDlg.isCreated()) {
		toolBarStatusType tbStatus = nppApp->_toolBar.getState();
		::SendDlgItemMessage(_barsDlg.getHSelf(), tbStatus+IDC_RADIO_SMALLICON, BM_SETCHECK, BST_CHECKED, 0);
		if(forPrefDLg) {
			::SendDlgItemMessage(_barsDlg.getHSelf(), (tbStatus+1)%3+IDC_RADIO_SMALLICON, BM_SETCHECK, BST_UNCHECKED, 0);
			::SendDlgItemMessage(_barsDlg.getHSelf(), (tbStatus+2)%3+IDC_RADIO_SMALLICON, BM_SETCHECK, BST_UNCHECKED, 0);
		}
		bool showTool = nppApp->_rebarTop.getIDVisible(REBAR_BAR_TOOLBAR);
		::SendDlgItemMessage(_barsDlg.getHSelf(), IDC_CHECK_HIDE, BM_SETCHECK, showTool?BST_UNCHECKED:BST_CHECKED, 0);
		::SendDlgItemMessage(_barsDlg.getHSelf(), IDC_CHECK_TAB_MULTILINE, BM_SETCHECK, TabBarPlus::isMultiLine(), 0); //tabBarStatus & TAB_MULTILINE
		::SendDlgItemMessage(_barsDlg.getHSelf(), IDC_CHECK_TAB_VERTICAL, BM_SETCHECK, TabBarPlus::isVertical(), 0); 
	}
}

INT_PTR CALLBACK BarsDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM)
{
		
	switch (message) 
	{
		case WM_INITDIALOG :
		{
			const NppGUI & nppGUI = nppParms->getNppGUI();
			int tabBarStatus = nppGUI._tabStatus;
			bool showStatus = nppGUI._statusBarShow;
			bool showMenu = nppGUI._menuBarShow;

			_preferenceDlg->invalidateRadioBtns();

			::SendDlgItemMessage(_hSelf, IDC_CHECK_REDUCE, BM_SETCHECK, tabBarStatus & TAB_REDUCE, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_LOCK, BM_SETCHECK, !(tabBarStatus & TAB_DRAGNDROP), 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_ORANGE, BM_SETCHECK, tabBarStatus & TAB_DRAWTOPBAR, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_DRAWINACTIVE, BM_SETCHECK, tabBarStatus & TAB_DRAWINACTIVETAB, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_ENABLETABCLOSE, BM_SETCHECK, tabBarStatus & TAB_CLOSEBUTTON, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_DBCLICK2CLOSE, BM_SETCHECK, tabBarStatus & TAB_DBCLK2CLOSE, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_TAB_LAST_EXIT, BM_SETCHECK, tabBarStatus & TAB_QUITONEMPTY, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_TAB_ALTICONS, BM_SETCHECK, tabBarStatus & TAB_ALTICONS, 0);
			
			::SendDlgItemMessage(_hSelf, IDC_CHECK_TAB_HIDE, BM_SETCHECK, tabBarStatus & TAB_HIDE, 0);
			::SendMessage(_hSelf, WM_COMMAND, IDC_CHECK_TAB_HIDE, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_SHOWBIGGERFONTS, BM_SETCHECK, nppGUI._useBigFonts, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_SHOWSTATUSBAR, BM_SETCHECK, showStatus, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_HIDEMENUBAR, BM_SETCHECK, !showMenu, 0);

			::SendDlgItemMessage(_hSelf, IDC_CHECK_DOCSWITCH_EXTCOLUMN, BM_SETCHECK, nppGUI._fileSwitcherWithExtColumn, 0);

			LocalizationSwitcher & localizationSwitcher = nppParms->getLocalizationSwitcher();

			for (size_t i = 0, len = localizationSwitcher.size(); i < len ; ++i)
			{
				pair<wstring, wstring> localizationInfo = localizationSwitcher.getElementFromIndex(i);
				::SendDlgItemMessage(_hSelf, IDC_COMBO_LOCALIZATION, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(localizationInfo.first.c_str()));
			}
			wstring lang = TEXT("English"); // Set default language as Englishs
			if (nppParms->getNativeLangA()) // if nativeLangA is not NULL, then we can be sure the default language (English) is not used
			{
				string fn = localizationSwitcher.getFileName();
				wstring fnW = s2ws(fn);
				lang = localizationSwitcher.getLangFromXmlFileName(fnW.c_str());
			}
			auto index = ::SendDlgItemMessage(_hSelf, IDC_COMBO_LOCALIZATION, CB_FINDSTRINGEXACT, static_cast<WPARAM>(-1), reinterpret_cast<LPARAM>(lang.c_str()));
			if (index != CB_ERR)
                ::SendDlgItemMessage(_hSelf, IDC_COMBO_LOCALIZATION, CB_SETCURSEL, index, 0);

			ETDTProc enableDlgTheme = reinterpret_cast<ETDTProc>(nppParms->getEnableThemeDlgTexture());
			if (enableDlgTheme)
				enableDlgTheme(_hSelf, ETDT_ENABLETAB);

			_preferenceDlg->refreshCurrentDlg(0);

			return TRUE;
		}
		
		case WM_COMMAND : 
		{
			switch (wParam)
			{
				case IDC_CHECK_SHOWSTATUSBAR :
				{
					bool isChecked = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECK_SHOWSTATUSBAR, BM_GETCHECK, 0, 0));
					::SendMessage(::GetParent(_hParent), NPPM_HIDESTATUSBAR, 0, isChecked?FALSE:TRUE);
				}
				return TRUE;
				case IDC_CHECK_SHOWBIGGERFONTS : 
				{
					bool isChecked = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECK_SHOWBIGGERFONTS, BM_GETCHECK, 0, 0));
					::SendMessage(::GetParent(_hParent), NPPM_SHOWBIGGERFONTS, 0, isChecked?FALSE:TRUE);
				} return TRUE;
				case IDC_CHECK_HIDEMENUBAR :
				{
					bool isChecked = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECK_HIDEMENUBAR, BM_GETCHECK, 0, 0));
					::SendMessage(::GetParent(_hParent), NPPM_HIDEMENU, 0, isChecked?TRUE:FALSE);
				}
				return TRUE;

				case IDC_CHECK_DOCSWITCH :
				{
					bool isChecked = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECK_DOCSWITCH, BM_GETCHECK, 0, 0));
					ClosePanelRequested=!isChecked;
					::SendMessage(::GetParent(_hParent), WM_COMMAND, IDM_VIEW_TABLIST_PANEL, 0);
					ClosePanelRequested=0;
					getFocus();
				}
				return TRUE;
				case IDC_CHECK_DOCSWITCH_EXTCOLUMN :
				{
					bool isChecked = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECK_DOCSWITCH_EXTCOLUMN, BM_GETCHECK, 0, 0));
					::SendMessage(::GetParent(_hParent), NPPM_DOCSWITCHERENABLECOLUMN, 0, isChecked?TRUE:FALSE);
				}
				return TRUE;

				case IDC_CHECK_TAB_HIDE :
				{
					bool toBeHidden = (BST_CHECKED == ::SendMessage(::GetDlgItem(_hSelf, IDC_CHECK_TAB_HIDE), BM_GETCHECK, 0, 0));
					::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_TAB_MULTILINE), !toBeHidden);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_TAB_VERTICAL), !toBeHidden);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_REDUCE), !toBeHidden);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_LOCK), !toBeHidden);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_ORANGE), !toBeHidden);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_DRAWINACTIVE), !toBeHidden);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_ENABLETABCLOSE), !toBeHidden);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_DBCLICK2CLOSE), !toBeHidden);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_TAB_LAST_EXIT), !toBeHidden);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_TAB_ALTICONS), !toBeHidden);

					::SendMessage(::GetParent(_hParent), NPPM_HIDETABBAR, 0, toBeHidden);
					return TRUE;
				}
				
				case  IDC_CHECK_TAB_VERTICAL:
					::SendMessage(_hParent, WM_COMMAND, IDM_VIEW_DRAWTABBAR_VERTICAL, 0);
					return TRUE;

				case IDC_CHECK_TAB_MULTILINE :
					::SendMessage(_hParent, WM_COMMAND, IDM_VIEW_DRAWTABBAR_MULTILINE, 0);
					return TRUE;

				case IDC_CHECK_TAB_LAST_EXIT:
				{
					NppGUI & nppGUI = const_cast<NppGUI &>(nppParms->getNppGUI());
					nppGUI._tabStatus ^= TAB_QUITONEMPTY;
				}
				return TRUE;

				case IDC_CHECK_TAB_ALTICONS:
				{
					NppGUI& nppGUI = const_cast<NppGUI&>(nppParms->getNppGUI());
					nppGUI._tabStatus ^= TAB_ALTICONS;
					bool isChecked = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECK_TAB_ALTICONS, BM_GETCHECK, 0, 0));
					::SendMessage(::GetParent(_hParent), NPPM_INTERNAL_CHANGETABBAEICONS, 0, isChecked ? 1 : 0);
					return TRUE;
				}

				case IDC_CHECK_REDUCE :
					::SendMessage(_hParent, WM_COMMAND, IDM_VIEW_REDUCETABBAR, 0);
					return TRUE;
					
				case IDC_CHECK_LOCK :
					::SendMessage(_hParent, WM_COMMAND, IDM_VIEW_LOCKTABBAR, 0);
					return TRUE;
					
				case IDC_CHECK_ORANGE :
					::SendMessage(_hParent, WM_COMMAND, IDM_VIEW_DRAWTABBAR_TOPBAR, 0);
					return TRUE;
					
				case IDC_CHECK_DRAWINACTIVE :
					::SendMessage(_hParent, WM_COMMAND, IDM_VIEW_DRAWTABBAR_INACIVETAB, 0);
					return TRUE;
					
				case IDC_CHECK_ENABLETABCLOSE :
					::SendMessage(_hParent, WM_COMMAND, IDM_VIEW_DRAWTABBAR_CLOSEBOTTUN, 0);
					return TRUE;

				case IDC_CHECK_DBCLICK2CLOSE :
					::SendMessage(_hParent, WM_COMMAND, IDM_VIEW_DRAWTABBAR_DBCLK2CLOSE, 0);
					return TRUE;

				case IDC_CHECK_HIDE :
				{
					bool isChecked = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECK_HIDE, BM_GETCHECK, 0, 0));
					::SendMessage(::GetParent(_hParent), NPPM_HIDETOOLBAR, 0, isChecked?TRUE:FALSE);
				}
				return TRUE;
					
				case IDC_RADIO_SMALLICON :
				case IDC_RADIO_BIGICON :
				case IDC_RADIO_STANDARD :
					nppApp->switchToIconMode((toolBarStatusType)(wParam-IDC_RADIO_SMALLICON), true);
					return TRUE;

				default :
					switch (HIWORD(wParam))
					{
						case CBN_SELCHANGE : // == case LBN_SELCHANGE :
						{
							switch (LOWORD(wParam))
							{
								case IDC_COMBO_LOCALIZATION :
								{
									LocalizationSwitcher & localizationSwitcher = nppParms->getLocalizationSwitcher();
									auto index = ::SendDlgItemMessage(_hSelf, IDC_COMBO_LOCALIZATION, CB_GETCURSEL, 0, 0);
									TCHAR langName[MAX_PATH];
									auto cbTextLen = ::SendDlgItemMessage(_hSelf, IDC_COMBO_LOCALIZATION, CB_GETLBTEXTLEN, index, 0);
									if (cbTextLen > MAX_PATH - 1)
										return TRUE;

									::SendDlgItemMessage(_hSelf, IDC_COMBO_LOCALIZATION, CB_GETLBTEXT, index, reinterpret_cast<LPARAM>(langName));
									if (langName[0])
									{
										// Make English as basic language
										if (localizationSwitcher.switchToLang(TEXT("English")))
										{
											::SendMessage(::GetParent(_hParent), NPPM_INTERNAL_RELOADNATIVELANG, 0, 0);
										}
										// Change the language 
										if (localizationSwitcher.switchToLang(langName))
										{
											::SendMessage(::GetParent(_hParent), NPPM_INTERNAL_RELOADNATIVELANG, 0, 0);
											::InvalidateRect(_hParent, NULL, TRUE);
										}
									}
								}
								return TRUE;
								default:
									break;
							}
						}
					}
			}
		}
	}
	return FALSE;
}

static WNDPROC oldFunclstToolbarProc = NULL;
static LRESULT CALLBACK editNumSpaceProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CHAR:
		{
			// All non decimal numbers and non white space and non backspace are ignored
			if ((wParam != 8 && wParam != 32 && wParam < 48) || wParam > 57)
			{
				return TRUE;
			}
		}
	}
	return oldFunclstToolbarProc(hwnd, message, wParam, lParam);
}


void DarkModeSubDlg::enableCustomizedColorCtrls(bool doEnable)
{
	::EnableWindow(_pBackgroundColorPicker->getHSelf(), doEnable);
	::EnableWindow(_pSofterBackgroundColorPicker->getHSelf(), doEnable);
	::EnableWindow(_pHotBackgroundColorPicker->getHSelf(), doEnable);
	::EnableWindow(_pPureBackgroundColorPicker->getHSelf(), doEnable);
	::EnableWindow(_pErrorBackgroundColorPicker->getHSelf(), doEnable);
	::EnableWindow(_pTextColorPicker->getHSelf(), doEnable);
	::EnableWindow(_pDarkerTextColorPicker->getHSelf(), doEnable);
	::EnableWindow(_pDisabledTextColorPicker->getHSelf(), doEnable);
	::EnableWindow(_pEdgeColorPicker->getHSelf(), doEnable);
	::EnableWindow(_pLinkColorPicker->getHSelf(), doEnable);

	::EnableWindow(::GetDlgItem(_hSelf, IDD_CUSTOMIZED_COLOR1_STATIC), doEnable);
	::EnableWindow(::GetDlgItem(_hSelf, IDD_CUSTOMIZED_COLOR2_STATIC), doEnable);
	::EnableWindow(::GetDlgItem(_hSelf, IDD_CUSTOMIZED_COLOR3_STATIC), doEnable);
	::EnableWindow(::GetDlgItem(_hSelf, IDD_CUSTOMIZED_COLOR4_STATIC), doEnable);
	::EnableWindow(::GetDlgItem(_hSelf, IDD_CUSTOMIZED_COLOR5_STATIC), doEnable);
	::EnableWindow(::GetDlgItem(_hSelf, IDD_CUSTOMIZED_COLOR6_STATIC), doEnable);
	::EnableWindow(::GetDlgItem(_hSelf, IDD_CUSTOMIZED_COLOR7_STATIC), doEnable);
	::EnableWindow(::GetDlgItem(_hSelf, IDD_CUSTOMIZED_COLOR8_STATIC), doEnable);
	::EnableWindow(::GetDlgItem(_hSelf, IDD_CUSTOMIZED_COLOR9_STATIC), doEnable);
	::EnableWindow(::GetDlgItem(_hSelf, IDD_CUSTOMIZED_COLOR10_STATIC), doEnable);

	::EnableWindow(::GetDlgItem(_hSelf, IDD_CUSTOMIZED_RESET_BUTTON), doEnable);

	if (doEnable)
	{
		_pBackgroundColorPicker->setColour(NppDarkMode::getBackgroundColor());
		_pSofterBackgroundColorPicker->setColour(NppDarkMode::getSofterBackgroundColor());
		_pHotBackgroundColorPicker->setColour(NppDarkMode::getHotBackgroundColor());
		_pPureBackgroundColorPicker->setColour(NppDarkMode::getDarkerBackgroundColor());
		_pErrorBackgroundColorPicker->setColour(NppDarkMode::getErrorBackgroundColor());
		_pTextColorPicker->setColour(NppDarkMode::getTextColor());
		_pDarkerTextColorPicker->setColour(NppDarkMode::getDarkerTextColor());
		_pDisabledTextColorPicker->setColour(NppDarkMode::getDisabledTextColor());
		_pEdgeColorPicker->setColour(NppDarkMode::getEdgeColor());
		_pLinkColorPicker->setColour(NppDarkMode::getLinkTextColor());

		redraw();
	}
}

INT_PTR CALLBACK DarkModeSubDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	NppParameters& nppParam = NppParameters::getInstance();
	NppGUI & nppGUI = const_cast<NppGUI &>(nppParms->getNppGUI());
	switch (message)
	{
		case WM_INITDIALOG:
		{
			::SendDlgItemMessage(_hSelf, IDC_CHECK_DARKMODE_ENABLE, BM_SETCHECK, nppGUI._darkmode._isEnabled, 0);

			int id = IDC_RADIO_DARKMODE_BLACK;
			switch (nppGUI._darkmode._colorTone)
			{
				case NppDarkMode::redTone:
					id = IDC_RADIO_DARKMODE_RED;
					break;
				case NppDarkMode::greenTone:
					id = IDC_RADIO_DARKMODE_GREEN;
					break;
				case NppDarkMode::blueTone:
					id = IDC_RADIO_DARKMODE_BLUE;
					break;
				case NppDarkMode::purpleTone:
					id = IDC_RADIO_DARKMODE_PURPLE;
					break;
				case NppDarkMode::cyanTone:
					id = IDC_RADIO_DARKMODE_CYAN;
					break;
				case NppDarkMode::oliveTone:
					id = IDC_RADIO_DARKMODE_OLIVE;
					break;
				case NppDarkMode::customizedTone:
					id = IDC_RADIO_DARKMODE_CUSTOMIZED;
					break;
			}
			::SendDlgItemMessage(_hSelf, id, BM_SETCHECK, TRUE, 0);

			_pBackgroundColorPicker = new ColourPicker;
			_pSofterBackgroundColorPicker = new ColourPicker;
			_pHotBackgroundColorPicker = new ColourPicker;
			_pPureBackgroundColorPicker = new ColourPicker;
			_pErrorBackgroundColorPicker = new ColourPicker;
			_pTextColorPicker = new ColourPicker;
			_pDarkerTextColorPicker = new ColourPicker;
			_pDisabledTextColorPicker = new ColourPicker;
			_pEdgeColorPicker = new ColourPicker;
			_pLinkColorPicker = new ColourPicker;

			_pBackgroundColorPicker->init(_hInst, _hSelf);
			_pSofterBackgroundColorPicker->init(_hInst, _hSelf);
			_pHotBackgroundColorPicker->init(_hInst, _hSelf);
			_pPureBackgroundColorPicker->init(_hInst, _hSelf);

			_pErrorBackgroundColorPicker->init(_hInst, _hSelf);
			_pTextColorPicker->init(_hInst, _hSelf);
			_pDarkerTextColorPicker->init(_hInst, _hSelf);
			_pDisabledTextColorPicker->init(_hInst, _hSelf);
			_pEdgeColorPicker->init(_hInst, _hSelf);
			_pLinkColorPicker->init(_hInst, _hSelf);

			POINT p1, p2, p3, p4, p5, p6, p7, p8, p9, p10;
			alignWith(::GetDlgItem(_hSelf, IDD_CUSTOMIZED_COLOR1_STATIC), _pPureBackgroundColorPicker->getHSelf(), PosAlign::left, p1);
			alignWith(::GetDlgItem(_hSelf, IDD_CUSTOMIZED_COLOR2_STATIC), _pHotBackgroundColorPicker->getHSelf(), PosAlign::left, p2);
			alignWith(::GetDlgItem(_hSelf, IDD_CUSTOMIZED_COLOR3_STATIC), _pSofterBackgroundColorPicker->getHSelf(), PosAlign::left, p3);
			alignWith(::GetDlgItem(_hSelf, IDD_CUSTOMIZED_COLOR4_STATIC), _pBackgroundColorPicker->getHSelf(), PosAlign::left, p4);
			alignWith(::GetDlgItem(_hSelf, IDD_CUSTOMIZED_COLOR5_STATIC), _pErrorBackgroundColorPicker->getHSelf(), PosAlign::left, p5);
			alignWith(::GetDlgItem(_hSelf, IDD_CUSTOMIZED_COLOR6_STATIC), _pTextColorPicker->getHSelf(), PosAlign::left, p6);
			alignWith(::GetDlgItem(_hSelf, IDD_CUSTOMIZED_COLOR7_STATIC), _pDarkerTextColorPicker->getHSelf(), PosAlign::left, p7);
			alignWith(::GetDlgItem(_hSelf, IDD_CUSTOMIZED_COLOR8_STATIC), _pDisabledTextColorPicker->getHSelf(), PosAlign::left, p8);
			alignWith(::GetDlgItem(_hSelf, IDD_CUSTOMIZED_COLOR9_STATIC), _pEdgeColorPicker->getHSelf(), PosAlign::left, p9);
			alignWith(::GetDlgItem(_hSelf, IDD_CUSTOMIZED_COLOR10_STATIC), _pLinkColorPicker->getHSelf(), PosAlign::left, p10);

			int cpDynamicalWidth = NppParameters::getInstance()._dpiManager.scaleX(25);
			int cpDynamicalHeight = NppParameters::getInstance()._dpiManager.scaleY(25);

			p1.x -= cpDynamicalWidth ; p1.y -= cpDynamicalHeight / 6;
			p2.x -= cpDynamicalWidth ; p2.y -= cpDynamicalHeight / 6;
			p3.x -= cpDynamicalWidth ; p3.y -= cpDynamicalHeight / 6;
			p4.x -= cpDynamicalWidth ; p4.y -= cpDynamicalHeight / 6;
			p5.x -= cpDynamicalWidth ; p5.y -= cpDynamicalHeight / 6;
			p6.x -= cpDynamicalWidth ; p6.y -= cpDynamicalHeight / 6;
			p7.x -= cpDynamicalWidth ; p7.y -= cpDynamicalHeight / 6;
			p8.x -= cpDynamicalWidth ; p8.y -= cpDynamicalHeight / 6;
			p9.x -= cpDynamicalWidth ; p9.y -= cpDynamicalHeight / 6;
			p10.x -= cpDynamicalWidth; p10.y -= cpDynamicalHeight / 6;

			::MoveWindow(reinterpret_cast<HWND>(_pPureBackgroundColorPicker->getHSelf()), p1.x, p1.y, cpDynamicalWidth, cpDynamicalHeight, TRUE);
			::MoveWindow(reinterpret_cast<HWND>(_pHotBackgroundColorPicker->getHSelf()), p2.x, p2.y, cpDynamicalWidth, cpDynamicalHeight, TRUE);
			::MoveWindow(reinterpret_cast<HWND>(_pSofterBackgroundColorPicker->getHSelf()), p3.x, p3.y, cpDynamicalWidth, cpDynamicalHeight, TRUE);
			::MoveWindow(reinterpret_cast<HWND>(_pBackgroundColorPicker->getHSelf()), p4.x, p4.y, cpDynamicalWidth, cpDynamicalHeight, TRUE);
			::MoveWindow(reinterpret_cast<HWND>(_pErrorBackgroundColorPicker->getHSelf()), p5.x, p5.y, cpDynamicalWidth, cpDynamicalHeight, TRUE);
			::MoveWindow(reinterpret_cast<HWND>(_pTextColorPicker->getHSelf()), p6.x, p6.y, cpDynamicalWidth, cpDynamicalHeight, TRUE);
			::MoveWindow(reinterpret_cast<HWND>(_pDarkerTextColorPicker->getHSelf()), p7.x, p7.y, cpDynamicalWidth, cpDynamicalHeight, TRUE);
			::MoveWindow(reinterpret_cast<HWND>(_pDisabledTextColorPicker->getHSelf()), p8.x, p8.y, cpDynamicalWidth, cpDynamicalHeight, TRUE);
			::MoveWindow(reinterpret_cast<HWND>(_pEdgeColorPicker->getHSelf()), p9.x, p9.y, cpDynamicalWidth, cpDynamicalHeight, TRUE);
			::MoveWindow(reinterpret_cast<HWND>(_pLinkColorPicker->getHSelf()), p10.x, p10.y, cpDynamicalWidth, cpDynamicalHeight, TRUE);

			_pBackgroundColorPicker->display();
			_pSofterBackgroundColorPicker->display();
			_pHotBackgroundColorPicker->display();
			_pPureBackgroundColorPicker->display();
			_pErrorBackgroundColorPicker->display();
			_pTextColorPicker->display();
			_pDarkerTextColorPicker->display();
			_pDisabledTextColorPicker->display();
			_pEdgeColorPicker->display();
			_pLinkColorPicker->display();

			::EnableWindow(::GetDlgItem(_hSelf, IDC_RADIO_DARKMODE_BLACK), nppGUI._darkmode._isEnabled);
			::EnableWindow(::GetDlgItem(_hSelf, IDC_RADIO_DARKMODE_RED), nppGUI._darkmode._isEnabled);
			::EnableWindow(::GetDlgItem(_hSelf, IDC_RADIO_DARKMODE_GREEN), nppGUI._darkmode._isEnabled);
			::EnableWindow(::GetDlgItem(_hSelf, IDC_RADIO_DARKMODE_BLUE), nppGUI._darkmode._isEnabled);
			::EnableWindow(::GetDlgItem(_hSelf, IDC_RADIO_DARKMODE_PURPLE), nppGUI._darkmode._isEnabled);
			::EnableWindow(::GetDlgItem(_hSelf, IDC_RADIO_DARKMODE_CYAN), nppGUI._darkmode._isEnabled);
			::EnableWindow(::GetDlgItem(_hSelf, IDC_RADIO_DARKMODE_OLIVE), nppGUI._darkmode._isEnabled);
			::EnableWindow(::GetDlgItem(_hSelf, IDC_RADIO_DARKMODE_CUSTOMIZED), nppGUI._darkmode._isEnabled);

			enableCustomizedColorCtrls(nppGUI._darkmode._isEnabled && id == IDC_RADIO_DARKMODE_CUSTOMIZED);
			
			ETDTProc enableDlgTheme = (ETDTProc)nppParam.getEnableThemeDlgTexture();
			if (enableDlgTheme)
				enableDlgTheme(_hSelf, ETDT_ENABLETAB);
			return TRUE;
		}

		case WM_DESTROY:
		{
			_pBackgroundColorPicker->destroy();
			_pSofterBackgroundColorPicker->destroy();
			_pHotBackgroundColorPicker->destroy();
			_pPureBackgroundColorPicker->destroy();
			_pErrorBackgroundColorPicker->destroy();
			_pTextColorPicker->destroy();
			_pDarkerTextColorPicker->destroy();
			_pDisabledTextColorPicker->destroy();
			_pEdgeColorPicker->destroy();
			_pLinkColorPicker->destroy();

			delete _pBackgroundColorPicker;
			delete _pSofterBackgroundColorPicker;
			delete _pHotBackgroundColorPicker;
			delete _pPureBackgroundColorPicker;
			delete _pErrorBackgroundColorPicker;
			delete _pTextColorPicker;
			delete _pDarkerTextColorPicker;
			delete _pDisabledTextColorPicker;
			delete _pEdgeColorPicker;
			delete _pLinkColorPicker;
		}

		case WM_COMMAND:
		{
			bool changed = false;
			bool forceRefresh = false;
			bool doEnableCustomizedColorCtrls = false;
			switch (wParam)
			{
				case IDC_CHECK_DARKMODE_ENABLE:
				{
					bool enableDarkMode = isCheckedOrNot(static_cast<int>(wParam));
					nppGUI._darkmode._isEnabled = enableDarkMode;

					::EnableWindow(::GetDlgItem(_hSelf, IDC_RADIO_DARKMODE_BLACK), enableDarkMode);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_RADIO_DARKMODE_RED), enableDarkMode);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_RADIO_DARKMODE_GREEN), enableDarkMode);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_RADIO_DARKMODE_BLUE), enableDarkMode);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_RADIO_DARKMODE_PURPLE), enableDarkMode);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_RADIO_DARKMODE_CYAN), enableDarkMode);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_RADIO_DARKMODE_OLIVE), enableDarkMode);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_RADIO_DARKMODE_CUSTOMIZED), enableDarkMode);

					enableCustomizedColorCtrls(enableDarkMode&& nppGUI._darkmode._colorTone == NppDarkMode::customizedTone);

					// Maintain the coherence in preferences
					if (nppGUI._darkmode._isEnabled)
					{
						// For toolbar: if dark mode enabled & TB_STANDARD is selected, switch to TB_SMALL
						bool isStandardChecked = false;
						::SendMessage(_hParent, PREF_MSG_ISCHECKED_GENERALPAGE, IDC_RADIO_STANDARD, LPARAM(&isStandardChecked));
						if (isStandardChecked)
							::SendMessage(_hParent, PREF_MSG_SETTOOLICONSFROMSTDTOSMALL, 0, 0);

						// For tabbar: uncheck Alternate icons checkbox
						::SendMessage(_hParent, PREF_MSG_DISABLETABBARALTERNATEICONS, 0, 0);
					}

					changed = true;
				}
				break;

				case IDC_RADIO_DARKMODE_BLACK:
				case IDC_RADIO_DARKMODE_RED:
				case IDC_RADIO_DARKMODE_GREEN:
				case IDC_RADIO_DARKMODE_BLUE:
				case IDC_RADIO_DARKMODE_PURPLE:
				case IDC_RADIO_DARKMODE_CYAN:
				case IDC_RADIO_DARKMODE_OLIVE:
				case IDC_RADIO_DARKMODE_CUSTOMIZED:
				case IDD_CUSTOMIZED_RESET_BUTTON:
					if (wParam == IDC_RADIO_DARKMODE_BLACK)
					{
						if (nppGUI._darkmode._colorTone == NppDarkMode::blackTone)
							return TRUE;
						nppGUI._darkmode._colorTone = NppDarkMode::blackTone;
					}
					else if (wParam == IDC_RADIO_DARKMODE_RED)
					{
						if (nppGUI._darkmode._colorTone == NppDarkMode::redTone)
							return TRUE;
						nppGUI._darkmode._colorTone = NppDarkMode::redTone;
					}
					else if (wParam == IDC_RADIO_DARKMODE_GREEN)
					{
						if (nppGUI._darkmode._colorTone == NppDarkMode::greenTone)
							return TRUE;
						nppGUI._darkmode._colorTone = NppDarkMode::greenTone;
					}
					else if (wParam == IDC_RADIO_DARKMODE_BLUE)
					{
						if (nppGUI._darkmode._colorTone == NppDarkMode::blueTone)
							return TRUE;
						nppGUI._darkmode._colorTone = NppDarkMode::blueTone;
					}
					else if (wParam == IDC_RADIO_DARKMODE_PURPLE)
					{
						if (nppGUI._darkmode._colorTone == NppDarkMode::purpleTone)
							return TRUE;
						nppGUI._darkmode._colorTone = NppDarkMode::purpleTone;
					}
					else if (wParam == IDC_RADIO_DARKMODE_CYAN)
					{
						if (nppGUI._darkmode._colorTone == NppDarkMode::cyanTone)
							return TRUE;
						nppGUI._darkmode._colorTone = NppDarkMode::cyanTone;
					}
					else if (wParam == IDC_RADIO_DARKMODE_OLIVE)
					{
						if (nppGUI._darkmode._colorTone == NppDarkMode::oliveTone)
							return TRUE;
						nppGUI._darkmode._colorTone = NppDarkMode::oliveTone;
					}
					else if (wParam == IDC_RADIO_DARKMODE_CUSTOMIZED)
					{
						if (nppGUI._darkmode._colorTone == NppDarkMode::customizedTone)
							return TRUE;
						nppGUI._darkmode._colorTone = NppDarkMode::customizedTone;
						doEnableCustomizedColorCtrls = true;
					}
					
					else if (wParam == IDD_CUSTOMIZED_RESET_BUTTON)
					{
						nppGUI._darkmode._customColors = NppDarkMode::getDarkModeDefaultColors();
						NppDarkMode::changeCustomTheme(nppGUI._darkmode._customColors);
						doEnableCustomizedColorCtrls = true;
					}


					// switch to chosen dark mode
					nppGUI._darkmode._isEnabled = true;
					NppDarkMode::setDarkTone(nppGUI._darkmode._colorTone);
					changed = true;
					forceRefresh = true;

					enableCustomizedColorCtrls(doEnableCustomizedColorCtrls);
					break;

				default:
					switch (HIWORD(wParam))
					{
						case CPN_COLOURPICKED:
						{
							COLORREF c;
							if (reinterpret_cast<HWND>(lParam) == _pBackgroundColorPicker->getHSelf())
							{
								c = _pBackgroundColorPicker->getColour();
								NppDarkMode::setBackgroundColor(c);
								nppGUI._darkmode._customColors.background = c;
							}
							else if (reinterpret_cast<HWND>(lParam) == _pSofterBackgroundColorPicker->getHSelf())
							{
								c = _pSofterBackgroundColorPicker->getColour();
								NppDarkMode::setSofterBackgroundColor(c);
								nppGUI._darkmode._customColors.softerBackground = c;
							}
							else if (reinterpret_cast<HWND>(lParam) == _pHotBackgroundColorPicker->getHSelf())
							{
								c = _pHotBackgroundColorPicker->getColour();
								NppDarkMode::setHotBackgroundColor(c);
								nppGUI._darkmode._customColors.hotBackground = c;
							}
							else if (reinterpret_cast<HWND>(lParam) == _pPureBackgroundColorPicker->getHSelf())
							{
								c = _pPureBackgroundColorPicker->getColour();
								NppDarkMode::setDarkerBackgroundColor(c);
								nppGUI._darkmode._customColors.pureBackground = c;
							}
							else if (reinterpret_cast<HWND>(lParam) == _pErrorBackgroundColorPicker->getHSelf())
							{
								c = _pErrorBackgroundColorPicker->getColour();
								NppDarkMode::setErrorBackgroundColor(c);
								nppGUI._darkmode._customColors.errorBackground = c;
							}
							else if (reinterpret_cast<HWND>(lParam) == _pTextColorPicker->getHSelf())
							{
								c = _pTextColorPicker->getColour();
								NppDarkMode::setTextColor(c);
								nppGUI._darkmode._customColors.text = c;
							}
							else if (reinterpret_cast<HWND>(lParam) == _pDarkerTextColorPicker->getHSelf())
							{
								c = _pDarkerTextColorPicker->getColour();
								NppDarkMode::setDarkerTextColor(c);
								nppGUI._darkmode._customColors.darkerText = c;
							}
							else if (reinterpret_cast<HWND>(lParam) == _pDisabledTextColorPicker->getHSelf())
							{
								c = _pDisabledTextColorPicker->getColour();
								NppDarkMode::setDisabledTextColor(c);
								nppGUI._darkmode._customColors.disabledText = c;
							}
							else if (reinterpret_cast<HWND>(lParam) == _pEdgeColorPicker->getHSelf())
							{
								c = _pEdgeColorPicker->getColour();
								NppDarkMode::setEdgeColor(c);
								nppGUI._darkmode._customColors.edge = c;
							}
							else if (reinterpret_cast<HWND>(lParam) == _pLinkColorPicker->getHSelf())
							{
								c = _pLinkColorPicker->getColour();
								NppDarkMode::setLinkTextColor(c);
								nppGUI._darkmode._customColors.linkText = c;
							}
							else
							{
								return FALSE;
							}

							nppGUI._darkmode._isEnabled = true;
							NppDarkMode::setDarkTone(nppGUI._darkmode._colorTone);
							changed = true;
							forceRefresh = true;
						}
						break;

						default:
						{
							return FALSE;
						}
					}
			}

			if (changed)
			{
				NppDarkMode::refreshDarkMode(_hSelf, forceRefresh);
				getFocus(); // to make black mode title bar appear
				return TRUE;
			}

			return FALSE;
		}
	}
	return FALSE;
}

void MarginsDlg::initScintParam()
{
		ScintillaViewParams & svp = const_cast<ScintillaViewParams &>(nppParms->getSVP());
	
	::SendDlgItemMessage(_hSelf, IDC_RADIO_BOX, BM_SETCHECK, FALSE, 0);
	::SendDlgItemMessage(_hSelf, IDC_RADIO_CIRCLE, BM_SETCHECK, FALSE, 0);
	::SendDlgItemMessage(_hSelf, IDC_RADIO_ARROW, BM_SETCHECK, FALSE, 0);
	::SendDlgItemMessage(_hSelf, IDC_RADIO_SIMPLE, BM_SETCHECK, FALSE, 0);
	::SendDlgItemMessage(_hSelf, IDC_RADIO_FOLDMARGENONE, BM_SETCHECK, FALSE, 0);

	int id = 0;
	switch (svp._folderStyle)
	{
		case FOLDER_STYLE_NONE:
			id = IDC_RADIO_FOLDMARGENONE;
			break;
		case FOLDER_STYLE_BOX:
			id = IDC_RADIO_BOX;
			break;
		case FOLDER_STYLE_CIRCLE:
			id = IDC_RADIO_CIRCLE;
			break;
		case FOLDER_STYLE_ARROW:
			id = IDC_RADIO_ARROW;
			break;
		default : // FOLDER_STYLE_SIMPLE:
			id = IDC_RADIO_SIMPLE;
	}
	::SendDlgItemMessage(_hSelf, id, BM_SETCHECK, TRUE, 0);
	
	switch (svp._lineWrapMethod)
	{
		case LINEWRAP_ALIGNED:
			id = IDC_RADIO_LWALIGN;
			break;
		case LINEWRAP_INDENT:
			id = IDC_RADIO_LWINDENT;
			break;
		default : // LINEWRAP_DEFAULT
			id = IDC_RADIO_LWDEF;
	}
	::SendDlgItemMessage(_hSelf, id, BM_SETCHECK, TRUE, 0);

	::SendDlgItemMessage(_hSelf, IDC_CHECK_SMOOTHFONT, BM_SETCHECK, svp._doSmoothFont, 0);
	::SendDlgItemMessage(_hSelf, IDC_CHECK_LINENUMBERMARGE, BM_SETCHECK, svp._lineNumberMarginShow, 0);
	::SendDlgItemMessage(_hSelf, IDC_CHECK_BOOKMARKMARGE, BM_SETCHECK, svp._bookMarkMarginShow, 0);
	::SendDlgItemMessage(_hSelf, IDC_CHECK_CURRENTLINEHILITE, BM_SETCHECK, svp._currentLineHilitingShow, 0);
	::SendDlgItemMessage(_hSelf, IDC_CHECK_SCROLLBEYONDLASTLINE, BM_SETCHECK, svp._scrollBeyondLastLine, 0);
	::SendDlgItemMessage(_hSelf, IDC_CHECK_RIGHTCLICKKEEPSSELECTION, BM_SETCHECK, svp._rightClickKeepsSelection, 0);
	::SendDlgItemMessage(_hSelf, IDC_CHECK_DISABLEADVANCEDSCROLL, BM_SETCHECK, svp._disableAdvancedScrolling, 0);

	::SendDlgItemMessage(_hSelf, IDC_CHECK_NOEDGE, BM_SETCHECK, !svp._showBorderEdge, 0);

	bool canBeBg = svp._edgeMultiColumnPos.size() == 1;
	if (!canBeBg)
	{
		svp._isEdgeBgMode = false;
		::SendDlgItemMessage(_hSelf, IDC_CHECK_EDGEBGMODE, BM_SETCHECK, FALSE, 0);
		::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_EDGEBGMODE), FALSE);
	}
	else
	{
		::SendDlgItemMessage(_hSelf, IDC_CHECK_EDGEBGMODE, BM_SETCHECK, svp._isEdgeBgMode, 0);
	}
	

	generic_string edgeColumnPosStr;
	for (auto i : svp._edgeMultiColumnPos)
	{
		std::string s = std::to_string(i);
		edgeColumnPosStr += generic_string(s.begin(), s.end());
		edgeColumnPosStr += TEXT(" ");
	}
	::SendDlgItemMessage(_hSelf, IDC_COLUMNPOS_EDIT, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(edgeColumnPosStr.c_str()));

	oldFunclstToolbarProc = reinterpret_cast<WNDPROC>(::SetWindowLongPtr(::GetDlgItem(_hSelf, IDC_COLUMNPOS_EDIT), GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(editNumSpaceProc)));
}

INT_PTR CALLBACK MarginsDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
		NppGUI & nppGUI = const_cast<NppGUI &>(nppParms->getNppGUI());
	switch (message) 
	{
		case WM_INITDIALOG :
		{
			::SendDlgItemMessage(_hSelf, IDC_WIDTH_COMBO, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("0")));
			::SendDlgItemMessage(_hSelf, IDC_WIDTH_COMBO, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("1")));
			::SendDlgItemMessage(_hSelf, IDC_WIDTH_COMBO, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("2")));
			::SendDlgItemMessage(_hSelf, IDC_WIDTH_COMBO, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("3")));
			::SendDlgItemMessage(_hSelf, IDC_WIDTH_COMBO, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("Block")));


			::SendMessage(::GetDlgItem(_hSelf, IDC_WIDTH_COMBO), CB_SETCURSEL, nppGUI._caretWidth, 0);
            ::SendDlgItemMessage(_hSelf, IDC_CHECK_MULTISELECTION, BM_SETCHECK, nppGUI._enableMultiSelection, 0);
			
			::SendMessage(::GetDlgItem(_hSelf, IDC_CARETBLINKRATE_SLIDER),TBM_SETRANGEMIN, TRUE, BLINKRATE_FASTEST);
			::SendMessage(::GetDlgItem(_hSelf, IDC_CARETBLINKRATE_SLIDER),TBM_SETRANGEMAX, TRUE, BLINKRATE_SLOWEST);
			::SendMessage(::GetDlgItem(_hSelf, IDC_CARETBLINKRATE_SLIDER),TBM_SETPAGESIZE, 0, BLINKRATE_INTERVAL);
			int blinkRate = (nppGUI._caretBlinkRate==0)?BLINKRATE_SLOWEST:nppGUI._caretBlinkRate;
			::SendMessage(::GetDlgItem(_hSelf, IDC_CARETBLINKRATE_SLIDER),TBM_SETPOS, TRUE, blinkRate);

			::SendMessage(::GetDlgItem(_hSelf, IDC_BORDERWIDTH_SLIDER),TBM_SETRANGEMIN, TRUE, BORDERWIDTH_SMALLEST);
			::SendMessage(::GetDlgItem(_hSelf, IDC_BORDERWIDTH_SLIDER),TBM_SETRANGEMAX, TRUE, BORDERWIDTH_LARGEST);
			::SendMessage(::GetDlgItem(_hSelf, IDC_BORDERWIDTH_SLIDER),TBM_SETPAGESIZE, 0, BLINKRATE_INTERVAL);
			const ScintillaViewParams & svp = nppParms->getSVP();
			::SendMessage(::GetDlgItem(_hSelf, IDC_BORDERWIDTH_SLIDER),TBM_SETPOS, TRUE, svp._borderWidth);
			::SetDlgItemInt(_hSelf, IDC_BORDERWIDTHVAL_STATIC, svp._borderWidth, FALSE);
			::SendMessage(::GetDlgItem(_hSelf, IDC_BORDERXCompWIDTH_SLIDER),TBM_SETPOS, TRUE, svp._borderWidthXCompat);
			::SetDlgItemInt(_hSelf, IDC_BORDERXCompWIDTHVAL_STATIC, svp._borderWidthXCompat, FALSE);

			//::SendMessage(::GetDlgItem(_hSelf, IDC_PADDINGLEFT_SLIDER), TBM_SETRANGEMIN, TRUE, PADDING_SMALLEST);
			//::SendMessage(::GetDlgItem(_hSelf, IDC_PADDINGLEFT_SLIDER), TBM_SETRANGEMAX, TRUE, PADDING_LARGEST);
			//::SendMessage(::GetDlgItem(_hSelf, IDC_PADDINGLEFT_SLIDER), TBM_SETPAGESIZE, 0, PADDING_INTERVAL);
			//::SendMessage(::GetDlgItem(_hSelf, IDC_PADDINGLEFT_SLIDER), TBM_SETPOS, TRUE, svp._paddingLeft);
			//::SetDlgItemInt(_hSelf, IDC_PADDINGLEFTVAL_STATIC, svp._paddingLeft, FALSE);
			//
			//::SendMessage(::GetDlgItem(_hSelf, IDC_PADDINGRIGHT_SLIDER), TBM_SETRANGEMIN, TRUE, PADDING_SMALLEST);
			//::SendMessage(::GetDlgItem(_hSelf, IDC_PADDINGRIGHT_SLIDER), TBM_SETRANGEMAX, TRUE, PADDING_LARGEST);
			//::SendMessage(::GetDlgItem(_hSelf, IDC_PADDINGRIGHT_SLIDER), TBM_SETPAGESIZE, 0, PADDING_INTERVAL);
			//::SendMessage(::GetDlgItem(_hSelf, IDC_PADDINGRIGHT_SLIDER), TBM_SETPOS, TRUE, svp._paddingRight);
			//::SetDlgItemInt(_hSelf, IDC_PADDINGRIGHTVAL_STATIC, svp._paddingRight, FALSE);
			//
			//::SendMessage(::GetDlgItem(_hSelf, IDC_DISTRACTIONFREE_SLIDER), TBM_SETRANGEMIN, TRUE, DISTRACTIONFREE_SMALLEST);
			//::SendMessage(::GetDlgItem(_hSelf, IDC_DISTRACTIONFREE_SLIDER), TBM_SETRANGEMAX, TRUE, DISTRACTIONFREE_LARGEST);
			//::SendMessage(::GetDlgItem(_hSelf, IDC_DISTRACTIONFREE_SLIDER), TBM_SETPAGESIZE, 0, DISTRACTIONFREE_INTERVAL);
			//::SendMessage(::GetDlgItem(_hSelf, IDC_DISTRACTIONFREE_SLIDER), TBM_SETPOS, TRUE, svp._distractionFreeDivPart);
			//::SetDlgItemInt(_hSelf, IDC_DISTRACTIONFREEVAL_STATIC, svp._distractionFreeDivPart, FALSE);


			initScintParam();
			
			ETDTProc enableDlgTheme = (ETDTProc)nppParms->getEnableThemeDlgTexture();
			if (enableDlgTheme)
				enableDlgTheme(_hSelf, ETDT_ENABLETAB);
			return TRUE;
		}

		case WM_HSCROLL:
		{
			ScintillaViewParams & svp = (ScintillaViewParams &)nppParms->getSVP();
			HWND hCaretBlikRateSlider = ::GetDlgItem(_hSelf, IDC_CARETBLINKRATE_SLIDER);
			HWND hBorderWidthSlider = ::GetDlgItem(_hSelf, IDC_BORDERWIDTH_SLIDER);
			HWND hBorderXCompWidthSlider = ::GetDlgItem(_hSelf, IDC_BORDERXCompWIDTH_SLIDER);
			HWND hPaddingLeftSlider = ::GetDlgItem(_hSelf, IDC_PADDINGLEFT_SLIDER);
			HWND hPaddingRightSlider = ::GetDlgItem(_hSelf, IDC_PADDINGRIGHT_SLIDER);
			//HWND hDistractionFreeSlider = ::GetDlgItem(_hSelf, IDC_DISTRACTIONFREE_SLIDER);
			if (reinterpret_cast<HWND>(lParam) == hCaretBlikRateSlider)
			{
				auto blinkRate = ::SendMessage(hCaretBlikRateSlider, TBM_GETPOS, 0, 0);
				if (blinkRate == BLINKRATE_SLOWEST)
					blinkRate = 0;
				nppGUI._caretBlinkRate = static_cast<int>(blinkRate);

				::SendMessage(::GetParent(_hParent), NPPM_INTERNAL_SETCARETBLINKRATE, 0, 0);
			}
			else if (reinterpret_cast<HWND>(lParam) == hBorderWidthSlider)
			{
				auto borderWidth = ::SendMessage(hBorderWidthSlider, TBM_GETPOS, 0, 0);
				svp._borderWidth = static_cast<int>(borderWidth);
				::SetDlgItemInt(_hSelf, IDC_BORDERWIDTHVAL_STATIC, static_cast<UINT>(borderWidth), FALSE);
				::SendMessage(::GetParent(_hParent), WM_SIZE, 0, 0);
			}
			else if (reinterpret_cast<HWND>(lParam) == hBorderXCompWidthSlider)
			{
				auto borderWidth = ::SendMessage(hBorderXCompWidthSlider, TBM_GETPOS, 0, 0);
				svp._borderWidthXCompat = static_cast<int>(borderWidth);
				::SetDlgItemInt(_hSelf, IDC_BORDERXCompWIDTHVAL_STATIC, static_cast<UINT>(borderWidth), FALSE);
				::SendMessage(::GetParent(_hParent), WM_SIZE, 0, 0);
			}
			//else if (reinterpret_cast<HWND>(lParam) == hPaddingLeftSlider)
			//{
			//	svp._paddingLeft = static_cast<unsigned char>(::SendMessage(hPaddingLeftSlider, TBM_GETPOS, 0, 0));
			//	::SetDlgItemInt(_hSelf, IDC_PADDINGLEFTVAL_STATIC, static_cast<UINT>(svp._paddingLeft), FALSE);
			//	::SendMessage(::GetParent(_hParent), NPPM_INTERNAL_UPDATETEXTZONEPADDING, 0, 0);
			//
			//}
			//else if (reinterpret_cast<HWND>(lParam) == hPaddingRightSlider)
			//{
			//	svp._paddingRight = static_cast<unsigned char>(::SendMessage(hPaddingRightSlider, TBM_GETPOS, 0, 0));
			//	::SetDlgItemInt(_hSelf, IDC_PADDINGRIGHTVAL_STATIC, static_cast<UINT>(svp._paddingRight), FALSE);
			//	::SendMessage(::GetParent(_hParent), NPPM_INTERNAL_UPDATETEXTZONEPADDING, 0, 0);
			//}
			//else if (reinterpret_cast<HWND>(lParam) == hDistractionFreeSlider)
			//{
			//	svp._distractionFreeDivPart = static_cast<unsigned char>(::SendMessage(hDistractionFreeSlider, TBM_GETPOS, 0, 0));
			//	::SetDlgItemInt(_hSelf, IDC_DISTRACTIONFREEVAL_STATIC, static_cast<UINT>(svp._distractionFreeDivPart), FALSE);
			//	::SendMessage(::GetParent(_hParent), NPPM_INTERNAL_UPDATETEXTZONEPADDING, 0, 0);
			//}
			return 0;	//return zero when handled
		}

		case WM_COMMAND : 
		{
			ScintillaViewParams & svp = const_cast<ScintillaViewParams &>(nppParms->getSVP());
			switch (wParam)
			{
				case IDC_CHECK_SMOOTHFONT:
					svp._doSmoothFont = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECK_SMOOTHFONT, BM_GETCHECK, 0, 0));
					::SendMessage(::GetParent(_hParent), NPPM_SETSMOOTHFONT, 0, svp._doSmoothFont);
					return TRUE;
				case IDC_CHECK_LINENUMBERMARGE:
					svp._lineNumberMarginShow = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECK_LINENUMBERMARGE, BM_GETCHECK, 0, 0));
					::SendMessage(_hParent, WM_COMMAND, IDM_VIEW_LINENUMBER, 0);
					return TRUE;
				
				case IDC_CHECK_BOOKMARKMARGE:
					svp._bookMarkMarginShow = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECK_BOOKMARKMARGE, BM_GETCHECK, 0, 0));
					::SendMessage(_hParent, WM_COMMAND, IDM_VIEW_SYMBOLMARGIN, 0);
					return TRUE;

				case IDC_CHECK_CURRENTLINEHILITE:
					svp._currentLineHilitingShow = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECK_CURRENTLINEHILITE, BM_GETCHECK, 0, 0));
					::SendMessage(_hParent, WM_COMMAND, IDM_VIEW_CURLINE_HILITING, 0);
					return TRUE;

				case IDC_CHECK_SCROLLBEYONDLASTLINE:
					svp._scrollBeyondLastLine = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECK_SCROLLBEYONDLASTLINE, BM_GETCHECK, 0, 0));
					::SendMessage(::GetParent(_hParent), NPPM_INTERNAL_SCROLLBEYONDLASTLINE, 0, 0);
					return TRUE;

				case IDC_CHECK_RIGHTCLICKKEEPSSELECTION:
					svp._rightClickKeepsSelection = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECK_RIGHTCLICKKEEPSSELECTION, BM_GETCHECK, 0, 0));
					return TRUE;

				case IDC_CHECK_DISABLEADVANCEDSCROLL:
					svp._disableAdvancedScrolling = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECK_DISABLEADVANCEDSCROLL, BM_GETCHECK, 0, 0));
					return TRUE;

                case IDC_CHECK_MULTISELECTION :
                    nppGUI._enableMultiSelection = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECK_MULTISELECTION, BM_GETCHECK, 0, 0));
                    ::SendMessage(::GetParent(_hParent), NPPM_INTERNAL_SETMULTISELCTION, 0, 0);
                    return TRUE;

				case IDC_CHECK_NOEDGE:
					svp._showBorderEdge = !(BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECK_NOEDGE, BM_GETCHECK, 0, 0));
					::SendMessage(::GetParent(_hParent), NPPM_SETEDITORBORDEREDGE, 0, svp._showBorderEdge ? TRUE : FALSE);
					return TRUE;

				case IDC_RADIO_SIMPLE:
					svp._folderStyle = FOLDER_STYLE_SIMPLE;
					::SendMessage(_hParent, WM_COMMAND, IDM_VIEW_FOLDERMAGIN_SIMPLE, 0);
					return TRUE;
				case IDC_RADIO_ARROW:
					svp._folderStyle = FOLDER_STYLE_ARROW;
					::SendMessage(_hParent, WM_COMMAND, IDM_VIEW_FOLDERMAGIN_ARROW, 0);
					return TRUE;
				case IDC_RADIO_CIRCLE:
					svp._folderStyle = FOLDER_STYLE_CIRCLE;
					::SendMessage(_hParent, WM_COMMAND, IDM_VIEW_FOLDERMAGIN_CIRCLE, 0);
					return TRUE;
				case IDC_RADIO_BOX:
					svp._folderStyle = FOLDER_STYLE_BOX;
					::SendMessage(_hParent, WM_COMMAND, IDM_VIEW_FOLDERMAGIN_BOX, 0);
					return TRUE;
					
				case IDC_RADIO_FOLDMARGENONE:
					svp._folderStyle = FOLDER_STYLE_NONE;
					::SendMessage(_hParent, WM_COMMAND, IDM_VIEW_FOLDERMAGIN, 0);
					return TRUE;

				case IDC_CHECK_EDGEBGMODE:
					svp._isEdgeBgMode = isCheckedOrNot(IDC_CHECK_EDGEBGMODE);
					::SendMessage(::GetParent(_hParent), NPPM_INTERNAL_EDGEMULTISETSIZE, 0, 0);
					return TRUE;

				case IDC_RADIO_LWDEF:
					svp._lineWrapMethod = LINEWRAP_DEFAULT;
					::SendMessage(_hParent, WM_COMMAND, IDM_VIEW_LWDEF, 0);
					return TRUE;

				case IDC_RADIO_LWALIGN:
					svp._lineWrapMethod = LINEWRAP_ALIGNED;
					::SendMessage(_hParent, WM_COMMAND, IDM_VIEW_LWALIGN, 0);
					return TRUE;

				case IDC_RADIO_LWINDENT:
					svp._lineWrapMethod = LINEWRAP_INDENT;
					::SendMessage(_hParent, WM_COMMAND, IDM_VIEW_LWINDENT, 0);
					return TRUE;

				default :
					switch (HIWORD(wParam))
					{
						case CBN_SELCHANGE : // == case LBN_SELCHANGE :
						{
							if (LOWORD(wParam) == IDC_WIDTH_COMBO)
							{
								nppGUI._caretWidth = static_cast<int32_t>(::SendDlgItemMessage(_hSelf, IDC_WIDTH_COMBO, CB_GETCURSEL, 0, 0));
								::SendMessage(::GetParent(_hParent), NPPM_INTERNAL_SETCARETWIDTH, 0, 0);
								return TRUE;
							}
						}
						break;

						case EN_CHANGE :
						{
							if (LOWORD(wParam) == IDC_COLUMNPOS_EDIT)
							{
								TCHAR text[MAX_PATH];
								::SendDlgItemMessage(_hSelf, IDC_COLUMNPOS_EDIT, WM_GETTEXT, MAX_PATH, reinterpret_cast<LPARAM>(text));

								if (str2numberVector(text, svp._edgeMultiColumnPos))
								{
									bool canBeBg = svp._edgeMultiColumnPos.size() == 1;
									if (!canBeBg)
									{
										svp._isEdgeBgMode = false;
										::SendDlgItemMessage(_hSelf, IDC_CHECK_EDGEBGMODE, BM_SETCHECK, FALSE, 0);
									}
									::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_EDGEBGMODE), canBeBg);
									::SendMessage(::GetParent(_hParent), NPPM_INTERNAL_EDGEMULTISETSIZE, 0, 0);
									return TRUE;
								}
							}
						}
						break;
					}
			}
		}
	}
	return FALSE;
}

const size_t fileUpdateChoiceEnable = 0;
const size_t fileUpdateChoiceEnable4All = 1;
const size_t fileUpdateChoiceDisable = 2;
INT_PTR CALLBACK MiscSettingsDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM)
{
		NppGUI & nppGUI = const_cast<NppGUI &>(nppParms->getNppGUI());
	switch (message) 
	{
		case WM_INITDIALOG :
		{
			::SendDlgItemMessage(_hSelf, IDC_COMBO_FILEUPDATECHOICE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("Enable")));
			::SendDlgItemMessage(_hSelf, IDC_COMBO_FILEUPDATECHOICE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("Enable for all opened files")));
			::SendDlgItemMessage(_hSelf, IDC_COMBO_FILEUPDATECHOICE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("Disable")));

			int selIndex = -1;
			
			if (nppGUI._fileAutoDetection & cdEnabledOld)
			{
				selIndex = fileUpdateChoiceEnable4All;
			}
			else if (nppGUI._fileAutoDetection & cdEnabledNew)
			{				
				selIndex = fileUpdateChoiceEnable;
			}
			else //cdDisabled
			{
				::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_UPDATESILENTLY), FALSE);
				::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_UPDATEGOTOEOF), FALSE);
				
				selIndex = fileUpdateChoiceDisable;
			}
			
			::SendDlgItemMessage(_hSelf, IDC_COMBO_FILEUPDATECHOICE, CB_SETCURSEL, selIndex, 0);

			bool bCheck = (nppGUI._fileAutoDetection & cdAutoUpdate) ? true : false;
			::SendDlgItemMessage(_hSelf, IDC_CHECK_UPDATESILENTLY, BM_SETCHECK, bCheck? BST_CHECKED: BST_UNCHECKED, 0);

			bCheck = (nppGUI._fileAutoDetection & cdGo2end) ? true : false;
			::SendDlgItemMessage(_hSelf, IDC_CHECK_UPDATEGOTOEOF, BM_SETCHECK, bCheck ? BST_CHECKED : BST_UNCHECKED, 0);

			::SendDlgItemMessage(_hSelf, IDC_CHECK_MIN2SYSTRAY, BM_SETCHECK, nppGUI._isMinimizedToTray, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_DETECTENCODING, BM_SETCHECK, nppGUI._detectEncoding, 0);
            ::SendDlgItemMessage(_hSelf, IDC_CHECK_AUTOUPDATE, BM_SETCHECK, 0, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_SAVEDLGEXTFILTALLTYPES, BM_SETCHECK, nppGUI._setSaveDlgExtFiltToAllTypes, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_DIRECTWRITE_ENABLE, BM_SETCHECK, nppGUI._writeTechnologyEngine == directWriteTechnology, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_ENABLEDOCPEEKER, BM_SETCHECK, nppGUI._isDocPeekOnTab ? BST_CHECKED : BST_UNCHECKED, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_ENABLEDOCPEEKONMAP, BM_SETCHECK, nppGUI._isDocPeekOnMap ? BST_CHECKED : BST_UNCHECKED, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_SAVEALLCONFIRM, BM_SETCHECK, nppGUI._saveAllConfirm, 0);

			::ShowWindow(::GetDlgItem(_hSelf, IDC_CHECK_AUTOUPDATE), 0?SW_SHOW:SW_HIDE);
			
			BOOL linkEnable =  nppGUI._styleURL != urlDisable;
			BOOL dontUnderline = (nppGUI._styleURL == urlNoUnderLineFg) || (nppGUI._styleURL == urlNoUnderLineBg);
			BOOL roundBoxMode  = (nppGUI._styleURL == urlNoUnderLineBg) || (nppGUI._styleURL == urlUnderLineBg);
			
			::SendDlgItemMessage(_hSelf, IDC_CHECK_CLICKABLELINK_ENABLE, BM_SETCHECK, linkEnable, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_CLICKABLELINK_NOUNDERLINE, BM_SETCHECK, dontUnderline, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_CLICKABLELINK_FULLBOXMODE, BM_SETCHECK, roundBoxMode, 0);
			::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_CLICKABLELINK_NOUNDERLINE), linkEnable);
			::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_CLICKABLELINK_FULLBOXMODE), linkEnable);

			::SendDlgItemMessage(_hSelf, IDC_EDIT_SESSIONFILEEXT, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(nppGUI._definedSessionExt.c_str()));
			::SendDlgItemMessage(_hSelf, IDC_EDIT_WORKSPACEFILEEXT, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(nppGUI._definedWorkspaceExt.c_str()));
			
			::SendDlgItemMessage(_hSelf, IDC_CHECK_ENABLEDOCSWITCHER, BM_SETCHECK, nppGUI._doTaskList, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_STYLEMRU, BM_SETCHECK, nppGUI._styleMRU, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_SHORTTITLE, BM_SETCHECK, nppGUI._shortTitlebar, 0);

			ETDTProc enableDlgTheme = (ETDTProc)nppParms->getEnableThemeDlgTexture();
			if (enableDlgTheme)
				enableDlgTheme(_hSelf, ETDT_ENABLETAB);

			return TRUE;
		}

		case WM_COMMAND : 
		{
			if (HIWORD(wParam) == EN_CHANGE)
			{
				switch (LOWORD(wParam))
				{
					case  IDC_EDIT_SESSIONFILEEXT:
					{
						TCHAR sessionExt[MAX_PATH];
						::SendDlgItemMessage(_hSelf, IDC_EDIT_SESSIONFILEEXT, WM_GETTEXT, MAX_PATH, reinterpret_cast<LPARAM>(sessionExt));
						nppGUI._definedSessionExt = sessionExt;
						return TRUE;
					}
					case  IDC_EDIT_WORKSPACEFILEEXT:
					{
						TCHAR workspaceExt[MAX_PATH];
						::SendDlgItemMessage(_hSelf, IDC_EDIT_WORKSPACEFILEEXT, WM_GETTEXT, MAX_PATH, reinterpret_cast<LPARAM>(workspaceExt));
						nppGUI._definedWorkspaceExt = workspaceExt;
						return TRUE;
					}
				}
			}
			
			switch (wParam)
			{
				case IDC_CHECK_UPDATESILENTLY:
				case IDC_CHECK_UPDATEGOTOEOF:
				{
					bool isSilent = isCheckedOrNot(IDC_CHECK_UPDATESILENTLY);
					bool isGo2End = isCheckedOrNot(IDC_CHECK_UPDATEGOTOEOF);

					auto index = ::SendDlgItemMessage(_hSelf, IDC_COMBO_FILEUPDATECHOICE, CB_GETCURSEL, 0, 0);

					int cd = cdDisabled;

					if (index == fileUpdateChoiceEnable || index == fileUpdateChoiceEnable4All)
					{
						if (index == fileUpdateChoiceEnable4All)
							cd |= cdEnabledOld;
						else
							cd |= cdEnabledNew;

						if (isSilent)
							cd |= cdAutoUpdate;
						if (isGo2End)
							cd |= cdGo2end;
					}

					nppGUI._fileAutoDetection = cd;
				}
				return TRUE;

				case IDC_CHECK_CLICKABLELINK_ENABLE:
				{
					bool isChecked = isCheckedOrNot(IDC_CHECK_CLICKABLELINK_ENABLE);
					if (!isChecked)
					{
						::SendDlgItemMessage(_hSelf, IDC_CHECK_CLICKABLELINK_NOUNDERLINE, BM_SETCHECK, BST_UNCHECKED, 0);
						::SendDlgItemMessage(_hSelf, IDC_CHECK_CLICKABLELINK_FULLBOXMODE, BM_SETCHECK, BST_UNCHECKED, 0);
					}
					::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_CLICKABLELINK_NOUNDERLINE), isChecked);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_CLICKABLELINK_FULLBOXMODE), isChecked);

					nppGUI._styleURL = isChecked ? urlUnderLineFg : urlDisable;
					HWND grandParent = ::GetParent(_hParent);
					::SendMessage(grandParent, NPPM_INTERNAL_UPDATECLICKABLELINKS, 0, 0);
				}
				return TRUE;

				case IDC_CHECK_CLICKABLELINK_NOUNDERLINE:
				case IDC_CHECK_CLICKABLELINK_FULLBOXMODE:
				{
					bool isChecked = isCheckedOrNot(IDC_CHECK_CLICKABLELINK_NOUNDERLINE);
					bool isNoUnderline = isCheckedOrNot(IDC_CHECK_CLICKABLELINK_NOUNDERLINE);
					bool isRoundBoxMode = isCheckedOrNot(IDC_CHECK_CLICKABLELINK_FULLBOXMODE);
					if (isRoundBoxMode)
						nppGUI._styleURL = isNoUnderline ? urlNoUnderLineBg : urlUnderLineBg;
					else
						nppGUI._styleURL = isNoUnderline ? urlNoUnderLineFg : urlUnderLineFg;
					HWND grandParent = ::GetParent(_hParent);
					::SendMessage(grandParent, NPPM_INTERNAL_UPDATECLICKABLELINKS, 0, 0);
				}
				return TRUE;

				case IDC_CHECK_AUTOUPDATE:
					return TRUE;

				//case IDC_CHECK_SAVEDLGEXTFILTALLTYPES:
				//	nppGUI._setSaveDlgExtFiltToAllTypes = isCheckedOrNot(static_cast<int32_t>(wParam));
				//	return TRUE;

				case IDC_CHECK_MIN2SYSTRAY:
					nppGUI._isMinimizedToTray = isCheckedOrNot(static_cast<int32_t>(wParam));
					return TRUE;

				case IDC_CHECK_DETECTENCODING:
					nppGUI._detectEncoding = isCheckedOrNot(static_cast<int32_t>(wParam));
					return TRUE;
				case IDC_CHECK_ENABLEDOCSWITCHER :
				{
					nppGUI._doTaskList = !nppGUI._doTaskList;
					if (nppGUI._doTaskList)
					{
						::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_STYLEMRU), TRUE);
					}
					else
					{
						nppGUI._styleMRU = false;
						::SendDlgItemMessage(_hSelf, IDC_CHECK_STYLEMRU, BM_SETCHECK, false, 0);
						::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_STYLEMRU), FALSE);
					}
					return TRUE;
				}

				case IDC_CHECK_STYLEMRU :
				{
					nppGUI._styleMRU = isCheckedOrNot(IDC_CHECK_STYLEMRU);
					return TRUE;
				}

				case IDC_CHECK_SHORTTITLE:
				{
					nppGUI._shortTitlebar = isCheckedOrNot(IDC_CHECK_SHORTTITLE);
					HWND grandParent = ::GetParent(_hParent);
					::SendMessage(grandParent, NPPM_INTERNAL_UPDATETITLEBAR, 0, 0);
					return TRUE;
				}

				case IDC_CHECK_DIRECTWRITE_ENABLE:
				{
					nppGUI._writeTechnologyEngine = isCheckedOrNot(IDC_CHECK_DIRECTWRITE_ENABLE) ? directWriteTechnology : defaultTechnology;
					return TRUE;
				}

				case IDC_CHECK_ENABLEDOCPEEKER:
				{
					nppGUI._isDocPeekOnTab = isCheckedOrNot(IDC_CHECK_ENABLEDOCPEEKER);
					return TRUE;
				}

				case IDC_CHECK_ENABLEDOCPEEKONMAP:
				{
					nppGUI._isDocPeekOnMap = isCheckedOrNot(IDC_CHECK_ENABLEDOCPEEKONMAP);
					return TRUE;
				}

				case IDC_CHECK_SAVEALLCONFIRM:
				{
					nppGUI._saveAllConfirm = isCheckedOrNot(IDC_CHECK_SAVEALLCONFIRM);
					return TRUE;
				}

				default:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						if (LOWORD(wParam) == IDC_COMBO_FILEUPDATECHOICE)
						{
							auto index = ::SendDlgItemMessage(_hSelf, IDC_COMBO_FILEUPDATECHOICE, CB_GETCURSEL, 0, 0);
							
							if (index == fileUpdateChoiceEnable || index == fileUpdateChoiceEnable4All)
							{
								bool isSilent = isCheckedOrNot(IDC_CHECK_UPDATESILENTLY);
								bool isGo2End = isCheckedOrNot(IDC_CHECK_UPDATEGOTOEOF);

								int cd = cdDisabled;

								if (index == fileUpdateChoiceEnable4All)
									cd |= cdEnabledOld;
								else
									cd |= cdEnabledNew;

								if (isSilent)
									cd |= cdAutoUpdate;
								if (isGo2End)
									cd |= cdGo2end;

								nppGUI._fileAutoDetection = cd;
								
								::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_UPDATESILENTLY), TRUE);
								::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_UPDATEGOTOEOF), TRUE);
							}
							else if (index == fileUpdateChoiceDisable)
							{
								::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_UPDATESILENTLY), FALSE);
								::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_UPDATEGOTOEOF), FALSE);
								
								nppGUI._fileAutoDetection = cdDisabled;
							}
							
							return TRUE;
						}
					}
				}
			}
		}
	}
	return FALSE;
}


void RecentFilesHistoryDlg::setCustomLen(int val)
{
	::EnableWindow(::GetDlgItem(_hSelf, IDC_CUSTOMIZELENGTHVAL_STATIC), val > 0);
	::SetDlgItemInt(_hSelf, IDC_CUSTOMIZELENGTHVAL_STATIC, val, FALSE);
	::ShowWindow(::GetDlgItem(_hSelf, IDC_CUSTOMIZELENGTHVAL_STATIC), val > 0?SW_SHOW:SW_HIDE);
}

INT_PTR CALLBACK DefaultNewDocDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM)
{
		NppGUI & nppGUI = (NppGUI & )nppParms->getNppGUI();
	NewDocDefaultSettings & ndds = (NewDocDefaultSettings &)nppGUI.getNewDocDefaultSettings();

	switch (message)
	{
		case WM_INITDIALOG:
		{
			int ID2Check = IDC_RADIO_F_WIN;
			switch (ndds._format)
			{
				case EolType::windows:
					ID2Check = IDC_RADIO_F_WIN;
					break;
				case EolType::macos:
					ID2Check = IDC_RADIO_F_MAC;
					break;
				case EolType::unix:
					ID2Check = IDC_RADIO_F_UNIX;
					break;
				case EolType::unknown:
					assert(false);
					break;
			}
			::SendDlgItemMessage(_hSelf, ID2Check, BM_SETCHECK, BST_CHECKED, 0);

			switch (ndds._unicodeMode)
			{
				case uni16BE :
					ID2Check = IDC_RADIO_UCS2BIG;
					break;
				case uni16LE :
					ID2Check = IDC_RADIO_UCS2SMALL;
					break;
				case uniUTF8 :
					ID2Check = IDC_RADIO_UTF8;
					break;
				case uniCookie :
					ID2Check = IDC_RADIO_UTF8SANSBOM;
					break;

				default : //uni8Bit
					ID2Check = IDC_RADIO_ANSI;
			}

			int selIndex = -1;
			generic_string str;
			EncodingMapper& em = EncodingMapper::getInstance();
			for (size_t i = 0, encodingArraySize = sizeof(encodings)/sizeof(int) ; i < encodingArraySize ; ++i)
			{
				int cmdID = em.getIndexFromEncoding(encodings[i]);
				if (cmdID != -1)
				{
					cmdID += IDM_FORMAT_ENCODE;
					getNameStrFromCmd(cmdID, str);
					int index = static_cast<int32_t>(::SendDlgItemMessage(_hSelf, IDC_COMBO_OTHERCP, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(str.c_str())));
					if (ndds._codepage == encodings[i])
						selIndex = index;
					::SendDlgItemMessage(_hSelf, IDC_COMBO_OTHERCP, CB_SETITEMDATA, index, encodings[i]);
				}
			}

			if (ndds._codepage == -1 || selIndex == -1)
			{
				::EnableWindow(::GetDlgItem(_hSelf, IDC_COMBO_OTHERCP), false);
			}
			else
			{
				ID2Check = IDC_RADIO_OTHERCP;
				::SendDlgItemMessage(_hSelf, IDC_COMBO_OTHERCP, CB_SETCURSEL, selIndex, 0);
			}

			::SendDlgItemMessage(_hSelf, ID2Check, BM_SETCHECK, BST_CHECKED, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_OPENANSIASUTF8, BM_SETCHECK, (ID2Check == IDC_RADIO_UTF8SANSBOM && ndds._openAnsiAsUtf8)?BST_CHECKED:BST_UNCHECKED, 0);
			::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_OPENANSIASUTF8), ID2Check == IDC_RADIO_UTF8SANSBOM);

			size_t index = 0;
			for (int i = L_TEXT ; i < nppParms->L_END ; ++i)
			{
				str.clear();
				if (static_cast<LangType>(i) != L_USER)
				{
					int cmdID = nppParms->langTypeToCommandID(static_cast<LangType>(i));
					if ((cmdID != -1))
					{
						getNameStrFromCmd(cmdID, str);
						if (str.length() > 0)
						{
							_langList.push_back(LangID_Name(static_cast<LangType>(i), str));
							::SendDlgItemMessage(_hSelf, IDC_COMBO_DEFAULTLANG, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(str.c_str()));
							if (ndds._lang == i)
								index = _langList.size() - 1;
						}
					}
				}
			}
			::SendDlgItemMessage(_hSelf, IDC_COMBO_DEFAULTLANG, CB_SETCURSEL, index, 0);

			//
			// To avoid the white control background to be displayed in dialog
			//
			ETDTProc enableDlgTheme = (ETDTProc)nppParms->getEnableThemeDlgTexture();
			if (enableDlgTheme)
				enableDlgTheme(_hSelf, ETDT_ENABLETAB);
		}

		case WM_COMMAND:
			switch (wParam)
			{
				case IDC_RADIO_UCS2BIG:
					ndds._unicodeMode = uni16BE;
					ndds._openAnsiAsUtf8 = false;
					makeOpenAnsiAsUtf8(false);
					ndds._codepage = -1;
					::EnableWindow(::GetDlgItem(_hSelf, IDC_COMBO_OTHERCP), false);
					return TRUE;
				case IDC_RADIO_UCS2SMALL:
					ndds._unicodeMode = uni16LE;
					ndds._openAnsiAsUtf8 = false;
					makeOpenAnsiAsUtf8(false);
					ndds._codepage = -1;
					::EnableWindow(::GetDlgItem(_hSelf, IDC_COMBO_OTHERCP), false);
					return TRUE;
				case IDC_RADIO_UTF8:
					ndds._unicodeMode = uniUTF8;
					ndds._openAnsiAsUtf8 = false;
					makeOpenAnsiAsUtf8(false);
					ndds._codepage = -1;
					::EnableWindow(::GetDlgItem(_hSelf, IDC_COMBO_OTHERCP), false);
					return TRUE;
				case IDC_RADIO_UTF8SANSBOM:
					ndds._unicodeMode = uniCookie;
					makeOpenAnsiAsUtf8(true);
					ndds._codepage = -1;
					::EnableWindow(::GetDlgItem(_hSelf, IDC_COMBO_OTHERCP), false);
					return TRUE;
				case IDC_RADIO_ANSI:
					ndds._unicodeMode = uni8Bit;
					ndds._openAnsiAsUtf8 = false;
					makeOpenAnsiAsUtf8(false);
					ndds._codepage = -1;
					::EnableWindow(::GetDlgItem(_hSelf, IDC_COMBO_OTHERCP), false);
					return TRUE;

				case IDC_CHECK_OPENANSIASUTF8 :
					ndds._openAnsiAsUtf8 = (BST_CHECKED == ::SendMessage(::GetDlgItem(_hSelf, IDC_CHECK_OPENANSIASUTF8), BM_GETCHECK, 0, 0));
					return TRUE;

				case IDC_RADIO_OTHERCP :
				{
					ndds._openAnsiAsUtf8 = false;
					makeOpenAnsiAsUtf8(false);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_COMBO_OTHERCP), true);
					auto index = ::SendDlgItemMessage(_hSelf, IDC_COMBO_OTHERCP, CB_GETCURSEL, 0, 0);
					ndds._codepage = static_cast<int32_t>(::SendDlgItemMessage(_hSelf, IDC_COMBO_OTHERCP, CB_GETITEMDATA, index, 0));
					return TRUE;
				}

				case IDC_RADIO_F_MAC:
				{
					ndds._format = EolType::macos;
					return TRUE;
				}
				case IDC_RADIO_F_UNIX:
				{
					ndds._format = EolType::unix;
					return TRUE;
				}
				case IDC_RADIO_F_WIN:
				{
					ndds._format = EolType::windows;
					return TRUE;
				}

				default:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						if (LOWORD(wParam) == IDC_COMBO_DEFAULTLANG)
						{
							auto index = ::SendDlgItemMessage(_hSelf, IDC_COMBO_DEFAULTLANG, CB_GETCURSEL, 0, 0);
							ndds._lang = _langList[index]._id;
							return TRUE;
						}
						else if (LOWORD(wParam) == IDC_COMBO_OTHERCP)
						{
							auto index = ::SendDlgItemMessage(_hSelf, IDC_COMBO_OTHERCP, CB_GETCURSEL, 0, 0);
							ndds._codepage = static_cast<int32_t>(::SendDlgItemMessage(_hSelf, IDC_COMBO_OTHERCP, CB_GETITEMDATA, index, 0));
							return TRUE;
						}
					}
					return FALSE;
				}
			}
	}
 	return FALSE;
}

LONG_PTR DefaultDirectoryDlg::originalComboEditProc = NULL;

void PreferenceDlg::init(HINSTANCE hInst, HWND parent)
{
	Window::init(hInst, parent);
}

void PreferenceDlg::doDialog(bool isRTL)
{
	if (!isCreated())
	{
		create(IDD_PREFERENCE_BOX, isRTL);
		if(CreateFonts(true)) {
			setWindowFont(_hSelf, hFontCategory);
		}
		goToCenter();
	}
	display();
}

void PreferenceDlg::NotifyReturnPressed(){
	if(currentSettingsIndex==3) {
		HWND hCombo = ::GetDlgItem(_defaultDirectoryDlg.getHSelf(), IDDROPFILTER);
		if(IsChild(hCombo, GetFocus())) {
			TCHAR text[1024];
			::SendMessage(hCombo, WM_GETTEXT, 1024 - 1, (LPARAM)text);

			auto i = ::SendMessage(hCombo, CB_FINDSTRINGEXACT, static_cast<WPARAM>(-1), reinterpret_cast<LPARAM>(text));
			if (i != CB_ERR) // found
			{
				::SendMessage(hCombo, CB_DELETESTRING, i, 0);
			}
			
			i = ::SendMessage(hCombo, CB_INSERTSTRING, 0, reinterpret_cast<LPARAM>(text));
			::SendMessage(hCombo, CB_SETCURSEL, i, 0);
						nppParms->_dropFiltersHistoryIdx=0;
			SetFocus(0);
		}
	}
}

extern int saveComboHistory(HWND _hSelf, int id, int maxcount, vector<generic_string> & strings, bool saveEmpty);

std::vector<generic_string> PreferenceDlg::getDropFiltersHistory() {
		if(_defaultDirectoryDlg.isCreated()) {
		saveComboHistory(_defaultDirectoryDlg.getHSelf(), IDDROPFILTER, 10, nppParms->_dropFiltersHistory, 0);
		nppParms->_dropFiltersHistoryIdx = ::SendDlgItemMessage(_defaultDirectoryDlg.getHSelf(), IDDROPFILTER, CB_GETCURSEL, 0, 0);
	}
	return nppParms->_dropFiltersHistory;
}

void PreferenceDlg::buildDropFilters(std::vector<generic_string> & pattern) {
		TCHAR* text=0;
	if(_defaultDirectoryDlg.isCreated()) {
		TCHAR tmp[1024];
		HWND hCombo = ::GetDlgItem(_defaultDirectoryDlg.getHSelf(), IDDROPFILTER);
		::SendMessage(hCombo, WM_GETTEXT, 1024 - 1, (LPARAM)tmp);
		text = tmp;
	} else {
		auto idx = nppParms->_dropFiltersHistoryIdx;
		if(idx>=0&&idx<nppParms->_dropFiltersHistory.size()) {
			text = (TCHAR*)nppParms->_dropFiltersHistory[idx].c_str();
		}
	}
	if(text) {
		TCHAR tmp[MAX_PATH];
		int start=0;
		int i=0, term = lstrlen(text);

		for(;i<=term;i++) {
			if(i==term||text[i]=='\\') {
				if(i) {
					int len=i-start;
					lstrcpyn(tmp, text+start, len+1);
					pattern.push_back(generic_string(tmp));
				}
				start=i+1;
			}
		}
	}
}

LRESULT FAR PASCAL DefaultDirectoryDlg::comboEditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_CHAR && wParam == 0x7F) // ASCII "DEL" (Ctrl+Backspace)
	{
		delLeftWordInEdit(hwnd, 1);
		return 0;
	}
	if (message == CBN_EDITCHANGE)
	{

		//::MessageBox(NULL, TEXT("111"), TEXT(""), MB_OK);
	}
	if (message == WM_KEYDOWN && wParam==VK_RETURN)
	{

		//::MessageBox(NULL, TEXT("111"), TEXT(""), MB_OK);
	}
	return CallWindowProc((WNDPROC)originalComboEditProc, hwnd, message, wParam, lParam);
}

INT_PTR CALLBACK DefaultDirectoryDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM)
{
	NppGUI & nppGUI = (NppGUI & )nppParms->getNppGUI();
	switch (message) 
	{
		case WM_INITDIALOG :
		{
			int ID2Check = 0;
			bool shouldActivated;
			switch (nppGUI._openSaveDir)
			{
				case dir_last :
					ID2Check = IDC_OPENSAVEDIR_REMEMBERLAST_RADIO;
					shouldActivated = false;
					break;
				case dir_userDef :
					ID2Check = IDC_OPENSAVEDIR_ALWAYSON_RADIO;
					shouldActivated = true;
					break;

				default : 
					ID2Check = IDC_OPENSAVEDIR_FOLLOWCURRENT_RADIO;
					shouldActivated = false;
			}
			::SendDlgItemMessage(_hSelf, ID2Check, BM_SETCHECK, BST_CHECKED, 0);
			::SendDlgItemMessage(_hSelf, IDC_OPENSAVEDIR_ALWAYSON_EDIT, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(nppGUI._defaultDir));
			::EnableWindow(::GetDlgItem(_hSelf, IDC_OPENSAVEDIR_ALWAYSON_EDIT), shouldActivated);
			::EnableWindow(::GetDlgItem(_hSelf, IDD_OPENSAVEDIR_ALWAYSON_BROWSE_BUTTON), shouldActivated);

			//
			// To avoid the white control background to be displayed in dialog
			//
			ETDTProc enableDlgTheme = (ETDTProc)nppParms->getEnableThemeDlgTexture();
			if (enableDlgTheme)
				enableDlgTheme(_hSelf, ETDT_ENABLETAB);

			::SendDlgItemMessage(_hSelf, IDC_OPENSAVEDIR_CHECK_USENEWSTYLESAVEDIALOG, BM_SETCHECK, nppGUI._useNewStyleSaveDlg ? BST_CHECKED : BST_UNCHECKED, 0);
			::SendDlgItemMessage(_hSelf, IDC_OPENSAVEDIR_CHECK_DROPOPENFILTER, BM_SETCHECK, nppGUI._dragOpenUseFilter ? BST_CHECKED : BST_UNCHECKED, 0);
			::SendDlgItemMessage(_hSelf, IDC_OPENSAVEDIR_CHECK_DROPOPENRECURSE, BM_SETCHECK, nppGUI._dragOpenRecursive ? BST_CHECKED : BST_UNCHECKED, 0);
			::SendDlgItemMessage(_hSelf, IDC_OPENSAVEDIR_CHECK_DRROPFOLDEROPENFILES, BM_SETCHECK, nppGUI._isFolderDroppedOpenFiles ? BST_CHECKED : BST_UNCHECKED, 0);
		

			HWND hCombo = ::GetDlgItem(_hSelf, IDDROPFILTER);
			auto & strings = nppParms->_dropFiltersHistory;

			for (vector<generic_string>::const_reverse_iterator i = strings.rbegin() ; i != strings.rend(); ++i)
			{
				addText2Combo(i->c_str(), hCombo);
			}

			SetWindowText(hCombo, _T(""));
			::SendMessage(hCombo, CB_SETCURSEL, nppParms->_dropFiltersHistoryIdx, 0);
			HWND hwndEdit = GetWindow(hCombo, GW_CHILD);
			originalComboEditProc = SetWindowLongPtr(hwndEdit, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(comboEditProc));
		}

		case WM_COMMAND : 
		{
			if (HIWORD(wParam) == EN_CHANGE)
			{
				switch (LOWORD(wParam))
				{
					case  IDC_OPENSAVEDIR_ALWAYSON_EDIT:
					{
						TCHAR inputDir[MAX_PATH];
						::SendDlgItemMessage(_hSelf, IDC_OPENSAVEDIR_ALWAYSON_EDIT, WM_GETTEXT, MAX_PATH, reinterpret_cast<LPARAM>(inputDir));
						wcscpy_s(nppGUI._defaultDir, inputDir);
						::ExpandEnvironmentStrings(nppGUI._defaultDir, nppGUI._defaultDirExp, _countof(nppGUI._defaultDirExp));
						nppParms->setWorkingDir(nppGUI._defaultDirExp);
						return TRUE;
					}
				}
			}

			switch (wParam)
			{
				case IDC_OPENSAVEDIR_FOLLOWCURRENT_RADIO:
					nppGUI._openSaveDir = dir_followCurrent;
					::EnableWindow(::GetDlgItem(_hSelf, IDC_OPENSAVEDIR_ALWAYSON_EDIT), false);
					::EnableWindow(::GetDlgItem(_hSelf, IDD_OPENSAVEDIR_ALWAYSON_BROWSE_BUTTON), false);
					return TRUE;
				case IDC_OPENSAVEDIR_REMEMBERLAST_RADIO:
					nppGUI._openSaveDir = dir_last;
					::EnableWindow(::GetDlgItem(_hSelf, IDC_OPENSAVEDIR_ALWAYSON_EDIT), false);
					::EnableWindow(::GetDlgItem(_hSelf, IDD_OPENSAVEDIR_ALWAYSON_BROWSE_BUTTON), false);
					return TRUE;
				case IDC_OPENSAVEDIR_ALWAYSON_RADIO:
					nppGUI._openSaveDir = dir_userDef;
					::EnableWindow(::GetDlgItem(_hSelf, IDC_OPENSAVEDIR_ALWAYSON_EDIT), true);
					::EnableWindow(::GetDlgItem(_hSelf, IDD_OPENSAVEDIR_ALWAYSON_BROWSE_BUTTON), true);
					return TRUE;

				case IDD_OPENSAVEDIR_ALWAYSON_BROWSE_BUTTON :
					folderBrowser(_hSelf, TEXT("Select a folder as default directory"), IDC_OPENSAVEDIR_ALWAYSON_EDIT);
					return TRUE;

				case IDC_OPENSAVEDIR_CHECK_USENEWSTYLESAVEDIALOG:
					nppGUI._useNewStyleSaveDlg = isCheckedOrNot(IDC_OPENSAVEDIR_CHECK_USENEWSTYLESAVEDIALOG);
					return TRUE;

				case IDC_OPENSAVEDIR_CHECK_DROPOPENFILTER:
					nppGUI._dragOpenUseFilter = isCheckedOrNot(IDC_OPENSAVEDIR_CHECK_DROPOPENFILTER);
					return TRUE;

				case IDC_OPENSAVEDIR_CHECK_DROPOPENRECURSE:
					nppGUI._dragOpenRecursive = isCheckedOrNot(IDC_OPENSAVEDIR_CHECK_DROPOPENRECURSE);
					return TRUE;

				case IDC_OPENSAVEDIR_CHECK_DRROPFOLDEROPENFILES:
					nppGUI._isFolderDroppedOpenFiles = isCheckedOrNot(IDC_OPENSAVEDIR_CHECK_DRROPFOLDEROPENFILES);
					return TRUE;

				default:
					return FALSE;
			}
		}
	}
	return FALSE;
}

INT_PTR CALLBACK RecentFilesHistoryDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM)
{
		NppGUI & nppGUI = (NppGUI & )nppParms->getNppGUI();
	NativeLangSpeaker *pNativeSpeaker = nppParms->getNativeLangSpeaker();

	switch (message) 
	{
		case WM_INITDIALOG :
		{
			// Max number recent file setting
			::SetDlgItemInt(_hSelf, IDC_MAXNBFILEVAL_STATIC, nppParms->getNbMaxRecentFile(), FALSE);
			_nbHistoryVal.init(_hInst, _hSelf);
			_nbHistoryVal.create(::GetDlgItem(_hSelf, IDC_MAXNBFILEVAL_STATIC), IDC_MAXNBFILEVAL_STATIC);

			// Check on launch time settings
			::SendDlgItemMessage(_hSelf, IDC_CHECK_DONTCHECKHISTORY, BM_SETCHECK, !nppGUI._checkHistoryFiles, 0);

			// Display in submenu setting
			::SendDlgItemMessage(_hSelf, IDC_CHECK_INSUBMENU, BM_SETCHECK, nppParms->putRecentFileInSubMenu(), 0);

			// Recent File menu entry length setting
			int customLength = nppParms->getRecentFileCustomLength();
			int id = IDC_RADIO_CUSTOMIZELENTH;
			int length = customLength;

			if (customLength == RECENTFILES_SHOWONLYFILENAME)
			{
				id = IDC_RADIO_ONLYFILENAME;
				length = 0;
			}
			else if (customLength == RECENTFILES_SHOWFULLPATH || customLength < 0)
			{
				id = IDC_RADIO_FULLFILENAMEPATH;
				length = 0;
			}
			::SendDlgItemMessage(_hSelf, id, BM_SETCHECK, BST_CHECKED, 0);
			::EnableWindow(::GetDlgItem(_hSelf, IDC_CUSTOMIZELENGTHVAL_STATIC), id == IDC_RADIO_CUSTOMIZELENTH);
			::ShowWindow(::GetDlgItem(_hSelf, IDC_CUSTOMIZELENGTHVAL_STATIC), id == IDC_RADIO_CUSTOMIZELENTH?SW_SHOW:SW_HIDE);
			
			::SetDlgItemInt(_hSelf, IDC_CUSTOMIZELENGTHVAL_STATIC, length, FALSE);
			_customLenVal.init(_hInst, _hSelf);
			_customLenVal.create(::GetDlgItem(_hSelf, IDC_CUSTOMIZELENGTHVAL_STATIC), nullptr);

			//
			// To avoid the white control background to be displayed in dialog
			//
			ETDTProc enableDlgTheme = (ETDTProc)nppParms->getEnableThemeDlgTexture();
			if (enableDlgTheme)
				enableDlgTheme(_hSelf, ETDT_ENABLETAB);
		}

		case WM_COMMAND : 
		{
			switch (wParam)
			{
				case IDC_CHECK_DONTCHECKHISTORY:
					nppGUI._checkHistoryFiles = !isCheckedOrNot(IDC_CHECK_DONTCHECKHISTORY);
					return TRUE;

				case IDC_MAXNBFILEVAL_STATIC:
				{
					generic_string staticText = pNativeSpeaker->getLocalizedStrFromID("recent-file-history-maxfile", TEXT("Max File: "));
					ValueDlg nbFileMaxDlg;
					nbFileMaxDlg.init(NULL, _hSelf, nppParms->getNbMaxRecentFile(), staticText.c_str());
					
					POINT p;
					::GetCursorPos(&p);

					int nbMaxFile = nbFileMaxDlg.doDialog(p);
					if (nbMaxFile != -1)
					{
						if (nbMaxFile > NB_MAX_LRF_FILE)
							nbMaxFile = NB_MAX_LRF_FILE;
						
						nppParms->setNbMaxRecentFile(nbMaxFile);
						::SetDlgItemInt(_hSelf, IDC_MAXNBFILEVAL_STATIC, nbMaxFile, FALSE);

						// Validate modified value
						::SendMessage(::GetParent(_hParent), NPPM_INTERNAL_SETTING_HISTORY_SIZE, 0, 0);
					}
					return TRUE;
				}

				case IDC_CHECK_INSUBMENU:
					nppParms->setPutRecentFileInSubMenu(isCheckedOrNot(IDC_CHECK_INSUBMENU));
					::SendMessage(::GetParent(_hParent), NPPM_INTERNAL_RECENTFILELIST_SWITCH, 0, 0);
					return TRUE;

				case IDC_RADIO_ONLYFILENAME:
					setCustomLen(0);
					nppParms->setRecentFileCustomLength(0);
					::SendMessage(::GetParent(_hParent), NPPM_INTERNAL_RECENTFILELIST_UPDATE, 0, 0);
					return TRUE;
				case IDC_RADIO_FULLFILENAMEPATH:
					setCustomLen(0);
					nppParms->setRecentFileCustomLength(-1);
					::SendMessage(::GetParent(_hParent), NPPM_INTERNAL_RECENTFILELIST_UPDATE, 0, 0);
					return TRUE;
				case IDC_RADIO_CUSTOMIZELENTH:
				{
					int len = nppParms->getRecentFileCustomLength();
					if (len <= 0)
					{
						setCustomLen(100);
						nppParms->setRecentFileCustomLength(100);
						::SendMessage(::GetParent(_hParent), NPPM_INTERNAL_RECENTFILELIST_UPDATE, 0, 0);
					}
					return TRUE;
				}

				case IDC_CUSTOMIZELENGTHVAL_STATIC:
				{
					ValueDlg customLengthDlg;
					customLengthDlg.init(NULL, _hSelf, nppParms->getRecentFileCustomLength(), TEXT("Length: "));
					customLengthDlg.setNBNumber(3);

					POINT p;
					::GetCursorPos(&p);

					int size = customLengthDlg.doDialog(p);
					if (size != -1)
					{
						::SetDlgItemInt(_hSelf, IDC_CUSTOMIZELENGTHVAL_STATIC, size, FALSE);
						nppParms->setRecentFileCustomLength(size);
						::SendMessage(::GetParent(_hParent), NPPM_INTERNAL_RECENTFILELIST_UPDATE, 0, 0);
					}
					return TRUE;
				}
				default:
					return FALSE;
			}
		}
	}
	return FALSE;
}

INT_PTR CALLBACK LangMenuDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
		NppGUI & nppGUI = const_cast<NppGUI &>(nppParms->getNppGUI());
	NativeLangSpeaker *pNativeSpeaker = nppParms->getNativeLangSpeaker();

	switch (message) 
	{
		case WM_INITDIALOG :
		{
			//
			// Lang Menu
			//
			for (int i = L_TEXT ; i < nppParms->L_END ; ++i)
			{
				generic_string str;
				if (static_cast<LangType>(i) != L_USER)
				{
					int cmdID = nppParms->langTypeToCommandID(static_cast<LangType>(i));
					if ((cmdID != -1))
					{
						getNameStrFromCmd(cmdID, str);
						if (str.length() > 0)
						{
							_langList.push_back(LangMenuItem(static_cast<LangType>(i), cmdID, str));
							::SendDlgItemMessage(_hSelf, IDC_LIST_ENABLEDLANG, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(str.c_str()));
						}
					}
				}
			}

			for (size_t i = 0, len = nppGUI._excludedLangList.size(); i < len ; ++i)
			{
				::SendDlgItemMessage(_hSelf, IDC_LIST_DISABLEDLANG, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(nppGUI._excludedLangList[i]._langName.c_str()));
			}

			::SendDlgItemMessage(_hSelf, IDC_CHECK_LANGMENUCOMPACT, BM_SETCHECK, nppGUI._isLangMenuCompact?BST_CHECKED:BST_UNCHECKED, 0);
			::EnableWindow(::GetDlgItem(_hSelf, IDC_BUTTON_REMOVE), FALSE);
			::EnableWindow(::GetDlgItem(_hSelf, IDC_BUTTON_RESTORE), FALSE);

			
			//
			// Tab settings
			//
			::SetDlgItemInt(_hSelf, IDC_TABSIZEVAL_STATIC, nppGUI._tabSize, FALSE);

			_tabSizeVal.init(_hInst, _hSelf);
			_tabSizeVal.create(::GetDlgItem(_hSelf, IDC_TABSIZEVAL_STATIC), IDC_TABSIZEVAL_STATIC);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_REPLACEBYSPACE, BM_SETCHECK, nppGUI._tabReplacedBySpace, 0);

			::SendDlgItemMessage(_hSelf, IDC_LIST_TABSETTNG, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("[Default]")));
			const int nbLang = nppParms->getNbLang();
			for (int i = 0; i < nbLang; ++i)
			{
				::SendDlgItemMessage(_hSelf, IDC_LIST_TABSETTNG, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(nppParms->getLangFromIndex(i)->_langName.c_str()));
			}
			const int index2Begin = 0;
			::SendDlgItemMessage(_hSelf, IDC_LIST_TABSETTNG, LB_SETCURSEL, 0, index2Begin);
			::ShowWindow(::GetDlgItem(_hSelf, IDC_GR_TABVALUE_STATIC), SW_HIDE);
			::ShowWindow(::GetDlgItem(_hSelf, IDC_CHECK_DEFAULTTABVALUE), SW_HIDE);
			::EnableWindow(::GetDlgItem(_hSelf, IDC_TABSIZEVAL_DISABLE_STATIC), FALSE);
			::ShowWindow(::GetDlgItem(_hSelf, IDC_TABSIZEVAL_DISABLE_STATIC), SW_HIDE);

			ETDTProc enableDlgTheme = reinterpret_cast<ETDTProc>(nppParms->getEnableThemeDlgTexture());
			if (enableDlgTheme)
				enableDlgTheme(_hSelf, ETDT_ENABLETAB);

			::SendDlgItemMessage(_hSelf, IDC_CHECK_BACKSLASHISESCAPECHARACTERFORSQL, BM_SETCHECK, nppGUI._backSlashIsEscapeCharacterForSql, 0);

			return TRUE;
		}

		case WM_COMMAND : 
		{
			if (HIWORD(wParam) == LBN_SELCHANGE)
            {
				// Lang Menu
				if (LOWORD(wParam) == IDC_LIST_DISABLEDLANG || LOWORD(wParam) == IDC_LIST_ENABLEDLANG)
				{
					int idButton2Enable;
					int idButton2Disable;

					if (LOWORD(wParam) == IDC_LIST_ENABLEDLANG)
					{
						idButton2Enable = IDC_BUTTON_REMOVE;
						idButton2Disable = IDC_BUTTON_RESTORE;
					}
					else //IDC_LIST_DISABLEDLANG
					{
						idButton2Enable = IDC_BUTTON_RESTORE;
						idButton2Disable = IDC_BUTTON_REMOVE;
					}

					auto i = ::SendDlgItemMessage(_hSelf, LOWORD(wParam), LB_GETCURSEL, 0, 0);
					if (i != LB_ERR)
					{
						::EnableWindow(::GetDlgItem(_hSelf, idButton2Enable), TRUE);
						int idListbox2Disable = (LOWORD(wParam)== IDC_LIST_ENABLEDLANG)?IDC_LIST_DISABLEDLANG:IDC_LIST_ENABLEDLANG;
						::SendDlgItemMessage(_hSelf, idListbox2Disable, LB_SETCURSEL, static_cast<WPARAM>(-1), 0);
						::EnableWindow(::GetDlgItem(_hSelf, idButton2Disable), FALSE);
					}
					return TRUE;

				}
				// Tab setting
				else if (LOWORD(wParam) == IDC_LIST_TABSETTNG)
				{
					auto index = ::SendDlgItemMessage(_hSelf, IDC_LIST_TABSETTNG, LB_GETCURSEL, 0, 0);
					if (index == LB_ERR)
						return FALSE;
					::ShowWindow(::GetDlgItem(_hSelf, IDC_GR_TABVALUE_STATIC), index ? SW_SHOW : SW_HIDE);
					::ShowWindow(::GetDlgItem(_hSelf, IDC_CHECK_DEFAULTTABVALUE), index ? SW_SHOW : SW_HIDE);

					if (index)
					{
						Lang *lang = nppParms->getLangFromIndex(index - 1);
						if (!lang) return FALSE;
						bool useDefaultTab = (lang->_tabSize == -1 || lang->_tabSize == 0);

						::SendMessage(::GetDlgItem(_hSelf, IDC_CHECK_DEFAULTTABVALUE), BM_SETCHECK, useDefaultTab, 0);
						::EnableWindow(::GetDlgItem(_hSelf, IDC_TABSIZE_STATIC), !useDefaultTab);

						int size = useDefaultTab ? nppGUI._tabSize : lang->_tabSize;
						::SetDlgItemInt(_hSelf, IDC_TABSIZEVAL_STATIC, size, FALSE);
						::SetDlgItemInt(_hSelf, IDC_TABSIZEVAL_DISABLE_STATIC, size, FALSE);

						::EnableWindow(::GetDlgItem(_hSelf, IDC_TABSIZEVAL_STATIC), !useDefaultTab);
						::ShowWindow(::GetDlgItem(_hSelf, IDC_TABSIZEVAL_DISABLE_STATIC), useDefaultTab);
						::ShowWindow(::GetDlgItem(_hSelf, IDC_TABSIZEVAL_STATIC), !useDefaultTab);
						::SendMessage(::GetDlgItem(_hSelf, IDC_CHECK_REPLACEBYSPACE), BM_SETCHECK, useDefaultTab ? nppGUI._tabReplacedBySpace : lang->_isTabReplacedBySpace, 0);
						::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_REPLACEBYSPACE), !useDefaultTab);

						if (!useDefaultTab)
						{
							::SetDlgItemInt(_hSelf, IDC_TABSIZEVAL_STATIC, lang->_tabSize, FALSE);
							::SendMessage(::GetDlgItem(_hSelf, IDC_CHECK_REPLACEBYSPACE), BM_SETCHECK, lang->_isTabReplacedBySpace, 0);
						}
					}
					else
					{
						::EnableWindow(::GetDlgItem(_hSelf, IDC_TABSIZE_STATIC), TRUE);
						::EnableWindow(::GetDlgItem(_hSelf, IDC_TABSIZEVAL_STATIC), TRUE);
						::ShowWindow(::GetDlgItem(_hSelf, IDC_TABSIZEVAL_STATIC), SW_SHOW);
						::SetDlgItemInt(_hSelf, IDC_TABSIZEVAL_STATIC, nppGUI._tabSize, FALSE);
						::ShowWindow(::GetDlgItem(_hSelf, IDC_TABSIZEVAL_DISABLE_STATIC), SW_HIDE);
						::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_REPLACEBYSPACE), TRUE);
						::SendMessage(::GetDlgItem(_hSelf, IDC_CHECK_REPLACEBYSPACE), BM_SETCHECK, nppGUI._tabReplacedBySpace, 0);
					}

					return TRUE;
				}
            }

			// Check if it is double click
			else if (HIWORD(wParam) == LBN_DBLCLK)
			{
				// Lang Menu
				if (LOWORD(wParam) == IDC_LIST_DISABLEDLANG || LOWORD(wParam) == IDC_LIST_ENABLEDLANG)
				{
					// On double click an item, the item should be moved
					// from one list to other list

					HWND(lParam) == ::GetDlgItem(_hSelf, IDC_LIST_ENABLEDLANG) ?
						::SendMessage(_hSelf, WM_COMMAND, IDC_BUTTON_REMOVE, 0) :
						::SendMessage(_hSelf, WM_COMMAND, IDC_BUTTON_RESTORE, 0);
					return TRUE;
				}

				// Tab setting - Double click is not used at this moment
				/*else if (LOWORD(wParam) == IDC_LIST_TABSETTNG)
				{
				}*/
			}

			switch (wParam)
            {
				//
				// Lang Menu
				//
				case IDC_CHECK_LANGMENUCOMPACT : 
				{
					nppGUI._isLangMenuCompact = (BST_CHECKED == ::SendMessage(::GetDlgItem(_hSelf, IDC_CHECK_LANGMENUCOMPACT), BM_GETCHECK, 0, 0));
					::MessageBox(_hSelf, 
						nppGUI._isLangMenuCompact?TEXT("This option will be enabled on the next launch."):TEXT("This option will be disabled on the next launch."), 
						TEXT("Compact Language Menu"), MB_OK);
					return TRUE;
				}
				
				case IDC_CHECK_BACKSLASHISESCAPECHARACTERFORSQL:
				{
					nppGUI._backSlashIsEscapeCharacterForSql = isCheckedOrNot(IDC_CHECK_BACKSLASHISESCAPECHARACTERFORSQL);
					return TRUE;
				}

				case IDC_BUTTON_RESTORE :
				case IDC_BUTTON_REMOVE :
				{
					int list2Remove, list2Add, idButton2Enable, idButton2Disable;
					vector<LangMenuItem> *pSrcLst, *pDestLst;

					if (LOWORD(wParam)==IDC_BUTTON_REMOVE)
					{
						list2Remove = IDC_LIST_ENABLEDLANG;
						list2Add = IDC_LIST_DISABLEDLANG;
						idButton2Enable = IDC_BUTTON_RESTORE;
						idButton2Disable = IDC_BUTTON_REMOVE;
						pSrcLst = &_langList;
						pDestLst = &nppGUI._excludedLangList;
					}
					else
					{
						list2Remove = IDC_LIST_DISABLEDLANG;
						list2Add = IDC_LIST_ENABLEDLANG;
						idButton2Enable = IDC_BUTTON_REMOVE;
						idButton2Disable = IDC_BUTTON_RESTORE;
						pSrcLst = &nppGUI._excludedLangList;
						pDestLst = &_langList;
					}
					size_t iRemove = ::SendDlgItemMessage(_hSelf, list2Remove, LB_GETCURSEL, 0, 0);
					if (iRemove == -1)
						return TRUE;

					const size_t sL = 31;
					TCHAR s[sL + 1];
					auto lbTextLen = ::SendDlgItemMessage(_hSelf, list2Remove, LB_GETTEXTLEN, iRemove, 0);
					if (lbTextLen > sL)
						return TRUE;

					::SendDlgItemMessage(_hSelf, list2Remove, LB_GETTEXT, iRemove, reinterpret_cast<LPARAM>(s));

					LangMenuItem lmi = pSrcLst->at(iRemove);
					vector<LangMenuItem>::iterator lang2Remove = pSrcLst->begin() + iRemove;
					pSrcLst->erase(lang2Remove);

					auto iAdd = ::SendDlgItemMessage(_hSelf, list2Add, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(s));
					::SendDlgItemMessage(_hSelf, list2Remove, LB_DELETESTRING, iRemove, 0);
					pDestLst->push_back(lmi);

					::SendDlgItemMessage(_hSelf, list2Add, LB_SETCURSEL, iAdd, 0);
					::SendDlgItemMessage(_hSelf, list2Remove, LB_SETCURSEL, static_cast<WPARAM>(-1), 0);
					::EnableWindow(::GetDlgItem(_hSelf, idButton2Enable), TRUE);
					::EnableWindow(::GetDlgItem(_hSelf, idButton2Disable), FALSE);

					if ((lmi._langType >= L_EXTERNAL) && (lmi._langType < nppParms->L_END))
					{
						bool found(false);
						for (size_t x = 0; x < nppParms->getExternalLexerDoc()->size() && !found; ++x)
						{
							TiXmlNode *lexersRoot = nppParms->getExternalLexerDoc()->at(x)->FirstChild(TEXT("NotepadPlus"))->FirstChildElement(TEXT("LexerStyles"));
							for (TiXmlNode *childNode = lexersRoot->FirstChildElement(TEXT("LexerType"));
								childNode ;
								childNode = childNode->NextSibling(TEXT("LexerType")))
							{
								TiXmlElement *element = childNode->ToElement();

								if (generic_string(element->Attribute(TEXT("name"))) == lmi._langName)
								{
									element->SetAttribute(TEXT("excluded"), (LOWORD(wParam)==IDC_BUTTON_REMOVE)?TEXT("yes"):TEXT("no"));
									nppParms->getExternalLexerDoc()->at(x)->SaveFile();
									found = true;
									break;
								}
							}
						}
					}

					HWND grandParent = ::GetParent(_hParent);

					if (LOWORD(wParam)==IDC_BUTTON_REMOVE)
					{
						HMENU menu2remove = reinterpret_cast<HMENU>(::SendMessage(grandParent, NPPM_INTERNAL_GETMENU, 0, 0));
						::DeleteMenu(menu2remove, lmi._cmdID, MF_BYCOMMAND);
					}
					else
					{
						HMENU menu = HMENU(::SendMessage(grandParent, NPPM_INTERNAL_GETMENU, 0, 0));
						HMENU subMenu = ::GetSubMenu(menu, MENUINDEX_LANGUAGE);

						// Add back a languge menu item always before the 3 last items:
						// 1. -----------------------
						// 2. Define your language...
						// 3. User-Defined
						int nbItem = ::GetMenuItemCount(subMenu);

						if (nbItem < 3)
							return FALSE;

						::InsertMenu(subMenu, nbItem - 3, MF_BYPOSITION, lmi._cmdID, lmi._langName.c_str());
					}
					::DrawMenuBar(grandParent);
					return TRUE;
				}

				//
				// Tab setting
				//
				case IDC_TABSIZEVAL_STATIC:
				{
					generic_string staticText = pNativeSpeaker->getLocalizedStrFromID("language-tabsize", TEXT("Tab Size: "));
					ValueDlg tabSizeDlg;
					tabSizeDlg.init(_hInst, _hParent, nppGUI._tabSize, staticText.c_str());
					POINT p;
					::GetCursorPos(&p);
					int size = tabSizeDlg.doDialog(p);

					//Tab size 0 removal
					if (size <= 0) return FALSE;

					::SetDlgItemInt(_hSelf, IDC_TABSIZEVAL_STATIC, size, FALSE);
					::SetDlgItemInt(_hSelf, IDC_TABSIZEVAL_DISABLE_STATIC, size, FALSE);

					auto index = ::SendDlgItemMessage(_hSelf, IDC_LIST_TABSETTNG, LB_GETCURSEL, 0, 0);
					if (index == LB_ERR) return FALSE;

					if (index != 0)
					{
						Lang *lang = nppParms->getLangFromIndex(index - 1);
						if (!lang) return FALSE;
						if (lang->_langID == L_JS)
						{
							Lang *ljs = nppParms->getLangFromID(L_JAVASCRIPT);
							ljs->_tabSize = size;
						}
						else if (lang->_langID == L_JAVASCRIPT)
						{
							Lang *ljavascript = nppParms->getLangFromID(L_JS);
							ljavascript->_tabSize = size;
						}

						lang->_tabSize = size;

						// write in langs.xml
						nppParms->insertTabInfo(lang->getLangName(), lang->getTabInfo());
					}
					else
					{
						nppGUI._tabSize = size;
					}

					::SendMessage(::GetParent(_hParent), NPPM_INTERNAL_SETTING_TAB_SIZE, 0, 0);
					return TRUE;
				}

				case IDC_CHECK_REPLACEBYSPACE:
				{
					bool isTabReplacedBySpace = BST_CHECKED == ::SendMessage(::GetDlgItem(_hSelf, IDC_CHECK_REPLACEBYSPACE), BM_GETCHECK, 0, 0);
					auto index = ::SendDlgItemMessage(_hSelf, IDC_LIST_TABSETTNG, LB_GETCURSEL, 0, 0);
					if (index == LB_ERR) return FALSE;
					if (index != 0)
					{
						Lang *lang = nppParms->getLangFromIndex(index - 1);
						if (!lang) return FALSE;
						if (!lang->_tabSize || lang->_tabSize == -1)
							lang->_tabSize = nppGUI._tabSize;

						if (lang->_langID == L_JS)
						{
							Lang *ljs = nppParms->getLangFromID(L_JAVASCRIPT);
							ljs->_isTabReplacedBySpace = isTabReplacedBySpace;
						}
						else if (lang->_langID == L_JAVASCRIPT)
						{
							Lang *ljavascript = nppParms->getLangFromID(L_JS);
							ljavascript->_isTabReplacedBySpace = isTabReplacedBySpace;
						}

						lang->_isTabReplacedBySpace = isTabReplacedBySpace;

						// write in langs.xml
						nppParms->insertTabInfo(lang->getLangName(), lang->getTabInfo());
					}
					else
					{
						nppGUI._tabReplacedBySpace = isTabReplacedBySpace;
					}
					::SendMessage(::GetParent(_hParent), NPPM_INTERNAL_SETTING_TAB_REPLCESPACE, 0, 0);
					return TRUE;
				}

				case IDC_CHECK_DEFAULTTABVALUE:
				{
					bool useDefaultTab = BST_CHECKED == ::SendMessage(::GetDlgItem(_hSelf, IDC_CHECK_DEFAULTTABVALUE), BM_GETCHECK, 0, 0);
					auto index = ::SendDlgItemMessage(_hSelf, IDC_LIST_TABSETTNG, LB_GETCURSEL, 0, 0);
					if (index == LB_ERR || index == 0) // index == 0 shouldn't happen
						return FALSE;

					Lang *lang = nppParms->getLangFromIndex(index - 1);
					if (!lang)
						return FALSE;

					//- Set tab setting in choosed language
					lang->_tabSize = useDefaultTab ? 0 : nppGUI._tabSize;
					lang->_isTabReplacedBySpace = useDefaultTab ? false : nppGUI._tabReplacedBySpace;

					//- set visual effect
					::EnableWindow(::GetDlgItem(_hSelf, IDC_TABSIZE_STATIC), !useDefaultTab);
					::SetDlgItemInt(_hSelf, IDC_TABSIZEVAL_STATIC, useDefaultTab ? nppGUI._tabSize : lang->_tabSize, FALSE);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_TABSIZEVAL_STATIC), !useDefaultTab);
					::ShowWindow(::GetDlgItem(_hSelf, IDC_TABSIZEVAL_DISABLE_STATIC), useDefaultTab);
					::ShowWindow(::GetDlgItem(_hSelf, IDC_TABSIZEVAL_STATIC), !useDefaultTab);
					::SendMessage(::GetDlgItem(_hSelf, IDC_CHECK_REPLACEBYSPACE), BM_SETCHECK, useDefaultTab ? nppGUI._tabReplacedBySpace : lang->_isTabReplacedBySpace, 0);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_REPLACEBYSPACE), !useDefaultTab);

					// write in langs.xml
					if (useDefaultTab)
						nppParms->insertTabInfo(lang->getLangName(), -1);

					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

INT_PTR CALLBACK Highlighting::run_dlgProc(UINT message, WPARAM wParam, LPARAM/* lParam*/)
{
		NppGUI & nppGUI = (NppGUI & )nppParms->getNppGUI();

	switch (message) 
	{
		case WM_INITDIALOG :
		{
			::SendDlgItemMessage(_hSelf, IDC_CHECK_MARKALLCASESENSITIVE, BM_SETCHECK, nppGUI._markAllCaseSensitive, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_MARKALLWHOLEWORDONLY, BM_SETCHECK, nppGUI._markAllWordOnly, 0);

			::SendDlgItemMessage(_hSelf, IDC_CHECK_ENABLSMARTHILITE, BM_SETCHECK, nppGUI._enableSmartHilite, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_SMARTHILITECASESENSITIVE, BM_SETCHECK, nppGUI._smartHiliteCaseSensitive, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_SMARTHILITEWHOLEWORDONLY, BM_SETCHECK, nppGUI._smartHiliteWordOnly, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_SMARTHILITEUSEFINDSETTINGS, BM_SETCHECK, nppGUI._smartHiliteUseFindSettings, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_SMARTHILITEANOTHERRVIEW, BM_SETCHECK, nppGUI._smartHiliteOnAnotherView, 0);

			::SendDlgItemMessage(_hSelf, IDC_CHECK_ENABLTAGSMATCHHILITE, BM_SETCHECK, nppGUI._enableTagsMatchHilite, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_ENABLTAGATTRHILITE, BM_SETCHECK, nppGUI._enableTagAttrsHilite, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_HIGHLITENONEHTMLZONE, BM_SETCHECK, nppGUI._enableHiliteNonHTMLZone, 0);

			::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_ENABLTAGATTRHILITE), nppGUI._enableTagsMatchHilite);
			::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_HIGHLITENONEHTMLZONE), nppGUI._enableTagsMatchHilite);

			::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_SMARTHILITECASESENSITIVE), nppGUI._enableSmartHilite);
			::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_SMARTHILITEWHOLEWORDONLY), nppGUI._enableSmartHilite);
			::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_SMARTHILITEUSEFINDSETTINGS), nppGUI._enableSmartHilite);
			::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_SMARTHILITEANOTHERRVIEW), nppGUI._enableSmartHilite);

			ETDTProc enableDlgTheme = reinterpret_cast<ETDTProc>(nppParms->getEnableThemeDlgTexture());
			if (enableDlgTheme)
				enableDlgTheme(_hSelf, ETDT_ENABLETAB);

			return TRUE;
		}

		case WM_COMMAND : 
		{
			switch (wParam)
            {
				case IDC_CHECK_MARKALLCASESENSITIVE:
				{
					nppGUI._markAllCaseSensitive = isCheckedOrNot(IDC_CHECK_MARKALLCASESENSITIVE);
					HWND grandParent = ::GetParent(_hParent);
					::SendMessage(grandParent, NPPM_INTERNAL_CLEARINDICATOR, 0, 0);
					return TRUE;
				}

				case IDC_CHECK_MARKALLWHOLEWORDONLY:
				{
					nppGUI._markAllWordOnly = isCheckedOrNot(IDC_CHECK_MARKALLWHOLEWORDONLY);
					HWND grandParent = ::GetParent(_hParent);
					::SendMessage(grandParent, NPPM_INTERNAL_CLEARINDICATOR, 0, 0);
					return TRUE;
				}

				case IDC_CHECK_ENABLSMARTHILITE:
				{
					nppGUI._enableSmartHilite = !nppGUI._enableSmartHilite;
					if (!nppGUI._enableSmartHilite)
					{
						//HWND grandParent = ::GetParent(_hParent);
						//::SendMessage(grandParent, NPPM_INTERNAL_CLEARINDICATOR, 0, 0);
					}
					::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_SMARTHILITECASESENSITIVE), nppGUI._enableSmartHilite);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_SMARTHILITEWHOLEWORDONLY), nppGUI._enableSmartHilite);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_SMARTHILITEUSEFINDSETTINGS), nppGUI._enableSmartHilite);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_SMARTHILITEANOTHERRVIEW), nppGUI._enableSmartHilite);
					HWND grandParent = ::GetParent(_hParent);
					::SendMessage(grandParent, NPPM_INTERNAL_CLEARINDICATOR, 0, 0);
					return TRUE;
				}

				case IDC_CHECK_SMARTHILITECASESENSITIVE:
				{
					nppGUI._smartHiliteCaseSensitive = isCheckedOrNot(IDC_CHECK_SMARTHILITECASESENSITIVE);
					if (nppGUI._smartHiliteCaseSensitive)
					{
						::SendDlgItemMessage(_hSelf, IDC_CHECK_SMARTHILITEUSEFINDSETTINGS, BM_SETCHECK, false, 0);
						nppGUI._smartHiliteUseFindSettings = false;
					}
					HWND grandParent = ::GetParent(_hParent);
					::SendMessage(grandParent, NPPM_INTERNAL_CLEARINDICATOR, 0, 0);
					return TRUE;
				}
				case IDC_CHECK_SMARTHILITEWHOLEWORDONLY:
				{
					nppGUI._smartHiliteWordOnly = isCheckedOrNot(IDC_CHECK_SMARTHILITEWHOLEWORDONLY);
					if (nppGUI._smartHiliteWordOnly)
					{
						::SendDlgItemMessage(_hSelf, IDC_CHECK_SMARTHILITEUSEFINDSETTINGS, BM_SETCHECK, false, 0);
						nppGUI._smartHiliteUseFindSettings = false;
					}
					HWND grandParent = ::GetParent(_hParent);
					::SendMessage(grandParent, NPPM_INTERNAL_CLEARINDICATOR, 0, 0);
					return TRUE;
				}
				case IDC_CHECK_SMARTHILITEUSEFINDSETTINGS:
				{
					nppGUI._smartHiliteUseFindSettings = isCheckedOrNot(IDC_CHECK_SMARTHILITEUSEFINDSETTINGS);
					if (nppGUI._smartHiliteUseFindSettings)
					{
						::SendDlgItemMessage(_hSelf, IDC_CHECK_SMARTHILITECASESENSITIVE, BM_SETCHECK, false, 0);
						::SendDlgItemMessage(_hSelf, IDC_CHECK_SMARTHILITEWHOLEWORDONLY, BM_SETCHECK, false, 0);
						nppGUI._smartHiliteCaseSensitive = false;
						nppGUI._smartHiliteWordOnly = false;
					}
					HWND grandParent = ::GetParent(_hParent);
					::SendMessage(grandParent, NPPM_INTERNAL_CLEARINDICATOR, 0, 0);
					return TRUE;
				}
				case IDC_CHECK_SMARTHILITEANOTHERRVIEW:
				{
					nppGUI._smartHiliteOnAnotherView = isCheckedOrNot(IDC_CHECK_SMARTHILITEANOTHERRVIEW);

					HWND grandParent = ::GetParent(_hParent);
					::SendMessage(grandParent, NPPM_INTERNAL_CLEARINDICATOR, 0, 0);
					return TRUE;
				}

				case IDC_CHECK_ENABLTAGSMATCHHILITE:
				{
					nppGUI._enableTagsMatchHilite = !nppGUI._enableTagsMatchHilite;
					if (!nppGUI._enableTagsMatchHilite)
					{
						HWND grandParent = ::GetParent(_hParent);
						::SendMessage(grandParent, NPPM_INTERNAL_CLEARINDICATORTAGMATCH, 0, 0);
					}
					::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_ENABLTAGATTRHILITE), nppGUI._enableTagsMatchHilite);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_HIGHLITENONEHTMLZONE), nppGUI._enableTagsMatchHilite);
					return TRUE;
				}

				case IDC_CHECK_ENABLTAGATTRHILITE:
				{
					nppGUI._enableTagAttrsHilite = !nppGUI._enableTagAttrsHilite;
					if (!nppGUI._enableTagAttrsHilite)
					{
						HWND grandParent = ::GetParent(_hParent);
						::SendMessage(grandParent, NPPM_INTERNAL_CLEARINDICATORTAGATTR, 0, 0);
					}
					return TRUE;
				}

				case IDC_CHECK_HIGHLITENONEHTMLZONE:
				{
					nppGUI._enableHiliteNonHTMLZone = !nppGUI._enableHiliteNonHTMLZone;
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

INT_PTR CALLBACK PrintSettingsDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM)
{
		NppGUI & nppGUI = (NppGUI & )nppParms->getNppGUI();

	switch (message) 
	{
		case WM_INITDIALOG :
		{
			bool printLN = nppGUI._printSettings._printLineNumber;
			::SendDlgItemMessage(_hSelf, IDC_CHECK_PRINTLINENUM, BM_SETCHECK, printLN, 0);

			int ID2Check = 0;
			switch (nppGUI._printSettings._printOption)
			{
				case SC_PRINT_NORMAL :
					ID2Check = IDC_RADIO_WYSIWYG;
					break;
				case SC_PRINT_INVERTLIGHT :
					ID2Check = IDC_RADIO_INVERT;
					break;
				case SC_PRINT_BLACKONWHITE :
					ID2Check = IDC_RADIO_BW;
					break;
				case SC_PRINT_COLOURONWHITE :
					ID2Check = IDC_RADIO_NOBG;
					break;
			}
			::SendDlgItemMessage(_hSelf, ID2Check, BM_SETCHECK, BST_CHECKED, 0);

			::SetDlgItemInt(_hSelf, IDC_EDIT_ML, nppGUI._printSettings._marge.left, FALSE);
			::SetDlgItemInt(_hSelf, IDC_EDIT_MR, nppGUI._printSettings._marge.right, FALSE);
			::SetDlgItemInt(_hSelf, IDC_EDIT_MT, nppGUI._printSettings._marge.top, FALSE);
			::SetDlgItemInt(_hSelf, IDC_EDIT_MB,  nppGUI._printSettings._marge.bottom, FALSE);
			::SetDlgItemText(_hSelf, IDC_EDIT_HLEFT, nppGUI._printSettings._headerLeft.c_str());
			::SetDlgItemText(_hSelf, IDC_EDIT_HMIDDLE, nppGUI._printSettings._headerMiddle.c_str());
			::SetDlgItemText(_hSelf, IDC_EDIT_HRIGHT, nppGUI._printSettings._headerRight.c_str());
			::SetDlgItemText(_hSelf, IDC_EDIT_FLEFT, nppGUI._printSettings._footerLeft.c_str());
			::SetDlgItemText(_hSelf, IDC_EDIT_FMIDDLE, nppGUI._printSettings._footerMiddle.c_str());
			::SetDlgItemText(_hSelf, IDC_EDIT_FRIGHT, nppGUI._printSettings._footerRight.c_str());

			TCHAR intStr[5];
			for (size_t i = 6 ; i < 15 ; ++i)
			{
				wsprintf(intStr, TEXT("%d"), i);
				::SendDlgItemMessage(_hSelf, IDC_COMBO_HFONTSIZE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(intStr));
				::SendDlgItemMessage(_hSelf, IDC_COMBO_FFONTSIZE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(intStr));
			}
			const std::vector<generic_string> & fontlist = nppParms->getFontList();
			for (size_t i = 0, len = fontlist.size() ; i < len ; ++i)
			{
				auto j = ::SendDlgItemMessage(_hSelf, IDC_COMBO_HFONTNAME, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(fontlist[i].c_str()));
				::SendDlgItemMessage(_hSelf, IDC_COMBO_FFONTNAME, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(fontlist[i].c_str()));

				::SendDlgItemMessage(_hSelf, IDC_COMBO_HFONTNAME, CB_SETITEMDATA, j, reinterpret_cast<LPARAM>(fontlist[i].c_str()));
				::SendDlgItemMessage(_hSelf, IDC_COMBO_FFONTNAME, CB_SETITEMDATA, j, reinterpret_cast<LPARAM>(fontlist[i].c_str()));
			}

			auto index = ::SendDlgItemMessage(_hSelf, IDC_COMBO_HFONTNAME, CB_FINDSTRINGEXACT, static_cast<WPARAM>(-1), reinterpret_cast<LPARAM>(nppGUI._printSettings._headerFontName.c_str()));
			if (index == CB_ERR)
				index = 0;
			::SendDlgItemMessage(_hSelf, IDC_COMBO_HFONTNAME, CB_SETCURSEL, index, 0);
			index = ::SendDlgItemMessage(_hSelf, IDC_COMBO_FFONTNAME, CB_FINDSTRINGEXACT, static_cast<WPARAM>(-1), reinterpret_cast<LPARAM>(nppGUI._printSettings._footerFontName.c_str()));
			if (index == CB_ERR)
				index = 0;
			::SendDlgItemMessage(_hSelf, IDC_COMBO_FFONTNAME, CB_SETCURSEL, index, 0);

			wsprintf(intStr, TEXT("%d"), nppGUI._printSettings._headerFontSize);
			::SendDlgItemMessage(_hSelf, IDC_COMBO_HFONTSIZE, CB_SELECTSTRING, static_cast<WPARAM>(-1), reinterpret_cast<LPARAM>(intStr));
			wsprintf(intStr, TEXT("%d"), nppGUI._printSettings._footerFontSize);
			::SendDlgItemMessage(_hSelf, IDC_COMBO_FFONTSIZE, CB_SELECTSTRING, static_cast<WPARAM>(-1), reinterpret_cast<LPARAM>(intStr));

			::SendDlgItemMessage(_hSelf, IDC_CHECK_HBOLD, BM_SETCHECK, nppGUI._printSettings._headerFontStyle & (FONTSTYLE_BOLD ? TRUE : FALSE), 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_HITALIC, BM_SETCHECK, nppGUI._printSettings._headerFontStyle & (FONTSTYLE_ITALIC ? TRUE : FALSE), 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_FBOLD, BM_SETCHECK, nppGUI._printSettings._footerFontStyle & (FONTSTYLE_BOLD ? TRUE : FALSE), 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_FITALIC, BM_SETCHECK, nppGUI._printSettings._footerFontStyle & (FONTSTYLE_ITALIC ? TRUE : FALSE), 0);

			varList.push_back(strCouple(TEXT("Full file name path"), TEXT("$(FULL_CURRENT_PATH)")));
			varList.push_back(strCouple(TEXT("File name"), TEXT("$(FILE_NAME)")));
			varList.push_back(strCouple(TEXT("File directory"), TEXT("$(CURRENT_DIRECTORY)")));
			varList.push_back(strCouple(TEXT("Page"), TEXT("$(CURRENT_PRINTING_PAGE)")));
			varList.push_back(strCouple(TEXT("Short date format"), TEXT("$(SHORT_DATE)")));
			varList.push_back(strCouple(TEXT("Long date format"), TEXT("$(LONG_DATE)")));
			varList.push_back(strCouple(TEXT("Time"), TEXT("$(TIME)")));

			for (size_t i = 0, len = varList.size() ; i < len ; ++i)
			{
				auto j = ::SendDlgItemMessage(_hSelf, IDC_COMBO_VARLIST, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(varList[i]._varDesc.c_str()));
				::SendDlgItemMessage(_hSelf, IDC_COMBO_VARLIST, CB_SETITEMDATA, j, reinterpret_cast<LPARAM>(varList[i]._var.c_str()));
			}
			::SendDlgItemMessage(_hSelf, IDC_COMBO_VARLIST, CB_SETCURSEL, 0, 0);



			ETDTProc enableDlgTheme = (ETDTProc)nppParms->getEnableThemeDlgTexture();
			if (enableDlgTheme)
				enableDlgTheme(_hSelf, ETDT_ENABLETAB);
			break;
		}
		case WM_COMMAND : 
		{
			if (HIWORD(wParam) == EN_CHANGE)
			{
				switch (LOWORD(wParam))
				{
					case  IDC_EDIT_ML:
						nppGUI._printSettings._marge.left = ::GetDlgItemInt(_hSelf, IDC_EDIT_ML, NULL, FALSE);
						return TRUE;

					case  IDC_EDIT_MR:
						nppGUI._printSettings._marge.right = ::GetDlgItemInt(_hSelf, IDC_EDIT_MR, NULL, FALSE);
						return TRUE;

					case IDC_EDIT_MT :
						nppGUI._printSettings._marge.top = ::GetDlgItemInt(_hSelf, IDC_EDIT_MT, NULL, FALSE);
						return TRUE;

					case IDC_EDIT_MB :
						nppGUI._printSettings._marge.bottom = ::GetDlgItemInt(_hSelf, IDC_EDIT_MB, NULL, FALSE);
						return TRUE;

					default :
					{
						const int stringSize = 256;
						TCHAR str[stringSize];
						_focusedEditCtrl = LOWORD(wParam);
						::GetDlgItemText(_hSelf, _focusedEditCtrl, str, stringSize);
						::SendDlgItemMessage(_hSelf, IDC_VIEWPANEL_STATIC, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(str));
						switch (LOWORD(wParam))
						{
							case  IDC_EDIT_HLEFT:
								nppGUI._printSettings._headerLeft = str;
								trim(nppGUI._printSettings._headerLeft);
								return TRUE;

							case  IDC_EDIT_HMIDDLE:
								nppGUI._printSettings._headerMiddle = str;
								trim(nppGUI._printSettings._headerMiddle);
								return TRUE;

							case IDC_EDIT_HRIGHT :
								nppGUI._printSettings._headerRight = str;
								trim(nppGUI._printSettings._headerRight);
								return TRUE;

							case  IDC_EDIT_FLEFT:
								nppGUI._printSettings._footerLeft = str;
								trim(nppGUI._printSettings._footerLeft);
								return TRUE;

							case  IDC_EDIT_FMIDDLE:
								nppGUI._printSettings._footerMiddle = str;
								trim(nppGUI._printSettings._footerMiddle);
								return TRUE;

							case IDC_EDIT_FRIGHT :
								nppGUI._printSettings._footerRight = str;
								trim(nppGUI._printSettings._footerRight);
								return TRUE;

							default :
								return FALSE;
						}
					}
				}
			}
			else if (HIWORD(wParam) == EN_SETFOCUS)
			{
				const int stringSize = 256;
				TCHAR str[stringSize];
				_focusedEditCtrl = LOWORD(wParam);
				
				int focusedEditStatic = 0;
				int groupStatic = 0;
				switch (_focusedEditCtrl)
				{
					case IDC_EDIT_HLEFT : focusedEditStatic = IDC_HL_STATIC; groupStatic = IDC_HGB_STATIC; break;
					case IDC_EDIT_HMIDDLE : focusedEditStatic = IDC_HM_STATIC; groupStatic = IDC_HGB_STATIC; break;
					case IDC_EDIT_HRIGHT : focusedEditStatic = IDC_HR_STATIC; groupStatic = IDC_HGB_STATIC; break;
					case IDC_EDIT_FLEFT : focusedEditStatic = IDC_FL_STATIC; groupStatic = IDC_FGB_STATIC; break;
					case IDC_EDIT_FMIDDLE : focusedEditStatic = IDC_FM_STATIC; groupStatic = IDC_FGB_STATIC; break;
					case IDC_EDIT_FRIGHT : focusedEditStatic = IDC_FR_STATIC; groupStatic = IDC_FGB_STATIC; break;
					default : return TRUE;
				}
				::GetDlgItemText(_hSelf, _focusedEditCtrl, str, stringSize);
				::SendDlgItemMessage(_hSelf, IDC_VIEWPANEL_STATIC, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(str));
				::GetDlgItemText(_hSelf, groupStatic, str, stringSize);
				generic_string title = str;
				title += TEXT(" ");
				::GetDlgItemText(_hSelf, focusedEditStatic, str, stringSize);
				title += str;
				title += TEXT(" : ");
					
				::SendDlgItemMessage(_hSelf, IDC_WHICHPART_STATIC, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(title.c_str()));
				return TRUE;
			}
			else if (HIWORD(wParam) == CBN_SELCHANGE)
			{
				auto iSel = ::SendDlgItemMessage(_hSelf, LOWORD(wParam), CB_GETCURSEL, 0, 0);

				switch (LOWORD(wParam))
				{
					case IDC_COMBO_HFONTNAME :
					case IDC_COMBO_FFONTNAME :
					{
						TCHAR *fnStr = (TCHAR *)::SendDlgItemMessage(_hSelf, LOWORD(wParam), CB_GETITEMDATA, iSel, 0);
						if (LOWORD(wParam) == IDC_COMBO_HFONTNAME)
							nppGUI._printSettings._headerFontName = fnStr;
						else
							nppGUI._printSettings._footerFontName = fnStr;
					}
					break;


					case IDC_COMBO_HFONTSIZE :
					case IDC_COMBO_FFONTSIZE :
					{
						const size_t intStrLen = 3;
						TCHAR intStr[intStrLen];

						auto lbTextLen = ::SendDlgItemMessage(_hSelf, LOWORD(wParam), CB_GETLBTEXTLEN, iSel, 0);
						if (lbTextLen >= intStrLen)
							return TRUE;

						::SendDlgItemMessage(_hSelf, LOWORD(wParam), CB_GETLBTEXT, iSel, reinterpret_cast<LPARAM>(intStr));

						int *pVal = (LOWORD(wParam) == IDC_COMBO_HFONTSIZE)?&(nppGUI._printSettings._headerFontSize):&(nppGUI._printSettings._footerFontSize);

						if (!intStr[0])
							*pVal = 0;
						else
							*pVal = generic_strtol(intStr, NULL, 10);
					}
					break;

					case IDC_COMBO_VARLIST :
					{
					}
					break;
				}
				return TRUE;
			
			}

			switch (wParam)
			{
				case IDC_CHECK_PRINTLINENUM:
					nppGUI._printSettings._printLineNumber = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECK_PRINTLINENUM, BM_GETCHECK, 0, 0));
					break;

				case  IDC_RADIO_WYSIWYG:
					nppGUI._printSettings._printOption = SC_PRINT_NORMAL;
					break;

				case  IDC_RADIO_INVERT:
					nppGUI._printSettings._printOption = SC_PRINT_INVERTLIGHT;
					break;

				case IDC_RADIO_BW :
					nppGUI._printSettings._printOption = SC_PRINT_BLACKONWHITE;
					break;

				case IDC_RADIO_NOBG :
					nppGUI._printSettings._printOption = SC_PRINT_COLOURONWHITE;
					break;
				case IDC_CHECK_HBOLD:
					nppGUI._printSettings._headerFontStyle ^= FONTSTYLE_BOLD;
					break;

				case  IDC_CHECK_HITALIC:
					nppGUI._printSettings._headerFontStyle ^= FONTSTYLE_ITALIC;
					break;

				case  IDC_CHECK_FBOLD:
					nppGUI._printSettings._footerFontStyle ^= FONTSTYLE_BOLD;
					break;

				case  IDC_CHECK_FITALIC:
					nppGUI._printSettings._footerFontStyle ^= FONTSTYLE_ITALIC;
					break;

				case  IDC_BUTTON_ADDVAR:
				{
					try {
						if (!_focusedEditCtrl)
							return TRUE;

						auto iSel = ::SendDlgItemMessage(_hSelf, IDC_COMBO_VARLIST, CB_GETCURSEL, 0, 0);
						TCHAR *varStr = (TCHAR *)::SendDlgItemMessage(_hSelf, IDC_COMBO_VARLIST, CB_GETITEMDATA, iSel, 0);
						size_t selStart = 0;
						size_t selEnd = 0;
						::SendDlgItemMessage(_hSelf, _focusedEditCtrl, EM_GETSEL, reinterpret_cast<WPARAM>(&selStart), reinterpret_cast<LPARAM>(&selEnd));

						const int stringSize = 256;
						TCHAR str[stringSize];
						::SendDlgItemMessage(_hSelf, _focusedEditCtrl, WM_GETTEXT, stringSize, reinterpret_cast<LPARAM>(str));

						generic_string str2Set(str);
						size_t strLen = str2Set.length();
						if (selStart > strLen || selEnd > strLen)
							selStart = selEnd = strLen;

						str2Set.replace(selStart, selEnd - selStart, varStr);

						::SetDlgItemText(_hSelf, _focusedEditCtrl, str2Set.c_str());
					}
					catch (...)
					{
						// Do nothing
					}
				}
				break;
			}
			return TRUE;
		}
	}
	return FALSE;
}


INT_PTR CALLBACK BackupDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM)
{
		NppGUI & nppGUI = const_cast<NppGUI &>(nppParms->getNppGUI());
	switch (message) 
	{
		case WM_INITDIALOG :
		{
			::SendDlgItemMessage(_hSelf, IDC_CHECK_REMEMBERSESSION, BM_SETCHECK, nppGUI._rememberLastSession, 0);
			bool snapshotCheck = nppGUI._rememberLastSession && nppGUI.isSnapshotMode();
			::SendDlgItemMessage(_hSelf, IDC_BACKUPDIR_RESTORESESSION_CHECK, BM_SETCHECK, snapshotCheck?BST_CHECKED:BST_UNCHECKED, 0);
			auto periodicBackupInSec = static_cast<UINT>(nppGUI._snapshotBackupTiming / 1000);
			::SetDlgItemInt(_hSelf, IDC_BACKUPDIR_RESTORESESSION_EDIT,periodicBackupInSec, FALSE);
			generic_string backupFilePath = nppParms->getUserPath();
			backupFilePath += TEXT("\\backup\\");
			::SetDlgItemText(_hSelf, IDD_BACKUPDIR_RESTORESESSION_PATH_EDIT, backupFilePath.c_str());

			int ID2CheckBackupOnSave = 0;

			switch (nppGUI._backup)
			{
				case bak_simple :
					ID2CheckBackupOnSave = IDC_RADIO_BKSIMPLE;
					break;
				case bak_verbose :
					ID2CheckBackupOnSave = IDC_RADIO_BKVERBOSE;
					break;
				
				default : //bak_none
					ID2CheckBackupOnSave = IDC_RADIO_BKNONE;
			}
			::SendDlgItemMessage(_hSelf, ID2CheckBackupOnSave, BM_SETCHECK, BST_CHECKED, 0);

			if (nppGUI._useDir)
				::SendDlgItemMessage(_hSelf, IDC_BACKUPDIR_CHECK, BM_SETCHECK, BST_CHECKED, 0);

			::SendDlgItemMessage(_hSelf, IDC_BACKUPDIR_EDIT, WM_SETTEXT, 0, reinterpret_cast<LPARAM>((nppGUI._backupDir.c_str())));

			updateBackupGUI();
			return TRUE;
		}
		case WM_COMMAND : 
		{
			if (HIWORD(wParam) == EN_CHANGE)
			{
				switch (LOWORD(wParam))
				{
					case  IDC_BACKUPDIR_EDIT:
					{
						TCHAR inputDir[MAX_PATH];
						::SendDlgItemMessage(_hSelf, IDC_BACKUPDIR_EDIT, WM_GETTEXT, MAX_PATH, reinterpret_cast<LPARAM>(inputDir));
						nppGUI._backupDir = inputDir;
						return TRUE;
					}

					case IDC_BACKUPDIR_RESTORESESSION_EDIT:
					{
						
						const int stringSize = 16;
						TCHAR str[stringSize];

						::GetDlgItemText(_hSelf, IDC_BACKUPDIR_RESTORESESSION_EDIT, str, stringSize);

						if (lstrcmp(str, TEXT("")) == 0)
							return TRUE;

						nppGUI._snapshotBackupTiming = ::GetDlgItemInt(_hSelf, IDC_BACKUPDIR_RESTORESESSION_EDIT, NULL, FALSE) * 1000;
						if (!nppGUI._snapshotBackupTiming)
						{
							nppGUI._snapshotBackupTiming = 1000;
							::SetDlgItemInt(_hSelf, IDC_BACKUPDIR_RESTORESESSION_EDIT, 1, FALSE);
						}
						return TRUE;
					}
				}
			}
			else if (HIWORD(wParam) == EN_KILLFOCUS)
			{
				switch (LOWORD(wParam))
				{
					case  IDC_BACKUPDIR_RESTORESESSION_EDIT:
					{
						//printStr(TEXT(""));
						const int stringSize = 16;
						TCHAR str[stringSize];

						::GetDlgItemText(_hSelf, IDC_BACKUPDIR_RESTORESESSION_EDIT, str, stringSize);

						if (lstrcmp(str, TEXT("")) == 0)
						{
							::SetDlgItemInt(_hSelf, IDC_BACKUPDIR_RESTORESESSION_EDIT, static_cast<int32_t>(nppGUI._snapshotBackupTiming / 1000), FALSE);
						}
					}
				}
			}

			switch (wParam)
			{
				case IDC_CHECK_REMEMBERSESSION:
				{
					nppGUI._rememberLastSession = isCheckedOrNot(IDC_CHECK_REMEMBERSESSION);
					if (!nppGUI._rememberLastSession)
					{
						::SendDlgItemMessage(_hSelf, IDC_BACKUPDIR_RESTORESESSION_CHECK, BM_SETCHECK, BST_UNCHECKED, 0);
						::SendMessage(_hSelf, WM_COMMAND, IDC_BACKUPDIR_RESTORESESSION_CHECK, 0);
					}
					updateBackupGUI();
					return TRUE;
				}
				case IDC_BACKUPDIR_RESTORESESSION_CHECK:
				{
					nppGUI._isSnapshotMode = BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_BACKUPDIR_RESTORESESSION_CHECK, BM_GETCHECK, 0, 0);
					updateBackupGUI();

					if (nppGUI._isSnapshotMode)
					{
						// Launch thread
						::SendMessage(::GetParent(_hParent), NPPM_INTERNAL_ENABLESNAPSHOT, 0, 0);
					}
					return TRUE;
				}

				case IDC_RADIO_BKSIMPLE:
				{
					nppGUI._backup = bak_simple;
					updateBackupGUI();
					return TRUE;
				}

				case IDC_RADIO_BKVERBOSE:
				{
					nppGUI._backup = bak_verbose;
					updateBackupGUI();
					return TRUE;
				}

				case IDC_RADIO_BKNONE:
				{
					nppGUI._backup = bak_none;
					updateBackupGUI();
					return TRUE;
				}

				case IDC_BACKUPDIR_CHECK:
				{
					nppGUI._useDir = !nppGUI._useDir;
					updateBackupGUI();
					return TRUE;
				}
				case IDD_BACKUPDIR_BROWSE_BUTTON :
				{
					folderBrowser(_hSelf, TEXT("Select a folder as backup directory"), IDC_BACKUPDIR_EDIT);
					return TRUE;
				}


				default :
					return FALSE;
			}
			
		}
	}
	return FALSE;
}

void BackupDlg::updateBackupGUI()
{
	bool rememberSession = isCheckedOrNot(IDC_CHECK_REMEMBERSESSION);
	bool isSnapshot = isCheckedOrNot(IDC_BACKUPDIR_RESTORESESSION_CHECK);
	::EnableWindow(::GetDlgItem(_hSelf, IDC_BACKUPDIR_RESTORESESSION_CHECK), rememberSession);
	::EnableWindow(::GetDlgItem(_hSelf, IDD_BACKUPDIR_RESTORESESSION_STATIC1), isSnapshot);
	::EnableWindow(::GetDlgItem(_hSelf, IDC_BACKUPDIR_RESTORESESSION_EDIT), isSnapshot);
	::EnableWindow(::GetDlgItem(_hSelf, IDD_BACKUPDIR_RESTORESESSION_STATIC2), isSnapshot);
	::EnableWindow(::GetDlgItem(_hSelf, IDD_BACKUPDIR_RESTORESESSION_PATHLABEL_STATIC), isSnapshot);
	::EnableWindow(::GetDlgItem(_hSelf, IDD_BACKUPDIR_RESTORESESSION_PATH_EDIT), isSnapshot);

	bool noBackup = BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_RADIO_BKNONE, BM_GETCHECK, 0, 0);
	bool isEnableGlobableCheck = false;
	bool isEnableLocalCheck = false;

	if (!noBackup)
	{
		isEnableGlobableCheck = true;
		isEnableLocalCheck = BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_BACKUPDIR_CHECK, BM_GETCHECK, 0, 0);
	}
	::EnableWindow(::GetDlgItem(_hSelf, IDC_BACKUPDIR_USERCUSTOMDIR_GRPSTATIC), isEnableGlobableCheck);
	::EnableWindow(::GetDlgItem(_hSelf, IDC_BACKUPDIR_CHECK), isEnableGlobableCheck);

	::EnableWindow(::GetDlgItem(_hSelf, IDD_BACKUPDIR_STATIC), isEnableLocalCheck);
	::EnableWindow(::GetDlgItem(_hSelf, IDC_BACKUPDIR_EDIT), isEnableLocalCheck);
	::EnableWindow(::GetDlgItem(_hSelf, IDD_BACKUPDIR_BROWSE_BUTTON), isEnableLocalCheck);
}


INT_PTR CALLBACK AutoCompletionDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM)
{
		NppGUI & nppGUI = const_cast<NppGUI &>(nppParms->getNppGUI());
	switch (message) 
	{
		case WM_INITDIALOG :
		{
			::SetDlgItemInt(_hSelf, IDD_AUTOC_STATIC_N,  static_cast<UINT>(nppGUI._autocFromLen), FALSE);
			_nbCharVal.init(_hInst, _hSelf);
			_nbCharVal.create(::GetDlgItem(_hSelf, IDD_AUTOC_STATIC_N), IDD_AUTOC_STATIC_N);

			bool isEnableAutoC = nppGUI._autocStatus != nppGUI.autoc_none;

			::SendDlgItemMessage(_hSelf, IDD_AUTOC_ENABLECHECK, BM_SETCHECK, isEnableAutoC?BST_CHECKED:BST_UNCHECKED, 0);
			
			int selectedID = IDD_AUTOC_BOTHRADIO;
			if (nppGUI._autocStatus == nppGUI.autoc_func)
				selectedID = IDD_AUTOC_FUNCRADIO;
			else if (nppGUI._autocStatus == nppGUI.autoc_word)
				selectedID = IDD_AUTOC_WORDRADIO;
			else if (nppGUI._autocStatus == nppGUI.autoc_both)
				selectedID = IDD_AUTOC_BOTHRADIO;
			
			::SendDlgItemMessage(_hSelf, selectedID, BM_SETCHECK, BST_CHECKED, 0);

			if (nppGUI._autocStatus == nppGUI.autoc_word || nppGUI._autocStatus == nppGUI.autoc_both)
				::SendDlgItemMessage(_hSelf, IDD_AUTOC_IGNORENUMBERS, BM_SETCHECK, nppGUI._autocIgnoreNumbers ? BST_CHECKED : BST_UNCHECKED, 0);

			if (!isEnableAutoC)
			{
				::EnableWindow(::GetDlgItem(_hSelf, IDD_AUTOC_FUNCRADIO), FALSE);
				::EnableWindow(::GetDlgItem(_hSelf, IDD_AUTOC_WORDRADIO), FALSE);
				::EnableWindow(::GetDlgItem(_hSelf, IDD_AUTOC_BOTHRADIO), FALSE);
				::EnableWindow(::GetDlgItem(_hSelf, IDD_AUTOC_IGNORENUMBERS), FALSE);
				::EnableWindow(::GetDlgItem(_hSelf, IDD_AUTOC_STATIC_FROM), FALSE);
				::EnableWindow(::GetDlgItem(_hSelf, IDD_AUTOC_STATIC_N), FALSE);
				::EnableWindow(::GetDlgItem(_hSelf, IDD_AUTOC_STATIC_CHAR), FALSE);
				::EnableWindow(::GetDlgItem(_hSelf, IDD_AUTOC_STATIC_NOTE), FALSE);
			}
			::SendDlgItemMessage(_hSelf, IDC_CHECK_MAINTAININDENT, BM_SETCHECK, nppGUI._maitainIndent, 0);

			::SendDlgItemMessage(_hSelf, IDD_FUNC_CHECK, BM_SETCHECK, nppGUI._funcParams ? BST_CHECKED : BST_UNCHECKED, 0);

			::SendDlgItemMessage(_hSelf, IDD_AUTOCPARENTHESES_CHECK, BM_SETCHECK, nppGUI._matchedPairConf._doParentheses?BST_CHECKED:BST_UNCHECKED, 0);
			if (nppGUI._matchedPairConf._doParentheses)
				::SendDlgItemMessage(_hSelf, IDD_AUTOCPARENTHESES_CHECK, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(TEXT(" (  )")));

			::SendDlgItemMessage(_hSelf, IDD_AUTOCBRACKET_CHECK, BM_SETCHECK, nppGUI._matchedPairConf._doBrackets?BST_CHECKED:BST_UNCHECKED, 0);
			if (nppGUI._matchedPairConf._doBrackets)
				::SendDlgItemMessage(_hSelf, IDD_AUTOCBRACKET_CHECK, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(TEXT(" [  ]")));

			::SendDlgItemMessage(_hSelf, IDD_AUTOCCURLYBRACKET_CHECK, BM_SETCHECK, nppGUI._matchedPairConf._doCurlyBrackets?BST_CHECKED:BST_UNCHECKED, 0);
			if (nppGUI._matchedPairConf._doCurlyBrackets)
				::SendDlgItemMessage(_hSelf, IDD_AUTOCCURLYBRACKET_CHECK, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(TEXT(" {  }")));
			
			::SendDlgItemMessage(_hSelf, IDD_AUTOC_QUOTESCHECK, BM_SETCHECK, nppGUI._matchedPairConf._doQuotes?BST_CHECKED:BST_UNCHECKED, 0);
			if (nppGUI._matchedPairConf._doQuotes)
				::SendDlgItemMessage(_hSelf, IDD_AUTOC_QUOTESCHECK, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(TEXT(" '  '")));
			
			::SendDlgItemMessage(_hSelf, IDD_AUTOC_DOUBLEQUOTESCHECK, BM_SETCHECK, nppGUI._matchedPairConf._doDoubleQuotes?BST_CHECKED:BST_UNCHECKED, 0);
			if (nppGUI._matchedPairConf._doDoubleQuotes)
				::SendDlgItemMessage(_hSelf, IDD_AUTOC_DOUBLEQUOTESCHECK, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(TEXT(" \"  \"")));
			
			::SendDlgItemMessage(_hSelf, IDD_AUTOCTAG_CHECK, BM_SETCHECK, nppGUI._matchedPairConf._doHtmlXmlTag?BST_CHECKED:BST_UNCHECKED, 0);

			::SendDlgItemMessage(_hSelf, IDC_MACHEDPAIROPEN_EDIT1, EM_LIMITTEXT, 1, 0);
			::SendDlgItemMessage(_hSelf, IDC_MACHEDPAIRCLOSE_EDIT1, EM_LIMITTEXT, 1, 0);
			::SendDlgItemMessage(_hSelf, IDC_MACHEDPAIROPEN_EDIT2, EM_LIMITTEXT, 1, 0);
			::SendDlgItemMessage(_hSelf, IDC_MACHEDPAIRCLOSE_EDIT2, EM_LIMITTEXT, 1, 0);
			::SendDlgItemMessage(_hSelf, IDC_MACHEDPAIROPEN_EDIT3, EM_LIMITTEXT, 1, 0);
			::SendDlgItemMessage(_hSelf, IDC_MACHEDPAIRCLOSE_EDIT3, EM_LIMITTEXT, 1, 0);

			size_t nbMatchedPair = nppGUI._matchedPairConf._matchedPairsInit.size();
			if (nbMatchedPair > 3)
				nbMatchedPair = 3;
			for (size_t i = 0; i < nbMatchedPair; ++i)
			{
				TCHAR openChar[2];
				openChar[0] = nppGUI._matchedPairConf._matchedPairsInit[i].first;
				openChar[1] = '\0';
				TCHAR closeChar[2];
				closeChar[0] = nppGUI._matchedPairConf._matchedPairsInit[i].second;
				closeChar[1] = '\0';

				if (i == 0)
				{
					::SendDlgItemMessage(_hSelf, IDC_MACHEDPAIROPEN_EDIT1, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(openChar));
					::SendDlgItemMessage(_hSelf, IDC_MACHEDPAIRCLOSE_EDIT1, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(closeChar));
				}
				else if (i == 1)
				{
					::SendDlgItemMessage(_hSelf, IDC_MACHEDPAIROPEN_EDIT2, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(openChar));
					::SendDlgItemMessage(_hSelf, IDC_MACHEDPAIRCLOSE_EDIT2, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(closeChar));
				}
				if (i == 2)
				{
					::SendDlgItemMessage(_hSelf, IDC_MACHEDPAIROPEN_EDIT3, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(openChar));
					::SendDlgItemMessage(_hSelf, IDC_MACHEDPAIRCLOSE_EDIT3, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(closeChar));
				}
			}

			return TRUE;
		}
		case WM_COMMAND : 
		{
			if (HIWORD(wParam) == EN_CHANGE)
			{
				switch (LOWORD(wParam))
				{
					case IDC_MACHEDPAIROPEN_EDIT1 :
					case IDC_MACHEDPAIRCLOSE_EDIT1:
					case IDC_MACHEDPAIROPEN_EDIT2 :
					case IDC_MACHEDPAIRCLOSE_EDIT2:
					case IDC_MACHEDPAIROPEN_EDIT3 :
					case IDC_MACHEDPAIRCLOSE_EDIT3:
					{
						nppGUI._matchedPairConf._matchedPairs.clear();

						TCHAR opener[2] = {'\0', '\0'};
						TCHAR closer[2] = {'\0', '\0'};

						::SendDlgItemMessage(_hSelf, IDC_MACHEDPAIROPEN_EDIT1, WM_GETTEXT, MAX_PATH, reinterpret_cast<LPARAM>(opener));
						::SendDlgItemMessage(_hSelf, IDC_MACHEDPAIRCLOSE_EDIT1, WM_GETTEXT, MAX_PATH, reinterpret_cast<LPARAM>(closer));
						if (opener[0] < 0x80 && opener[0] != '\0' && closer[0] < 0x80 && closer[0] != '\0')
							nppGUI._matchedPairConf._matchedPairs.push_back(pair<char, char>(static_cast<char>(opener[0]), static_cast<char>(closer[0])));

						::SendDlgItemMessage(_hSelf, IDC_MACHEDPAIROPEN_EDIT2, WM_GETTEXT, MAX_PATH, reinterpret_cast<LPARAM>(opener));
						::SendDlgItemMessage(_hSelf, IDC_MACHEDPAIRCLOSE_EDIT2, WM_GETTEXT, MAX_PATH, reinterpret_cast<LPARAM>(closer));
						if (opener[0] < 0x80 && opener[0] != '\0' && closer[0] < 0x80 && closer[0] != '\0')
							nppGUI._matchedPairConf._matchedPairs.push_back(pair<char, char>(static_cast<char>(opener[0]), static_cast<char>(closer[0])));

						::SendDlgItemMessage(_hSelf, IDC_MACHEDPAIROPEN_EDIT3, WM_GETTEXT, MAX_PATH, reinterpret_cast<LPARAM>(opener));
						::SendDlgItemMessage(_hSelf, IDC_MACHEDPAIRCLOSE_EDIT3, WM_GETTEXT, MAX_PATH, reinterpret_cast<LPARAM>(closer));
						if (opener[0] < 0x80 && opener[0] != '\0' && closer[0] < 0x80 && closer[0] != '\0')
							nppGUI._matchedPairConf._matchedPairs.push_back(pair<char, char>(static_cast<char>(opener[0]), static_cast<char>(closer[0])));
						 
						return TRUE;
					}
				}
			}

			switch (wParam)
			{
				case IDC_CHECK_MAINTAININDENT:
				{
					nppGUI._maitainIndent = isCheckedOrNot(IDC_CHECK_MAINTAININDENT);
					return TRUE;
				}

				case IDD_AUTOC_ENABLECHECK :
				{
					bool isEnableAutoC = BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDD_AUTOC_ENABLECHECK, BM_GETCHECK, 0, 0);

					if (isEnableAutoC)
					{
						::SendDlgItemMessage(_hSelf, IDD_AUTOC_BOTHRADIO, BM_SETCHECK, BST_CHECKED, 0);
						nppGUI._autocStatus = nppGUI.autoc_both;

						::SendDlgItemMessage(_hSelf, IDD_AUTOC_IGNORENUMBERS, BM_SETCHECK, BST_UNCHECKED, 0);
						nppGUI._autocIgnoreNumbers = false;
					}
					else 
					{
						::SendDlgItemMessage(_hSelf, IDD_AUTOC_FUNCRADIO, BM_SETCHECK, BST_UNCHECKED, 0);
						::SendDlgItemMessage(_hSelf, IDD_AUTOC_WORDRADIO, BM_SETCHECK, BST_UNCHECKED, 0);
						::SendDlgItemMessage(_hSelf, IDD_AUTOC_BOTHRADIO, BM_SETCHECK, BST_UNCHECKED, 0);
						nppGUI._autocStatus = nppGUI.autoc_none;

						::SendDlgItemMessage(_hSelf, IDD_AUTOC_IGNORENUMBERS, BM_SETCHECK, BST_UNCHECKED, 0);
						nppGUI._autocIgnoreNumbers = false;
					}
					::EnableWindow(::GetDlgItem(_hSelf, IDD_AUTOC_FUNCRADIO), isEnableAutoC);
					::EnableWindow(::GetDlgItem(_hSelf, IDD_AUTOC_WORDRADIO), isEnableAutoC);
					::EnableWindow(::GetDlgItem(_hSelf, IDD_AUTOC_BOTHRADIO), isEnableAutoC);
					::EnableWindow(::GetDlgItem(_hSelf, IDD_AUTOC_IGNORENUMBERS), isEnableAutoC);
					::EnableWindow(::GetDlgItem(_hSelf, IDD_AUTOC_STATIC_FROM), isEnableAutoC);
					::EnableWindow(::GetDlgItem(_hSelf, IDD_AUTOC_STATIC_N), isEnableAutoC);
					::EnableWindow(::GetDlgItem(_hSelf, IDD_AUTOC_STATIC_CHAR), isEnableAutoC);
					::EnableWindow(::GetDlgItem(_hSelf, IDD_AUTOC_STATIC_NOTE), isEnableAutoC);
					return TRUE;
				}

				case IDD_AUTOC_FUNCRADIO :
				{
					nppGUI._autocStatus = nppGUI.autoc_func;

					::SendDlgItemMessage(_hSelf, IDD_AUTOC_IGNORENUMBERS, BM_SETCHECK, BST_UNCHECKED, 0);
					::EnableWindow(::GetDlgItem(_hSelf, IDD_AUTOC_IGNORENUMBERS), FALSE);
					nppGUI._autocIgnoreNumbers = false;

					return TRUE;
				}

				case IDD_AUTOC_WORDRADIO :
				{
					nppGUI._autocStatus = nppGUI.autoc_word;
					::EnableWindow(::GetDlgItem(_hSelf, IDD_AUTOC_IGNORENUMBERS), TRUE);
					return TRUE;
				}

				case IDD_AUTOC_BOTHRADIO :
				{
					nppGUI._autocStatus = nppGUI.autoc_both;
					::EnableWindow(::GetDlgItem(_hSelf, IDD_AUTOC_IGNORENUMBERS), TRUE);
					return TRUE;
				}

				case IDD_AUTOC_IGNORENUMBERS:
				{
					nppGUI._autocIgnoreNumbers = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDD_AUTOC_IGNORENUMBERS, BM_GETCHECK, 0, 0));
					return TRUE;
				}

				case IDD_FUNC_CHECK :
				{
					nppGUI._funcParams = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDD_FUNC_CHECK, BM_GETCHECK, 0, 0));
					return TRUE;
				}
				
				case IDD_AUTOC_STATIC_N :
				{
					const int NB_MIN_CHAR = 1;
					const int NB_MAX_CHAR = 9;

					NativeLangSpeaker *pNativeSpeaker = nppParms->getNativeLangSpeaker();
					generic_string strNbChar = pNativeSpeaker->getLocalizedStrFromID("autocomplete-nb-char", TEXT("Nb char : "));

					ValueDlg valDlg;
					valDlg.init(NULL, _hSelf, static_cast<int32_t>(nppGUI._autocFromLen), strNbChar.c_str());
					valDlg.setNBNumber(1);

					POINT p;
					::GetCursorPos(&p);

					int size = valDlg.doDialog(p);
					if (size != -1)
					{
						if (size > NB_MAX_CHAR)
							size = NB_MAX_CHAR;
						else if (size < NB_MIN_CHAR)
							size = NB_MIN_CHAR;
						
						nppGUI._autocFromLen = size;
						::SetDlgItemInt(_hSelf, IDD_AUTOC_STATIC_N, static_cast<int32_t>(nppGUI._autocFromLen), FALSE);
					}
					return TRUE;
				}

				case IDD_AUTOCPARENTHESES_CHECK :
				case IDD_AUTOCBRACKET_CHECK :
				case IDD_AUTOCCURLYBRACKET_CHECK :
				case IDD_AUTOC_DOUBLEQUOTESCHECK :
				case IDD_AUTOC_QUOTESCHECK :
				{
					bool isChecked = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, static_cast<int32_t>(wParam), BM_GETCHECK, 0, 0));
					const TCHAR *label;
					if (wParam == IDD_AUTOCPARENTHESES_CHECK)
					{
						nppGUI._matchedPairConf._doParentheses = isChecked;
						label = isChecked?TEXT(" (  )"):TEXT(" (");
					}
					else if (wParam == IDD_AUTOCBRACKET_CHECK)
					{
						nppGUI._matchedPairConf._doBrackets = isChecked;
						label = isChecked?TEXT(" [  ]"):TEXT(" [");
					}
					else if (wParam == IDD_AUTOCCURLYBRACKET_CHECK)
					{
						nppGUI._matchedPairConf._doCurlyBrackets = isChecked;
						label = isChecked?TEXT(" {  }"):TEXT(" {");
					}
					else if (wParam == IDD_AUTOC_DOUBLEQUOTESCHECK)
					{
						nppGUI._matchedPairConf._doDoubleQuotes = isChecked;
						label = isChecked?TEXT(" \"  \""):TEXT(" \"");
					}
					else // if (wParam == IDD_AUTOC_QUOTESCHECK)
					{
						nppGUI._matchedPairConf._doQuotes = isChecked;
						label = isChecked?TEXT(" '  '"):TEXT(" '");
					}
					::SendDlgItemMessage(_hSelf, static_cast<int32_t>(wParam), WM_SETTEXT, 0, reinterpret_cast<LPARAM>(label));
					return TRUE;
				}

				case IDD_AUTOCTAG_CHECK :
				{
					nppGUI._matchedPairConf._doHtmlXmlTag = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDD_AUTOCTAG_CHECK, BM_GETCHECK, 0, 0));
					return TRUE;
				}
				default :
					return FALSE;
			}
			
		}
	}
	return FALSE;
}


INT_PTR CALLBACK MultiInstDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM)
{
	NppGUI & nppGUI = *nppUIParms;
	switch (message) 
	{
		case WM_INITDIALOG :
		{
			MultiInstSetting multiInstSetting = nppGUI._multiInstSetting;

			::SendDlgItemMessage(_hSelf, IDC_SESSIONININST_RADIO, BM_SETCHECK, multiInstSetting == multiInstOnSession?BST_CHECKED:BST_UNCHECKED, 0);
			::SendDlgItemMessage(_hSelf, IDC_MULTIINST_RADIO, BM_SETCHECK, multiInstSetting == multiInst?BST_CHECKED:BST_UNCHECKED, 0);
			::SendDlgItemMessage(_hSelf, IDC_MONOINST_RADIO, BM_SETCHECK, multiInstSetting == monoInst?BST_CHECKED:BST_UNCHECKED, 0);
		}
		break;

		case WM_COMMAND : 
		{
			switch (wParam)
			{
				case IDC_SESSIONININST_RADIO :
				{
					nppGUI._multiInstSetting = multiInstOnSession;
				}
				break;

				case IDC_MULTIINST_RADIO :
				{
					nppGUI._multiInstSetting = multiInst;
				}
				break;

				case IDC_MONOINST_RADIO :
				{
					nppGUI._multiInstSetting = monoInst;
				}
				break;
				default :
					return FALSE;
			}
		}
		break;
	}

	return FALSE;
}

void DelimiterSettingsDlg::detectSpace(const char *text2Check, int & nbSp, int & nbTab) const
{
	nbSp = nbTab = 0;
	for (size_t i = 0; i < strlen(text2Check); ++i)
	{
		if (text2Check[i] == ' ')
			++nbSp;
		else if (text2Check[i] == '\t')
			++nbTab;
	}
}


generic_string DelimiterSettingsDlg::getWarningText(size_t nbSp, size_t nbTab) const
{
	NativeLangSpeaker *pNativeSpeaker = nppParms->getNativeLangSpeaker();

	generic_string msg;
	if (nbSp && nbTab)
	{
		generic_string nbSpStr = std::to_wstring(nbSp);
		generic_string nbTabStr = std::to_wstring(nbTab);
		generic_string warnBegin = pNativeSpeaker->getLocalizedStrFromID("word-chars-list-warning-begin", TEXT(""));
		generic_string space = pNativeSpeaker->getLocalizedStrFromID("word-chars-list-space-warning", TEXT(""));
		generic_string tab = pNativeSpeaker->getLocalizedStrFromID("word-chars-list-tab-warning", TEXT(""));
		generic_string warnEnd = pNativeSpeaker->getLocalizedStrFromID("word-chars-list-warning-end", TEXT(""));

		// half translation is not allowed
		if (not warnBegin.empty() && not space.empty() && not tab.empty() && not warnEnd.empty())
		{
			space = stringReplace(space, TEXT("$INT_REPLACE$"), nbSpStr);
			tab = stringReplace(tab, TEXT("$INT_REPLACE$"), nbTabStr);
			msg = warnBegin;
			msg += space;
			msg += TEXT(" && ");
			msg += tab;
			msg += warnEnd;
		}
		else
		{
			msg = TEXT("Be aware: ");
			msg += nbSpStr;
			msg += TEXT(" space(s) && ");
			msg += std::to_wstring(nbTab);
			msg += TEXT(" TAB(s) in your character list.");
		}
	}
	else if (nbSp && not nbTab)
	{
		generic_string nbSpStr = std::to_wstring(nbSp);
		generic_string warnBegin = pNativeSpeaker->getLocalizedStrFromID("word-chars-list-warning-begin", TEXT(""));
		generic_string space = pNativeSpeaker->getLocalizedStrFromID("word-chars-list-space-warning", TEXT(""));
		generic_string warnEnd = pNativeSpeaker->getLocalizedStrFromID("word-chars-list-warning-end", TEXT(""));

		// half translation is not allowed
		if (not warnBegin.empty() && not space.empty() && not warnEnd.empty())
		{
			space = stringReplace(space, TEXT("$INT_REPLACE$"), nbSpStr);
			msg = warnBegin;
			msg += space;
			msg += warnEnd;
		}
		else
		{
			msg = TEXT("Be aware: ");
			msg += std::to_wstring(nbSp);
			msg += TEXT(" space(s) in your character list.");
		}
	}
	else if (not nbSp && nbTab)
	{
		generic_string nbTabStr = std::to_wstring(nbTab);
		generic_string warnBegin = pNativeSpeaker->getLocalizedStrFromID("word-chars-list-warning-begin", TEXT(""));
		generic_string tab = pNativeSpeaker->getLocalizedStrFromID("word-chars-list-tab-warning", TEXT(""));
		generic_string warnEnd = pNativeSpeaker->getLocalizedStrFromID("word-chars-list-warning-end", TEXT(""));

		// half translation is not allowed
		if (not warnBegin.empty() && not tab.empty() && not warnEnd.empty())
		{
			tab = stringReplace(tab, TEXT("$INT_REPLACE$"), nbTabStr);
			msg = warnBegin;
			msg += tab;
			msg += warnEnd;
		}
		else
		{
			msg = TEXT("Be aware: ");
			msg += std::to_wstring(nbTab);
			msg += TEXT(" TAB(s) in your character list.");
		}
	}
	else //(not nbSp && not nbTab)
	{
		// do nothing
	}

	return msg;
}

void DelimiterSettingsDlg::setWarningIfNeed() const
{
	generic_string msg;
	NppGUI & nppGUI = *nppUIParms;
	if (not nppGUI._isWordCharDefault)
	{
		int nbSp = 0;
		int nbTab = 0;
		detectSpace(nppGUI._customWordChars.c_str(), nbSp, nbTab);
		msg = getWarningText(nbSp, nbTab);
	}
	::SetDlgItemText(_hSelf, IDD_STATIC_WORDCHAR_WARNING, msg.c_str());
}

INT_PTR CALLBACK DelimiterSettingsDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	NppGUI & nppGUI = *nppUIParms;
	switch (message) 
	{
		case WM_INITDIALOG :
		{
			//
			// Delimiter
			//
			TCHAR opener[2];
			opener[0] = nppGUI._leftmostDelimiter;
			opener[1] = '\0';
			TCHAR closer[2];
			closer[0] = nppGUI._rightmostDelimiter;
			closer[1] = '\0';
			bool onSeveralLines = nppGUI._delimiterSelectionOnEntireDocument;
			
			::SendDlgItemMessage(_hSelf, IDC_EDIT_OPENDELIMITER, EM_LIMITTEXT, 1, 0);
			::SendDlgItemMessage(_hSelf, IDC_EDIT_CLOSEDELIMITER, EM_LIMITTEXT, 1, 0);
			::SendDlgItemMessage(_hSelf, IDC_EDIT_OPENDELIMITER, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(opener));
			::SendDlgItemMessage(_hSelf, IDC_EDIT_CLOSEDELIMITER, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(closer));
			::SendDlgItemMessage(_hSelf, IDD_SEVERALLINEMODEON_CHECK, BM_SETCHECK, onSeveralLines?BST_CHECKED:BST_UNCHECKED, 0);

			POINT point = getTopPoint(::GetDlgItem(_hSelf, IDD_STATIC_BLABLA), false);
			_singleLineModePoint.x = point.x + 4;
			_singleLineModePoint.y = point.y - 4;

			point = getTopPoint(::GetDlgItem(_hSelf, IDD_STATIC_BLABLA2NDLINE), false);
			_multiLineModePoint.x = point.x + 4;
			_multiLineModePoint.y = point.y - 4;

			::GetClientRect(::GetDlgItem(_hSelf, IDC_EDIT_CLOSEDELIMITER), &_closerRect);
			_closerRect.right = _closerRect.right - _closerRect.left + 4;
			_closerRect.bottom = _closerRect.bottom - _closerRect.top + 4;

			::GetClientRect(::GetDlgItem(_hSelf, IDD_STATIC_CLOSEDELIMITER), &_closerLabelRect);
			_closerLabelRect.right = _closerLabelRect.right - _closerLabelRect.left + 4;
			_closerLabelRect.bottom = _closerLabelRect.bottom - _closerLabelRect.top + 4;


			::ShowWindow(::GetDlgItem(_hSelf, IDD_STATIC_BLABLA2NDLINE),onSeveralLines?SW_SHOW:SW_HIDE);

			POINT *p = onSeveralLines?&_multiLineModePoint:&_singleLineModePoint;
			::MoveWindow(::GetDlgItem(_hSelf, IDC_EDIT_CLOSEDELIMITER), p->x, p->y, _closerRect.right, _closerRect.bottom, TRUE);
			::MoveWindow(::GetDlgItem(_hSelf, IDD_STATIC_CLOSEDELIMITER), p->x + _closerRect.right + 4, p->y + 4, _closerLabelRect.right, _closerLabelRect.bottom, TRUE);

			//
			// Word Char List
			//
			
			::SetDlgItemTextA(_hSelf, IDC_WORDCHAR_CUSTOM_EDIT, nppGUI._customWordChars.c_str());
			::SendDlgItemMessage(_hSelf, IDC_RADIO_WORDCHAR_DEFAULT, BM_SETCHECK, nppGUI._isWordCharDefault ? BST_CHECKED : BST_UNCHECKED, 0);
			::SendDlgItemMessage(_hSelf, IDC_RADIO_WORDCHAR_CUSTOM, BM_SETCHECK, not nppGUI._isWordCharDefault ? BST_CHECKED : BST_UNCHECKED, 0);
			::EnableWindow(::GetDlgItem(_hSelf, IDC_WORDCHAR_CUSTOM_EDIT), not nppGUI._isWordCharDefault);

			setWarningIfNeed();

			NativeLangSpeaker *pNativeSpeaker = nppParms->getNativeLangSpeaker();
			generic_string tip2show = pNativeSpeaker->getLocalizedStrFromID("word-chars-list-tip", TEXT("This allows you to include additional character into current word characters while double clicking for selection or searching with \"Match whole word only\" option checked."));

			_tip = CreateToolTip(IDD_WORDCHAR_QUESTION_BUTTON, _hSelf, _hInst, const_cast<PTSTR>(tip2show.c_str()));
			if (_tip)
			{
				SendMessage(_tip, TTM_ACTIVATE, TRUE, 0);
				SendMessage(_tip, TTM_SETMAXTIPWIDTH, 0, 200);

				// Make tip stay 30 seconds
				SendMessage(_tip, TTM_SETDELAYTIME, TTDT_AUTOPOP, MAKELPARAM((30000), (0)));
			}
			return TRUE;
		}

		case WM_CTLCOLORSTATIC:
		{
			HDC hdcStatic = (HDC) wParam;
			HWND hwnd = reinterpret_cast<HWND>(lParam);
			if (hwnd == ::GetDlgItem(_hSelf, IDD_STATIC_BLABLA) || hwnd == ::GetDlgItem(_hSelf, IDD_STATIC_BLABLA2NDLINE))
			{
				COLORREF bgColor = getCtrlBgColor(_hSelf);
				SetTextColor(hdcStatic, RGB(0, 0, 0));
				BYTE r = GetRValue(bgColor) - 30;
				BYTE g = MyGetGValue(bgColor) - 30;
				BYTE b = GetBValue(bgColor) - 30;
				SetBkColor(hdcStatic, RGB(r, g, b));
				return TRUE;
			}
			return FALSE;
		}

		case WM_COMMAND : 
		{
			if (HIWORD(wParam) == EN_CHANGE)
			{
				switch (LOWORD(wParam))
				{
					case  IDC_EDIT_OPENDELIMITER:
					{
						TCHAR opener[2];
						::SendDlgItemMessage(_hSelf, IDC_EDIT_OPENDELIMITER, WM_GETTEXT, MAX_PATH, reinterpret_cast<LPARAM>(opener));
						nppGUI._leftmostDelimiter =  static_cast<char>(opener[0]);
						return TRUE;
					}
					case  IDC_EDIT_CLOSEDELIMITER:
					{
						TCHAR closer[2];
						::SendDlgItemMessage(_hSelf, IDC_EDIT_CLOSEDELIMITER, WM_GETTEXT, MAX_PATH, reinterpret_cast<LPARAM>(closer));
						nppGUI._rightmostDelimiter =  static_cast<char>(closer[0]);
						return TRUE;
					}

					case  IDC_WORDCHAR_CUSTOM_EDIT:
					{
						char customText[MAX_PATH];
						::GetDlgItemTextA(_hSelf, IDC_WORDCHAR_CUSTOM_EDIT, customText, MAX_PATH-1);
						nppGUI._customWordChars = customText;
						setWarningIfNeed();
						::SendMessage(::GetParent(_hParent), NPPM_INTERNAL_SETWORDCHARS, 0, 0);
						return TRUE;
					}

					default:
						return FALSE;
				}
			}

			switch (wParam)
			{

				case IDD_SEVERALLINEMODEON_CHECK :
				{
					bool isChecked = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDD_SEVERALLINEMODEON_CHECK, BM_GETCHECK, 0, 0));
					nppGUI._delimiterSelectionOnEntireDocument = isChecked;
					
					::ShowWindow(::GetDlgItem(_hSelf, IDD_STATIC_BLABLA2NDLINE),isChecked?SW_SHOW:SW_HIDE);

					POINT *p = isChecked?&_multiLineModePoint:&_singleLineModePoint;
					::MoveWindow(::GetDlgItem(_hSelf, IDC_EDIT_CLOSEDELIMITER), p->x, p->y, _closerRect.right, _closerRect.bottom, TRUE);
					::MoveWindow(::GetDlgItem(_hSelf, IDD_STATIC_CLOSEDELIMITER), p->x + _closerRect.right + 4, p->y + 4, _closerLabelRect.right, _closerLabelRect.bottom, TRUE);

					return TRUE;
				}

				case IDC_RADIO_WORDCHAR_DEFAULT:
				{
					::SendDlgItemMessage(_hSelf, IDC_RADIO_WORDCHAR_CUSTOM, BM_SETCHECK, BST_UNCHECKED, 0);
					nppGUI._isWordCharDefault = true;
					::SendMessage(::GetParent(_hParent), NPPM_INTERNAL_SETWORDCHARS, 0, 0);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_WORDCHAR_CUSTOM_EDIT), not nppGUI._isWordCharDefault);
					::SetDlgItemText(_hSelf, IDD_STATIC_WORDCHAR_WARNING, TEXT(""));
					return TRUE;
				}

				case IDC_RADIO_WORDCHAR_CUSTOM:
				{
					::SendDlgItemMessage(_hSelf, IDC_RADIO_WORDCHAR_DEFAULT, BM_SETCHECK, BST_UNCHECKED, 0);
					nppGUI._isWordCharDefault = false;
					::SendMessage(::GetParent(_hParent), NPPM_INTERNAL_SETWORDCHARS, 0, 0);
					::EnableWindow(::GetDlgItem(_hSelf, IDC_WORDCHAR_CUSTOM_EDIT), not nppGUI._isWordCharDefault);
					setWarningIfNeed();
					return TRUE;
				}

				default :
					return FALSE;
			}
		}
		break;
	}

	return FALSE;
}

INT_PTR CALLBACK SettingsOnCloudDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM)
{
	NppParameters& nppParams = *nppParms;
	NppGUI& nppGUI = *nppUIParms;

	if (HIWORD(wParam) == EN_CHANGE)
	{
		switch (LOWORD(wParam))
		{
			case  IDC_CLOUDPATH_EDIT:
			{
				TCHAR inputDir[MAX_PATH] = {'\0'};
				TCHAR inputDirExpanded[MAX_PATH] = {'\0'};
				::SendDlgItemMessage(_hSelf, IDC_CLOUDPATH_EDIT, WM_GETTEXT, MAX_PATH, reinterpret_cast<LPARAM>(inputDir));
				::ExpandEnvironmentStrings(inputDir, inputDirExpanded, MAX_PATH);
				NativeLangSpeaker *pNativeSpeaker = nppParms->getNativeLangSpeaker();
				if (::PathFileExists(inputDirExpanded))
				{
					nppGUI._cloudPath = inputDirExpanded;
					nppParams.setCloudChoice(inputDirExpanded);
					
					generic_string message;
					if (nppParams.isCloudPathChanged())
					{
						message = pNativeSpeaker->getLocalizedStrFromID("cloud-restart-warning", TEXT("Please restart Notepad++ to take effect."));
					}
					::SetDlgItemText(_hSelf, IDC_SETTINGSONCLOUD_WARNING_STATIC, message.c_str());
				}
				else
				{
					bool isChecked = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_WITHCLOUD_RADIO, BM_GETCHECK, 0, 0));
					if (isChecked)
					{
						generic_string message = pNativeSpeaker->getLocalizedStrFromID("cloud-invalid-warning", TEXT("Invalid path."));

						::SetDlgItemText(_hSelf, IDC_SETTINGSONCLOUD_WARNING_STATIC, message.c_str());
						nppParams.removeCloudChoice();
					}
				}
				return TRUE;
			}
		}
	}

	switch (message)
	{
		case WM_INITDIALOG:
		{
			generic_string message = TEXT("");

			bool withCloud = nppGUI._cloudPath != TEXT("");
			if (withCloud)
			{
				// detect validation of path
				if (!::PathFileExists(nppGUI._cloudPath.c_str()))
					message = TEXT("Invalid path");
			}
			
			::SetDlgItemText(_hSelf, IDC_SETTINGSONCLOUD_WARNING_STATIC, message.c_str());

			::SendDlgItemMessage(_hSelf, IDC_NOCLOUD_RADIO, BM_SETCHECK, !withCloud ? BST_CHECKED : BST_UNCHECKED, 0);
			::SendDlgItemMessage(_hSelf, IDC_WITHCLOUD_RADIO, BM_SETCHECK, withCloud ? BST_CHECKED : BST_UNCHECKED, 0);
			::SendDlgItemMessage(_hSelf, IDC_CLOUDPATH_EDIT, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(nppGUI._cloudPath.c_str()));
			::EnableWindow(::GetDlgItem(_hSelf, IDC_CLOUDPATH_EDIT), withCloud);
			::EnableWindow(::GetDlgItem(_hSelf, IDD_CLOUDPATH_BROWSE_BUTTON), withCloud);
		}
		break;

		case WM_COMMAND:
		{
			NativeLangSpeaker *pNativeSpeaker = nppParms->getNativeLangSpeaker();
			switch (wParam)
			{
				case IDC_NOCLOUD_RADIO:
				{
					nppGUI._cloudPath = TEXT("");
					nppParams.removeCloudChoice();

					generic_string message;
					if (nppParams.isCloudPathChanged())
					{
						message = pNativeSpeaker->getLocalizedStrFromID("cloud-restart-warning", TEXT("Please restart Notepad++ to take effect."));
					}
					// else set empty string
					::SetDlgItemText(_hSelf, IDC_SETTINGSONCLOUD_WARNING_STATIC, message.c_str());

					::SendDlgItemMessage(_hSelf, IDC_CLOUDPATH_EDIT, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(nppGUI._cloudPath.c_str()));
					::EnableWindow(::GetDlgItem(_hSelf, IDC_CLOUDPATH_EDIT), false);
					::EnableWindow(::GetDlgItem(_hSelf, IDD_CLOUDPATH_BROWSE_BUTTON), false);
				}
				break;

				case IDC_WITHCLOUD_RADIO:
				{
					generic_string message = pNativeSpeaker->getLocalizedStrFromID("cloud-invalid-warning", TEXT("Invalid path."));
					::SetDlgItemText(_hSelf, IDC_SETTINGSONCLOUD_WARNING_STATIC, message.c_str());

					::EnableWindow(::GetDlgItem(_hSelf, IDC_CLOUDPATH_EDIT), true);
					::EnableWindow(::GetDlgItem(_hSelf, IDD_CLOUDPATH_BROWSE_BUTTON), true);
				}
				break;

				case IDD_CLOUDPATH_BROWSE_BUTTON:
				{
					generic_string message = pNativeSpeaker->getLocalizedStrFromID("cloud-select-folder", TEXT("Select a folder from/to where Notepad++ reads/writes its settings"));
					folderBrowser(_hSelf, message, IDC_CLOUDPATH_EDIT);
				}
				break;

				default:
					return FALSE;

			}
		}																						
	}
	return FALSE;
}

INT_PTR CALLBACK SearchEngineChoiceDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM)
{
	NppParameters& nppParams = *nppParms;
	NppGUI& nppGUI = *nppUIParms;

	if (HIWORD(wParam) == EN_CHANGE)
	{
		switch (LOWORD(wParam))
		{
			case  IDC_SEARCHENGINE_EDIT:
			{
				TCHAR input[MAX_PATH] = { '\0' };
				::SendDlgItemMessage(_hSelf, IDC_SEARCHENGINE_EDIT, WM_GETTEXT, MAX_PATH, reinterpret_cast<LPARAM>(input));
				nppGUI._searchEngineCustom = input;
				return TRUE;
			}
		}
	}

	switch (message)
	{
		case WM_INITDIALOG:
		{
			if (nppGUI._searchEngineChoice == nppGUI.se_custom)
			{
				if (nppGUI._searchEngineCustom.empty())
				{
					nppGUI._searchEngineChoice = nppGUI.se_google;
				}
			}
			::SendDlgItemMessage(_hSelf, IDC_SEARCHENGINE_CUSTOM_RADIO, BM_SETCHECK, nppGUI._searchEngineChoice == nppGUI.se_custom ? BST_CHECKED : BST_UNCHECKED, 0);
			::SendDlgItemMessage(_hSelf, IDC_SEARCHENGINE_DUCKDUCKGO_RADIO, BM_SETCHECK, nppGUI._searchEngineChoice == nppGUI.se_duckDuckGo ? BST_CHECKED : BST_UNCHECKED, 0);
			::SendDlgItemMessage(_hSelf, IDC_SEARCHENGINE_GOOGLE_RADIO, BM_SETCHECK, nppGUI._searchEngineChoice == nppGUI.se_google ? BST_CHECKED : BST_UNCHECKED, 0);
			::SendDlgItemMessage(_hSelf, IDC_SEARCHENGINE_BING_RADIO, BM_SETCHECK, nppGUI._searchEngineChoice == nppGUI.se_bing ? BST_CHECKED : BST_UNCHECKED, 0);
			::SendDlgItemMessage(_hSelf, IDC_SEARCHENGINE_YAHOO_RADIO, BM_SETCHECK, nppGUI._searchEngineChoice == nppGUI.se_yahoo ? BST_CHECKED : BST_UNCHECKED, 0);
			::SendDlgItemMessage(_hSelf, IDC_SEARCHENGINE_STACKOVERFLOW_RADIO, BM_SETCHECK, nppGUI._searchEngineChoice == nppGUI.se_stackoverflow ? BST_CHECKED : BST_UNCHECKED, 0);

			::SendDlgItemMessage(_hSelf, IDC_SEARCHENGINE_EDIT, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(nppGUI._searchEngineCustom.c_str()));
			::EnableWindow(::GetDlgItem(_hSelf, IDC_SEARCHENGINE_EDIT), nppGUI._searchEngineChoice == nppGUI.se_custom);
		}
		break;

		case WM_COMMAND:
		{
			switch (wParam)
			{
				case IDC_SEARCHENGINE_DUCKDUCKGO_RADIO:
				{
					nppGUI._searchEngineChoice = nppGUI.se_duckDuckGo;
					::EnableWindow(::GetDlgItem(_hSelf, IDC_SEARCHENGINE_EDIT), false);
				}
				break;

				case IDC_SEARCHENGINE_GOOGLE_RADIO:
				{
					nppGUI._searchEngineChoice = nppGUI.se_google;
					::EnableWindow(::GetDlgItem(_hSelf, IDC_SEARCHENGINE_EDIT), false);
				}
				break;

				case IDC_SEARCHENGINE_BING_RADIO:
				{
					nppGUI._searchEngineChoice = nppGUI.se_bing;
					::EnableWindow(::GetDlgItem(_hSelf, IDC_SEARCHENGINE_EDIT), false);
				}
				break;

				case IDC_SEARCHENGINE_YAHOO_RADIO:
				{
					nppGUI._searchEngineChoice = nppGUI.se_yahoo;
					::EnableWindow(::GetDlgItem(_hSelf, IDC_SEARCHENGINE_EDIT), false);
				}
				break;

				case IDC_SEARCHENGINE_STACKOVERFLOW_RADIO:
				{
					nppGUI._searchEngineChoice = nppGUI.se_stackoverflow;
					::EnableWindow(::GetDlgItem(_hSelf, IDC_SEARCHENGINE_EDIT), false);
				}
				break;

				case IDC_SEARCHENGINE_CUSTOM_RADIO:
				{
					nppGUI._searchEngineChoice = nppGUI.se_custom;
					::EnableWindow(::GetDlgItem(_hSelf, IDC_SEARCHENGINE_EDIT), true);
				}
				break;

				case IDC_CHECK_CONFIRMREPLOPENDOCS:
				{
					nppGUI._confirmReplaceInAllOpenDocs = isCheckedOrNot(IDC_CHECK_CONFIRMREPLOPENDOCS);
					return TRUE;
				}
				break;

				default:
					return FALSE;
			}
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK SearchingSettingsDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM)
{
	NppParameters& nppParams = *nppParms;
	NppGUI& nppGUI = *nppUIParms;

	switch (message)
	{
		case WM_INITDIALOG:
		{
			::SendDlgItemMessage(_hSelf, IDC_CHECK_STOPFILLINGFINDFIELD, BM_SETCHECK, nppGUI._fillFindFieldTS, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_FILLCARETFINDFIELD, BM_SETCHECK, nppGUI._fillFindFieldNS, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_MONOSPACEDFONT_FINDDLG, BM_SETCHECK, nppGUI._monospacedFontFindDlg, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_ENLARGED_FINDDLG, BM_SETCHECK, nppGUI._enlargedFontFindDlg, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_FINDDLG_ALWAYS_VISIBLE, BM_SETCHECK, nppGUI._findDlgAlwaysVisible, 0);
			::SendDlgItemMessage(_hSelf, IDC_CHECK_CONFIRMREPLOPENDOCS, BM_SETCHECK, nppGUI._confirmReplaceInAllOpenDocs, 0);
		}
		break;

		case WM_COMMAND:
		{
			switch (wParam)
			{
				case IDC_CHECK_STOPFILLINGFINDFIELD:
				{
					nppGUI._fillFindFieldTS = isCheckedOrNot(IDC_CHECK_STOPFILLINGFINDFIELD);
					return TRUE;
				}
				break;

				case IDC_CHECK_FILLCARETFINDFIELD:
				{
					nppGUI._fillFindFieldNS = isCheckedOrNot(IDC_CHECK_FILLCARETFINDFIELD);
					return TRUE;
				}
				break;

				case IDC_CHECK_MONOSPACEDFONT_FINDDLG:
				case IDC_CHECK_ENLARGED_FINDDLG:
				{
					nppGUI._monospacedFontFindDlg = isCheckedOrNot(IDC_CHECK_MONOSPACEDFONT_FINDDLG);
					nppGUI._enlargedFontFindDlg = isCheckedOrNot(IDC_CHECK_ENLARGED_FINDDLG);
					nppApp->_findReplaceDlg.RefreshComboFonts(0);
					return TRUE;
				}
				break;

				case IDC_CHECK_FINDDLG_ALWAYS_VISIBLE:
				{
					nppGUI._findDlgAlwaysVisible = isCheckedOrNot(IDC_CHECK_FINDDLG_ALWAYS_VISIBLE);
					return TRUE;
				}
				break;

				default:
					return FALSE;
			}
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK PreferenceDlg::DlgProcShellSettings(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	#define GUID_SIZE			128
	#define GUID_STRING_SIZE	40
	#define TITLE_SIZE			64
	static DWORD isDynamic = 1;
	static DWORD isDynamIconShow = 2; //0 off, 1 on, 2 unknown

	static TCHAR cmdArgs[MAX_PATH] = {0};
	static TCHAR szKeyTemp[MAX_PATH + GUID_STRING_SIZE];

	static TCHAR menuText[TITLE_SIZE] = TEXT("Edit with &Textrument"); 
	static DWORD menuShow = 0;	//0 off, 1 on
	static DWORD menuIconShow = 2;	// 0 off, otherwise on

	HKEY hKey;
	LRESULT lResult;
	DWORD size = 0;
	//---------------------------------------------------------------------------
	// RegisterServer
	// Create registry entries and setup the shell extension
	//---------------------------------------------------------------------------
	switch(uMsg) {
		// 获取已经存在的数值，并显示对话框。
		case 0: {
			wsprintf(szKeyTemp, TEXT("CLSID\\%s\\InprocServer32"), szGUID);
			lResult = RegOpenKeyEx(HKEY_CLASSES_ROOT, szKeyTemp, 0, KEY_READ, &hKey);
			if (lResult == ERROR_SUCCESS) {
				size = sizeof(TCHAR)*MAX_PATH;
				if(RegQueryValueEx(hKey, TEXT("ThreadingModel"), NULL, NULL, (LPBYTE)(cmdArgs), &size)==ERROR_SUCCESS) {
					menuShow = lstrcmp(cmdArgs, TEXT("Apartment"))==0;
				}
				RegCloseKey(hKey);
			}

			cmdArgs[0]='\0';

			wsprintf(szKeyTemp, TEXT("CLSID\\%s\\Settings"), szGUID);
			lResult = RegOpenKeyEx(HKEY_CLASSES_ROOT, szKeyTemp, 0, KEY_READ, &hKey);
			
			if (lResult == ERROR_SUCCESS) {
				size = sizeof(TCHAR)*TITLE_SIZE;
				RegQueryValueEx(hKey, TEXT("Title"), NULL, NULL, (LPBYTE)(menuText), &size);

				size = sizeof(TCHAR)*MAX_PATH;
				RegQueryValueEx(hKey, TEXT("Custom"), NULL, NULL, (LPBYTE)(cmdArgs), &size);

				size = sizeof(DWORD);
				RegQueryValueEx(hKey, TEXT("Dynamic"), NULL, NULL, (BYTE*)(&isDynamic), &size);

				size = sizeof(DWORD);
				RegQueryValueEx(hKey, TEXT("ShowIcon"), NULL, NULL, (BYTE*)(&menuIconShow), &size);

				RegCloseKey(hKey);
			}

			//DlgProcSettings(NULL,1,0,0);
			DialogBox(nppApp->getHinst()
				, MAKEINTRESOURCE(IDD_DIALOG_SHELLSETTINGS)
				, hwndDlg, (DLGPROC)DlgProcShellSettings);
		} break;
		// 卸载
		case 1: {
			if(IDYES==::MessageBox(hwndDlg, TEXT("确认卸载注册信息？"), TEXT(""), MB_YESNO | MB_DEFBUTTON2 | MB_TASKMODAL)) {
#ifdef _WIN64
				TCHAR szShellExtensionKey[] = TEXT("*\\shellex\\ContextMenuHandlers\\ATextrument64");
#else
				TCHAR szShellExtensionKey[] = TEXT("*\\shellex\\ContextMenuHandlers\\ATextrument");
#endif
				RegDeleteKey(HKEY_CLASSES_ROOT, szShellExtensionKey);
				RegDeleteKey(HKEY_CLASSES_ROOT, TEXT("Notepad++_file\\shellex\\IconHandler"));
				RegDeleteKey(HKEY_CLASSES_ROOT, TEXT("Notepad++_file\\shellex"));
				wsprintf(szKeyTemp, TEXT("CLSID\\%s\\InprocServer32"), szGUID);
				RegDeleteKey(HKEY_CLASSES_ROOT, szKeyTemp);
				wsprintf(szKeyTemp, TEXT("CLSID\\%s\\Settings"), szGUID);
				RegDeleteKey(HKEY_CLASSES_ROOT, szKeyTemp);
				wsprintf(szKeyTemp, TEXT("CLSID\\%s"), szGUID);
				RegDeleteKey(HKEY_CLASSES_ROOT, szKeyTemp);
			}
		} return TRUE;
		case WM_INITDIALOG: {
			//初始化UI
			Button_SetCheck(GetDlgItem(hwndDlg, IDC_CHECK_USECONTEXT), menuShow?BST_CHECKED:BST_UNCHECKED);
			Button_SetCheck(GetDlgItem(hwndDlg, IDC_CHECK_CONTEXTICON), menuIconShow?BST_CHECKED:BST_UNCHECKED);

			Button_SetCheck(GetDlgItem(hwndDlg, IDC_CHECK_ISDYNAMIC), isDynamic?BST_CHECKED:BST_UNCHECKED);
			Button_SetCheck(GetDlgItem(hwndDlg, IDC_CHECK_USEICON), isDynamIconShow?(isDynamIconShow==1?BST_CHECKED:BST_INDETERMINATE):BST_UNCHECKED);

			SetDlgItemText(hwndDlg, IDC_EDIT_MENU, menuText);
			SetDlgItemText(hwndDlg, IDC_EDIT_COMMAND, cmdArgs);

			_preferenceDlg->goToCenter(hwndDlg);
		} return TRUE; 
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case IDOK: {
					TCHAR szSubKey[MAX_PATH];
					TCHAR szDefaultCustomcommand[] = TEXT("");
#ifdef _WIN64
					TCHAR szShellExtensionTitle[] = TEXT("ATextrument64");
					TCHAR szShellExtensionKey[] = TEXT("*\\shellex\\ContextMenuHandlers\\ATextrument64");
					TCHAR ShellDllName[] = TEXT("NppShell64.dll");
#else
					TCHAR szShellExtensionTitle[] = TEXT("ATextrument");
					TCHAR szShellExtensionKey[] = TEXT("*\\shellex\\ContextMenuHandlers\\ATextrument");
					TCHAR ShellDllName[] = TEXT("NppShell.dll");
#endif
					const TCHAR* szModule = nppApp->getModuleFileName();

					//todo fuck path append

					//todo fuck path remove acrom

					TCHAR shellDir[MAX_PATH];
					lstrcpy(shellDir, nppParms->_nppPath);
					PathAppend(shellDir, ShellDllName);

					bool checkShell=false;
					static DOREGSTRUCT ClsidEntries[] = {
						{HKEY_CLASSES_ROOT,	TEXT("CLSID\\%s"), NULL,  REG_SZ, szShellExtensionTitle},
						{HKEY_CLASSES_ROOT,	TEXT("CLSID\\%s\\InprocServer32"), NULL, REG_SZ, shellDir},
						{HKEY_CLASSES_ROOT,	TEXT("CLSID\\%s\\InprocServer32"), TEXT("ThreadingModel"), REG_SZ, TEXT("Apartment")},
						{HKEY_CLASSES_ROOT,	szShellExtensionKey, NULL, REG_SZ, szGUID},
					};
					// Register the CLSID entries
					for(int i = 0; i<4; i++) {
						auto & regInfoI = ClsidEntries[i];
						wsprintf(szSubKey, regInfoI.szSubKey, szGUID); // 填入ID
						hKey=0;
						bool doit=i==2||RegOpenKeyEx(regInfoI.hRootKey, szSubKey, 0, KEY_READ, &hKey);
						if(i==1 && !doit && hKey) {
							checkShell=1;
							size = MAX_PATH*sizeof(TCHAR);
							lResult = RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE)(szSubKey), &size);
							if (NOERROR == lResult) {
								if(lstrcmp(szSubKey, shellDir)) {
									if(szSubKey[0]==0||IDYES==::MessageBox(hwndDlg, TEXT("Do you wish to overwrite the path of NppShell.dll?"), TEXT(""), MB_YESNO | MB_DEFBUTTON2 | MB_TASKMODAL)) {
										if(!PathFileExists(shellDir)) { //todo zip dll in exe
											::MessageBox(hwndDlg, TEXT("NppShell.dll not found : Ignore!"), TEXT(""), MB_OK | MB_TASKMODAL);
										} else {
											doit=1;
											wsprintf(szSubKey, regInfoI.szSubKey, szGUID); // 再次填入ID
										}
									}
								}
							}
						}
						if(i==4 && !menuShow) {
							if(!doit) {
								RegDeleteKey(HKEY_CLASSES_ROOT, szShellExtensionKey);
							}
							doit=0;
						}
						if(doit) {
							auto szData = i==2&&!menuShow?_T("x"):regInfoI.szData; // 是否抹除 Apartment 使服务失效
							RegCloseKey(hKey);
							lResult = RegCreateKeyEx(regInfoI.hRootKey, szSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
							if (lResult == ERROR_SUCCESS) {
								lResult = RegSetValueEx(hKey, regInfoI.lpszValueName, 0, regInfoI.type, (LPBYTE)szData, regInfoI.type==REG_SZ?(lstrlen(szData)+1) * sizeof(TCHAR):sizeof(DWORD));
							}
						}
						RegCloseKey(hKey);
					}

					//Register settings
					GetDlgItemText(hwndDlg, IDC_EDIT_MENU, menuText, TITLE_SIZE);
					GetDlgItemText(hwndDlg, IDC_EDIT_COMMAND, cmdArgs, MAX_PATH);

					wsprintf(szKeyTemp, TEXT("CLSID\\%s\\Settings"), szGUID);
					lResult = RegCreateKeyEx(HKEY_CLASSES_ROOT, szKeyTemp, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
					if (lResult == ERROR_SUCCESS) {
						RegSetValueEx(hKey, TEXT("Title"), 0,REG_SZ, (LPBYTE)menuText, (lstrlen(menuText)+1)*sizeof(TCHAR));
						RegSetValueEx(hKey, TEXT("Custom"), 0,REG_SZ, (LPBYTE)cmdArgs, (lstrlen(cmdArgs)+1)*sizeof(TCHAR));
						RegSetValueEx(hKey, TEXT("Path"), 0,REG_SZ, (LPBYTE)szModule, (lstrlen(szModule)+1)*sizeof(TCHAR));				
						RegSetValueEx(hKey, TEXT("Dynamic"), 0, REG_DWORD, (LPBYTE)&isDynamic, sizeof(DWORD));
						RegSetValueEx(hKey, TEXT("ShowIcon"), 0, REG_DWORD, (LPBYTE)&menuIconShow, sizeof(DWORD));
						RegCloseKey(hKey);
					}

					if (isDynamIconShow == 1) {
						lResult = RegCreateKeyEx(HKEY_CLASSES_ROOT, TEXT("Notepad++_file\\shellex\\IconHandler"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
						if (lResult == ERROR_SUCCESS) {
							lResult = RegSetValueEx(hKey, NULL, 0,REG_SZ, (LPBYTE)szGUID, (lstrlen(szGUID)+1)*sizeof(TCHAR));
							RegCloseKey(hKey);
						}
					} else if (isDynamIconShow == 0) {
						RegDeleteKey(HKEY_CLASSES_ROOT, TEXT("Notepad++_file\\shellex\\IconHandler"));
						RegDeleteKey(HKEY_CLASSES_ROOT, TEXT("Notepad++_file\\shellex"));
					}

					PostMessage(hwndDlg, WM_CLOSE, 0, 0);
				} break; 
				case IDC_CHECK_USECONTEXT: {
					menuShow = Button_GetCheck((HWND)lParam)==BST_CHECKED;
				} break; 
				case IDC_CHECK_USEICON: {
					int state = Button_GetCheck((HWND)lParam);
					if (state == BST_CHECKED)
						isDynamIconShow = 1;
					else if (state == BST_UNCHECKED)
						isDynamIconShow = 0;
					else
						isDynamIconShow = 2;
				} break; 
				case IDC_CHECK_CONTEXTICON: {
					menuIconShow = Button_GetCheck((HWND)lParam)==BST_CHECKED;
				} break; 
				case IDC_CHECK_ISDYNAMIC: {
					isDynamic = Button_GetCheck((HWND)lParam)==BST_CHECKED;
				} break;
			}
			return TRUE;
		} break; 
		case WM_CLOSE: {
			EndDialog(hwndDlg, 0);
			return TRUE;
		} break; 
	}

	return FALSE;
}
