#pragma once

#include "windows.h"

void LogIs(int show, HWND tag, const TCHAR* msg, ...);

void LogIs(int show, const TCHAR* msg, ...);

void LogIs(const TCHAR* msg, ...);

#if UNICODE
void LogIs(int show, HWND tag, const CHAR* msg, ...);

void LogIs(int show, const CHAR* msg, ...);

void LogIs(const CHAR* msg, ...);
#endif