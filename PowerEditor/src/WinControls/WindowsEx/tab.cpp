/*
* Tab control
*
* Copyright 1998 Anders Carlsson
* Copyright 1999 Alex Priem <alexp@sci.kun.nl>
* Copyright 1999 Francis Beaudet
* Copyright 2003 Vitaliy Margolen
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
*
* NOTES (of Wine)
*
* This code was audited for completeness against the documented features
* of Comctl32.dll version 6.0 on May. 20, 2005, by James Hawkins.
*
* Unless otherwise noted, we believe this code to be complete, as per
* the specification mentioned above.
* If you discover missing features, or bugs, please note them below.
*
* TODO: (of Wine)
*
*  Styles:
*   TCS_MULTISELECT - implement for VK_SPACE selection
*   TCS_RIGHT
*   TCS_RIGHTJUSTIFY
*   TCS_SCROLLOPPOSITE
*   TCS_SINGLELINE
*   TCIF_RTLREADING
*
*  Extended Styles:
*   TCS_EX_REGISTERDROP
*
*  Notifications:
*   NM_RELEASEDCAPTURE
*   TCN_FOCUSCHANGE
*   TCN_GETOBJECT
*
*  Macros:
*   TabCtrl_AdjustRect
*
* New Features : ( MyTabControl )
*	Vertical mode, totally "neck-harmless". Tabs are listed in a resizable side bar, better than that of Edge. 
*		Style : TCS_VERTICAL
*	Maximum rows capacity for multiline mode. Save Screen Space!
*		Macro : TabCtrl_SetMaxRows
*/

#include <assert.h>
#include <stdarg.h>
#include <string.h>

#include "windows.h"
#include "strsafe.h"
#include "windef.h"
#include "winbase.h"
#include "wingdi.h"
#include "winuser.h"
#include "winnls.h"
#include "commctrl.h"
#include "./commctrl.h"
#include "comctl32.h"
#include "uxtheme.h"
#include "vssym32.h"
#include "dpa_dsa.h"
#include "debug.h"
#include <math.h>
#include <windowsx.h>

#include <vector>

//WINE_DEFAULT_DEBUG_CHANNEL(tab);

typedef struct
{
	DWORD  dwState;
	LPWSTR pszText;
	INT    iImage;
	RECT   rect;      /* bounding rectangle of the item relative to the
					  * leftmost item (the leftmost item, 0, would have a
					  * "left" member of 0 in this rectangle)
					  *
					  * additionally the top member holds the row number
					  * and bottom is unused and should be 0 */
	BYTE   extra[1];  /* Space for caller supplied info, variable size */
} TAB_ITEM;

/* The size of a tab item depends on how much extra data is requested.
TCM_INSERTITEM always stores at least LPARAM sized data. */
#define EXTRA_ITEM_SIZE(infoPtr) (max((infoPtr)->cbInfo, sizeof(LPARAM)))
#define TAB_ITEM_SIZE(infoPtr) FIELD_OFFSET(TAB_ITEM, extra[EXTRA_ITEM_SIZE(infoPtr)])

typedef struct
{
	HWND       hwnd;            /* Tab control window */
	HWND       hwndNotify;      /* notification window (parent) */
	UINT       uNumItem;        /* number of tab items */
	UINT       uNumRows;	      /* number of tab rows */
	INT        tabHeight;       /* height of the tab row */
	INT        tabWidth;        /* width of tabs */
	INT        tabMinWidth;     /* minimum width of items */
	USHORT     uHItemPadding;   /* amount of horizontal padding, in pixels */
	USHORT     uVItemPadding;   /* amount of vertical padding, in pixels */
	USHORT     uHItemPadding_s; /* Set amount of horizontal padding, in pixels */
	USHORT     uVItemPadding_s; /* Set amount of vertical padding, in pixels */
	HFONT      hFont;           /* handle to the current font */
	HCURSOR    hcurArrow;       /* handle to the current cursor */
	HIMAGELIST himl;            /* handle to an image list (may be 0) */
	HWND       hwndToolTip;     /* handle to tab's tooltip */
	INT        leftmostVisible; /* Used for scrolling, this member contains
								* the index of the first visible item */
	INT        iSelected;       /* the currently selected item */
	INT        iHotTracked;     /* the highlighted item under the mouse */
	INT        uFocus;          /* item which has the focus */
	BOOL       DoRedraw;        /* flag for redrawing when tab contents is changed*/
	BOOL       needsScrolling;  /* TRUE if the size of the tabs is greater than
								* the size of the control */
	BOOL       fHeightSet;      /* was the height of the tabs explicitly set? */
	BOOL       bUnicode;        /* Unicode control? */
	HWND       hwndUpDown;      /* Updown control used for scrolling */
	INT        cbInfo;          /* Number of bytes of caller supplied info per tab */

	DWORD      exStyle;         /* Extended style used, currently:
								TCS_EX_FLATSEPARATORS, TCS_EX_REGISTERDROP */
	DWORD      dwStyle;         /* the cached window GWL_STYLE */

	HDPA       items;           /* dynamic array of TAB_ITEM* pointers */

	INT        tabMaxRows;
	INT        tabMaxRowSet;
	BOOL multilineScrolling;
	INT        topmostVisible; /* Used for scrolling, this member contains  the index of the first visible row */
	INT        maxRange; /* Used for scrolling */

	UINT		vModeWidth; /* Used for vertical mode */
	BOOL		vModeDragging; /* Used for vertical mode */
	UINT		vModeDragSt; /* Used for vertical mode */
	UINT		vModeWidthSt; /* Used for vertical mode */

	HTHEME htheme; /* https://stackoverflow.com/questions/55319559/why-is-my-window-losing-its-htmeme-when-i-call-setwindowlongptrgwl-style-on-it */
} TAB_INFO;

/******************************************************************************
* Positioning constants
*/
#define SELECTED_TAB_OFFSET     2
#define ROUND_CORNER_SIZE       2
#define DISPLAY_AREA_PADDINGX   2
#define DISPLAY_AREA_PADDINGY   2
#define CONTROL_BORDER_SIZEX    2
#define CONTROL_BORDER_SIZEY    2
#define BUTTON_SPACINGX         3
#define BUTTON_SPACINGY         3
#define FLAT_BTN_SPACINGX       8
#define DEFAULT_MIN_TAB_WIDTH   54
#define DEFAULT_PADDING_X       6
#define EXTRA_ICON_PADDING      3

#define TAB_GetInfoPtr(hwnd) ((TAB_INFO *)GetWindowLongPtrW(hwnd,0))

#define GET_DEFAULT_MIN_TAB_WIDTH(infoPtr) (DEFAULT_MIN_TAB_WIDTH - (DEFAULT_PADDING_X - (infoPtr)->uHItemPadding) * 2)

/******************************************************************************
* Hot-tracking timer constants
*/
#define TAB_HOTTRACK_TIMER            1
#define TAB_HOTTRACK_TIMER_INTERVAL   100   /* milliseconds */

static const WCHAR themeClass[] = L"Tab";

static inline TAB_ITEM* TAB_GetItem(const TAB_INFO *infoPtr, INT i)
{
	assert(i >= 0 && i < infoPtr->uNumItem);
	return (TAB_ITEM*)DPA_GetPtr(infoPtr->items, i);
}

/******************************************************************************
* Prototypes
*/
static void TAB_InvalidateTabArea(const TAB_INFO *);
static void TAB_EnsureSelectionVisible(TAB_INFO *);
static void TAB_DrawItemInterior(const TAB_INFO *, HDC, INT, RECT*);
static LRESULT _DeselectAll(TAB_INFO *, BOOL);
static BOOL TAB_InternalGetItemRect(const TAB_INFO *, INT, RECT*, RECT*);
static void TAB_SetItemBounds (TAB_INFO *infoPtr);
static BOOL _GetIsVerticalResizeArea (TAB_INFO *infoPtr, int x, int y);
static void Tab_TrackMouseStart (TAB_INFO *infoPtr, UINT start);
static inline void Tab_TrackMouseEnd (TAB_INFO *infoPtr);

static BOOL TAB_SendSimpleNotify (const TAB_INFO *infoPtr, UINT code)
{
	NMHDR nmhdr;

	nmhdr.hwndFrom = infoPtr->hwnd;
	nmhdr.idFrom = GetWindowLongPtrW(infoPtr->hwnd, GWLP_ID);
	nmhdr.code = code;

	return (BOOL) SendMessageW (infoPtr->hwndNotify, WM_NOTIFY,
		nmhdr.idFrom, (LPARAM) &nmhdr);
}

static void TAB_RelayEvent (HWND hwndTip, HWND hwndMsg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MSG msg;

	msg.hwnd = hwndMsg;
	msg.message = uMsg;
	msg.wParam = wParam;
	msg.lParam = lParam;
	msg.time = GetMessageTime ();
	msg.pt.x = (short)LOWORD(GetMessagePos ());
	msg.pt.y = (short)HIWORD(GetMessagePos ());

	SendMessageW (hwndTip, TTM_RELAYEVENT, 0, (LPARAM)&msg);
}

static void TAB_DumpItemExternalT(const TCITEMW *pti, UINT iItem, BOOL isW)
{
	if (TRACE_ON(tab)) {
		//TRACE("external tab %d, mask=0x%08x, dwState=0x%08x, dwStateMask=0x%08x, cchTextMax=0x%08x\n",
		//	iItem, pti->mask, pti->dwState, pti->dwStateMask, pti->cchTextMax);
		//TRACE("external tab %d,   iImage=%d, lParam=0x%08lx, pszTextW=%s\n",
		//	iItem, pti->iImage, pti->lParam, isW ? debugstr_w(pti->pszText) : (LPWSTR)debugstr_a((LPSTR)pti->pszText));
	}
}

static void TAB_DumpItemInternal(const TAB_INFO *infoPtr, UINT iItem)
{
	if (TRACE_ON(tab)) {
		TAB_ITEM *ti = TAB_GetItem(infoPtr, iItem);

		TRACE("tab %d, dwState=0x%08x, pszText=%s, iImage=%d\n",
			iItem, ti->dwState, debugstr_w(ti->pszText), ti->iImage);
		TRACE("tab %d, rect.left=%d, rect.top(row)=%d\n",
			iItem, ti->rect.left, ti->rect.top);
	}
}

/* RETURNS
*   the index of the selected tab, or -1 if no tab is selected. */
static inline LRESULT _GetCurSel (const TAB_INFO *infoPtr)
{
	TRACE("(%p)\n", infoPtr);
	return infoPtr->iSelected;
}

/* RETURNS
*   the index of the tab item that has the focus. */
static inline LRESULT _GetCurFocus (const TAB_INFO *infoPtr)
{
	TRACE("(%p)\n", infoPtr);
	return infoPtr->uFocus;
}

static inline LRESULT _GetToolTips (const TAB_INFO *infoPtr)
{
	TRACE("_GetToolTips::(%p)\n", infoPtr);
	return (LRESULT)infoPtr->hwndToolTip;
}

static inline LRESULT _SetCurSel (TAB_INFO *infoPtr, INT iItem)
{
	INT prevItem = infoPtr->iSelected;

	TRACE("_SetCurSel::(%p %d)\n", infoPtr, iItem);

	if (iItem >= (INT)infoPtr->uNumItem)
		return -1;

	if (prevItem != iItem) {
		if (prevItem != -1)
			TAB_GetItem(infoPtr, prevItem)->dwState &= ~TCIS_BUTTONPRESSED;

		if (iItem >= 0)
		{
			TAB_GetItem(infoPtr, iItem)->dwState |= TCIS_BUTTONPRESSED;
			infoPtr->iSelected = iItem;
			infoPtr->uFocus = iItem;
		}
		else
		{
			infoPtr->iSelected = -1;
			infoPtr->uFocus = -1;
		}

		TAB_EnsureSelectionVisible(infoPtr);
		TAB_InvalidateTabArea(infoPtr);
	}

	return prevItem;
}

static LRESULT _SetCurFocus (TAB_INFO *infoPtr, INT iItem)
{
	TRACE("(%p %d)\n", infoPtr, iItem);

	if (iItem < 0) {
		infoPtr->uFocus = -1;
		if (infoPtr->iSelected != -1) {
			infoPtr->iSelected = -1;
			TAB_SendSimpleNotify(infoPtr, TCN_SELCHANGE);
			TAB_InvalidateTabArea(infoPtr);
		}
	}
	else if (iItem < infoPtr->uNumItem) {
		if (infoPtr->dwStyle & TCS_BUTTONS) {
			/* set focus to new item, leave selection as is */
			if (infoPtr->uFocus != iItem) {
				INT prev_focus = infoPtr->uFocus;
				RECT r;

				infoPtr->uFocus = iItem;

				if (prev_focus != infoPtr->iSelected) {
					if (TAB_InternalGetItemRect(infoPtr, prev_focus, &r, NULL))
						InvalidateRect(infoPtr->hwnd, &r, FALSE);
				}

				if (TAB_InternalGetItemRect(infoPtr, iItem, &r, NULL))
					InvalidateRect(infoPtr->hwnd, &r, FALSE);

				TAB_SendSimpleNotify(infoPtr, TCN_FOCUSCHANGE);
			}
		} else {
			INT oldFocus = infoPtr->uFocus;
			if (infoPtr->iSelected != iItem || oldFocus == -1 ) {
				infoPtr->uFocus = iItem;
				if (oldFocus != -1) {
					if (!TAB_SendSimpleNotify(infoPtr, TCN_SELCHANGING))  {
						infoPtr->iSelected = iItem;
						TAB_SendSimpleNotify(infoPtr, TCN_SELCHANGE);
					}
					else
						infoPtr->iSelected = iItem;
					TAB_EnsureSelectionVisible(infoPtr);
					TAB_InvalidateTabArea(infoPtr);
				}
			}
		}
	}
	return 0;
}

static inline LRESULT _SetToolTips (TAB_INFO *infoPtr, HWND hwndToolTip)
{
	TRACE("%p %p\n", infoPtr, hwndToolTip);
	infoPtr->hwndToolTip = hwndToolTip;
	return 0;
}

static inline LRESULT _SetPadding (TAB_INFO *infoPtr, LPARAM lParam)
{
	TRACE("(%p %d %d)\n", infoPtr, LOWORD(lParam), HIWORD(lParam));
	infoPtr->uHItemPadding_s = LOWORD(lParam);
	infoPtr->uVItemPadding_s = HIWORD(lParam);

	return 0;
}

