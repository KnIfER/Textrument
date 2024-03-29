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


#pragma once

#include "ControlsTab.h"
#include "preference_rc.h"
#include "URLCtrl.h"
#include "Parameters.h"
#include "regExtDlg.h"
#include "WordStyleDlg.h"

class MiscSettingsDlg : public StaticDialog
{
public :
	MiscSettingsDlg() = default;

private :
	INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
};

class GeneralSubDlg : public StaticDialog
{
public :
	GeneralSubDlg() = default;
private :
	INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
};

class EditingSubDlg : public StaticDialog
{
public :
	EditingSubDlg() = default;
	virtual void destroy() {
		if (_multiEdgeTip)
			::DestroyWindow(_multiEdgeTip);
	};
	
private :
	HWND _multiEdgeTip = nullptr;
	INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
	void initScintParam();
};

struct LangID_Name
{
	LangType _id;
	generic_string _name;
	LangID_Name(LangType id, const generic_string& name) : _id(id), _name(name){};
};

class DefaultNewDocDlg : public StaticDialog
{
public :
	DefaultNewDocDlg() = default;

private :
	std::vector<LangID_Name> _langList;
	void makeOpenAnsiAsUtf8(bool doIt){
		if (!doIt)
			::SendDlgItemMessage(_hSelf, IDC_CHECK_OPENANSIASUTF8, BM_SETCHECK, BST_UNCHECKED, 0);
		::EnableWindow(::GetDlgItem(_hSelf, IDC_CHECK_OPENANSIASUTF8), doIt);
	};
	INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
};

class DefaultDirectoryDlg : public StaticDialog
{
public :
	DefaultDirectoryDlg() = default;
	static LONG_PTR originalComboEditProc;
	// Window procedure for the filter
	static LRESULT FAR PASCAL comboEditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
private :
	INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
};

class RecentFilesHistoryDlg : public StaticDialog
{
public :
	RecentFilesHistoryDlg() = default;
	virtual void destroy() {
		_nbHistoryVal.destroy();
		_customLenVal.destroy();
	};
private :
	URLCtrl _nbHistoryVal;
	URLCtrl _customLenVal;
	std::vector<LangID_Name> _langList;
	void setCustomLen(int val);
	INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
};

class LangMenuDlg : public StaticDialog
{
public :
	LangMenuDlg() = default;
	virtual void destroy() {
		_tabSizeVal.destroy();
	};

private :
    LexerStylerArray _lsArray;
	URLCtrl _tabSizeVal;
	INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
	std::vector<LangMenuItem> _langList;
};

class Highlighting : public StaticDialog
{
public :
	Highlighting() = default;

private :

	INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
};


class SearchingSettingsDlg : public StaticDialog
{
public:
	SearchingSettingsDlg() = default;

private:
	INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
};

struct strCouple {
	generic_string _varDesc;
	generic_string _var;
	strCouple(const TCHAR *varDesc, const TCHAR *var): _varDesc(varDesc), _var(var){};
};

class PrintSettingsDlg : public StaticDialog
{
public :
	PrintSettingsDlg() = default;

private :
	INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
	std::vector<strCouple> varList;
	int _focusedEditCtrl = 0;
};

class BackupDlg : public StaticDialog
{
public :
	BackupDlg() = default;

private :
	void updateBackupGUI();
	INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
};


class AutoCompletionDlg : public StaticDialog
{
public :
	AutoCompletionDlg() = default;
private :
	URLCtrl _nbCharVal;
	INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
};

class MultiInstDlg : public StaticDialog
{
public :
	MultiInstDlg() = default;

private :
	INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
};

class DelimiterSettingsDlg : public StaticDialog
{
public :
	DelimiterSettingsDlg() = default;
	~DelimiterSettingsDlg() {
		if (_tip)
			::DestroyWindow(_tip);
	};

private :
	POINT _singleLineModePoint, _multiLineModePoint;
	RECT _closerRect, _closerLabelRect;
	HWND _tip = nullptr;

	INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
	void detectSpace(const char *text2Check, int & nbSp, int & nbTab) const;
	generic_string getWarningText(size_t nbSp, size_t nbTab) const;
	void setWarningIfNeed() const;
};

class SettingsOnCloudDlg : public StaticDialog
{
public :
	SettingsOnCloudDlg() = default;

private :
	INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
};

class SearchEngineChoiceDlg : public StaticDialog
{
public :
	SearchEngineChoiceDlg() = default;

private :
	INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
};

class DarkModeSubDlg : public StaticDialog
{
public:
	DarkModeSubDlg() = default;

private:
	ColourPicker* _pBackgroundColorPicker = nullptr;
	ColourPicker* _pSofterBackgroundColorPicker = nullptr;
	ColourPicker* _pHotBackgroundColorPicker = nullptr;
	ColourPicker* _pPureBackgroundColorPicker = nullptr;
	ColourPicker* _pErrorBackgroundColorPicker = nullptr;
	ColourPicker* _pTextColorPicker = nullptr;
	ColourPicker* _pDarkerTextColorPicker = nullptr;
	ColourPicker* _pDisabledTextColorPicker = nullptr;
	ColourPicker* _pEdgeColorPicker = nullptr;
	ColourPicker* _pLinkColorPicker = nullptr;

	INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
	void enableCustomizedColorCtrls(bool doEnable);
	void move2CtrlLeft(int ctrlID, HWND handle2Move, int handle2MoveWidth, int handle2MoveHeight);
};


class PreferenceDlg : public StaticDialog
{
friend class NativeLangSpeaker;
friend class Notepad_plus;
public :
	PreferenceDlg() = default;

    void init(HINSTANCE hInst, HWND parent);

	void NotifyReturnPressed();

	std::vector<generic_string> getDropFiltersHistory();

	void buildDropFilters(std::vector<generic_string> & pattern);

    void doDialog(bool isRTL = false);

	bool renameDialogTitle(const TCHAR *internalName, const TCHAR *newName);
	
	int getListSelectedIndex() const {
		return static_cast<int32_t>(::SendDlgItemMessage(_hSelf, IDC_LIST_DLGTITLE, LB_GETCURSEL, 0, 0));
	};

	void showDialogByName(const TCHAR *name) const;
	bool setListSelection(size_t currentSel) const;

	void initAllPanels() const;
	void refresh() const;
	void refreshCurrentDlg(int index) const;

	virtual void destroy();

	void invalidateRadioBtns(bool forPrefDLg=false);

	static INT_PTR CALLBACK DlgProcShellSettings(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
private :
	INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
	void makeCategoryList();
	int32_t getIndexFromName(const TCHAR *name) const;
	void showDialogByIndex(size_t index, bool init=false) const;
	WindowVector _wVector;
	GeneralSubDlg _barsDlg;
	EditingSubDlg _marginsDlg;
	MiscSettingsDlg _miscSettingsDlg;
	RegExtDlg _fileAssocDlg;
	LangMenuDlg _langMenuDlg;
	Highlighting _highlighting;
	PrintSettingsDlg _printSettingsDlg;
	DefaultNewDocDlg _defaultNewDocDlg;
	DefaultDirectoryDlg	_defaultDirectoryDlg;
	RecentFilesHistoryDlg _recentFilesHistoryDlg;
	BackupDlg _backupDlg;
	AutoCompletionDlg _autoCompletionDlg;
	MultiInstDlg _multiInstDlg;
	DelimiterSettingsDlg _delimiterSettingsDlg;
	SettingsOnCloudDlg _settingsOnCloudDlg;
	SearchEngineChoiceDlg _searchEngineDlg;
	SearchingSettingsDlg _searchingSettingsDlg;
	DarkModeSubDlg _darkModeSubDlg;
};

