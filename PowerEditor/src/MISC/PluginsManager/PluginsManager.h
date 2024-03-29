// This file is part of Notepad++ project
// Copyright (C)2020 Don HO <don.h@free.fr>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// Note that the GPL places important restrictions on "derived works", yet
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

#include "resource.h"
#include "Parameters.h"
#include "PluginInterface.h"
#include "IDAllocator.h"
#include <map>

typedef BOOL (__cdecl * PFUNCISUNICODE)();

struct PluginCommand
{
	generic_string _pluginName;
	int _funcID;
	PFUNCPLUGINCMD _pFunc;
	PluginCommand(const TCHAR *pluginName, int funcID, PFUNCPLUGINCMD pFunc): _funcID(funcID), _pFunc(pFunc), _pluginName(pluginName){};
};

__declspec(selectany) int _regstrDynCmdMax=0;


struct LoadedDllInfo
{
	generic_string _fullFilePath;
	long _loadingTm;
};

__declspec(selectany) LoadedDllInfo dummyLoadInfo{};

struct PluginInfo
{
	PluginInfo() = default;
	~PluginInfo()
	{
		if (_pluginMenu) {
			::DestroyMenu(_pluginMenu);
			_pluginMenu = 0;
		}

		if (_hLib) {
			::FreeLibrary(_hLib);
			_hLib = 0;
		}
	}

	HINSTANCE _hLib = nullptr;
	HMENU _pluginMenu = nullptr;

	PFUNCSETINFO _pFuncSetInfo = nullptr;
	PFUNCGETNAME _pFuncGetName = nullptr;
	PBENOTIFIED	_pBeNotified = nullptr;
	PFUNCGETFUNCSARRAY _pFuncGetFuncsArray = nullptr;
	PMESSAGEPROC _pMessageProc = nullptr;
	PFUNCISUNICODE _pFuncIsUnicode = nullptr;

	FuncItem *_funcItems = nullptr;
	int _nbFuncItem = 0;
	generic_string _moduleName;
	generic_string _funcName;
	int _toolbarICStart=-1; // count from 0 from dynmaic icons
	int _toolbarICCount=0;
	int _regstrDynCmdSt=0;
	LoadedDllInfo* loadInfo = &dummyLoadInfo;
};

class PluginsManager
{
friend class PluginsAdminDlg;
public:
	PluginsManager() : _dynamicIDAlloc(ID_PLUGINS_CMD_DYNAMIC, ID_PLUGINS_CMD_DYNAMIC_LIMIT),
					   _markerAlloc(MARKER_PLUGINS, MARKER_PLUGINS_LIMIT)	{}
	~PluginsManager()
	{
		for (size_t i = 0, len = _pluginInfos.size(); i < len; ++i)
			delete _pluginInfos[i];

		if (_hPluginsMenu)
			DestroyMenu(_hPluginsMenu);
	}

	void init(const NppData & nppData)
	{
		_nppData = nppData;
	}

    int loadPlugin(const TCHAR *pluginFilePath);
	bool loadPluginsV2(const TCHAR *dir = NULL);

    bool unloadPlugin(int index, HWND nppHandle);

	void runPluginCommand(size_t i);
	void runPluginCommand(const TCHAR *pluginName, int commandID);

    void installPluginMenuAt(int i);
	HMENU setMenu(HMENU hMenu, const TCHAR *menuName, bool enablePluginAdmin = false);
	bool getShortcutByCmdID(int cmdID, ShortcutKey *sk);
	bool removeShortcutByCmdID(int cmdID);

	void notify(size_t indexPluginInfo, const SCNotification *notification); // to a plugin
	void notify(const SCNotification *notification); // broadcast
	void relayNppMessages(UINT Message, WPARAM wParam, LPARAM lParam);
	bool relayPluginMessages(UINT Message, WPARAM wParam, LPARAM lParam);

	HMENU getMenuHandle() const { return _hPluginsMenu; }

	void disable() {_isDisabled = true;}
	bool hasPlugins() {return (_pluginInfos.size()!= 0);}

	bool allocateCmdID(int numberRequired, int *start);
	bool inDynamicRange(int id) { return _dynamicIDAlloc.isInRange(id); }

	bool allocateMarker(int numberRequired, int *start);
	generic_string getLoadedPluginNames() const;

	std::vector<PluginInfo *> _pluginInfos;

	std::map<int, int> _plugin_cid_table;

	std::map<long, int> _plugin_module_table;

	static struct ptrCmp
	{
		bool operator()( const TCHAR * s1, const TCHAR * s2 ) const
		{
			return lstrcmp( s1, s2 ) < 0;
		}
	};
	std::map<TCHAR*, int, ptrCmp> _plugin_module_name_table;

	HMENU getMenuForCommand(int cmdID);

	PluginInfo* getInfoForCommand(int cmdID);

	PluginInfo* getInfoForDynCommand(int cmdID);

	int getIdForCommand(int cmdID);

	//TCHAR* getModuleNameForCommand(int cmdID);

	HMENU getMenuForModule(HINSTANCE moduleID);
//private:
	NppData _nppData;
	HMENU _hPluginsMenu = NULL;

	std::vector<PluginCommand> _pluginsCommands;
	bool _isDisabled = false;
	IDAllocator _dynamicIDAlloc;
	IDAllocator _markerAlloc;
	bool _noMoreNotification = false;

	void pluginCrashAlert(const TCHAR *pluginName, const TCHAR *funcSignature)
	{
		generic_string msg = pluginName;
		msg += TEXT(" just crashed in\r");
		msg += funcSignature;
		::MessageBox(NULL, msg.c_str(), TEXT("Plugin Crash"), MB_OK|MB_ICONSTOP);
	}

	void pluginExceptionAlert(const TCHAR *pluginName, const std::exception& e)
	{
		generic_string msg = TEXT("An exception occurred due to plugin: ");
		msg += pluginName;
		msg += TEXT("\r\n\r\nException reason: ");
		msg += s2ws(e.what());

		::MessageBox(NULL, msg.c_str(), TEXT("Plugin Exception"), MB_OK);
	}

	bool isInLoadedDlls(const TCHAR *fn);
};

#define EXT_LEXER_DECL __stdcall

// External Lexer function definitions...
typedef int (EXT_LEXER_DECL *GetLexerCountFn)();
typedef void (EXT_LEXER_DECL *GetLexerNameFn)(unsigned int Index, char *name, int buflength);
typedef void (EXT_LEXER_DECL *GetLexerStatusTextFn)(unsigned int Index, TCHAR *desc, int buflength);