/******************************************************************************
* TAB_InternalGetItemRect
*
* This method will calculate the rectangle representing a given tab item in
* client coordinates. This method takes scrolling into account.
*
* This method returns TRUE if the item is visible in the window and FALSE
* if it is completely outside the client area.
*/
static BOOL TAB_InternalGetItemRect(
	const TAB_INFO* infoPtr,
	INT         itemIndex,
	RECT*       itemRect,
	RECT*       selectedRect)
{
	RECT tmpItemRect,clientRect;

	/* Perform a sanity check and a trivial visibility check. */
	if ( (infoPtr->uNumItem <= 0) ||
		(itemIndex >= infoPtr->uNumItem) ||
		(!(((infoPtr->dwStyle & TCS_MULTILINE) || (infoPtr->dwStyle & TCS_VERTICAL))) &&
			(itemIndex < infoPtr->leftmostVisible)))
	{
		//TRACE("Not Visible\n");
		SetRect(itemRect, 0, 0, 0, infoPtr->tabHeight);
		SetRectEmpty(selectedRect);
		return FALSE;
	}
	
	/*
	* Avoid special cases in this procedure by assigning the "out"
	* parameters if the caller didn't supply them
	*/
	if (itemRect == NULL)
		itemRect = &tmpItemRect;

	/* Retrieve the unmodified item rect. */
	*itemRect = TAB_GetItem(infoPtr,itemIndex)->rect;

	if (infoPtr->multilineScrolling 
		&& (itemRect->top < infoPtr->topmostVisible
			|| itemRect->top >= infoPtr->topmostVisible+infoPtr->tabMaxRows)
		)
	{
		//TRACE("Not Visible 111\n");
		SetRect(itemRect, 0, 0, 0, infoPtr->tabHeight);
		SetRectEmpty(selectedRect);
		return FALSE;
	}
	/* calculate the times bottom and top based on the row */
	GetClientRect(infoPtr->hwnd, &clientRect);

	if (infoPtr->dwStyle & TCS_BOTTOM)
	{
		itemRect->bottom = clientRect.bottom - itemRect->top * infoPtr->tabHeight -
			((infoPtr->dwStyle & TCS_BUTTONS) ? itemRect->top * BUTTON_SPACINGY : SELECTED_TAB_OFFSET);
		itemRect->top    = itemRect->bottom - infoPtr->tabHeight;
	}
	else /* not TCS_BOTTOM and not TCS_VERTICAL */
	{
		itemRect->top    = clientRect.top + itemRect->top * infoPtr->tabHeight +
			((infoPtr->dwStyle & TCS_BUTTONS) ? itemRect->top * BUTTON_SPACINGY : SELECTED_TAB_OFFSET);
		itemRect->bottom = itemRect->top + infoPtr->tabHeight;
	}

	/*
	* "scroll" it to make sure the item at the very left of the
	* tab control is the leftmost visible tab.
	*/
	if(infoPtr->dwStyle & TCS_MULTILINE || infoPtr->dwStyle & TCS_VERTICAL)
	{
		OffsetRect(itemRect,
			0,
			-infoPtr->topmostVisible * infoPtr->tabHeight);

		/*
		* Move the rectangle so the first item is slightly offset from
		* the bottom of the tab control.
		*/
		//OffsetRect(itemRect, 0, SELECTED_TAB_OFFSET);
	} 
	else
	{
		OffsetRect(itemRect,
			-TAB_GetItem(infoPtr, infoPtr->leftmostVisible)->rect.left,
			0);

		/*
		* Move the rectangle so the first item is slightly offset from
		* the left of the tab control.
		*/
		OffsetRect(itemRect, SELECTED_TAB_OFFSET, 0);
	}
	TRACE("item %d tab h=%d, rect=(%s)\n",itemIndex,infoPtr->tabHeight,wine_dbgstr_rect(itemRect));

	/* Now, calculate the position of the item as if it were selected. */
	if (selectedRect!=NULL)
	{
		*selectedRect = *itemRect;

		/* The rectangle of a selected item is a bit wider. */
		InflateRect(selectedRect, SELECTED_TAB_OFFSET, 0);

		/* If it also a bit higher. */
		if (infoPtr->dwStyle & TCS_BOTTOM)
		{
			selectedRect->bottom += SELECTED_TAB_OFFSET;
		}
		else /* not TCS_BOTTOM and not TCS_VERTICAL */
		{
			selectedRect->top    -= SELECTED_TAB_OFFSET;
			selectedRect->bottom -= 1;
		}
	}

	/* Check for visibility */
	return (itemRect->left < clientRect.right) && (itemRect->right > clientRect.left);
}

static inline BOOL _GetItemRect(const TAB_INFO *infoPtr, INT item, RECT *rect)
{
	TRACE("(%p, %d, %p)\n", infoPtr, item, rect);
	return TAB_InternalGetItemRect(infoPtr, item, rect, NULL);
}

/******************************************************************************
* _KeyDown
*
* This method is called to handle keyboard input
*/
static LRESULT _KeyDown(TAB_INFO* infoPtr, WPARAM keyCode, LPARAM lParam)
{
	INT newItem = -1;
	NMTCKEYDOWN nm;

	/* TCN_KEYDOWN notification sent always */
	nm.hdr.hwndFrom = infoPtr->hwnd;
	nm.hdr.idFrom = GetWindowLongPtrW(infoPtr->hwnd, GWLP_ID);
	nm.hdr.code = TCN_KEYDOWN;
	nm.wVKey = keyCode;
	nm.flags = lParam;
	SendMessageW(infoPtr->hwndNotify, WM_NOTIFY, nm.hdr.idFrom, (LPARAM)&nm);

	switch (keyCode)
	{
	case VK_LEFT:
		newItem = infoPtr->uFocus - 1;
		break;
	case VK_RIGHT:
		newItem = infoPtr->uFocus + 1;
		break;
	}

	/* If we changed to a valid item, change focused item */
	if (newItem >= 0 && newItem < infoPtr->uNumItem && infoPtr->uFocus != newItem)
		_SetCurFocus(infoPtr, newItem);

	return 0;
}

/*
* WM_KILLFOCUS handler
*/
static void _KillFocus(TAB_INFO *infoPtr)
{
	/* clear current focused item back to selected for TCS_BUTTONS */
	if ((infoPtr->dwStyle & TCS_BUTTONS) && (infoPtr->uFocus != infoPtr->iSelected))
	{
		RECT r;

		if (TAB_InternalGetItemRect(infoPtr, infoPtr->uFocus, &r, NULL))
			InvalidateRect(infoPtr->hwnd, &r, FALSE);

		infoPtr->uFocus = infoPtr->iSelected;
	}
}

/******************************************************************************
* _FocusChanging
*
* This method is called whenever the focus goes in or out of this control
* it is used to update the visual state of the control.
*/
static void _FocusChanging(const TAB_INFO *infoPtr)
{
	RECT      selectedRect;
	BOOL      isVisible;

	/*
	* Get the rectangle for the item.
	*/
	isVisible = TAB_InternalGetItemRect(infoPtr,
		infoPtr->uFocus,
		NULL,
		&selectedRect);

	/*
	* If the rectangle is not completely invisible, invalidate that
	* portion of the window.
	*/
	if (isVisible)
	{
		TRACE("invalidate (%s)\n", wine_dbgstr_rect(&selectedRect));
		InvalidateRect(infoPtr->hwnd, &selectedRect, TRUE);
	}
}

static INT TAB_InternalHitTest (const TAB_INFO *infoPtr, POINT pt, UINT *flags)
{
	RECT rect;
	INT iCount;

	for (iCount = 0; iCount < infoPtr->uNumItem; iCount++)
	{
		TAB_InternalGetItemRect(infoPtr, iCount, &rect, NULL);

		if (PtInRect(&rect, pt))
		{
			*flags = TCHT_ONITEM;
			return iCount;
		}
	}

	*flags = TCHT_NOWHERE;
	return -1;
}

static inline LRESULT _HitTest (const TAB_INFO *infoPtr, LPTCHITTESTINFO lptest)
{
	TRACE("(%p, %p)\n", infoPtr, lptest);
	return TAB_InternalHitTest (infoPtr, lptest->pt, &lptest->flags);
}

/******************************************************************************
* _NCHitTest
*
* Napster v2b5 has a tab control for its main navigation which has a client
* area that covers the whole area of the dialog pages.
* That's why it receives all msgs for that area and the underlying dialog ctrls
* are dead.
* So I decided that we should handle WM_NCHITTEST here and return
* HTTRANSPARENT if we don't hit the tab control buttons.
* FIXME: WM_NCHITTEST handling correct ? Fix it if you know that Windows
* doesn't do it that way. Maybe depends on tab control styles ?
*/
static inline LRESULT _NCHitTest (const TAB_INFO *infoPtr, LPARAM lParam)
{
	POINT pt;
	UINT dummyflag;

	pt.x = (short)LOWORD(lParam);
	pt.y = (short)HIWORD(lParam);
	ScreenToClient(infoPtr->hwnd, &pt);

	if (infoPtr->dwStyle & TCS_VERTICAL)
	{
		if (pt.x>=0&&pt.x<=infoPtr->vModeWidth+8)
		{
			if (pt.y<=infoPtr->tabHeight*infoPtr->uNumRows)
			{
				return HTCLIENT;
			}
			if (pt.x+10>=infoPtr->vModeWidth)
			{
				return HTCLIENT;
			}
		}
			
		return HTTRANSPARENT;
	}
	else
	{
		if (TAB_InternalHitTest(infoPtr, pt, &dummyflag) == -1)
			return HTTRANSPARENT;
		else
			return HTCLIENT;
	}
}

static LRESULT _LButtonDown (TAB_INFO *infoPtr, WPARAM wParam, LPARAM lParam)
{
	POINT pt;
	INT newItem;
	UINT dummy;

	if (infoPtr->hwndToolTip)
		TAB_RelayEvent (infoPtr->hwndToolTip, infoPtr->hwnd,
			WM_LBUTTONDOWN, wParam, lParam);

	if (!(infoPtr->dwStyle & TCS_FOCUSNEVER)) {
		SetFocus (infoPtr->hwnd);
	}

	if (infoPtr->hwndToolTip)
		TAB_RelayEvent (infoPtr->hwndToolTip, infoPtr->hwnd,
			WM_LBUTTONDOWN, wParam, lParam);

	pt.x = (short)LOWORD(lParam);
	pt.y = (short)HIWORD(lParam);

	if (infoPtr->dwStyle & TCS_VERTICAL
		&&(_GetIsVerticalResizeArea(infoPtr, pt.x, pt.y)))
	{
		Tab_TrackMouseStart(infoPtr, pt.x);
		return 0;
	}

	newItem = TAB_InternalHitTest (infoPtr, pt, &dummy);

	TRACE("On Tab, item %d!=%d\n", newItem, infoPtr->iSelected);

	if ((newItem != -1) && (infoPtr->iSelected != newItem))
	{
		if ((infoPtr->dwStyle & TCS_BUTTONS) && (infoPtr->dwStyle & TCS_MULTISELECT) &&
			(wParam & MK_CONTROL))
		{
			RECT r;

			/* toggle multiselection */
			TAB_GetItem(infoPtr, newItem)->dwState ^= TCIS_BUTTONPRESSED;
			if (TAB_InternalGetItemRect (infoPtr, newItem, &r, NULL))
				InvalidateRect (infoPtr->hwnd, &r, TRUE);
		}
		else
		{
			INT i;

			if (TAB_SendSimpleNotify(infoPtr, TCN_SELCHANGING))
				return 0;

			_SetCurSel(infoPtr, newItem);

			TAB_SendSimpleNotify(infoPtr, TCN_SELCHANGE);
		}
	}

	// ||pt.y>infoPtr->tabHeight*infoPtr->uNumRows
	if (newItem==-1
		&&(infoPtr->dwStyle & TCS_VERTICAL) && pt.y>infoPtr->tabHeight*infoPtr->uNumRows+20)
	{
		Tab_TrackMouseStart(infoPtr, pt.x);
	}

	return 0;
}

static inline LRESULT _LButtonUp (TAB_INFO *infoPtr)
{
	TAB_SendSimpleNotify(infoPtr, NM_CLICK);

	Tab_TrackMouseEnd(infoPtr);

	return 0;
}

static inline void _RButtonUp (const TAB_INFO *infoPtr)
{
	TAB_SendSimpleNotify(infoPtr, NM_RCLICK);
}

/******************************************************************************
* TAB_DrawLoneItemInterior
*
* This calls TAB_DrawItemInterior.  However, TAB_DrawItemInterior is normally
* called by TAB_DrawItem which is normally called by TAB_Refresh which sets
* up the device context and font.  This routine does the same setup but
* only calls TAB_DrawItemInterior for the single specified item.
*/
static void TAB_DrawLoneItemInterior(const TAB_INFO* infoPtr, int iItem)
{
	HDC hdc = GetDC(infoPtr->hwnd);
	RECT r, rC;

	/* Clip UpDown control to not draw over it */
	if (infoPtr->needsScrolling)
	{
		GetWindowRect(infoPtr->hwnd, &rC);
		GetWindowRect(infoPtr->hwndUpDown, &r);
		ExcludeClipRect(hdc, r.left - rC.left, r.top - rC.top, r.right - rC.left, r.bottom - rC.top);
	}
	TAB_DrawItemInterior(infoPtr, hdc, iItem, NULL);
	ReleaseDC(infoPtr->hwnd, hdc);
}

/* update a tab after hottracking - invalidate it or just redraw the interior,
* based on whether theming is used or not */
static inline void hottrack_refresh(const TAB_INFO *infoPtr, int tabIndex)
{
	if (tabIndex == -1) return;

	if (infoPtr->htheme)
	{
		RECT rect;
		TAB_InternalGetItemRect(infoPtr, tabIndex, &rect, NULL);
		InvalidateRect (infoPtr->hwnd, &rect, FALSE);
	}
	else
		TAB_DrawLoneItemInterior(infoPtr, tabIndex);
}

/******************************************************************************
* TAB_HotTrackTimerProc
*
* When a mouse-move event causes a tab to be highlighted (hot-tracking), a
* timer is setup so we can check if the mouse is moved out of our window.
* (We don't get an event when the mouse leaves, the mouse-move events just
* stop being delivered to our window and just start being delivered to
* another window.)  This function is called when the timer triggers so
* we can check if the mouse has left our window.  If so, we un-highlight
* the hot-tracked tab.
*/
static void CALLBACK TAB_HotTrackTimerProc
(
	HWND hwnd,    /* handle of window for timer messages */
	UINT uMsg,    /* WM_TIMER message */
	UINT_PTR idEvent, /* timer identifier */
	DWORD dwTime  /* current system time */
)
{
	TAB_INFO* infoPtr = TAB_GetInfoPtr(hwnd);

	if (infoPtr != NULL && infoPtr->iHotTracked >= 0)
	{
		POINT pt;

		/*
		** If we can't get the cursor position, or if the cursor is outside our
		** window, we un-highlight the hot-tracked tab.  Note that the cursor is
		** "outside" even if it is within our bounding rect if another window
		** overlaps.  Note also that the case where the cursor stayed within our
		** window but has moved off the hot-tracked tab will be handled by the
		** WM_MOUSEMOVE event.
		*/
		if (!GetCursorPos(&pt) || WindowFromPoint(pt) != hwnd)
		{
			/* Redraw iHotTracked to look normal */
			INT iRedraw = infoPtr->iHotTracked;
			infoPtr->iHotTracked = -1;
			hottrack_refresh (infoPtr, iRedraw);

			/* Kill this timer */
			KillTimer(hwnd, TAB_HOTTRACK_TIMER);
		}
	}
}

/******************************************************************************
* TAB_RecalcHotTrack
*
* If a tab control has the TCS_HOTTRACK style, then the tab under the mouse
* should be highlighted.  This function determines which tab in a tab control,
* if any, is under the mouse and records that information.  The caller may
* supply output parameters to receive the item number of the tab item which
* was highlighted but isn't any longer and of the tab item which is now
* highlighted but wasn't previously.  The caller can use this information to
* selectively redraw those tab items.
*
* If the caller has a mouse position, it can supply it through the pos
* parameter.  For example, _MouseMove does this.  Otherwise, the caller
* supplies NULL and this function determines the current mouse position
* itself.
*/
static void TAB_RecalcHotTrack
(
	TAB_INFO*       infoPtr,
	const LPARAM*   pos,
	int*            out_redrawLeave,
	int*            out_redrawEnter
)
{
	int item = -1;


	if (out_redrawLeave != NULL)
		*out_redrawLeave = -1;
	if (out_redrawEnter != NULL)
		*out_redrawEnter = -1;

	if ((infoPtr->dwStyle & TCS_HOTTRACK) || infoPtr->htheme)
	{
		POINT pt;
		UINT  flags;

		if (pos == NULL)
		{
			GetCursorPos(&pt);
			ScreenToClient(infoPtr->hwnd, &pt);
		}
		else
		{
			pt.x = (short)LOWORD(*pos);
			pt.y = (short)HIWORD(*pos);
		}

		item = TAB_InternalHitTest(infoPtr, pt, &flags);
	}

	if (item != infoPtr->iHotTracked)
	{
		if (infoPtr->iHotTracked >= 0)
		{
			/* Mark currently hot-tracked to be redrawn to look normal */
			if (out_redrawLeave != NULL)
				*out_redrawLeave = infoPtr->iHotTracked;

			if (item < 0)
			{
				/* Kill timer which forces recheck of mouse pos */
				KillTimer(infoPtr->hwnd, TAB_HOTTRACK_TIMER);
			}
		}
		else
		{
			/* Start timer so we recheck mouse pos */
			UINT timerID = SetTimer
			(
				infoPtr->hwnd,
				TAB_HOTTRACK_TIMER,
				TAB_HOTTRACK_TIMER_INTERVAL,
				TAB_HotTrackTimerProc
			);

			if (timerID == 0)
				return; /* Hot tracking not available */
		}

		infoPtr->iHotTracked = item;

		if (item >= 0)
		{
			/* Mark new hot-tracked to be redrawn to look highlighted */
			if (out_redrawEnter != NULL)
				*out_redrawEnter = item;
		}
	}
}

