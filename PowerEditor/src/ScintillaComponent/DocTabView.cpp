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



#include "DocTabView.h"
#include "ScintillaEditView.h"
#include "Notepad_plus.h"

#include "WindowsEx/comctl32.h"

#ifndef _WIN32_IE
#define _WIN32_IE	0x0600
#endif //_WIN32_IE

bool bNewTabFarRight = 1;

bool DocTabView::_hideTabBarStatus = false;

extern Notepad_plus* nppApp;

void DocTabView::addBuffer(BufferID buffer)
{
	if (buffer == BUFFER_INVALID)	//valid only
		return;
	if (this->getIndexByBuffer(buffer) != -1)	//no duplicates
		return;
	Buffer * buf = MainFileManager.getBufferByID(buffer);
	TCITEM tie;
	tie.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM;

	int index = -1;
	if (_hasImgLst)
		index = 0;
	tie.iImage = index;
	tie.pszText = const_cast<TCHAR *>(buf->getFileName());
	tie.lParam = reinterpret_cast<LPARAM>(buffer);

	size_t newTabIndex;

	if (!bNewTabFarRight)
		newTabIndex = getCurrentTabIndex() + 1;
	else
		newTabIndex = _nbItem;

	_nbItem++;

	::SendMessage(_hSelf, TCM_INSERTITEM, newTabIndex, reinterpret_cast<LPARAM>(&tie));
	
	//::SendMessage(_hSelf, TCM_INSERTITEM, _nbItem++, reinterpret_cast<LPARAM>(&tie));
	bufferUpdated(buf, BufferChangeMask);

	::SendMessage(_hParent, WM_SIZE, 0, 0);
}


void DocTabView::closeBuffer(BufferID buffer)
{
	int indexToClose = getIndexByBuffer(buffer);
	deletItemAt((size_t)indexToClose);
	::SendMessage(_hParent, WM_SIZE, 0, 0);
}


bool DocTabView::activateBuffer(BufferID buffer)
{
	int indexToActivate = getIndexByBuffer(buffer);
	if (indexToActivate == -1)
		return false;	//cannot activate

	activateAt(indexToActivate);
	return true;
}


BufferID DocTabView::activeBuffer()
{
	int index = getCurrentTabIndex();
	return static_cast<BufferID>(getBufferByIndex(index));
}


BufferID DocTabView::findBufferByName(const TCHAR * fullfilename) //-1 if not found, something else otherwise
{
	TCITEM tie;
	tie.lParam = -1;
	tie.mask = TCIF_PARAM;
	for (size_t i = 0; i < _nbItem; ++i)
	{
		::SendMessage(_hSelf, TCM_GETITEM, i, reinterpret_cast<LPARAM>(&tie));
		BufferID id = reinterpret_cast<BufferID>(tie.lParam);
		Buffer * buf = MainFileManager.getBufferByID(id);
		if (OrdinalIgnoreCaseCompareStrings(fullfilename, buf->getFullPathName()) == 0)
		{
			return id;
		}
	}
	return BUFFER_INVALID;
}


int DocTabView::getIndexByBuffer(BufferID id)
{
	TCITEM tie;
	tie.lParam = -1;
	tie.mask = TCIF_PARAM;
	for (size_t i = 0; i < _nbItem; ++i)
	{
		::SendMessage(_hSelf, TCM_GETITEM, i, reinterpret_cast<LPARAM>(&tie));
		if (reinterpret_cast<BufferID>(tie.lParam) == id)
			return static_cast<int>(i);
	}
	return -1;
}


BufferID DocTabView::getBufferByIndex(size_t index)
{
	TCITEM tie;
	tie.lParam = -1;
	tie.mask = TCIF_PARAM;
	::SendMessage(_hSelf, TCM_GETITEM, index, reinterpret_cast<LPARAM>(&tie));

	return reinterpret_cast<BufferID>(tie.lParam);
}


