
#include "InsituDebug.h"
#include "Notepad_plus_msgs.h"
#include <stdarg.h>

extern HWND mainAppWnd;

void LogIs(int show, HWND hWnd, const TCHAR* msg, va_list & args)
{
	if (hWnd==(HWND)-2)
	{
		return;
	}
	if(show)
	{
		if (hWnd==(HWND)-1)
		{
			hWnd = mainAppWnd;
		}

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

void LogIs(int show, HWND hWnd, const TCHAR* msg, ...)
{
	va_list args = NULL;
	va_start(args, msg);
	LogIs(show, hWnd, msg, args);
}

void LogIs(int show, const TCHAR* msg, ...)
{
	va_list args = NULL;
	va_start(args, msg);
	LogIs(show, (HWND)-1, msg, args);
}
