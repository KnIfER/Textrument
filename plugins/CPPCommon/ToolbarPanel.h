/*
This file is part of MultiClipboard Plugin for Notepad++
Copyright (C) 2009 LoonyChewy

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef TOOLBAR_PANEL_H
#define TOOLBAR_PANEL_H

#ifndef UNITY_BUILD_SINGLE_INCLUDE
#include <Control.h>
#include "Window.h"
#include "ToolBar.h"
#endif

class ToolbarPanel : public Window
{
public:
	ToolbarPanel();

	virtual void init(HINSTANCE hInst, HWND parent);
	virtual void destroy() {}

	void create(HWND itemHandle);

	// Warning: Not tested with changing toolbar in runtime!
	void SetToolbar( ToolBar * pNewToolBar );
	void SetChildWindow( Window * pNewChildWin );
	Window * GetChildWindow();

protected:
	ToolBar * pToolbar;
	ReBar Rebar;
	Window * pChildWin;
	HPEN hChildWindowPen;
	// For passing through drag list box messages
	UINT DragListMessage;
	WNDPROC			_oldproc;

	void GetPanelRect( RECT &PanelRect );
	void GetToolbarRect( RECT &ToolbarRect );
	void GetChildWinRect( RECT &ChildWinRect );
	void ResizeChildren();

	static LRESULT CALLBACK CtrlProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam){
		return ((ToolbarPanel *)(::GetWindowLongPtr(hwnd, GWLP_USERDATA)))->ToolbarPanelProc(hwnd, Message, wParam, lParam);
	};

	virtual LRESULT CALLBACK ToolbarPanelProc( HWND hwnd ,UINT Message, WPARAM wParam, LPARAM lParam );

	static LRESULT CALLBACK StaticToolbarPanelProc( HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam );
};

#endif //TOOLBAR_PANEL_H