void DocTabView::bufferUpdated(Buffer * buffer, int mask)
{
	int index = getIndexByBuffer(buffer->getID());
	if (index == -1)
		return;

	TCITEM tie;
	tie.lParam = -1;
	tie.mask = 0;

	if (mask & BufferChangeReadonly || mask & BufferChangeDirty)
	{
		tie.mask |= TCIF_IMAGE;
		tie.iImage = buffer->isDirty()?UNSAVED_IMG_INDEX:SAVED_IMG_INDEX;
		if (buffer->isMonitoringOn())
		{
			tie.iImage = MONITORING_IMG_INDEX;
		}
		else if (buffer->isReadOnly())
		{
			tie.iImage = REDONLY_IMG_INDEX;
		}
	}

	//We must make space for the added ampersand characters.
	TCHAR encodedLabel[2 * MAX_PATH];

	if (mask & BufferChangeFilename)
	{
		tie.mask |= TCIF_TEXT;
		tie.pszText = const_cast<TCHAR *>(encodedLabel);

		{
			const TCHAR* in = buffer->getFileName();
			TCHAR* out = encodedLabel;

			//This code will read in one character at a time and duplicate every first ampersand(&).
			//ex. If input is "test & test && test &&&" then output will be "test && test &&& test &&&&".
			//Tab's caption must be encoded like this because otherwise tab control would make tab too small or too big for the text.

			while (*in != 0)
			if (*in == '&')
			{
				*out++ = '&';
				*out++ = '&';
				while (*(++in) == '&')
					*out++ = '&';
			}
			else
				*out++ = *in++;
			*out = '\0';
		}
	}

	::SendMessage(_hSelf, TCM_SETITEM, index, reinterpret_cast<LPARAM>(&tie));

	// send WM_SIZE only when change tab
	// It is needed while a tab is closed (so tab changed) in multi-line tab mode
	// here causes tab flickering
	//if (mask & BufferChangeRecentTag) ::SendMessage(_hParent, WM_SIZE, 0, 0);
}


void DocTabView::setBuffer(size_t index, BufferID id)
{
	if (index < 0 || index >= _nbItem)
		return;

	TCITEM tie;
	tie.lParam = reinterpret_cast<LPARAM>(id);
	tie.mask = TCIF_PARAM;
	::SendMessage(_hSelf, TCM_SETITEM, index, reinterpret_cast<LPARAM>(&tie));

	bufferUpdated(MainFileManager.getBufferByID(id), BufferChangeMask);	//update tab, everything has changed

	::SendMessage(_hParent, WM_SIZE, 0, 0);
}


void DocTabView::reSizeTo(RECT & rc)
{
	int borderWidth = ((NppParameters::getInstance()).getSVP())._borderWidth;
	int borderXComp = ((NppParameters::getInstance()).getSVP())._borderWidthXCompat;
	if (_hideTabBarStatus)
	{
		RECT rcTmp = rc;
		TabBar::reSizeTo(rcTmp);
		_pView->reSizeTo(rc);
	}
	else
	{
		int thisTop = rc.top;
		TabBar::reSizeTo(rc);
		//static int cc=0;LogIs("%d reSizeTo:: %d %d %d %d", cc++, rc.left, rc.top, rc.right, rc.bottom);
		rc.left	 += borderWidth;
		rc.right -= borderWidth * 2;
		if(borderXComp) {
			rc.left	 -= borderXComp;
			rc.right += borderXComp * 2;
			auto vInf = nppApp->_dockingManager.getContainerInfo();
			if(vInf[0]->isVisible() || nppApp->_subSplitter.isVisible()&&this==&nppApp->_subDocTab) {
				rc.left	 += borderXComp;
				rc.right	 -= borderXComp;
			}
			if(vInf[1]->isVisible()|| nppApp->_subSplitter.isVisible()&&this==&nppApp->_mainDocTab) {
				rc.right	 -= borderXComp;
			}
		}
		if (_isVertical)
		{
			rc.right -= TabCtrl_GetVerticalModeWidth(_hSelf);
		}
		rc.top   += borderWidth;
		if (_isVertical||rowCount>1)
		{
			rc.bottom -= (rc.top-thisTop-borderWidth*3);
		} 
		else
		{
			rc.bottom -= (borderWidth * 2);
		}
		_pView->reSizeTo(rc);
	}
	SendMessage(_hParent, NPPM_INTERNAL_UPDATECLICKABLELINKS, reinterpret_cast<WPARAM>(_pView), 0);
}

