// this file is part of Notepad++
// Copyright (C)2005 Jens Lorenz <jens.plugin.npp@gmx.de>
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
// 
// // Note that the GPL places important restrictions on "derived works", yet
// it does not provide a detailed definition of that term.  To avoid      
// misunderstandings, we consider an application to constitute a          
// "derivative work" for the purpose of this license if it does any of the
// following:                                                             
// 1. Integrates source code from Notepad++.
// 2. Integrates/includes/aggregates Notepad++ into a proprietary executable
//    installer, such as those produced by InstallShield.
// 3. Links to a library or executes a program that does any of the above.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


#pragma once

#include <windows.h>

// ATTENTION : It's a part of interface header, so don't include the others header here

// styles for containers
#define	CAPTION_TOP				TRUE
#define	CAPTION_BOTTOM			FALSE

//   defines for docking manager
#define	APP_LAYOUT_WIDTH		 -1
#define	APP_LAYOUT_HEIGHT		 -2
#define	APP_LAYOUT_RNG_LEFT		 0
#define	APP_LAYOUT_RNG_RIGHT	 1
#define	APP_LAYOUT_RNG_TOP		 2
#define	APP_LAYOUT_RNG_BOTTOM	 3
#define	APP_LAYOUT_RNG_LEFT_SUB		 4
#define	APP_LAYOUT_RNG_RIGHT_SUB	 5
#define	APP_LAYOUT_RNG_TOP_SUB		 6
#define	APP_LAYOUT_RNG_BOTTOM_SUB	 7
#define	APP_LAYOUT_RNG_MAX	     8

// mask params for plugins of internal dialogs
#define DWS_ICONTAB			0x00000001			// Icon for tabs are available
#define DWS_ICONBAR			0x00000002			// Icon for icon bar are available (currently not supported)
#define DWS_ADDINFO			0x00000004			// Additional information are in use
#define DWS_PARAMSALL		(DWS_ICONTAB|DWS_ICONBAR|DWS_ADDINFO)

// default docking values for first call of plugin
#define DWS_DF_CONT_LEFT	(APP_LAYOUT_RNG_LEFT	<< 28)	// default docking on left
#define DWS_DF_CONT_RIGHT	(APP_LAYOUT_RNG_RIGHT	<< 28)	// default docking on right
#define DWS_DF_CONT_TOP		(APP_LAYOUT_RNG_TOP	<< 28)	// default docking on top
#define DWS_DF_CONT_BOTTOM	(APP_LAYOUT_RNG_BOTTOM << 28)	// default docking on bottom
#define DWS_DF_FLOATING		0x80000000			// default state is floating


typedef struct {
	HWND		hClient;		// client Window Handle
	const TCHAR	*pszName;		// name of plugin (shown in window)
	int			dlgID;			// a funcItem provides the function pointer to start a dialog. Please parse here these ID

	// user modifications
	UINT		uMask;			// mask params: look to above defines
	HICON		hIconTab;		// icon for tabs
	const TCHAR *pszAddInfo;		// for plugin to display additional informations

	// internal data, do not use !!!
	RECT		rcFloat;		// floating position
	int			iPrevCont;		// stores the privious container (toggling between float and dock)
	const TCHAR*	pszModuleName;	// it's the plugin file name. It's used to identify the plugin
	int magicNum;
} tTbData;


typedef struct {
	HWND		hWnd;							// the docking manager wnd
	RECT		rcRegion[APP_LAYOUT_RNG_MAX];			// position of docked dialogs
} tDockMgr;


#define	HIT_TEST_THICKNESS		20
#define SPLITTER_WIDTH			4

