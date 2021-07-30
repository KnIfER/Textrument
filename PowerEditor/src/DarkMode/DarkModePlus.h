#pragma once

#include "NppDarkMode.h"

#define DMPlus_handleEdit \
case WM_CTLCOLOREDIT:\
{\
if (NppDarkMode::isEnabled())\
{\
return NppDarkMode::onCtlColorSofter(reinterpret_cast<HDC>(wParam));\
}\
break;\
}

#define DMPlus_handleListBox \
case WM_CTLCOLORLISTBOX:\
{\
if (NppDarkMode::isEnabled())\
	{\
		return NppDarkMode::onCtlColor(reinterpret_cast<HDC>(wParam));\
	}\
break;\
}

#define DMPlus_handleDLG \
case WM_CTLCOLORDLG:\
case WM_CTLCOLORSTATIC:\
{\
	if (NppDarkMode::isEnabled())\
	{\
		return NppDarkMode::onCtlColorDarker(reinterpret_cast<HDC>(wParam));\
	}\
	break;\
}

#define DMPlus_handlePrint \
case WM_PRINTCLIENT:\
{\
	if (NppDarkMode::isEnabled())\
	{\
		return TRUE;\
	}\
	break;\
}


#define DMPlus_handleErase \
case WM_ERASEBKGND:\
{\
	if (NppDarkMode::isEnabled())\
	{\
		RECT rc = { 0 };\
		GetClientRect(_hSelf, &rc);\
		::FillRect(reinterpret_cast<HDC>(wParam), &rc, NppDarkMode::getDarkerBackgroundBrush());\
		return TRUE;\
	}\
	break;\
}

#define DMPlus_handleRefresh \
case NPPM_INTERNAL_REFRESHDARKMODE:\
{\
	NppDarkMode::autoThemeChildControls(_hSelf);\
	return TRUE;\
}
