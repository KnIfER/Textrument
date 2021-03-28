
#include "InsituDebug.h"
#include "Notepad_plus_msgs.h"
#include <stdarg.h>


#include "Notepad_plus.h"

extern Notepad_plus *nppApp;

void LogIs(int show, HWND hWnd, const TCHAR* msg, ...)
{

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
		::SendMessage(hWnd, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, (LPARAM)buffer);

	}


}