/******************************************************************************
* _MouseMove
*
* Handles the mouse-move event.  Updates tooltips.  Updates hot-tracking.
*/

static void Tab_TrackMouseStart (TAB_INFO *infoPtr, UINT start)
{
	if (!infoPtr->vModeDragging)
	{
		infoPtr->vModeDragging = true;
		infoPtr->vModeDragSt = start;
		infoPtr->vModeWidthSt = infoPtr->vModeWidth;
		HCURSOR hcur = ::LoadCursor(NULL, IDC_SIZEWE);
		::SetCursor(hcur);
		SetCapture(infoPtr->hwnd);
	}
}

static void Tab_TrackMouseMove (TAB_INFO *infoPtr, WPARAM wParam, LPARAM lParam)
{
	int clientX = GET_X_LPARAM(lParam) - infoPtr->vModeDragSt + infoPtr->vModeWidthSt;
	int minMax = infoPtr->tabHeight + 4;
	if (clientX<minMax)
	{
		clientX=minMax;
	}
	RECT rc;
	::GetClientRect(infoPtr->hwnd, &rc);
	minMax = rc.right - rc.left;
	if (clientX>minMax)
	{
		clientX=minMax;
	}
	infoPtr->vModeWidth = clientX;
	TAB_SetItemBounds(infoPtr);
	SendMessage(infoPtr->hwndNotify, WM_SIZE, 0, 0);
	TAB_InvalidateTabArea(infoPtr);
	//TAB_SendSimpleNotify(infoPtr, TCN_SIZECHANGE);
}

static LRESULT _TrackVerticalTabs (TAB_INFO *infoPtr, int x, int y)
{
	Tab_TrackMouseStart(infoPtr, x);
	Tab_TrackMouseMove(infoPtr, x, y);
	return 0;
}

static inline void Tab_TrackMouseEnd (TAB_INFO *infoPtr)
{
	if (infoPtr->vModeDragging)
	{
		infoPtr->vModeDragging = false;
		ReleaseCapture();
	}
}

static inline bool _GetIsTrackingMouse (TAB_INFO *infoPtr)
{
	return infoPtr->vModeDragging;
}

static inline LRESULT _DismissToolTips (TAB_INFO *infoPtr)
{
	if (infoPtr->hwndToolTip)
		TAB_RelayEvent (infoPtr->hwndToolTip, infoPtr->hwndToolTip,
			WM_NCMOUSEMOVE, 0, 0);
	return 0;
}

static inline LRESULT _GetVerticalModeWidth (TAB_INFO *infoPtr)
{
	return infoPtr->vModeWidth;
}

static inline LRESULT _SetVerticalModeWidth (TAB_INFO *infoPtr, UINT val)
{
	infoPtr->vModeWidth = val;
	return 0;
}

static inline LRESULT _GetTopMostRow (TAB_INFO *infoPtr)
{
	return infoPtr->topmostVisible;
}

