
#include "InsituDebug.h"
#include "Notepad_plus_msgs.h"
#include <iostream>
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
		else if (show==3)
		{
			::OutputDebugString(buffer);
			::OutputDebugStringA("\n");
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

void LogIs(const TCHAR* msg, ...)
{
	va_list args = NULL;
	va_start(args, msg);
	LogIs(1, (HWND)-1, msg, args);
}

#if UNICODE

void LogIs(int show, HWND hWnd, const CHAR* msg, va_list & args)
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

		CHAR buffer[256]={};
		vsprintf(buffer,msg, args);

		if (show==1)
		{
			TCHAR buffer1[256]={};
			MultiByteToWideChar(CP_ACP, 0, buffer, -1, buffer1, 255);
			::SendMessage(hWnd, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, (LPARAM)buffer1);
		}
		else if (show==2)
		{
			::MessageBoxA(hWnd, buffer, (""), MB_OK);
		}
		else if (show==3)
		{
			::OutputDebugStringA(buffer);
			::OutputDebugStringA("\n");
		}
	}
}

void LogIs(int show, HWND hWnd, const CHAR* msg, ...)
{
	va_list args = NULL;
	va_start(args, msg);
	LogIs(show, hWnd, msg, args);
}

void LogIs(int show, const CHAR* msg, ...)
{
	va_list args = NULL;
	va_start(args, msg);
	LogIs(show, (HWND)-1, msg, args);
}

void LogIs(const CHAR* msg, ...)
{
	va_list args = NULL;
	va_start(args, msg);
	LogIs(1, (HWND)-1, msg, args);
}
#endif