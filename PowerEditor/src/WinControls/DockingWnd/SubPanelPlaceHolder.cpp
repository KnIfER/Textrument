#include "SubPanelPlaceHolder.h"
#include "localization.h"
#include "Notepad_plus.h"

INT_PTR CALLBACK SubPanelPlaceHolder::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG :
        {

            return TRUE;
        }

		case NPPM_INTERNAL_REFRESHDARKMODE:
		{
			//NppDarkMode::setDarkListView(_listView.getHSelf());
			return TRUE;
		}

        default :
            return DockingDlgInterface::run_dlgProc(message, wParam, lParam);
    }
	return DockingDlgInterface::run_dlgProc(message, wParam, lParam);
}