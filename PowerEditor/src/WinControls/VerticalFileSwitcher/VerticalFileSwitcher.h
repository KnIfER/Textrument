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

#include "DockingDlgInterface.h"
#include "VerticalFileSwitcher_rc.h"
#include "VerticalFileSwitcherListView.h"

#define FS_PROJECTPANELTITLE		TEXT("标签页列表")

struct sortCompareData {
  HWND hListView;
  int columnIndex;
  int sortDirection;
};

class VerticalFileSwitcher : public DockingDlgInterface {
public:
	VerticalFileSwitcher(): DockingDlgInterface(IDD_FILESWITCHER_PANEL) {};

	void init(HINSTANCE hInst, HWND hPere, HIMAGELIST hImaLst) {
		DockingDlgInterface::init(hInst, hPere);
		_hImaLst = hImaLst;
	};

    virtual void display(bool toShow = true) const {
        DockingDlgInterface::display(toShow);
    };

    void setParent(HWND parent2set){
        _hParent = parent2set;
    };

	virtual void setClosed(bool toClose);

	//Activate document in scintilla by using the internal index
	void activateDoc(TaskLstFnStatus *tlfs) const;

	int newItem(BufferID bufferID, int iView){
		return _fileListView.newItem(bufferID, iView);
	};

	int closeItem(BufferID bufferID, int iView){
		return _fileListView.closeItem(bufferID, iView);
	};

	void activateItem(BufferID bufferID, int iView) {
		_fileListView.activateItem(bufferID, iView);
	};

	void setItemIconStatus(BufferID bufferID) {
		_fileListView.setItemIconStatus(bufferID) ;
	};

	generic_string getFullFilePath(size_t i) const {
		return _fileListView.getFullFilePath(i);
	};

	int setHeaderOrder(int columnIndex);
	void updateHeaderArrow();

	int nbSelectedFiles() const {
		return _fileListView.nbSelectedFiles();
	};

	std::vector<SwitcherFileInfo> getSelectedFiles(bool reverse = false) const {
		return _fileListView.getSelectedFiles(reverse);
	};
	
	void startColumnSort();
	
	void reload(){
		_fileListView.reload();
		startColumnSort();
	};
	
	void updateTabOrder(){
		if (_lastSortingDirection == SORT_DIRECTION_NONE) {
			_fileListView.reload();
		}
	};

	virtual void setBackgroundColor(COLORREF bgColour) {
		_fileListView.setBackgroundColor(bgColour);
    };

	virtual void setForegroundColor(COLORREF fgColour) {
		_fileListView.setForegroundColor(fgColour);
    };
protected:
	virtual INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
private:
	int _lastSortingColumn = 0;
	int _lastSortingDirection = SORT_DIRECTION_NONE;
	VerticalFileSwitcherListView _fileListView;
	HIMAGELIST _hImaLst = nullptr;
};
