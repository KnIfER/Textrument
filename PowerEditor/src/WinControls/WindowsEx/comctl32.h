/******************************************************************************
*
* Common definitions (resource ids and global variables)
*
* Copyright 1999 Thuy Nguyen
* Copyright 1999 Eric Kohl
* Copyright 2002 Dimitrie O. Paun
* Copyright 2021 KnIfER JK. Chen
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
*/

#ifndef __WINE_COMCTL32_H
#define __WINE_COMCTL32_H

#ifndef RC_INVOKED
#include <stdarg.h>
#endif

#include "windef.h"
#include "winbase.h"
#include "wingdi.h"
#include "winuser.h"
#include "winnls.h"
#include "commctrl.h"

/* Property sheet / Wizard */
#define IDD_PROPSHEET 1006
#define IDD_WIZARD    1020

#define IDC_TABCONTROL   12320
#define IDC_APPLY_BUTTON 12321
#define IDC_BACK_BUTTON  12323
#define IDC_NEXT_BUTTON  12324
#define IDC_FINISH_BUTTON 12325
#define IDC_SUNKEN_LINE   12326
#define IDC_SUNKEN_LINEHEADER 12327

#define IDS_CLOSE	  4160

/* Toolbar customization dialog */
#define IDD_TBCUSTOMIZE     200

#define IDC_AVAILBTN_LBOX   201
#define IDC_RESET_BTN       202
#define IDC_TOOLBARBTN_LBOX 203
#define IDC_REMOVE_BTN      204
#define IDC_HELP_BTN        205
#define IDC_MOVEUP_BTN      206
#define IDC_MOVEDN_BTN      207

#define IDS_SEPARATOR      1024

/* Toolbar imagelist bitmaps */
#define IDB_STD_SMALL       120
#define IDB_STD_LARGE       121
#define IDB_VIEW_SMALL      124
#define IDB_VIEW_LARGE      125
#define IDB_HIST_SMALL      130
#define IDB_HIST_LARGE      131

#define IDM_TODAY                      4163
#define IDM_GOTODAY                    4164

/* Treeview Checkboxes */

#define IDT_CHECK        401

/* Command Link arrow */
#define IDB_CMDLINK      402


/* Cursors */
#define IDC_MOVEBUTTON                  102
#define IDC_COPY                        104
#define IDC_DIVIDER                     106
#define IDC_DIVIDEROPEN                 107


/* DragList resources */
#define IDI_DRAGARROW                   501

/* HOTKEY internal strings */
#define HKY_NONE                        2048

/* Tooltip icons */
#define IDI_TT_INFO_SM                   22
#define IDI_TT_WARN_SM                   25
#define IDI_TT_ERROR_SM                  28

/* Taskdialog strings */
#define IDS_BUTTON_YES    3000
#define IDS_BUTTON_NO     3001
#define IDS_BUTTON_RETRY  3002
#define IDS_BUTTON_OK     3003
#define IDS_BUTTON_CANCEL 3004
#define IDS_BUTTON_CLOSE  3005

/* Task dialog expando control default text */
#define IDS_TD_EXPANDED   3020
#define IDS_TD_COLLAPSED  3021

#define WM_SYSTIMER     0x0118

#define TCS_FLICKERFREE          0x10000

#define TCN_SIZECHANGE         (TCN_FIRST - 6)

typedef struct
{
    COLORREF clrBtnHighlight;       /* COLOR_BTNHIGHLIGHT                  */
    COLORREF clrBtnShadow;          /* COLOR_BTNSHADOW                     */
    COLORREF clrBtnText;            /* COLOR_BTNTEXT                       */
    COLORREF clrBtnFace;            /* COLOR_BTNFACE                       */
    COLORREF clrHighlight;          /* COLOR_HIGHLIGHT                     */
    COLORREF clrHighlightText;      /* COLOR_HIGHLIGHTTEXT                 */
    COLORREF clrHotTrackingColor;   /* COLOR_HOTLIGHT                      */
    COLORREF clr3dHilight;          /* COLOR_3DHILIGHT                     */
    COLORREF clr3dShadow;           /* COLOR_3DSHADOW                      */
    COLORREF clr3dDkShadow;         /* COLOR_3DDKSHADOW                    */
    COLORREF clr3dFace;             /* COLOR_3DFACE                        */
    COLORREF clrWindow;             /* COLOR_WINDOW                        */
    COLORREF clrWindowText;         /* COLOR_WINDOWTEXT                    */
    COLORREF clrGrayText;           /* COLOR_GREYTEXT                      */
    COLORREF clrActiveCaption;      /* COLOR_ACTIVECAPTION                 */
    COLORREF clrInfoBk;             /* COLOR_INFOBK                        */
    COLORREF clrInfoText;           /* COLOR_INFOTEXT                      */
} COMCTL32_SysColor;

__declspec(selectany) COMCTL32_SysColor  comctl32_color;
__declspec(selectany) HBRUSH  COMCTL32_hPattern55AABrush;

#define TCM_SETMAXROWS        (TCM_FIRST + 66)
#define TabCtrl_SetMaxRows(hwnd, maxLn) \
    (HIMAGELIST)SNDMSG((hwnd), TCM_SETMAXROWS, maxLn, 0)

#define TCM_GETISVERTICALRESIZEAREA    (TCM_FIRST + 67)
#define TabCtrl_GetIsVerticalResizeArea(hwnd, x, y)\
        (bool)SNDMSG((hwnd), TCM_GETISVERTICALRESIZEAREA, x, y)

#define TCM_TRACKVERTICALTABS    (TCM_FIRST + 68)
#define TabCtrl_TrackVerticalTabs(hwnd, x, y)\
        (int)SNDMSG((hwnd), TCM_STRATTRACKING, x, y)

#define TCM_DISMISSTOOLTIPS    (TCM_FIRST + 69)
#define TabCtrl_DismissToolTips(hwnd)\
        (int)SNDMSG((hwnd), TCM_DISMISSTOOLTIPS, 0, 0)

#define TCM_GETVERTICALMODEWIDTH    (TCM_FIRST + 70)
#define TabCtrl_GetVerticalModeWidth(hwnd)\
        (int)SNDMSG((hwnd), TCM_GETVERTICALMODEWIDTH, 0, 0)

#define TCM_SETVERTICALMODEWIDTH    (TCM_FIRST + 71)
#define TabCtrl_SetVerticalModeWidth(hwnd)\
        (int)SNDMSG((hwnd), TCM_GETVERTICALMODEWIDTH, 0, 0)

#define TCM_GETTOPMOSTROW    (TCM_FIRST + 72)
#define TabCtrl_GetTopMostRow(hwnd)\
        (int)SNDMSG((hwnd), TCM_GETTOPMOSTROW, 0, 0)


void ReadColors();
#endif  /* __WINE_COMCTL32_H */
