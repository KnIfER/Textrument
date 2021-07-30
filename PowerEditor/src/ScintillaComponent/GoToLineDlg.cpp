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


#include "GoToLineDlg.h"
#include "DarkMode\DarkModePlus.h"


INT_PTR CALLBACK GoToLineDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM)
{
	switch (message) 
	{
		case WM_INITDIALOG :
		{
			NppDarkMode::autoSubclassAndThemeChildControls(_hSelf);

			::SendDlgItemMessage(_hSelf, IDC_RADIO_GOTOLINE, BM_SETCHECK, TRUE, 0);
			goToCenter();
			return TRUE;
		}

		DMPlus_handleEdit
		DMPlus_handleDLG
		DMPlus_handlePrint
		DMPlus_handleErase
		DMPlus_handleRefresh


		case WM_COMMAND : 
		{
			switch (wParam)
			{
				case IDCANCEL : // Close
					display(false);
                    //cleanLineEdit();
					return TRUE;

				case IDOK :
                {
					execGoToLn(_hSelf, ID_GOLINE_EDIT);
					display(false);
                    return TRUE;
                }

				case IDC_RADIO_GOTOLINE :
				case IDC_RADIO_GOTOOFFSET :
				{
				
					bool isLine, isOffset;
					if (wParam == IDC_RADIO_GOTOLINE)
					{
						isLine = true;
						isOffset = false;
						_mode = go2line;
					}
					else
					{
						isLine = false;
						isOffset = true;
						_mode = go2offsset;
					}
					::SendDlgItemMessage(_hSelf, IDC_RADIO_GOTOLINE, BM_SETCHECK, isLine, 0);
					::SendDlgItemMessage(_hSelf, IDC_RADIO_GOTOOFFSET, BM_SETCHECK, isOffset, 0);
					updateLinesNumbers();
					return TRUE;
				}
				default :
				{
					switch (HIWORD(wParam))
					{
						case EN_SETFOCUS :
						case BN_SETFOCUS :
							updateLinesNumbers();
							return TRUE;
						default :
							return TRUE;
					}
					break;
				}
			}
		}

		default :
			return FALSE;
	}
	return FALSE;
}

void GoToLineDlg::updateLinesNumbers() const 
{
	unsigned int current = 0;
	unsigned int limit = 0;
	
	if (_mode == go2line)
	{
		current = static_cast<unsigned int>((*_ppEditView)->getCurrentLineNumber() + 1);
		limit = static_cast<unsigned int>((*_ppEditView)->execute(SCI_GETLINECOUNT));
	}
	else
	{
		current = static_cast<unsigned int>((*_ppEditView)->execute(SCI_GETCURRENTPOS));
		limit = static_cast<unsigned int>((*_ppEditView)->getCurrentDocLen() - 1);
	}

    ::SetDlgItemInt(_hSelf, ID_CURRLINE, current, FALSE);
    ::SetDlgItemInt(_hSelf, ID_LASTLINE, limit, FALSE);
}

int GoToLineDlg::getLine(int& outOffset, HWND hDlg, int controlID) 
{
	//BOOL isSuccessful;
	//int line = ::GetDlgItemInt(_hSelf, ID_GOLINE_EDIT, &isSuccessful, FALSE);
	//return (isSuccessful?line:-1);

	const int max = 256;
	TCHAR filters[max+1];

	int len = GetDlgItemText(hDlg, controlID, filters, max);

	bool intClosed=false;
	outOffset=-1;
	int number=-1;
	int intVal, valval;
	for(int i=0;i<len;i++) {
		intVal = filters[i]-'0';
		valval = intVal>=0&&intVal<=9;
		if(!valval) {
			if(number>=0) {
				intClosed = true;
				if(filters[i]==':') {
					continue;
				}
				break;
			}
			if(outOffset>=0) {
				break;
			}
		} else {
			if(intClosed) {
				if(outOffset==-1) {
					outOffset = 0;
				}
			} else if(number==-1){
				number = 0;
			}
		}
		if(outOffset>=0) {
			outOffset = outOffset*10+intVal;
		}
		else if(number>=0) {
			number = number*10+intVal;
		}
	}

	return number;
}

int GoToLineDlg::execGoToLn(HWND hDlg, int controlID)
{
	int offset=0;
	int line = getLine(offset, hDlg, controlID);
	auto mode = _mode;
	if (hDlg!=_hSelf)
	{
		mode = go2line;
	}
	if(offset>0) 
	{
		if(mode==go2offsset) {
			line+=offset;
		} else {
			mode = go2offsset;
			(*_ppEditView)->execute(SCI_ENSUREVISIBLE, line-1);
			line = (*_ppEditView)->execute(SCI_POSITIONFROMLINE, line-1)+offset;
		}
	}
	if (line>=0)
	{
		//display(false);
		//cleanLineEdit();
		if (mode == go2line)
		{
			(*_ppEditView)->execute(SCI_ENSUREVISIBLE, line-1);
			(*_ppEditView)->execute(SCI_GOTOLINE, line-1);
		}
		else
		{
			auto sci_line = (*_ppEditView)->execute(SCI_LINEFROMPOSITION, line);
			(*_ppEditView)->execute(SCI_ENSUREVISIBLE, sci_line);
			(*_ppEditView)->execute(SCI_GOTOPOS, line);
		}
	}

	SCNotification notification = {};
	notification.nmhdr.code = SCN_PAINTED;
	notification.nmhdr.hwndFrom = _hSelf;
	notification.nmhdr.idFrom = ::GetDlgCtrlID(_hSelf);
	::SendMessage(_hParent, WM_NOTIFY, LINKTRIGGERED, reinterpret_cast<LPARAM>(&notification));


	(*_ppEditView)->getFocus();

	return line;
}