static LRESULT _MouseMove (TAB_INFO *infoPtr, WPARAM wParam, LPARAM lParam)
{
	int redrawLeave;
	int redrawEnter;

	if (infoPtr->hwndToolTip)
		TAB_RelayEvent (infoPtr->hwndToolTip, infoPtr->hwnd,
			WM_MOUSEMOVE, wParam, lParam);

	/* Determine which tab to highlight.  Redraw tabs which change highlight
	** status. */
	TAB_RecalcHotTrack(infoPtr, &lParam, &redrawLeave, &redrawEnter);

	hottrack_refresh (infoPtr, redrawLeave);
	hottrack_refresh (infoPtr, redrawEnter);

	if (infoPtr->dwStyle & TCS_VERTICAL)
	{
		if (infoPtr->vModeDragging 
			|| _GetIsVerticalResizeArea(infoPtr, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
		{ 
			HCURSOR hcur = ::LoadCursor(NULL, IDC_SIZEWE);
			::SetCursor(hcur);
			if (infoPtr->vModeDragging)
			{
				Tab_TrackMouseMove(infoPtr, wParam, lParam);
			}
		}
	}

	return 0;
}

/******************************************************************************
* TAB_AdjustRect
*
* Calculates the tab control's display area given the window rectangle or
* the window rectangle given the requested display rectangle.
*/
static LRESULT _AdjustRect(const TAB_INFO *infoPtr, WPARAM fLarger, LPRECT prc)
{
	LONG *iRightBottom, *iLeftTop;

	//TRACE ("hwnd=%p fLarger=%ld (%s)\n", infoPtr->hwnd, fLarger, wine_dbgstr_rect(prc));

	if (!prc) return -1;

	if(infoPtr->dwStyle & TCS_VERTICAL)
	{
		iRightBottom = &(prc->right);
		iLeftTop     = &(prc->left);
	}
	else
	{
		iRightBottom = &(prc->bottom);
		iLeftTop     = &(prc->top);
	}

	int rows = infoPtr->uNumRows;

	if (infoPtr->tabMaxRows>0 && infoPtr->tabMaxRows<rows)
	{
		rows = infoPtr->tabMaxRows;
	}

	if (fLarger) /* Go from display rectangle */
	{
		/* Add the height of the tabs. */
		if(infoPtr->dwStyle & TCS_VERTICAL)
		{
			*iLeftTop -= infoPtr->vModeWidth;
		}
		else
		{
			if (infoPtr->dwStyle & TCS_BOTTOM)
				*iRightBottom += infoPtr->tabHeight * rows;
			else
				*iLeftTop -= infoPtr->tabHeight * rows +
				((infoPtr->dwStyle & TCS_BUTTONS)? 3 * (rows - 1) : 0);
		}

		/* Inflate the rectangle for the padding */
		InflateRect(prc, DISPLAY_AREA_PADDINGX, DISPLAY_AREA_PADDINGY); 

		/* Inflate for the border */
		InflateRect(prc, CONTROL_BORDER_SIZEX, CONTROL_BORDER_SIZEY);
	}
	else /* Go from window rectangle. */
	{
		/* Deflate the rectangle for the border */
		InflateRect(prc, -CONTROL_BORDER_SIZEX, -CONTROL_BORDER_SIZEY);

		/* Deflate the rectangle for the padding */
		InflateRect(prc, -DISPLAY_AREA_PADDINGX, -DISPLAY_AREA_PADDINGY);

		if(infoPtr->dwStyle & TCS_VERTICAL)
		{
			*iLeftTop += infoPtr->vModeWidth;
		}
		else
		{
			/* Remove the height of the tabs. */
			if (infoPtr->dwStyle & TCS_BOTTOM)
				*iRightBottom -= infoPtr->tabHeight * rows;
			else
				*iLeftTop += (infoPtr->tabHeight) * rows +
				((infoPtr->dwStyle & TCS_BUTTONS)? 3 * (rows - 1) : 0);
		}
	}

	return 0;
}

/******************************************************************************
* _OnHScroll
*
* This method will handle the notification from the scroll control and
* perform the scrolling operation on the tab control.
*/
static LRESULT _OnHScroll(TAB_INFO *infoPtr, int nScrollCode, int nPos)
{
	if(nScrollCode == SB_THUMBPOSITION 
		&& nPos != infoPtr->leftmostVisible && (infoPtr->dwStyle&TCS_MULTILINE)==0)
	{
		if(nPos < infoPtr->leftmostVisible)
			infoPtr->leftmostVisible--;
		else
			infoPtr->leftmostVisible++;

		TAB_RecalcHotTrack(infoPtr, NULL, NULL, NULL);
		TAB_InvalidateTabArea(infoPtr);
		SendMessageW(infoPtr->hwndUpDown, UDM_SETPOS, 0,
			MAKELONG(infoPtr->leftmostVisible, 0));
	}

	return 0;
}

static LRESULT _OnVScroll(TAB_INFO *infoPtr, int nScrollCode, int nPos)
{
	//TRACE("TAB_OnVScroll:: %d -> %d curr=%d \n", nPos, nPos, infoPtr->topmostVisible);

	//if(1) return 0;
	//nPos = infoPtr->maxRange-nPos;

	if(nScrollCode == SB_THUMBPOSITION 
		&& nPos != infoPtr->topmostVisible 
		&& infoPtr->multilineScrolling)
	{

		if(nPos < infoPtr->topmostVisible)
		{
			infoPtr->topmostVisible--;
		}
		else
		{
			infoPtr->topmostVisible++;
		}

		if(infoPtr->topmostVisible<0)
			infoPtr->topmostVisible = 0;
		else if(infoPtr->topmostVisible > infoPtr->uNumRows - infoPtr->tabMaxRows)
			infoPtr->topmostVisible = infoPtr->uNumRows - infoPtr->tabMaxRows;

		TAB_RecalcHotTrack(infoPtr, NULL, NULL, NULL);
		TAB_InvalidateTabArea(infoPtr);
		_DismissToolTips(infoPtr);
		SendMessageW(infoPtr->hwndUpDown, UDM_SETPOS, 0, MAKELONG(infoPtr->topmostVisible, 0));
	}

	return 0;
}

/******************************************************************************
* TAB_SetupScrolling
*
* This method will check the current scrolling state and make sure the
* scrolling control is displayed (or not).
*/
static void TAB_SetupScrolling(TAB_INFO*   infoPtr, const RECT* clientRect)
{
	INT maxRange = 0;

	if (infoPtr->needsScrolling || infoPtr->multilineScrolling)
	{
		RECT controlPos;
		INT vsize, tabwidth;
		INT dimX=2, dimY=1;
		if (infoPtr->multilineScrolling)
		{
			dimX=1;
			dimY=2;
		}
		/*
		* Calculate the position of the scroll control.
		*/
		controlPos.right = clientRect->right;
		if (infoPtr->dwStyle & TCS_VERTICAL)
		{
			controlPos.right = infoPtr->vModeWidth;
		}
		controlPos.left  = controlPos.right - dimX * GetSystemMetrics(SM_CXHSCROLL);


		if (infoPtr->dwStyle & TCS_BOTTOM)
		{
			controlPos.top    = clientRect->bottom - infoPtr->tabHeight * (infoPtr->multilineScrolling?infoPtr->tabMaxRows:1);
			controlPos.bottom = controlPos.top + dimY * GetSystemMetrics(SM_CYHSCROLL);
		}
		else
		{
			controlPos.bottom = clientRect->top + infoPtr->tabHeight * (infoPtr->multilineScrolling?infoPtr->tabMaxRows:1);
			controlPos.top    = controlPos.bottom - dimY * GetSystemMetrics(SM_CYHSCROLL);
		}

		/*
		* If we don't have a scroll control yet, we want to create one.
		* If we have one, we want to make sure it's positioned properly.
		*/
		if (infoPtr->hwndUpDown 
			&& (GetWindowLongPtr(infoPtr->hwndUpDown, GWL_STYLE)&UDS_HORZ) ^ !infoPtr->multilineScrolling)
		{
			DestroyWindow(infoPtr->hwndUpDown);
			infoPtr->hwndUpDown = 0;
		}
		if (infoPtr->hwndUpDown==0)
		{
			infoPtr->hwndUpDown = CreateWindowW(UPDOWN_CLASSW, L"",
				WS_VISIBLE | WS_CHILD | (infoPtr->multilineScrolling?0:UDS_HORZ),
				controlPos.left, controlPos.top,
				controlPos.right - controlPos.left,
				controlPos.bottom - controlPos.top,
				infoPtr->hwnd, NULL, NULL, NULL);
		}
		else
		{
			SetWindowPos(infoPtr->hwndUpDown,
				NULL,
				controlPos.left, controlPos.top,
				controlPos.right - controlPos.left,
				controlPos.bottom - controlPos.top,
				SWP_SHOWWINDOW | SWP_NOZORDER);
		}

		/* Now calculate upper limit of the updown control range.
		* We do this by calculating how many tabs will be offscreen when the
		* last tab is visible.
		*/
		if (infoPtr->multilineScrolling)
		{
			maxRange = infoPtr->uNumRows - infoPtr->tabMaxRows;
		}
		else if(infoPtr->uNumItem)
		{
			vsize = clientRect->right - (controlPos.right - controlPos.left + 1);
			maxRange = infoPtr->uNumItem;
			tabwidth = TAB_GetItem(infoPtr, infoPtr->uNumItem - 1)->rect.right;

			for(; maxRange > 0; maxRange--)
			{
				if(tabwidth - TAB_GetItem(infoPtr,maxRange - 1)->rect.left > vsize)
					break;
			}

			if(maxRange == infoPtr->uNumItem)
				maxRange--;
		}
		//infoPtr->maxRange = maxRange;
	}
	else
	{
		/* If we once had a scroll control... hide it */
		if (infoPtr->hwndUpDown)
			ShowWindow(infoPtr->hwndUpDown, SW_HIDE);
	}
	if (infoPtr->hwndUpDown)
	{
		if (infoPtr->multilineScrolling)
		{
			SendMessageW(infoPtr->hwndUpDown, UDM_SETRANGE32, maxRange, 0);
		}
		else
		{
			SendMessageW(infoPtr->hwndUpDown, UDM_SETRANGE32, 0, maxRange);
		}
	}
}

/******************************************************************************
* TAB_SetItemBounds
*
* This method will calculate the position rectangles of all the items in the
* control. The rectangle calculated starts at 0 for the first item in the
* list and ignores scrolling and selection.
* It also uses the current font to determine the height of the tab row and
* it checks if all the tabs fit in the client area of the window. If they
* don't, a scrolling control is added.
*/
static void TAB_SetItemBounds (TAB_INFO *infoPtr)
{
	TEXTMETRICW fontMetrics;
	UINT        curItem;
	INT         curItemLeftPos;
	INT         curItemRowCount;
	HFONT       hFont, hOldFont;
	HDC         hdc;
	RECT        clientRect;
	INT         iTemp;
	RECT*       rcItem;
	INT         iIndex;
	INT         icon_width = 0;

	/*
	* We need to get text information so we need a DC and we need to select
	* a font.
	*/
	hdc = GetDC(infoPtr->hwnd);

	hFont = infoPtr->hFont ? infoPtr->hFont : (HFONT)GetStockObject (SYSTEM_FONT);
	hOldFont = (HFONT) SelectObject (hdc, hFont);

	/*
	* We will base the rectangle calculations on the client rectangle
	* of the control.
	*/
	GetClientRect(infoPtr->hwnd, &clientRect);

	int win_width = clientRect.right - clientRect.left;
	int win_height = clientRect.bottom - clientRect.top;

	/* Now use hPadding and vPadding */
	infoPtr->uHItemPadding = infoPtr->uHItemPadding_s;
	infoPtr->uVItemPadding = infoPtr->uVItemPadding_s;

	/* The leftmost item will be "0" aligned */
	curItemLeftPos = 0;
	curItemRowCount = infoPtr->uNumItem ? 1 : 0;

	if (!(infoPtr->fHeightSet))
	{
		int item_height;
		INT icon_height = 0, cx;

		/* Use the current font to determine the height of a tab. */
		GetTextMetricsW(hdc, &fontMetrics);

		/* Get the icon height */
		if (infoPtr->himl)
			ImageList_GetIconSize(infoPtr->himl, &cx, &icon_height);

		/* Take the highest between font or icon */
		if (fontMetrics.tmHeight > icon_height)
			item_height = fontMetrics.tmHeight + 2;
		else
			item_height = icon_height + 2; // enlarge 2 px in height

		/*
		* Make sure there is enough space for the letters + icon + growing the
		* selected item + extra space for the selected item.
		*/
		infoPtr->tabHeight = item_height + 
			((infoPtr->dwStyle & TCS_BUTTONS) ? 2 : 1) *
			infoPtr->uVItemPadding;

		TRACE("tabH=%d, tmH=%d, iconh=%d\n",
			infoPtr->tabHeight, fontMetrics.tmHeight, icon_height);
	}

	TRACE("client right=%d\n", clientRect.right);

	/* Get the icon width */
	if (infoPtr->himl)
	{
		INT cy;

		ImageList_GetIconSize(infoPtr->himl, &icon_width, &cy);

		if (infoPtr->dwStyle & TCS_FIXEDWIDTH)
			icon_width += 4;
		else
			/* Add padding if icon is present */
			icon_width += infoPtr->uHItemPadding;
	}

	int allTabsWidth=0;

	std::vector<LONG> rowWidSepMap;

	TAB_ITEM *curr;
	curItem = 0;
	for (; curItem < infoPtr->uNumItem; curItem++)
	{
		curr = TAB_GetItem(infoPtr, curItem);

		/* Set the leftmost position of the tab. */
		curr->rect.left = curItemLeftPos;

		if (infoPtr->dwStyle & TCS_FIXEDWIDTH)
		{
			curr->rect.right = curr->rect.left +
				max(infoPtr->tabWidth, icon_width);
		}
		else if (!curr->pszText)
		{
			/* If no text use minimum tab width including padding. */
			if (infoPtr->tabMinWidth < 0)
				curr->rect.right = curr->rect.left + GET_DEFAULT_MIN_TAB_WIDTH(infoPtr);
			else
			{
				curr->rect.right = curr->rect.left + infoPtr->tabMinWidth;

				/* Add extra padding if icon is present */
				if (infoPtr->himl && infoPtr->tabMinWidth > 0 && infoPtr->tabMinWidth < DEFAULT_MIN_TAB_WIDTH
					&& infoPtr->uHItemPadding > 1)
					curr->rect.right += EXTRA_ICON_PADDING * (infoPtr->uHItemPadding-1);
			}
		}
		else
		{
			int tabwidth;
			SIZE size;
			/* Calculate how wide the tab is depending on the text it contains */
			GetTextExtentPoint32W(hdc, curr->pszText,
				lstrlenW(curr->pszText), &size);

			tabwidth = size.cx + icon_width + 2 * infoPtr->uHItemPadding;

			if (infoPtr->tabMinWidth < 0)
				tabwidth = max(tabwidth, GET_DEFAULT_MIN_TAB_WIDTH(infoPtr));
			else
				tabwidth = max(tabwidth, infoPtr->tabMinWidth);

			curr->rect.right = curr->rect.left + tabwidth;
			TRACE("for <%s>, rect %s\n", debugstr_w(curr->pszText), wine_dbgstr_rect(&curr->rect));
		}

		/*
		* Check if this is a multiline tab control and if so
		* check to see if we should wrap the tabs
		*
		* Wrap all these tabs. We will arrange them evenly later.
		*
		*/
		if (infoPtr->dwStyle & TCS_VERTICAL)
		{
			curr->rect.right = infoPtr->vModeWidth;
			curr->rect.left = 0;
			curr->rect.bottom = 0;
			curr->rect.top = curItemRowCount - 1;
			curItemRowCount++;
			continue;
		}
		else if (((infoPtr->dwStyle & TCS_MULTILINE) || (infoPtr->dwStyle & TCS_VERTICAL)) &&
			(curr->rect.right > 
				(clientRect.right - CONTROL_BORDER_SIZEX - DISPLAY_AREA_PADDINGX)))
		{
			curr->rect.right -= curr->rect.left;

			rowWidSepMap.push_back(MAKELONG(curr->rect.left, curItem));

			//LogIs(3, "rowsWidSepMap, %d/%d row.%d :: %d,%d == %d,%d"
			//	, curItem,infoPtr->uNumItem
			//	, curItemRowCount-1
			//
			//	, curr->rect.left
			//	, curItem
			//
			//	, (int)LOWORD(rowWidSepMap[curItemRowCount-1])
			//	, (int)HIWORD(rowWidSepMap[curItemRowCount-1]));

			curr->rect.left = 0;
			curItemRowCount++;
			// 第一轮分割
			TRACE("wrapping <%s>, rect %s\n", debugstr_w(curr->pszText), wine_dbgstr_rect(&curr->rect));
		}

		allTabsWidth += curr->rect.right-curr->rect.left;

		curr->rect.bottom = 0;
		curr->rect.top = curItemRowCount - 1;

		TRACE("Rect: %s\n", wine_dbgstr_rect(&curr->rect));

		/*
		* The leftmost position of the next item is the rightmost position
		* of this one.
		*/
		if (infoPtr->dwStyle & TCS_BUTTONS)
		{
			curItemLeftPos = curr->rect.right + BUTTON_SPACINGX;
			if (infoPtr->dwStyle & TCS_FLATBUTTONS)
				curItemLeftPos += FLAT_BTN_SPACINGX;
		}
		else
			curItemLeftPos = curr->rect.right;


		if (curItem==infoPtr->uNumItem-1)
		{
			rowWidSepMap.push_back(MAKELONG(curr->rect.right, curItem+1));
		}
	}

	if (infoPtr->dwStyle & TCS_VERTICAL)
	{
		infoPtr->tabMaxRows = win_height / infoPtr->tabHeight;
		infoPtr->needsScrolling = FALSE;
		infoPtr->leftmostVisible = 0;
	}
	else if (!(infoPtr->dwStyle & TCS_MULTILINE))
	{
		/*
		* Check if we need a scrolling control.
		*/
		infoPtr->needsScrolling = (curItemLeftPos + (2 * SELECTED_TAB_OFFSET) >
			clientRect.right);

		/* Don't need scrolling, then update infoPtr->leftmostVisible */
		if(!infoPtr->needsScrolling)
			infoPtr->leftmostVisible = 0;
		infoPtr->multilineScrolling = FALSE;
	}
	else
	{
		infoPtr->tabMaxRows = infoPtr->tabMaxRowSet;
		infoPtr->needsScrolling = FALSE;
		infoPtr->leftmostVisible = 0;
	}

	/* Set the number of rows */
	infoPtr->uNumRows = curItemRowCount;

	// 第二轮分割. Arrange all tabs evenly if style says so
	if ((infoPtr->dwStyle & TCS_MULTILINE)
		&& (infoPtr->dwStyle & TCS_VERTICAL)==0
		&& (infoPtr->dwStyle & TCS_RAGGEDRIGHT)==0
		&& (infoPtr->uNumItem > 0)
		&& (infoPtr->uNumRows > 1)
		)
	{
		UINT iRow,thisRowSep;
		if(rowWidSepMap.size()>1)
		{
			UINT thisRowWidth,iItm
				,thisRow,upperRow,nextRowSep,upperRowWidth
				,thisBorrowCount,itemToBorrowWid;

			//UINT avgTabItemWid = win_width && infoPtr->uNumItem && infoPtr->uNumRows?win_width/(infoPtr->uNumItem/infoPtr->uNumRows):0;
			TAB_ITEM *curr;
			//Even out rows
			int maxBorrowSz = infoPtr->uNumRows;

			for (thisRow = infoPtr->uNumRows-1; thisRow > 0; thisRow--)
			{
				upperRow = thisRow-1;
				thisRowWidth=LOWORD(rowWidSepMap[thisRow]);
				nextRowSep=HIWORD(rowWidSepMap[thisRow]);
				upperRowWidth=LOWORD(rowWidSepMap[upperRow]);
				thisRowSep = HIWORD(rowWidSepMap[upperRow]);
				//LogIs(3, "rowsWidSepMap.size() %d", rowWidSepMap.size());
				//LogIs(3, "thisRowSep&Wid, row.%d    %d,%d"
				//	, thisRow
				//	, HIWORD(rowWidSepMap[thisRow])), LOWORD(rowWidSepMap[thisRow]);
				//LogIs(3, "upperRowSep&Wid, row.%d   %d,%d"
				//	, upperRow
				//	, HIWORD(rowWidSepMap[upperRow])), LOWORD(rowWidSepMap[upperRow]);
				//LogIs(3, "avgTabItemWid==, %d" , avgTabItemWid);
				for (thisBorrowCount = 0
					; thisBorrowCount < maxBorrowSz && thisRowSep>0
					; thisBorrowCount++)
				{
					curr = TAB_GetItem(infoPtr, thisRowSep-1);
					itemToBorrowWid = curr->rect.right - curr->rect.left;
					if (thisRowWidth+itemToBorrowWid<upperRowWidth-itemToBorrowWid+0)// avgTabItemWid
					{
						upperRowWidth-=itemToBorrowWid;
						thisRowWidth+=itemToBorrowWid;
						--thisRowSep;
					}
					else
					{
						break;
					}
				}
				rowWidSepMap[thisRow]=MAKELONG(thisRowWidth, nextRowSep);
				rowWidSepMap[upperRow]=MAKELONG(upperRowWidth, thisRowSep);

				maxBorrowSz--;
			}

			// Apply modified layout.
			thisRowSep=HIWORD(rowWidSepMap[0]);
			thisRowWidth=LOWORD(rowWidSepMap[0]);
			for (iItm=0,iRow=0,curItemLeftPos=0;
				iItm<infoPtr->uNumItem;
				iItm++)
			{
				curr = TAB_GetItem(infoPtr, iItm);
				curr->rect.right -= curr->rect.left;
				curr->rect.left = 0;
				if (iItm >= thisRowSep) 
				{
					iRow++;
					curItemLeftPos = 0;
					thisRowSep = HIWORD(rowWidSepMap[iRow]);
					thisRowWidth = LOWORD(rowWidSepMap[iRow]);
				}
				//curr->rect.right = curr->rect.right*1.0f/thisRowWidth*win_width;
				curr->rect.left += curItemLeftPos;
				curr->rect.right += curItemLeftPos;
				curr->rect.top = iRow;
				if (infoPtr->dwStyle & TCS_BUTTONS)
				{
					curItemLeftPos = curr->rect.right + 1;
					if (infoPtr->dwStyle & TCS_FLATBUTTONS)
						curItemLeftPos += FLAT_BTN_SPACINGX;
				}
				else
					curItemLeftPos = curr->rect.right;
			}

		}

		/*
		* Justify the rows
		*/
		//if (false)
		{
			// todo merge this pass to the above loop 
			INT widthDiff, iIndexStart=0, iIndexEnd=0;
			INT remainder;
			INT iCount=0;
			iRow=0;
			thisRowSep=HIWORD(rowWidSepMap[0]);
			while(iIndexStart < infoPtr->uNumItem)
			{
				TAB_ITEM *start = TAB_GetItem(infoPtr, iIndexStart);

				if (iIndexStart >= thisRowSep) 
				{
					iRow++;
					thisRowSep = HIWORD(rowWidSepMap[iRow]);
				}

				/*
				* find the index of the row
				*/
				/* find the first item on the next row */
				iIndexEnd = iRow+1<infoPtr->uNumRows?HIWORD(rowWidSepMap[iRow]):infoPtr->uNumItem;

				/*
				* we need to justify these tabs so they fill the whole given
				* client area
				*
				*/
				/* find the amount of space remaining on this row */
				widthDiff = clientRect.right - (2 * SELECTED_TAB_OFFSET) -
					TAB_GetItem(infoPtr, iIndexEnd - 1)->rect.right;

				/* iCount is the number of tab items on this row */
				iCount = iIndexEnd - iIndexStart;

				if (iCount > 1)
				{
					remainder = widthDiff % iCount;
					widthDiff = widthDiff / iCount;
					/* add widthDiff/iCount, or extra space/items on row, to each item on this row */
					for (iIndex=iIndexStart, iCount=0; iIndex < iIndexEnd; iIndex++, iCount++)
					{
						TAB_ITEM *item = TAB_GetItem(infoPtr, iIndex);

						item->rect.left += iCount * widthDiff;
						item->rect.right += (iCount + 1) * widthDiff;

						TRACE("adjusting 1 <%s>, rect %s\n", debugstr_w(item->pszText), wine_dbgstr_rect(&item->rect));

					}
					TAB_GetItem(infoPtr, iIndex - 1)->rect.right += remainder;
				}
				else /* we have only one item on this row, make it take up the entire row */
				{
					start->rect.left = clientRect.left;
					start->rect.right = clientRect.right - 4;

					TRACE("adjusting 2 <%s>, rect %s\n", debugstr_w(start->pszText), wine_dbgstr_rect(&start->rect));
				}

				iIndexStart = iIndexEnd;
			}
		}
	}

	if (infoPtr->dwStyle & TCS_MULTILINE || infoPtr->dwStyle & TCS_VERTICAL)
	{
		infoPtr->multilineScrolling 
			= infoPtr->tabMaxRows>0 && infoPtr->uNumRows > infoPtr->tabMaxRows;
		if (infoPtr->multilineScrolling)
		{
			if(infoPtr->topmostVisible<0)
				infoPtr->topmostVisible = 0;
			else if(infoPtr->topmostVisible > infoPtr->uNumRows - infoPtr->tabMaxRows)
				infoPtr->topmostVisible = infoPtr->uNumRows - infoPtr->tabMaxRows;
		}
		else 
		{
			infoPtr->topmostVisible = 0;
		}
	}

	TAB_SetupScrolling(infoPtr, &clientRect);

	TAB_EnsureSelectionVisible(infoPtr);
	TAB_RecalcHotTrack(infoPtr, NULL, NULL, NULL);

	/* Cleanup */
	SelectObject (hdc, hOldFont);
	ReleaseDC (infoPtr->hwnd, hdc);
}


static void TAB_EraseTabInterior(const TAB_INFO *infoPtr, HDC hdc, INT iItem, const RECT *drawRect)
{
	HBRUSH   hbr = CreateSolidBrush (comctl32_color.clrBtnFace);
	BOOL     deleteBrush = TRUE;
	RECT     rTemp = *drawRect;

	if (infoPtr->dwStyle & TCS_BUTTONS)
	{
		if (iItem == infoPtr->iSelected)
		{
			/* Background color */
			if (!(infoPtr->dwStyle & TCS_OWNERDRAWFIXED))
			{
				DeleteObject(hbr);
				hbr = GetSysColorBrush(COLOR_SCROLLBAR);

				SetTextColor(hdc, comctl32_color.clr3dFace);
				SetBkColor(hdc, comctl32_color.clr3dHilight);

				/* if COLOR_WINDOW happens to be the same as COLOR_3DHILIGHT
				* we better use 0x55aa bitmap brush to make scrollbar's background
				* look different from the window background.
				*/
				if (comctl32_color.clr3dHilight == comctl32_color.clrWindow)
					hbr = COMCTL32_hPattern55AABrush;

				deleteBrush = FALSE;
			}
			FillRect(hdc, &rTemp, hbr);
		}
		else  /* ! selected */
		{
			if (infoPtr->dwStyle & TCS_FLATBUTTONS)
			{
				InflateRect(&rTemp, 2, 2);
				FillRect(hdc, &rTemp, hbr);
				if (iItem == infoPtr->iHotTracked ||
					(iItem != infoPtr->iSelected && iItem == infoPtr->uFocus))
					DrawEdge(hdc, &rTemp, BDR_RAISEDINNER, BF_RECT);
			}
			else
				FillRect(hdc, &rTemp, hbr);
		}

	}
	else /* !TCS_BUTTONS */
	{
		InflateRect(&rTemp, -2, -2);
		if (!infoPtr->htheme)
			FillRect(hdc, &rTemp, hbr);
	}

	/* highlighting is drawn on top of previous fills */
	if (false && TAB_GetItem(infoPtr, iItem)->dwState & TCIS_HIGHLIGHTED)
	{
		if (deleteBrush)
		{
			DeleteObject(hbr);
			deleteBrush = FALSE;
		}
		hbr = GetSysColorBrush(COLOR_HIGHLIGHT);
		FillRect(hdc, &rTemp, hbr);
	}

	/* Cleanup */
	if (deleteBrush) DeleteObject(hbr);
}

/******************************************************************************
* TAB_DrawItemInterior
*
* This method is used to draw the interior (text and icon) of a single tab
* into the tab control.
*/
static void TAB_DrawItemInterior(const TAB_INFO *infoPtr, HDC hdc, INT iItem, RECT *drawRect)
{
	RECT localRect;

	HPEN   htextPen;
	HPEN   holdPen;
	INT    oldBkMode;
	HFONT  hOldFont;

	/*  if (drawRect == NULL) */
	{
		BOOL isVisible;
		RECT itemRect;
		RECT selectedRect;

		/*
		* Get the rectangle for the item.
		*/
		isVisible = TAB_InternalGetItemRect(infoPtr, iItem, &itemRect, &selectedRect);
		if (!isVisible)
			return;

		/*
		* Make sure drawRect points to something valid; simplifies code.
		*/
		drawRect = &localRect;

		/*
		* This logic copied from the part of TAB_DrawItem which draws
		* the tab background.  It's important to keep it in sync.  I
		* would have liked to avoid code duplication, but couldn't figure
		* out how without making spaghetti of TAB_DrawItem.
		*/
		if (iItem == infoPtr->iSelected)
			*drawRect = selectedRect;
		else
			*drawRect = itemRect;

		if (infoPtr->dwStyle & TCS_BUTTONS)
		{
			if (iItem == infoPtr->iSelected)
			{
				drawRect->left   += 4;
				drawRect->top    += 4;
				drawRect->right  -= 4;

				{
					if (infoPtr->dwStyle & TCS_BOTTOM)
					{
						drawRect->top    -= 2;
						drawRect->bottom -= 4;
					}
					else
						drawRect->bottom -= 1;
				}
			}
			else
				InflateRect(drawRect, -2, -2);
		}
		else
		{
			if (infoPtr->dwStyle & TCS_BOTTOM)
			{
				if (iItem == infoPtr->iSelected)
				{
					drawRect->top    -= 2;
				}
				else
				{
					InflateRect(drawRect, -2, -2);
					drawRect->bottom += 2;
				}
			}
			else
			{
				if (iItem == infoPtr->iSelected)
				{
					drawRect->bottom += 3;
				}
				else
				{
					drawRect->bottom -= 2;
					InflateRect(drawRect, -2, 0);
				}
			}
		}
	}
	TRACE("drawRect=(%s)\n", wine_dbgstr_rect(drawRect));

	/* Clear interior */
	TAB_EraseTabInterior (infoPtr, hdc, iItem, drawRect);

	/* Draw the focus rectangle */
	if (!(infoPtr->dwStyle & TCS_FOCUSNEVER) &&
		(GetFocus() == infoPtr->hwnd) &&
		(iItem == infoPtr->uFocus) )
	{
		RECT rFocus = *drawRect;

		if (!(infoPtr->dwStyle & TCS_BUTTONS)) InflateRect(&rFocus, -3, -3);
		if (infoPtr->dwStyle & TCS_BOTTOM && !(infoPtr->dwStyle & TCS_VERTICAL))
			rFocus.top -= 3;

		/* focus should stay on selected item for TCS_BUTTONS style */
		if (!((infoPtr->dwStyle & TCS_BUTTONS) && (infoPtr->iSelected != iItem)))
			DrawFocusRect(hdc, &rFocus);
	}

	/*
	* Text pen
	*/
	htextPen = CreatePen( PS_SOLID, 1, comctl32_color.clrBtnText );
	holdPen  = (HPEN) SelectObject(hdc, htextPen);
	hOldFont = (HFONT)SelectObject(hdc, infoPtr->hFont);

	/*
	* Setup for text output
	*/
	oldBkMode = SetBkMode(hdc, TRANSPARENT);
	if (!infoPtr->htheme || (infoPtr->dwStyle & TCS_BUTTONS))
	{
		if ((infoPtr->dwStyle & TCS_HOTTRACK) && (iItem == infoPtr->iHotTracked) &&
			!(infoPtr->dwStyle & TCS_FLATBUTTONS))
			SetTextColor(hdc, comctl32_color.clrHighlight);
		else if (TAB_GetItem(infoPtr, iItem)->dwState & TCIS_HIGHLIGHTED)
			SetTextColor(hdc, comctl32_color.clrHighlightText);
		else
			SetTextColor(hdc, comctl32_color.clrBtnText);
	}

	/*
	* if owner draw, tell the owner to draw
	*/
	if ((infoPtr->dwStyle & TCS_OWNERDRAWFIXED) && IsWindow(infoPtr->hwndNotify))
	{
		DRAWITEMSTRUCT dis;
		UINT id;

		drawRect->top += 2;
		drawRect->right -= 1;
		if ( iItem == infoPtr->iSelected )
			InflateRect(drawRect, -1, 0);

		id = (UINT)GetWindowLongPtrW( infoPtr->hwnd, GWLP_ID );

		/* fill DRAWITEMSTRUCT */
		dis.CtlType    = ODT_TAB;
		dis.CtlID      = id;
		dis.itemID     = iItem;
		dis.itemAction = ODA_DRAWENTIRE;
		dis.itemState = 0;
		if ( iItem == infoPtr->iSelected )
			dis.itemState |= ODS_SELECTED;
		if (infoPtr->uFocus == iItem) 
			dis.itemState |= ODS_FOCUS;
		dis.hwndItem = infoPtr->hwnd;
		dis.hDC      = hdc;
		dis.rcItem = *drawRect;

		/* when extra data fits ULONG_PTR, store it directly */
		if (infoPtr->cbInfo > sizeof(LPARAM))
			dis.itemData =  (ULONG_PTR) TAB_GetItem(infoPtr, iItem)->extra;
		else
		{
			/* this could be considered broken on 64 bit, but that's how it works -
			only first 4 bytes are copied */
			dis.itemData = 0;
			memcpy(&dis.itemData, (ULONG_PTR*)TAB_GetItem(infoPtr, iItem)->extra, 4);
		}

		/* draw notification */
		SendMessageW( infoPtr->hwndNotify, WM_DRAWITEM, id, (LPARAM)&dis );
	}
	else
	{
		TAB_ITEM *item = TAB_GetItem(infoPtr, iItem);
		RECT rcTemp;
		RECT rcImage;

		/* used to center the icon and text in the tab */
		RECT rcText;
		INT center_offset_h, center_offset_v;

		/* set rcImage to drawRect, we will use top & left in our ImageList_Draw call */
		rcImage = *drawRect;

		rcTemp = *drawRect;
		SetRectEmpty(&rcText);

		/* get the rectangle that the text fits in */
		if (item->pszText)
		{
			DrawTextW(hdc, item->pszText, -1, &rcText, DT_CALCRECT);
		}
		/*
		* If not owner draw, then do the drawing ourselves.
		*
		* Draw the icon.
		*/
		bool selected = iItem == infoPtr->iSelected;
		if (infoPtr->himl && item->iImage != -1)
		{
			INT cx;
			INT cy;

			ImageList_GetIconSize(infoPtr->himl, &cx, &cy);

			{
				center_offset_h = ((drawRect->right - drawRect->left) - (cx + infoPtr->uHItemPadding + (rcText.right  - rcText.left))) / 2;
				center_offset_v = ((drawRect->bottom - drawRect->top) - cy) / 2;
			}

			/* if an item is selected, the icon is shifted up instead of down */
			if (iItem == infoPtr->iSelected)
				center_offset_v -= infoPtr->uVItemPadding / 2;
			else
				center_offset_v += infoPtr->uVItemPadding / 2;

			if (infoPtr->dwStyle & TCS_FIXEDWIDTH && infoPtr->dwStyle & (TCS_FORCELABELLEFT | TCS_FORCEICONLEFT))
				center_offset_h = infoPtr->uHItemPadding;

			if (center_offset_h < 2)
				center_offset_h = 2;

			if (center_offset_v < 0)
				center_offset_v = 0;

			TRACE("for <%s>, c_o_h=%d, c_o_v=%d, draw=(%s), textlen=%d\n",
				debugstr_w(item->pszText), center_offset_h, center_offset_v,
				wine_dbgstr_rect(drawRect), (rcText.right-rcText.left));

			/* normal style, whether TCS_BOTTOM or not */
			{
				rcImage.left = drawRect->left + center_offset_h;
				if (selected) // selected
				{
					rcImage.top = drawRect->top + center_offset_v + 0;
				} 
				else 
				{
					rcImage.top = drawRect->top + center_offset_v + 1;
				}
				drawRect->left += cx + infoPtr->uHItemPadding;
			}

			TRACE("drawing image=%d, left=%d, top=%d\n",
				item->iImage, rcImage.left, rcImage.top-1);
			ImageList_Draw
			(
				infoPtr->himl,
				item->iImage,
				hdc,
				rcImage.left,
				rcImage.top,
				ILD_NORMAL
			);
		}

		/* Now position text */
		if (infoPtr->dwStyle & TCS_FIXEDWIDTH && infoPtr->dwStyle & TCS_FORCELABELLEFT)
			center_offset_h = infoPtr->uHItemPadding;
		else
			center_offset_h = ((drawRect->right - drawRect->left) - (rcText.right - rcText.left)) / 2;

		{
			drawRect->left += center_offset_h;
			center_offset_v = ((drawRect->bottom - drawRect->top) - (rcText.bottom - rcText.top)) / 2;
		}

		/* if an item is selected, the text is shifted up instead of down */
		if (selected)
			center_offset_v -= infoPtr->uVItemPadding / 2;
		else
			center_offset_v += infoPtr->uVItemPadding / 2;

		if (center_offset_v < 0)
			center_offset_v = 0;

		//drawRect->top += center_offset_v;

		//if(false)
		if (selected)
		{
			drawRect->top +=    center_offset_v;
			drawRect->bottom += center_offset_v;
		} 
		else 
		{
			drawRect->top +=    1;
			drawRect->bottom += 1;
		}

		/* Draw the text */
		{
			TRACE("for <%s>, c_o_h=%d, c_o_v=%d, draw=(%s), textlen=%d\n",
				debugstr_w(item->pszText), center_offset_h, center_offset_v,
				wine_dbgstr_rect(drawRect), (rcText.right-rcText.left));
			if (item->pszText)
			{
				DrawTextW
				(
					hdc,
					item->pszText,
					lstrlenW(item->pszText),
					drawRect,
					DT_LEFT | DT_SINGLELINE
					| (selected?0:DT_VCENTER)
				);
			}
		}

		*drawRect = rcTemp; /* restore drawRect */
	}

	/*
	* Cleanup
	*/
	SelectObject(hdc, hOldFont);
	SetBkMode(hdc, oldBkMode);
	SelectObject(hdc, holdPen);
	DeleteObject( htextPen );
}

/******************************************************************************
* TAB_DrawItem
*
* This method is used to draw a single tab into the tab control.
*/
static void TAB_DrawItem(const TAB_INFO *infoPtr, HDC  hdc, INT  iItem)
{
	RECT      itemRect;
	RECT      selectedRect;
	BOOL      isVisible;
	RECT      r, fillRect, r1;
	INT       clRight = 0;
	INT       clBottom = 0;
	COLORREF  bkgnd, corner;
	HTHEME    theme;

	/*
	* Get the rectangle for the item.
	*/
	isVisible = TAB_InternalGetItemRect(infoPtr,
		iItem,
		&itemRect,
		&selectedRect);

	if (isVisible)
	{
		RECT rUD, rC;

		/* Clip UpDown control to not draw over it */
		if (infoPtr->needsScrolling)
		{
			GetWindowRect(infoPtr->hwnd, &rC);
			GetWindowRect(infoPtr->hwndUpDown, &rUD);
			ExcludeClipRect(hdc, rUD.left - rC.left, rUD.top - rC.top, rUD.right - rC.left, rUD.bottom - rC.top);
		}

		/* If you need to see what the control is doing,
		* then override these variables. They will change what
		* fill colors are used for filling the tabs, and the
		* corners when drawing the edge.
		*/
		bkgnd = comctl32_color.clrBtnFace;
		corner = comctl32_color.clrBtnFace;

		if (infoPtr->dwStyle & TCS_BUTTONS)
		{
			/* Get item rectangle */
			r = itemRect;

			/* Separators between flat buttons */
			if ((infoPtr->dwStyle & TCS_FLATBUTTONS) && (infoPtr->exStyle & TCS_EX_FLATSEPARATORS))
			{
				r1 = r;
				r1.right += (FLAT_BTN_SPACINGX -2);
				DrawEdge(hdc, &r1, EDGE_ETCHED, BF_RIGHT);
			}

			if (iItem == infoPtr->iSelected)
			{
				DrawEdge(hdc, &r, EDGE_SUNKEN, BF_SOFT|BF_RECT);

				OffsetRect(&r, 1, 1);
			}
			else  /* ! selected */
			{
				DWORD state = TAB_GetItem(infoPtr, iItem)->dwState;

				if ((state & TCIS_BUTTONPRESSED) || (iItem == infoPtr->uFocus))
					DrawEdge(hdc, &r, EDGE_SUNKEN, BF_SOFT|BF_RECT);
				else
					if (!(infoPtr->dwStyle & TCS_FLATBUTTONS))
						DrawEdge(hdc, &r, EDGE_RAISED, BF_SOFT|BF_RECT);
			}
		}
		else /* !TCS_BUTTONS */
		{
			/* We draw a rectangle of different sizes depending on the selection
			* state. */
			if (iItem == infoPtr->iSelected) {
				RECT rect;
				GetClientRect (infoPtr->hwnd, &rect);
				clRight = rect.right;
				clBottom = rect.bottom;
				r = selectedRect;
			}
			else
				r = itemRect;

			/*
			* Erase the background. (Delay it but setup rectangle.)
			* This is necessary when drawing the selected item since it is larger
			* than the others, it might overlap with stuff already drawn by the
			* other tabs
			*/
			fillRect = r;

			/* Draw themed tabs - but only if they are at the top.
			* Windows draws even side or bottom tabs themed, with wacky results.
			* However, since in Wine apps may get themed that did not opt in via
			* a manifest avoid theming when we know the result will be wrong */

			//LogIs(2, "GetWindowTheme (infoPtr->hwnd) %d %d", infoPtr->hwnd, ::GetWindowTheme(infoPtr->hwnd));

			// todo separate the draw theme branch
			if (((infoPtr->dwStyle & TCS_OWNERDRAWFIXED)==0) && 
				(theme = infoPtr->htheme ) 
				&& ((infoPtr->dwStyle & (TCS_VERTICAL | TCS_BOTTOM)) == 0)
				)
			{
				static const int partIds[8] = {
					/* Normal item */
					TABP_TABITEM,
					TABP_TABITEMLEFTEDGE,
					TABP_TABITEMRIGHTEDGE,
					TABP_TABITEMBOTHEDGE,
					/* Selected tab */
					TABP_TOPTABITEM,
					TABP_TOPTABITEMLEFTEDGE,
					TABP_TOPTABITEMRIGHTEDGE,
					TABP_TOPTABITEMBOTHEDGE,
				};
				int partIndex = 0;
				int stateId = TIS_NORMAL;

				/* selected and unselected tabs have different parts */
				if (iItem == infoPtr->iSelected)
					partIndex += 4;
				/* The part also differs on the position of a tab on a line.
				* "Visually" determining the position works well enough. */
				GetClientRect(infoPtr->hwnd, &r1);
				if(selectedRect.left == 0)
					partIndex += 1;
				if(selectedRect.right == r1.right)
					partIndex += 2;

				if (iItem == infoPtr->iSelected)
					stateId = TIS_SELECTED;
				else if (iItem == infoPtr->iHotTracked)
					stateId = TIS_HOT;
				else if (iItem == infoPtr->uFocus)
					stateId = TIS_FOCUSED;

				/* Adjust rectangle for bottommost row */
				if (TAB_GetItem(infoPtr, iItem)->rect.top == infoPtr->uNumRows-1)
					r.bottom += 3;

				DrawThemeBackground (theme, hdc, partIds[partIndex], stateId, &r, NULL);
				GetThemeBackgroundContentRect (theme, hdc, partIds[partIndex], stateId, &r, &r);
			}
			// vert...
			else  /* ! TCS_VERTICAL */
			{
				/* These are for adjusting the drawing of a Selected tab      */
				/* The initial values are for the normal case of non-Selected */
				if (iItem == infoPtr->iSelected) {
					/* if leftmost draw the line longer */
					if(selectedRect.left == 0)
						fillRect.left += CONTROL_BORDER_SIZEX;
					/* if rightmost draw the line longer */
					if(selectedRect.right == clRight)
						fillRect.right -= CONTROL_BORDER_SIZEX;
				}

				if (infoPtr->dwStyle & TCS_BOTTOM)
				{
					/* Adjust both rectangles for topmost row */
					if (TAB_GetItem(infoPtr, iItem)->rect.top == infoPtr->uNumRows-1)
					{
						fillRect.top -= 2;
						r.top -= 1;
					}

					TRACE("<bottom> item=%d, fill=(%s), edge=(%s)\n",
						iItem, wine_dbgstr_rect(&fillRect), wine_dbgstr_rect(&r));

					/* Clear interior */
					SetBkColor(hdc, bkgnd);
					ExtTextOutW(hdc, 0, 0, 2, &fillRect, NULL, 0, 0);

					/* Draw rectangular edge around tab */
					DrawEdge(hdc, &r, EDGE_RAISED, BF_SOFT|BF_LEFT|BF_BOTTOM|BF_RIGHT);

					/* Now erase the righthand corner and draw diagonal edge */
					SetBkColor(hdc, corner);
					r1.left = r.right - ROUND_CORNER_SIZE;
					r1.bottom = r.bottom;
					r1.right = r.right;
					r1.top = r1.bottom - ROUND_CORNER_SIZE - 1;
					ExtTextOutW(hdc, 0, 0, 2, &r1, NULL, 0, 0);
					r1.bottom--;
					DrawEdge(hdc, &r1, EDGE_RAISED, BF_SOFT|BF_DIAGONAL_ENDBOTTOMLEFT);

					/* Now erase the lefthand corner and draw diagonal edge */
					r1.left = r.left;
					r1.bottom = r.bottom;
					r1.right = r1.left + ROUND_CORNER_SIZE;
					r1.top = r1.bottom - ROUND_CORNER_SIZE - 1;
					ExtTextOutW(hdc, 0, 0, 2, &r1, NULL, 0, 0);
					r1.bottom--;
					DrawEdge(hdc, &r1, EDGE_RAISED, BF_SOFT|BF_DIAGONAL_ENDTOPLEFT);

					if (iItem == infoPtr->iSelected)
					{
						r.top += 2;
						r.left += 1;
						if (selectedRect.left == 0)
						{
							r1 = r;
							r1.bottom = r1.top;
							r1.top--;
							DrawEdge(hdc, &r1, EDGE_RAISED, BF_SOFT|BF_LEFT);
						}
					}

				}
				else
				{
					/* Adjust both rectangles for bottommost row */
					if (TAB_GetItem(infoPtr, iItem)->rect.top == infoPtr->uNumRows-1)
					{
						fillRect.bottom += 3;
						r.bottom += 2;
					}

					TRACE("<top> item=%d, fill=(%s), edge=(%s)\n",
						iItem, wine_dbgstr_rect(&fillRect), wine_dbgstr_rect(&r));

					/* Clear interior */
					SetBkColor(hdc, bkgnd);
					ExtTextOutW(hdc, 0, 0, 2, &fillRect, NULL, 0, 0);

					/* Draw rectangular edge around tab */
					DrawEdge(hdc, &r, EDGE_RAISED, BF_SOFT|BF_LEFT|BF_TOP|BF_RIGHT);

					/* Now erase the righthand corner and draw diagonal edge */
					SetBkColor(hdc, corner);
					r1.left = r.right - ROUND_CORNER_SIZE;
					r1.top = r.top;
					r1.right = r.right;
					r1.bottom = r1.top + ROUND_CORNER_SIZE + 1;
					ExtTextOutW(hdc, 0, 0, 2, &r1, NULL, 0, 0);
					r1.top++;
					DrawEdge(hdc, &r1, EDGE_RAISED, BF_SOFT|BF_DIAGONAL_ENDBOTTOMRIGHT);

					/* Now erase the lefthand corner and draw diagonal edge */
					r1.left = r.left;
					r1.top = r.top;
					r1.right = r1.left + ROUND_CORNER_SIZE;
					r1.bottom = r1.top + ROUND_CORNER_SIZE + 1;
					ExtTextOutW(hdc, 0, 0, 2, &r1, NULL, 0, 0);
					r1.top++;
					DrawEdge(hdc, &r1, EDGE_RAISED, BF_SOFT|BF_DIAGONAL_ENDTOPRIGHT);
				}
			}
		}

		TAB_DumpItemInternal(infoPtr, iItem);

		/* This modifies r to be the text rectangle. */
		TAB_DrawItemInterior(infoPtr, hdc, iItem, &r);
	}
}

/******************************************************************************
* TAB_DrawBorder
*
* This method is used to draw the raised border around the tab control
* "content" area.
*/
static void TAB_DrawBorder(const TAB_INFO *infoPtr, HDC hdc)
{
	RECT rect;
	HTHEME theme = infoPtr->htheme;

	GetClientRect (infoPtr->hwnd, &rect);

	/*
	* Adjust for the style
	*/

	if (infoPtr->uNumItem)
	{
		int rows = infoPtr->uNumRows;

		if (infoPtr->tabMaxRows>0 && infoPtr->tabMaxRows<rows)
		{
			rows = infoPtr->tabMaxRows;
		}

		if (infoPtr->dwStyle & TCS_VERTICAL)
		{
			rect.top    += rect.bottom;
		}
		else if ((infoPtr->dwStyle & TCS_BOTTOM))
			rect.bottom -= infoPtr->tabHeight * rows + CONTROL_BORDER_SIZEX;
		else /* not TCS_VERTICAL and not TCS_BOTTOM */
			rect.top    += infoPtr->tabHeight * rows + CONTROL_BORDER_SIZEX;
	}

	TRACE("border=(%s)\n", wine_dbgstr_rect(&rect));

	if (theme)
		DrawThemeBackground (theme, hdc, TABP_PANE, 0, &rect, NULL);
	else
		DrawEdge(hdc, &rect, EDGE_RAISED, BF_SOFT|BF_RECT);
}

/******************************************************************************
* TAB_Refresh
*
* This method repaints the tab control..
*/
static void TAB_Refresh (const TAB_INFO *infoPtr, HDC hdc)
{
	HFONT hOldFont;
	INT i;

	if (!infoPtr->DoRedraw)
		return;

	// clear the background manually
	if (infoPtr->dwStyle&TCS_FLICKERFREE)
	{
		RECT clientRect;
		GetClientRect(infoPtr->hwnd, &clientRect);

		if (infoPtr->dwStyle & TCS_VERTICAL)
		{
			//clientRect.right = clientRect.left + infoPtr->vModeWidth + 10;
		}
		else
		{
			clientRect.bottom = clientRect.top + infoPtr->tabHeight*infoPtr->uNumRows + 8;
		}
		FillRect(hdc, &clientRect, GetSysColorBrush(COLOR_BTNFACE));
	}

	hOldFont = (HFONT)SelectObject (hdc, infoPtr->hFont);

	if (infoPtr->dwStyle & TCS_BUTTONS)
	{
		for (i = 0; i < infoPtr->uNumItem; i++)
			TAB_DrawItem (infoPtr, hdc, i);
	}
	else
	{
		/* Draw all the non selected item first */
		for (i = 0; i < infoPtr->uNumItem; i++)
		{
			if (i != infoPtr->iSelected)
				TAB_DrawItem (infoPtr, hdc, i);
		}

		/* Now, draw the border, draw it before the selected item
		* since the selected item overwrites part of the border. */
		TAB_DrawBorder (infoPtr, hdc);

		/* Then, draw the selected item */
		TAB_DrawItem (infoPtr, hdc, infoPtr->iSelected);
	}

	SelectObject (hdc, hOldFont);
}

static inline DWORD _GetRowCount (const TAB_INFO *infoPtr)
{
	TRACE("(%p)\n", infoPtr);
	return infoPtr->uNumRows;
}

static inline LRESULT _SetRedraw (TAB_INFO *infoPtr, BOOL doRedraw)
{
	infoPtr->DoRedraw = doRedraw;
	return 0;
}

/******************************************************************************
* TAB_EnsureSelectionVisible
*
* This method will make sure that the current selection is completely
* visible by scrolling until it is.
*/
static void TAB_EnsureSelectionVisible(TAB_INFO* infoPtr)
{
	INT iSelected = infoPtr->iSelected;

	if (iSelected < 0)
		return;

	/* set the items row to the bottommost row or topmost row depending on
	* style */ // 不需要
	if (false && (infoPtr->uNumRows > 1) && !(infoPtr->dwStyle & TCS_BUTTONS))
	{
		TAB_ITEM *selected = TAB_GetItem(infoPtr, iSelected);
		INT newselected;
		INT iTargetRow;

		newselected = selected->rect.top;

		/* the target row is always (number of rows - 1)
		as row 0 is furthest from the clientRect */
		iTargetRow = infoPtr->uNumRows - 1;

		if (newselected != iTargetRow)
		{
			UINT i;
			{
				for (i=0; i < infoPtr->uNumItem; i++)
				{
					TAB_ITEM *item = TAB_GetItem(infoPtr, i);

					if (item->rect.top == newselected )
						item->rect.top = iTargetRow;
					else
					{
						if (item->rect.top > newselected)
							item->rect.top-=1;
					}
				}
			}
			TAB_RecalcHotTrack(infoPtr, NULL, NULL, NULL);
		}
	}

	/*
	* Do the trivial cases first.
	*/
	if ( !infoPtr->needsScrolling && !infoPtr->multilineScrolling || infoPtr->hwndUpDown==0)
		return;

	INT * acroVisible;

	bool b1 = infoPtr->dwStyle&TCS_VERTICAL||infoPtr->dwStyle&TCS_MULTILINE;

	if (b1)
	{
		acroVisible = &infoPtr->topmostVisible;
	}
	else
	{
		acroVisible = &infoPtr->leftmostVisible;
	}

	INT iOrigAcroVisible = *acroVisible;

	if (iOrigAcroVisible >= iSelected
		&&(infoPtr->dwStyle&TCS_VERTICAL||!b1))
	{
		*acroVisible = iSelected;
	}
	else
	{
		TAB_ITEM *selected = TAB_GetItem(infoPtr, iSelected);
		RECT r;
		UINT i;
		/* Calculate the part of the client area that is visible */
		GetClientRect(infoPtr->hwnd, &r);
		INT widthHeight = r.right;
		if (!widthHeight) return;
		if (b1 && (infoPtr->dwStyle&TCS_VERTICAL)==0
			&&iOrigAcroVisible >= (iSelected=selected->rect.top))
		{
			*acroVisible = r.top;
		}
		else if (b1)
		{
			widthHeight = r.bottom;
			//GetClientRect(infoPtr->hwndUpDown, &r);
			//widthHeight -= r.right;
			INT minRowTop = iSelected - infoPtr->tabMaxRows + 1;
			if (minRowTop<0)
			{
				minRowTop = 0;
			}
			if (*acroVisible<minRowTop)
			{
				*acroVisible = minRowTop;
			}
			//LogIs(3, "TAB_EnsureSelectionVisible cap=%d sel=%d minRowTop=%d acro=%d orig=%d"
			//	, widthHeight/infoPtr->tabHeight, iSelected, minRowTop, *acroVisible, iOrigAcroVisible);
		}
		else
		{
			GetClientRect(infoPtr->hwndUpDown, &r);
			widthHeight -= r.right;

			if (selected->rect.right - selected->rect.left >= widthHeight )
			{
				/* Special case: height of selected item is greater than visible
				* part of control.
				*/
				*acroVisible = iSelected;
			}
			else
			{
				for (i = *acroVisible; i < infoPtr->uNumItem; i++)
				{
					if ((selected->rect.right - TAB_GetItem(infoPtr, i)->rect.left) < widthHeight)
						break;
				}
				*acroVisible = i;
			}
		}
	}

	if (*acroVisible != iOrigAcroVisible)
		TAB_RecalcHotTrack(infoPtr, NULL, NULL, NULL);

	SendMessageW(infoPtr->hwndUpDown, UDM_SETPOS, 0
		, MAKELONG(*acroVisible, 0));

}

/******************************************************************************
* TAB_InvalidateTabArea
*
* This method will invalidate the portion of the control that contains the
* tabs. It is called when the state of the control changes and needs
* to be redisplayed
*/
static void TAB_InvalidateTabArea(const TAB_INFO *infoPtr)
{
	RECT clientRect, rInvalidate, rAdjClient;
	INT lastRow = infoPtr->uNumRows - 1;
	RECT rect;

	if (lastRow < 0) return;

	GetClientRect(infoPtr->hwnd, &clientRect);
	rInvalidate = clientRect;
	rAdjClient = clientRect;

	_AdjustRect(infoPtr, 0, &rAdjClient);

	TAB_InternalGetItemRect(infoPtr, infoPtr->uNumItem-1 , &rect, NULL);

	if (infoPtr->dwStyle & TCS_VERTICAL)
	{
		rInvalidate.right = rInvalidate.left + infoPtr->vModeWidth + 2;
	}
	else if (infoPtr->dwStyle & TCS_BOTTOM)
	{
		rInvalidate.top = rAdjClient.bottom;
		if (infoPtr->uNumRows == 1)
			rInvalidate.right = clientRect.left + rect.right + 2 * SELECTED_TAB_OFFSET;
	}
	else 
	{
		rInvalidate.bottom = rAdjClient.top;
		if (infoPtr->uNumRows == 1)
			rInvalidate.right = clientRect.left + rect.right + 2 * SELECTED_TAB_OFFSET;
	}

	/* Punch out the updown control */
	if (infoPtr->needsScrolling && (rInvalidate.right > 0)) {
		RECT r;
		GetClientRect(infoPtr->hwndUpDown, &r);
		if (rInvalidate.right > clientRect.right - r.left)
			rInvalidate.right = rInvalidate.right - (r.right - r.left);
		else
			rInvalidate.right = clientRect.right - r.left;
	}

	TRACE("invalidate (%s)\n", wine_dbgstr_rect(&rInvalidate));

	InvalidateRect(infoPtr->hwnd, &rInvalidate, (infoPtr->dwStyle&TCS_FLICKERFREE)==0);
}

HDC         hdcMem;
HBITMAP     hbmMem;
HANDLE      hOld;
PAINTSTRUCT ps;
HDC         _hdc;
int win_width, win_height;

static inline LRESULT _Paint (TAB_INFO *infoPtr, HDC hdcPaint)
{
	HDC hdc;
	PAINTSTRUCT ps;

	if (hdcPaint)
		hdc = hdcPaint;
	else
	{
		hdc = BeginPaint (infoPtr->hwnd, &ps);
		TRACE("erase %d, rect=(%s)\n", ps.fErase, wine_dbgstr_rect(&ps.rcPaint));
	}

	if (infoPtr->dwStyle&TCS_FLICKERFREE)
	{
		RECT rect;
		GetClientRect(infoPtr->hwnd, &rect);
		win_width=rect.right;
		win_height=rect.bottom;
		// Create an off-screen DC for double-buffering
		hdcMem = CreateCompatibleDC(hdc);
		hbmMem = CreateCompatibleBitmap(hdc, win_width, win_height);
		hOld = SelectObject(hdcMem, hbmMem);
		_hdc = hdc;
		hdc = hdcMem;
	}

	TAB_Refresh (infoPtr, hdc);

	if (hdcMem)
	{	
		BitBlt(_hdc, 0, 0, win_width, win_height, hdcMem, 0, 0, SRCCOPY);
		SelectObject(hdcMem, hOld);
		DeleteObject(hbmMem);
		DeleteDC (hdcMem);
		hdcMem = NULL;
	}

	if (!hdcPaint)
		EndPaint (infoPtr->hwnd, &ps);

	return 0;
}

static LRESULT _InsertItemT (TAB_INFO *infoPtr, INT iItem, const TCITEMW *pti, BOOL bUnicode)
{
	TAB_ITEM *item;
	RECT rect;

	GetClientRect (infoPtr->hwnd, &rect);
	TRACE("Rect: %p %s\n", infoPtr->hwnd, wine_dbgstr_rect(&rect));

	if (iItem < 0) return -1;
	if (iItem > infoPtr->uNumItem)
		iItem = infoPtr->uNumItem;

	TAB_DumpItemExternalT(pti, iItem, bUnicode);

	if (!(item = (TAB_ITEM*)Alloc(TAB_ITEM_SIZE(infoPtr)))) return FALSE;
	if (DPA_InsertPtr(infoPtr->items, iItem, item) == -1)
	{
		Free(item);
		return FALSE;
	}

	if (infoPtr->uNumItem == 0)
		infoPtr->iSelected = 0;
	else if (iItem <= infoPtr->iSelected)
		infoPtr->iSelected++;

	infoPtr->uNumItem++;

	item->pszText = NULL;
	if (pti->mask & TCIF_TEXT)
	{
		if (bUnicode)
			Str_SetPtrW (&item->pszText, pti->pszText);
		else 
		{ } //fixme
	}

	if (pti->mask & TCIF_IMAGE)
		item->iImage = pti->iImage;
	else
		item->iImage = -1;

	if (pti->mask & TCIF_PARAM)
		memcpy(item->extra, &pti->lParam, EXTRA_ITEM_SIZE(infoPtr));
	else
		memset(item->extra, 0, EXTRA_ITEM_SIZE(infoPtr));

	TAB_SetItemBounds(infoPtr);
	if (infoPtr->uNumItem > 1)
		TAB_InvalidateTabArea(infoPtr);
	else
		InvalidateRect(infoPtr->hwnd, NULL, TRUE);

	TRACE("[%p]: added item %d %s\n",
		infoPtr->hwnd, iItem, debugstr_w(item->pszText));

	/* If we haven't set the current focus yet, set it now. */
	if (infoPtr->uFocus == -1)
		_SetCurFocus(infoPtr, iItem);

	return iItem;
}

static LRESULT _SetItemSize (TAB_INFO *infoPtr, INT cx, INT cy)
{
	LONG lResult = 0;
	BOOL bNeedPaint = FALSE;

	lResult = MAKELONG(infoPtr->tabWidth, infoPtr->tabHeight);

	/* UNDOCUMENTED: If requested Width or Height is 0 this means that program wants to use auto size. */
	if (infoPtr->dwStyle & TCS_FIXEDWIDTH && (infoPtr->tabWidth != cx))
	{
		infoPtr->tabWidth = cx;
		bNeedPaint = TRUE;
	}

	//if (infoPtr->tabHeight != cy)
	{
		if ((infoPtr->fHeightSet = (cy != 0)))
			infoPtr->tabHeight = cy;

		bNeedPaint = TRUE;
	}
	TRACE("was h=%d,w=%d, now h=%d,w=%d\n",
		HIWORD(lResult), LOWORD(lResult),
		infoPtr->tabHeight, infoPtr->tabWidth);

	if (bNeedPaint)
	{
		TAB_SetItemBounds(infoPtr);
		RedrawWindow(infoPtr->hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
	}

	return lResult;
}

static inline LRESULT _SetMinTabWidth (TAB_INFO *infoPtr, INT cx)
{
	INT oldcx = 0;

	TRACE("(%p,%d)\n", infoPtr, cx);

	if (infoPtr->tabMinWidth < 0)
		oldcx = DEFAULT_MIN_TAB_WIDTH;
	else
		oldcx = infoPtr->tabMinWidth;
	infoPtr->tabMinWidth = cx;
	TAB_SetItemBounds(infoPtr);
	return oldcx;
}

static inline LRESULT _HighlightItem (TAB_INFO *infoPtr, INT iItem, BOOL fHighlight)
{
	LPDWORD lpState;
	DWORD oldState;
	RECT r;

	TRACE("(%p,%d,%s)\n", infoPtr, iItem, fHighlight ? "true" : "false");

	if (iItem < 0 || iItem >= infoPtr->uNumItem)
		return FALSE;

	lpState = &TAB_GetItem(infoPtr, iItem)->dwState;
	oldState = *lpState;

	if (fHighlight)
		*lpState |= TCIS_HIGHLIGHTED;
	else
		*lpState &= ~TCIS_HIGHLIGHTED;

	if ((oldState != *lpState) && TAB_InternalGetItemRect (infoPtr, iItem, &r, NULL))
		InvalidateRect (infoPtr->hwnd, &r, TRUE);

	return TRUE;
}

static LRESULT _SetItemT (TAB_INFO *infoPtr, INT iItem, LPTCITEMW tabItem, BOOL bUnicode)
{
	TAB_ITEM *wineItem;

	TRACE("(%p,%d,%p,%s)\n", infoPtr, iItem, tabItem, bUnicode ? "true" : "false");

	if (iItem < 0 || iItem >= infoPtr->uNumItem)
		return FALSE;

	TAB_DumpItemExternalT(tabItem, iItem, bUnicode);

	wineItem = TAB_GetItem(infoPtr, iItem);

	if (tabItem->mask & TCIF_IMAGE)
		wineItem->iImage = tabItem->iImage;

	if (tabItem->mask & TCIF_PARAM)
		memcpy(wineItem->extra, &tabItem->lParam, infoPtr->cbInfo);

	if (tabItem->mask & TCIF_RTLREADING)
		FIXME("TCIF_RTLREADING\n");

	if (tabItem->mask & TCIF_STATE)
		wineItem->dwState = (wineItem->dwState & ~tabItem->dwStateMask) |
		( tabItem->dwState &  tabItem->dwStateMask);

	if (tabItem->mask & TCIF_TEXT)
	{
		Free(wineItem->pszText);
		wineItem->pszText = NULL;
		if (bUnicode)
			Str_SetPtrW(&wineItem->pszText, tabItem->pszText);
		else
		{}//fixme
	}

	/* Update and repaint tabs */
	TAB_SetItemBounds(infoPtr);
	TAB_InvalidateTabArea(infoPtr);

	return TRUE;
}

static inline LRESULT _GetItemCount (const TAB_INFO *infoPtr)
{
	TRACE("\n");
	return infoPtr->uNumItem;
}


static LRESULT _GetItemT (TAB_INFO *infoPtr, INT iItem, LPTCITEMW tabItem, BOOL bUnicode)
{
	TAB_ITEM *wineItem;

	TRACE("(%p,%d,%p,%s)\n", infoPtr, iItem, tabItem, bUnicode ? "true" : "false");

	if (!tabItem) return FALSE;

	if (iItem < 0 || iItem >= infoPtr->uNumItem)
	{
		/* init requested fields */
		if (tabItem->mask & TCIF_IMAGE) tabItem->iImage  = 0;
		if (tabItem->mask & TCIF_PARAM) tabItem->lParam  = 0;
		if (tabItem->mask & TCIF_STATE) tabItem->dwState = 0;
		return FALSE;
	}

	wineItem = TAB_GetItem(infoPtr, iItem);

	if (tabItem->mask & TCIF_IMAGE)
		tabItem->iImage = wineItem->iImage;

	if (tabItem->mask & TCIF_PARAM)
		memcpy(&tabItem->lParam, wineItem->extra, infoPtr->cbInfo);

	if (tabItem->mask & TCIF_RTLREADING)
		FIXME("TCIF_RTLREADING\n");

	if (tabItem->mask & TCIF_STATE)
		tabItem->dwState = wineItem->dwState & tabItem->dwStateMask;

	if (tabItem->mask & TCIF_TEXT)
	{
		if (bUnicode)
			StringCchCopy(tabItem->pszText, tabItem->cchTextMax, wineItem->pszText);
		else
		{}//fixme
	}

	TAB_DumpItemExternalT(tabItem, iItem, bUnicode);

	return TRUE;
}


static LRESULT _DeleteItem (TAB_INFO *infoPtr, INT iItem)
{
	TAB_ITEM *item;

	TRACE("(%p, %d)\n", infoPtr, iItem);

	if (iItem < 0 || iItem >= infoPtr->uNumItem) return FALSE;

	TAB_InvalidateTabArea(infoPtr);
	item = TAB_GetItem(infoPtr, iItem);
	Free(item->pszText);
	Free(item);
	infoPtr->uNumItem--;
	DPA_DeletePtr(infoPtr->items, iItem);

	if (infoPtr->uNumItem == 0)
	{
		if (infoPtr->iHotTracked >= 0)
		{
			KillTimer(infoPtr->hwnd, TAB_HOTTRACK_TIMER);
			infoPtr->iHotTracked = -1;
		}

		infoPtr->iSelected = -1;
	}
	else
	{
		if (iItem <= infoPtr->iHotTracked)
		{
			/* When tabs move left/up, the hot track item may change */
			FIXME("Recalc hot track\n");
		}
	}

	/* adjust the selected index */
	if (iItem == infoPtr->iSelected)
		infoPtr->iSelected = -1;
	else if (iItem < infoPtr->iSelected)
		infoPtr->iSelected--;

	/* reposition and repaint tabs */
	TAB_SetItemBounds(infoPtr);

	return TRUE;
}

static inline LRESULT _DeleteAllItems (TAB_INFO *infoPtr)
{
	TRACE("(%p)\n", infoPtr);
	while (infoPtr->uNumItem)
		_DeleteItem (infoPtr, 0);
	return TRUE;
}


static inline LRESULT _GetFont (const TAB_INFO *infoPtr)
{
	TRACE("(%p) returning %p\n", infoPtr, infoPtr->hFont);
	return (LRESULT)infoPtr->hFont;
}

static inline LRESULT _SetFont (TAB_INFO *infoPtr, HFONT hNewFont)
{
	TRACE("(%p,%p)\n", infoPtr, hNewFont);

	infoPtr->hFont = hNewFont;

	TAB_SetItemBounds(infoPtr);

	TAB_InvalidateTabArea(infoPtr);

	return 0;
}


static inline LRESULT _GetImageList (const TAB_INFO *infoPtr)
{
	TRACE("\n");
	return (LRESULT)infoPtr->himl;
}

static inline LRESULT _SetImageList (TAB_INFO *infoPtr, HIMAGELIST himlNew)
{
	HIMAGELIST himlPrev = infoPtr->himl;
	TRACE("himl=%p\n", himlNew);
	infoPtr->himl = himlNew;
	TAB_SetItemBounds(infoPtr);
	InvalidateRect(infoPtr->hwnd, NULL, TRUE);
	return (LRESULT)himlPrev;
}

static inline LRESULT _GetUnicodeFormat (const TAB_INFO *infoPtr)
{
	TRACE("(%p)\n", infoPtr);
	return infoPtr->bUnicode;
}

static inline LRESULT _SetUnicodeFormat (TAB_INFO *infoPtr, BOOL bUnicode)
{
	BOOL bTemp = infoPtr->bUnicode;

	TRACE("(%p %d)\n", infoPtr, bUnicode);
	infoPtr->bUnicode = bUnicode;

	return bTemp;
}

static inline LRESULT _Size (TAB_INFO *infoPtr)
{
	/* I'm not really sure what the following code was meant to do.
	This is what it is doing:
	When WM_SIZE is sent with SIZE_RESTORED, the control
	gets positioned in the top left corner.

	RECT parent_rect;
	HWND parent;
	UINT uPosFlags,cx,cy;

	uPosFlags=0;
	if (!wParam) {
	parent = GetParent (hwnd);
	GetClientRect(parent, &parent_rect);
	cx=LOWORD (lParam);
	cy=HIWORD (lParam);
	if (GetWindowLongW(hwnd, GWL_STYLE) & CCS_NORESIZE)
	uPosFlags |= (SWP_NOSIZE | SWP_NOMOVE);

	SetWindowPos (hwnd, 0, parent_rect.left, parent_rect.top,
	cx, cy, uPosFlags | SWP_NOZORDER);
	} else {
	FIXME("WM_SIZE flag %x %lx not handled\n", wParam, lParam);
	} */

	/* Recompute the size/position of the tabs. */
	TAB_SetItemBounds (infoPtr);

	/* Force a repaint of the control. */
	InvalidateRect(infoPtr->hwnd, NULL, TRUE);

	return 0;
}


static LRESULT _Create (HWND hwnd, LPARAM lParam)
{
	TAB_INFO *infoPtr;
	TEXTMETRICW fontMetrics;
	HDC hdc;
	HFONT hOldFont;
	DWORD style;

	ReadColors();

	infoPtr = (TAB_INFO*)Alloc (sizeof(TAB_INFO));

	SetWindowLongPtrW(hwnd, 0, (DWORD_PTR)infoPtr);

	infoPtr->hwnd            = hwnd;
	infoPtr->hwndNotify      = ((LPCREATESTRUCTW)lParam)->hwndParent;
	infoPtr->uNumItem        = 0;
	infoPtr->uNumRows        = 0;
	infoPtr->uHItemPadding   = 6;
	infoPtr->uVItemPadding   = 3;
	infoPtr->uHItemPadding_s = 6;
	infoPtr->uVItemPadding_s = 3;
	infoPtr->hFont           = 0;
	infoPtr->items           = DPA_Create(8);
	infoPtr->hcurArrow       = LoadCursorW (0, (LPWSTR)IDC_ARROW);
	infoPtr->iSelected       = -1;
	infoPtr->iHotTracked     = -1;
	infoPtr->uFocus          = -1;
	infoPtr->hwndToolTip     = 0;
	infoPtr->DoRedraw        = TRUE;
	infoPtr->needsScrolling  = FALSE;
	infoPtr->hwndUpDown      = 0;
	infoPtr->leftmostVisible = 0;
	infoPtr->topmostVisible = 0;
	infoPtr->multilineScrolling = FALSE;
	infoPtr->fHeightSet      = FALSE;
	infoPtr->bUnicode        = IsWindowUnicode (hwnd);
	infoPtr->cbInfo          = sizeof(LPARAM);

	infoPtr->himl = 0;

	TRACE("Created tab control, hwnd [%p]\n", hwnd);

	/* The tab control always has the WS_CLIPSIBLINGS style. Even
	if you don't specify it in CreateWindow. This is necessary in
	order for paint to work correctly. This follows windows behaviour. */
	style = GetWindowLongW(hwnd, GWL_STYLE);
	if (style & TCS_VERTICAL) style |= TCS_MULTILINE;
	style |= WS_CLIPSIBLINGS;
	SetWindowLongW(hwnd, GWL_STYLE, style);

	infoPtr->dwStyle = style;
	infoPtr->exStyle = (style & TCS_FLATBUTTONS) ? TCS_EX_FLATSEPARATORS : 0;

	if (infoPtr->dwStyle & TCS_TOOLTIPS) {
		/* Create tooltip control */
		infoPtr->hwndToolTip =
			CreateWindowExW (0, TOOLTIPS_CLASSW, NULL, WS_POPUP,
				CW_USEDEFAULT, CW_USEDEFAULT,
				CW_USEDEFAULT, CW_USEDEFAULT,
				hwnd, 0, 0, 0);

		/* Send NM_TOOLTIPSCREATED notification */
		if (infoPtr->hwndToolTip) {
			NMTOOLTIPSCREATED nmttc;

			nmttc.hdr.hwndFrom = hwnd;
			nmttc.hdr.idFrom = GetWindowLongPtrW(hwnd, GWLP_ID);
			nmttc.hdr.code = NM_TOOLTIPSCREATED;
			nmttc.hwndToolTips = infoPtr->hwndToolTip;

			SendMessageW (infoPtr->hwndNotify, WM_NOTIFY,
				GetWindowLongPtrW(hwnd, GWLP_ID), (LPARAM)&nmttc);


			SetWindowPos(infoPtr->hwndToolTip, HWND_TOPMOST, 0, 0, 0, 0,
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE
			);

			// Associate the tooltip with the tool.
			TOOLINFO toolInfo = { 0 };
			toolInfo.cbSize = sizeof(toolInfo);
			toolInfo.hwnd = infoPtr->hwndNotify;
			toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
			toolInfo.uId = (UINT_PTR)infoPtr->hwnd;
			toolInfo.lpszText = LPSTR_TEXTCALLBACK; // pszText;
			SendMessage(infoPtr->hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);

		}
	}

	OpenThemeData (infoPtr->hwnd, themeClass);

	/*
	* We need to get text information so we need a DC and we need to select
	* a font.
	*/
	hdc = GetDC(hwnd);
	hOldFont = (HFONT)SelectObject (hdc, GetStockObject (SYSTEM_FONT));

	/* Use the system font to determine the initial height of a tab. */
	GetTextMetricsW(hdc, &fontMetrics);

	/*
	* Make sure there is enough space for the letters + growing the
	* selected item + extra space for the selected item.
	*/
	infoPtr->tabHeight = fontMetrics.tmHeight + SELECTED_TAB_OFFSET +
		((infoPtr->dwStyle & TCS_BUTTONS) ? 2 : 1) *
		infoPtr->uVItemPadding;

	/* Initialize the width of a tab. */
	if (infoPtr->dwStyle & TCS_FIXEDWIDTH)
		infoPtr->tabWidth = GetDeviceCaps(hdc, LOGPIXELSX);

	infoPtr->tabMinWidth = -1;
	infoPtr->tabMaxRows = -1;

	infoPtr->vModeWidth = 180;
	infoPtr->vModeDragging = false;

	infoPtr->htheme = ::GetWindowTheme(infoPtr->hwnd);

	TRACE("tabH=%d, tabW=%d\n", infoPtr->tabHeight, infoPtr->tabWidth);

	SelectObject (hdc, hOldFont);
	ReleaseDC(hwnd, hdc);

	return 0;
}

static LRESULT _Destroy (TAB_INFO *infoPtr)
{
	INT iItem;

	SetWindowLongPtrW(infoPtr->hwnd, 0, 0);

	for (iItem = infoPtr->uNumItem - 1; iItem >= 0; iItem--)
	{
		TAB_ITEM *tab = TAB_GetItem(infoPtr, iItem);

		DPA_DeletePtr(infoPtr->items, iItem);
		infoPtr->uNumItem--;

		Free(tab->pszText);
		Free(tab);
	}
	DPA_Destroy(infoPtr->items);
	infoPtr->items = NULL;

	if (infoPtr->hwndToolTip)
		DestroyWindow (infoPtr->hwndToolTip);

	if (infoPtr->hwndUpDown)
		DestroyWindow(infoPtr->hwndUpDown);

	if (infoPtr->iHotTracked >= 0)
		KillTimer(infoPtr->hwnd, TAB_HOTTRACK_TIMER);

	CloseThemeData (infoPtr->htheme);

	Free (infoPtr);
	return 0;
}

/* update theme after a WM_THEMECHANGED message */
static LRESULT theme_changed(const TAB_INFO *infoPtr)
{
	HTHEME theme = infoPtr->htheme;
	CloseThemeData (theme);
	OpenThemeData (infoPtr->hwnd, themeClass);
	return 0;
}

static LRESULT TAB_NCCalcSize(WPARAM wParam)
{
	if (!wParam)
		return 0;
	return WVR_ALIGNTOP;
}

static inline LRESULT _SetItemExtra (TAB_INFO *infoPtr, INT cbInfo)
{
	TRACE("(%p %d)\n", infoPtr, cbInfo);

	if (cbInfo < 0 || infoPtr->uNumItem) return FALSE;

	infoPtr->cbInfo = cbInfo;
	return TRUE;
}

static LRESULT _RemoveImage (TAB_INFO *infoPtr, INT image)
{
	TRACE("%p %d\n", infoPtr, image);

	if (ImageList_Remove (infoPtr->himl, image))
	{
		INT i, *idx;
		RECT r;

		/* shift indices, repaint items if needed */
		for (i = 0; i < infoPtr->uNumItem; i++)
		{
			idx = &TAB_GetItem(infoPtr, i)->iImage;
			if (*idx >= image)
			{
				if (*idx == image)
					*idx = -1;
				else
					(*idx)--;

				/* repaint item */
				if (TAB_InternalGetItemRect (infoPtr, i, &r, NULL))
					InvalidateRect (infoPtr->hwnd, &r, TRUE);
			}
		}
	}

	return 0;
}

static LRESULT _SetExtendedStyle (TAB_INFO *infoPtr, DWORD exMask, DWORD exStyle)
{
	DWORD prevstyle = infoPtr->exStyle;

	/* zero mask means all styles */
	if (exMask == 0) exMask = ~0;

	if (exMask & TCS_EX_REGISTERDROP)
	{
		FIXME("TCS_EX_REGISTERDROP style unimplemented\n");
		exMask  &= ~TCS_EX_REGISTERDROP;
		exStyle &= ~TCS_EX_REGISTERDROP;
	}

	if (exMask & TCS_EX_FLATSEPARATORS)
	{
		if ((prevstyle ^ exStyle) & TCS_EX_FLATSEPARATORS)
		{
			infoPtr->exStyle ^= TCS_EX_FLATSEPARATORS;
			TAB_InvalidateTabArea(infoPtr);
		}
	}

	return prevstyle;
}

static inline LRESULT _GetExtendedStyle (const TAB_INFO *infoPtr)
{
	return infoPtr->exStyle;
}

static LRESULT _SetMaxRows (TAB_INFO *infoPtr, DWORD maxRows)
{
	infoPtr->tabMaxRowSet = maxRows;
	return 0;
}

static BOOL _GetIsVerticalResizeArea (TAB_INFO *infoPtr, int x, int y)
{
	if (x+8>infoPtr->vModeWidth)
	{
		return true;
	}
	if (y>infoPtr->tabHeight*infoPtr->uNumRows && x+15>infoPtr->vModeWidth)
	{
		return true;
	}
	return false;
}

static LRESULT _DeselectAll (TAB_INFO *infoPtr, BOOL excludesel)
{
	BOOL paint = FALSE;
	INT i, selected = infoPtr->iSelected;

	TRACE("(%p, %d)\n", infoPtr, excludesel);

	if (!(infoPtr->dwStyle & TCS_BUTTONS))
		return 0;

	for (i = 0; i < infoPtr->uNumItem; i++)
	{
		if ((TAB_GetItem(infoPtr, i)->dwState & TCIS_BUTTONPRESSED) &&
			(selected != i))
		{
			TAB_GetItem(infoPtr, i)->dwState &= ~TCIS_BUTTONPRESSED;
			paint = TRUE;
		}
	}

	if (!excludesel && (selected != -1))
	{
		TAB_GetItem(infoPtr, selected)->dwState &= ~TCIS_BUTTONPRESSED;
		infoPtr->iSelected = -1;
		paint = TRUE;
	}

	if (paint)
		TAB_InvalidateTabArea (infoPtr);

	return 0;
}

/***
* DESCRIPTION:
* Processes WM_STYLECHANGED messages.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the tab data structure
* [I] wStyleType : window style type (normal or extended)
* [I] lpss : window style information
*
* RETURN:
* Zero
*/
static INT _StyleChanged(TAB_INFO *infoPtr, WPARAM wStyleType, const STYLESTRUCT *lpss)
{
	TRACE("(styletype=%lx, styleOld=0x%08x, styleNew=0x%08x)\n", wStyleType, lpss->styleOld, lpss->styleNew);

	if (wStyleType != GWL_STYLE) return 0;

	infoPtr->dwStyle = lpss->styleNew;
	TAB_SetItemBounds (infoPtr);
	InvalidateRect(infoPtr->hwnd, NULL, TRUE);

	return 0;
}

static LRESULT WINAPI TAB_WindowProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TAB_INFO *infoPtr = TAB_GetInfoPtr(hwnd);

	TRACE("hwnd=%p msg=%x wParam=%lx lParam=%lx\n", hwnd, uMsg, wParam, lParam);

	if (!infoPtr && (uMsg != WM_CREATE))
		return DefWindowProcW (hwnd, uMsg, wParam, lParam);

	switch (uMsg)
	{
	case TCM_GETIMAGELIST: return _GetImageList (infoPtr);
	case TCM_SETIMAGELIST: return _SetImageList (infoPtr, (HIMAGELIST)lParam);
	case TCM_GETITEMCOUNT: return _GetItemCount (infoPtr);
	case TCM_GETITEMA:
	case TCM_GETITEMW: return _GetItemT (infoPtr, (INT)wParam, (LPTCITEMW)lParam, uMsg == TCM_GETITEMW);
	case TCM_SETITEMA:
	case TCM_SETITEMW: return _SetItemT (infoPtr, (INT)wParam, (LPTCITEMW)lParam, uMsg == TCM_SETITEMW);
	case TCM_DELETEITEM: return _DeleteItem (infoPtr, (INT)wParam);
	case TCM_DELETEALLITEMS: return _DeleteAllItems (infoPtr);
	case TCM_GETITEMRECT: return _GetItemRect (infoPtr, (INT)wParam, (LPRECT)lParam);
	case TCM_GETCURSEL: return _GetCurSel (infoPtr);
	case TCM_HITTEST: return _HitTest (infoPtr, (LPTCHITTESTINFO)lParam);
	case TCM_SETCURSEL: return _SetCurSel (infoPtr, (INT)wParam);
	case TCM_INSERTITEMA:
	case TCM_INSERTITEMW: return _InsertItemT (infoPtr, (INT)wParam, (TCITEMW*)lParam, uMsg == TCM_INSERTITEMW);

	case TCM_SETITEMEXTRA: return _SetItemExtra (infoPtr, (INT)wParam);
	case TCM_ADJUSTRECT: return _AdjustRect (infoPtr, (BOOL)wParam, (LPRECT)lParam);
	case TCM_SETITEMSIZE: return _SetItemSize (infoPtr, (INT)LOWORD(lParam), (INT)HIWORD(lParam));
	case TCM_REMOVEIMAGE: return _RemoveImage (infoPtr, (INT)wParam);
	case TCM_SETPADDING: return _SetPadding (infoPtr, lParam);
	case TCM_GETROWCOUNT: return _GetRowCount(infoPtr);
	case TCM_GETUNICODEFORMAT: return _GetUnicodeFormat (infoPtr);
	case TCM_SETUNICODEFORMAT: return _SetUnicodeFormat (infoPtr, (BOOL)wParam);
	case TCM_HIGHLIGHTITEM: return _HighlightItem (infoPtr, (INT)wParam, (BOOL)LOWORD(lParam));
	case TCM_GETTOOLTIPS: return _GetToolTips (infoPtr);
	case TCM_SETTOOLTIPS: return _SetToolTips (infoPtr, (HWND)wParam);
	case TCM_GETCURFOCUS: return _GetCurFocus (infoPtr);
	case TCM_SETCURFOCUS: return _SetCurFocus (infoPtr, (INT)wParam);
	case TCM_SETMINTABWIDTH: return _SetMinTabWidth(infoPtr, (INT)lParam);
	case TCM_DESELECTALL: return _DeselectAll (infoPtr, (BOOL)wParam);
	case TCM_GETEXTENDEDSTYLE: return _GetExtendedStyle (infoPtr);
	case TCM_SETEXTENDEDSTYLE: return _SetExtendedStyle (infoPtr, wParam, lParam);
	case TCM_SETMAXROWS: return _SetMaxRows (infoPtr, (INT)wParam);
	
	case TCM_GETISVERTICALRESIZEAREA: return _GetIsVerticalResizeArea(infoPtr, wParam, lParam);
	case TCM_TRACKVERTICALTABS: return _TrackVerticalTabs(infoPtr, wParam, lParam);
	case TCM_DISMISSTOOLTIPS: return _DismissToolTips(infoPtr);
	case TCM_GETVERTICALMODEWIDTH: return _GetVerticalModeWidth(infoPtr);
	case TCM_SETVERTICALMODEWIDTH: return _SetVerticalModeWidth(infoPtr, wParam);
	case TCM_GETTOPMOSTROW: return _GetTopMostRow(infoPtr);

	case WM_GETFONT: return _GetFont (infoPtr);
	case WM_SETFONT: return _SetFont (infoPtr, (HFONT)wParam);
	case WM_CREATE: return _Create (hwnd, lParam);
	case WM_NCDESTROY: return _Destroy (infoPtr);
	case WM_GETDLGCODE: return DLGC_WANTARROWS | DLGC_WANTCHARS;
	case WM_LBUTTONDOWN: return _LButtonDown (infoPtr, wParam, lParam);
	case WM_LBUTTONUP: return _LButtonUp (infoPtr);
	case WM_NOTIFY:
		return SendMessageW(infoPtr->hwndNotify, WM_NOTIFY, wParam, lParam);
	case WM_RBUTTONUP:        _RButtonUp (infoPtr);
		return DefWindowProcW (hwnd, uMsg, wParam, lParam);
	case WM_MOUSEMOVE: return _MouseMove (infoPtr, wParam, lParam);
	case WM_PRINTCLIENT:
	case WM_PAINT: return _Paint (infoPtr, (HDC)wParam);
	case WM_ERASEBKGND: 
		if(infoPtr->dwStyle&TCS_FLICKERFREE) 
			return 1;
		else
			break;
	case WM_SIZE: return _Size (infoPtr);
	case WM_SETREDRAW: return _SetRedraw (infoPtr, (BOOL)wParam);

	case WM_HSCROLL: return _OnHScroll(infoPtr, (int)LOWORD(wParam), (int)HIWORD(wParam));
	case WM_VSCROLL: return _OnVScroll(infoPtr, (int)LOWORD(wParam), (int)HIWORD(wParam));

	case WM_STYLECHANGED: return _StyleChanged(infoPtr, wParam, (LPSTYLESTRUCT)lParam);
	case WM_SYSCOLORCHANGE: /* COMCTL32_RefreshSysColors(); //fixme */ return 0;
	case WM_THEMECHANGED: return theme_changed (infoPtr);

	case WM_KILLFOCUS: _KillFocus(infoPtr);
	case WM_SETFOCUS:  _FocusChanging(infoPtr); break;   /* Don't disturb normal focus behavior */
	case WM_KEYDOWN: return _KeyDown(infoPtr, wParam, lParam);
	case WM_NCHITTEST: return _NCHitTest(infoPtr, lParam);
	case WM_NCCALCSIZE: return TAB_NCCalcSize(wParam);

	default:
		if (uMsg >= WM_USER && uMsg < WM_APP && !COMCTL32_IsReflectedMessage(uMsg))
			WARN("unknown msg %04x wp=%08lx lp=%08lx\n", uMsg, wParam, lParam);
		break;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}


#define WC_TABCONTROLM          L"MyTabControl32"

void TAB_Register()
{
	WNDCLASSW wndClass;

	ZeroMemory (&wndClass, sizeof(WNDCLASSW));
	wndClass.style         = CS_GLOBALCLASS | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc   = TAB_WindowProc;
	wndClass.cbClsExtra    = 0;
	wndClass.cbWndExtra    = sizeof(TAB_INFO *);
	wndClass.hCursor       = LoadCursorW (0, (LPWSTR)IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
	wndClass.lpszClassName = WC_TABCONTROLM;

	RegisterClassW (&wndClass);
}


void TAB_Unregister ()
{
	UnregisterClassW (WC_TABCONTROLM, NULL);
}
