
#include "InsituDebug.h"
#include "Notepad_plus_msgs.h"
#include <stdarg.h>


#include "Notepad_plus.h"

extern Notepad_plus *nppApp;

void LogIs(int show, HWND hWnd, const TCHAR* msg, ...)
{
	if (hWnd==(HWND)-2)
	{
		return;
	}
	if(show)
	{
		if (hWnd==(HWND)-1)
		{
			hWnd = nppApp->_dockingManager.getHParent();
		}
		va_list args = NULL;
		va_start(args, msg);

		TCHAR buffer[256]={};
		_vstprintf(buffer,msg, args);

		if (show==1)
		{
			::SendMessage(hWnd, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, (LPARAM)buffer);
		}
		else if (show==2)
		{
			::MessageBox(hWnd, buffer, TEXT(""), MB_OK);
		}
	}


}
