#include <windows.h>
#include "comctl32.h"

void ReadColors()
{
	comctl32_color.clrBtnHighlight     =  GetSysColor(COLOR_BTNHIGHLIGHT);
	comctl32_color.clrBtnShadow        =  GetSysColor(COLOR_BTNSHADOW);
	comctl32_color.clrBtnText          =  GetSysColor(COLOR_BTNTEXT);
	comctl32_color.clrBtnFace          =  GetSysColor(COLOR_BTNFACE);
	comctl32_color.clrHighlight        =  GetSysColor(COLOR_HIGHLIGHT);
	comctl32_color.clrHighlightText    =  GetSysColor(COLOR_HIGHLIGHTTEXT);
	//comctl32_color.clrHotTrackingColor =  GetSysColor(COLOR_BTNFACE);
	comctl32_color.clr3dHilight        =  GetSysColor(COLOR_3DHILIGHT);
	comctl32_color.clr3dShadow         =  GetSysColor(COLOR_3DSHADOW);
	comctl32_color.clr3dDkShadow       =  GetSysColor(COLOR_3DDKSHADOW);
	comctl32_color.clr3dFace           =  GetSysColor(COLOR_3DFACE);
	comctl32_color.clrWindow           =  GetSysColor(COLOR_WINDOW);
	comctl32_color.clrWindowText       =  GetSysColor(COLOR_WINDOWTEXT);
	comctl32_color.clrGrayText         =  GetSysColor(COLOR_GRAYTEXT);
	comctl32_color.clrActiveCaption    =  GetSysColor(COLOR_ACTIVECAPTION);
	comctl32_color.clrInfoBk           =  GetSysColor(COLOR_INFOBK);
	comctl32_color.clrInfoText         =  GetSysColor(COLOR_INFOTEXT);
}
