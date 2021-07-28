#pragma once

#include <windows.h>
#include <commctrl.h>

#include "DockingDlgInterface.h"
#include "SubPanelPlaceHolder_rc.h"

class SubPanelPlaceHolder : public DockingDlgInterface 
{
public:
	SubPanelPlaceHolder(): DockingDlgInterface(IDD_SUBPLACEHODLER_PANEL) {};

	void init(HINSTANCE hInst, HWND hPere) 
	{
		DockingDlgInterface::init(hInst, hPere);
	}

	virtual void setClosed(bool toClose);

	virtual INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
};

