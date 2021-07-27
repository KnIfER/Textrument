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


#include <shlwapi.h>
#include <dbghelp.h>
#include <algorithm>
#include <cinttypes>
#include <map>
#include "PluginsManager.h"
#include "resource.h"

using namespace std;

const TCHAR * USERMSG = TEXT(" is not compatible with the current version of Notepad++.\n\n\
Do you want to remove this plugin from the plugins directory to prevent this message from the next launch?");

#ifdef _WIN64
#define ARCH_TYPE IMAGE_FILE_MACHINE_AMD64
const TCHAR *ARCH_ERR_MSG = TEXT("Cannot load 32-bit plugin.");
#else
#define ARCH_TYPE IMAGE_FILE_MACHINE_I386
const TCHAR *ARCH_ERR_MSG = TEXT("Cannot load 64-bit plugin.");
#endif




bool PluginsManager::unloadPlugin(int index, HWND nppHandle)
{
    SCNotification scnN;
	scnN.nmhdr.code = NPPN_SHUTDOWN;
	scnN.nmhdr.hwndFrom = nppHandle;
	scnN.nmhdr.idFrom = 0;
	_pluginInfos[index]->_pBeNotified(&scnN);

    //::DestroyMenu(_pluginInfos[index]->_pluginMenu);
    //_pluginInfos[index]->_pluginMenu = NULL;

	if (::FreeLibrary(_pluginInfos[index]->_hLib))
	{
		_pluginInfos[index]->_hLib = nullptr;
		printStr(TEXT("we're good"));
	}
    else
        printStr(TEXT("not ok"));

    //delete _pluginInfos[index];
//      printInt(index);
    //vector<PluginInfo *>::iterator it = _pluginInfos.begin() + index;
    //_pluginInfos.erase(it);
    //printStr(TEXT("remove"));
    return true;
}

static WORD getBinaryArchitectureType(const TCHAR *filePath)
{
	HANDLE hFile = CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return IMAGE_FILE_MACHINE_UNKNOWN;
	}

	HANDLE hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY | SEC_IMAGE, 0, 0, NULL);
	if (hMapping == NULL)
	{
		CloseHandle(hFile);
		return IMAGE_FILE_MACHINE_UNKNOWN;
	}

	LPVOID addrHeader = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
	if (addrHeader == NULL) // couldn't memory map the file
	{
		CloseHandle(hFile);
		CloseHandle(hMapping);
		return IMAGE_FILE_MACHINE_UNKNOWN;
	}

	PIMAGE_NT_HEADERS peHdr = ImageNtHeader(addrHeader);

	// Found the binary and architecture type, if peHdr is !NULL
	WORD machine_type = (peHdr == NULL) ? IMAGE_FILE_MACHINE_UNKNOWN : peHdr->FileHeader.Machine;

	// release all of our handles
	UnmapViewOfFile(addrHeader);
	CloseHandle(hMapping);
	CloseHandle(hFile);

	return machine_type;
}

#ifndef LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR
	#define	LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR	0x00000100
#endif
#ifndef LOAD_LIBRARY_SEARCH_DEFAULT_DIRS
	#define	LOAD_LIBRARY_SEARCH_DEFAULT_DIRS	0x00001000
#endif

#include <set>

std::set<TCHAR*, PluginsManager::ptrCmp> dllSet;

int PluginsManager::loadPlugin(const TCHAR *pluginFilePath)
{
	const TCHAR *pluginFileName = ::PathFindFileName(pluginFilePath);
	if (isInLoadedDlls(pluginFileName))
		return 0;

	NppParameters& nppParams = NppParameters::getInstance();

	PluginInfo *pi = new PluginInfo;
	try
	{
		long tm = clock();
		pi->_moduleName = pluginFileName;

		int archType = nppParams.archType();
		if (getBinaryArchitectureType(pluginFilePath) != archType)
		{
			const TCHAR *archErrMsg = TEXT("Cannot load 64-bit plugin."); // IMAGE_FILE_MACHINE_I386 by default
			if (archType == IMAGE_FILE_MACHINE_ARM64)
				archErrMsg = TEXT("Cannot load 32-bit or non-ARM64 plugin.");
			else if(archType == IMAGE_FILE_MACHINE_AMD64)
				archErrMsg = TEXT("Cannot load 32-bit plugin.");

			throw generic_string(archErrMsg);
		}

        const DWORD dwFlags = GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "AddDllDirectory") != NULL ? LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS : 0;
        pi->_hLib = ::LoadLibraryEx(pluginFilePath, NULL, dwFlags);
        if (!pi->_hLib)
        {
			generic_string lastErrorMsg = GetLastErrorAsString();
            if (lastErrorMsg.empty())
                throw generic_string(TEXT("Load Library has failed.\nChanging the project's \"Runtime Library\" setting to \"Multi-threaded(/MT)\" might solve this problem."));
            else
                throw generic_string(lastErrorMsg.c_str());
        }
        
		pi->_pFuncIsUnicode = (PFUNCISUNICODE)GetProcAddress(pi->_hLib, "isUnicode");
		if (!pi->_pFuncIsUnicode || !pi->_pFuncIsUnicode())
			throw generic_string(TEXT("This ANSI plugin is not compatible with your Unicode Notepad++."));

		pi->_pFuncSetInfo = (PFUNCSETINFO)GetProcAddress(pi->_hLib, "setInfo");

		if (!pi->_pFuncSetInfo)
			throw generic_string(TEXT("Missing \"setInfo\" function"));

		pi->_pFuncGetName = (PFUNCGETNAME)GetProcAddress(pi->_hLib, "getName");
		if (!pi->_pFuncGetName)
			throw generic_string(TEXT("Missing \"getName\" function"));
		pi->_funcName = pi->_pFuncGetName();

		pi->_pBeNotified = (PBENOTIFIED)GetProcAddress(pi->_hLib, "beNotified");
		if (!pi->_pBeNotified)
			throw generic_string(TEXT("Missing \"beNotified\" function"));

		pi->_pMessageProc = (PMESSAGEPROC)GetProcAddress(pi->_hLib, "messageProc");
		if (!pi->_pMessageProc)
			throw generic_string(TEXT("Missing \"messageProc\" function"));

		pi->_pFuncSetInfo(_nppData);

		pi->_pFuncGetFuncsArray = (PFUNCGETFUNCSARRAY)GetProcAddress(pi->_hLib, "getFuncsArray");
		if (!pi->_pFuncGetFuncsArray)
			throw generic_string(TEXT("Missing \"getFuncsArray\" function"));

		pi->_funcItems = pi->_pFuncGetFuncsArray(&pi->_nbFuncItem);

		if ((!pi->_funcItems) || (pi->_nbFuncItem <= 0))
			throw generic_string(TEXT("Missing \"FuncItems\" array, or the nb of Function Item is not set correctly"));

		pi->_pluginMenu = ::CreateMenu();

		GetLexerCountFn GetLexerCount = (GetLexerCountFn)::GetProcAddress(pi->_hLib, "GetLexerCount");
		// it's a lexer plugin
		if (GetLexerCount)
		{
			GetLexerNameFn GetLexerName = (GetLexerNameFn)::GetProcAddress(pi->_hLib, "GetLexerName");
			if (!GetLexerName)
				throw generic_string(TEXT("Loading GetLexerName function failed."));

			GetLexerStatusTextFn GetLexerStatusText = (GetLexerStatusTextFn)::GetProcAddress(pi->_hLib, "GetLexerStatusText");

			if (!GetLexerStatusText)
				throw generic_string(TEXT("Loading GetLexerStatusText function failed."));

			// Assign a buffer for the lexer name.
			char lexName[MAX_EXTERNAL_LEXER_NAME_LEN];
			lexName[0] = '\0';
			TCHAR lexDesc[MAX_EXTERNAL_LEXER_DESC_LEN];
			lexDesc[0] = '\0';

			int numLexers = GetLexerCount();

			ExternalLangContainer *containers[30];

			WcharMbcsConvertor& wmc = WcharMbcsConvertor::getInstance();
			for (int x = 0; x < numLexers; ++x)
			{
				GetLexerName(x, lexName, MAX_EXTERNAL_LEXER_NAME_LEN);
				GetLexerStatusText(x, lexDesc, MAX_EXTERNAL_LEXER_DESC_LEN);
				const TCHAR *pLexerName = wmc.char2wchar(lexName, CP_ACP);
				if (!nppParams.isExistingExternalLangName(pLexerName) && nppParams.ExternalLangHasRoom())
					containers[x] = new ExternalLangContainer(pLexerName, lexDesc);
				else
					containers[x] = NULL;
			}

			TCHAR xmlPath[MAX_PATH];
			auto pluginModuleName = pi->_moduleName.c_str();
			wcscpy_s(xmlPath, nppParams.getNppPath());
			PathAppend(xmlPath, TEXT("plugins\\Config"));
            PathAppend(xmlPath, pluginModuleName);
			PathRemoveExtension(xmlPath);
			PathAddExtension(xmlPath, TEXT(".xml"));

			if (!PathFileExists(xmlPath))
			{
				lstrcpyn(xmlPath, TEXT("\0"), MAX_PATH );
				wcscpy_s(xmlPath, nppParams.getAppDataNppDir() );
				PathAppend(xmlPath, TEXT("plugins\\Config"));
                PathAppend(xmlPath, pluginModuleName);
				PathRemoveExtension( xmlPath );
				PathAddExtension( xmlPath, TEXT(".xml") );

				if (! PathFileExists( xmlPath ) )
				{
					throw generic_string(generic_string(xmlPath) + TEXT(" is missing."));
				}
			}

			TiXmlDocument *pXmlDoc = new TiXmlDocument(xmlPath);

			if (!pXmlDoc->LoadFile())
			{
				delete pXmlDoc;
				pXmlDoc = NULL;
				throw generic_string(generic_string(xmlPath) + TEXT(" failed to load."));
			}

			for (int x = 0; x < numLexers; ++x) // postpone adding in case the xml is missing/corrupt
			{
				if (containers[x] != NULL)
					nppParams.addExternalLangToEnd(containers[x]);
			}

			nppParams.getExternalLexerFromXmlTree(pXmlDoc);
			nppParams.getExternalLexerDoc()->push_back(pXmlDoc);
			const char *pDllName = wmc.wchar2char(pluginFilePath, CP_ACP);
			::SendMessage(_nppData._scintillaMainHandle, SCI_LOADLEXERLIBRARY, 0, reinterpret_cast<LPARAM>(pDllName));

		}
		_pluginInfos.push_back(pi);
		pi->loadInfo = new LoadedDllInfo{pluginFilePath, clock()-tm};
		int ret = _pluginInfos.size()-1;
		installPluginMenuAt(ret);
		return static_cast<int32_t>(ret);
	}
	catch (std::exception& e)
	{
		pluginExceptionAlert(pluginFileName, e);
		return -1;
	}
	catch (generic_string& s)
	{
		s += TEXT("\n\n");
		s += pluginFileName;
		s += USERMSG;
		if (::MessageBox(NULL, s.c_str(), pluginFilePath, MB_YESNO) == IDYES)
		{
			if (pi && pi->_hLib)
			{
				::FreeLibrary(pi->_hLib);
			}
			::DeleteFile(pluginFilePath);
		}
		delete pi;
		return -1;
	}
	catch (...)
	{
		generic_string msg = TEXT("Failed to load");
		msg += TEXT("\n\n");
		msg += pluginFileName;
		msg += USERMSG;
		if (::MessageBox(NULL, msg.c_str(), pluginFilePath, MB_YESNO) == IDYES)
		{
			if (pi && pi->_hLib)
			{
				::FreeLibrary(pi->_hLib);
			}
			::DeleteFile(pluginFilePath);
		}
		delete pi;
		return -1;
	}
}

bool PluginsManager::isInLoadedDlls(const TCHAR *fn)
{
	auto pluginID = _plugin_module_name_table.find((TCHAR *)fn);
	return pluginID!=_plugin_module_name_table.end();
}


bool PluginsManager::loadPluginsV2(const TCHAR* dir)
{
	//if (_isDisabled) return false;

	vector<generic_string> dllNames;

	NppParameters& nppParams = NppParameters::getInstance();
	generic_string nppPath = nppParams.getNppPath();
	
	generic_string pluginsFolder;
	if (dir && dir[0])
	{
		pluginsFolder = dir;
	}
	else
	{
		pluginsFolder = nppPath;
		PathAppendCompat(pluginsFolder, TEXT("plugins"));
	}
	generic_string pluginsFolderFilter = pluginsFolder;
	PathAppendCompat(pluginsFolderFilter, TEXT("*.*"));
	
	WIN32_FIND_DATA foundData;
	HANDLE hFindFolder = ::FindFirstFile(pluginsFolderFilter.c_str(), &foundData);
	HANDLE hFindDll = INVALID_HANDLE_VALUE;

	// get plugin folder
	if (hFindFolder != INVALID_HANDLE_VALUE && (foundData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		generic_string foundFileName = foundData.cFileName;
		if (foundFileName != TEXT(".") && foundFileName != TEXT("..") && generic_stricmp(foundFileName.c_str(), TEXT("Config")) != 0)
		{
			generic_string pluginsFullPathFilter = pluginsFolder;
			PathAppendCompat(pluginsFullPathFilter, foundFileName);
			generic_string  dllName = foundFileName;
			dllName += TEXT(".dll");
			PathAppendCompat(pluginsFullPathFilter, dllName);

			// get plugin
			hFindDll = ::FindFirstFile(pluginsFullPathFilter.c_str(), &foundData);
			if (hFindDll != INVALID_HANDLE_VALUE && !(foundData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				dllNames.push_back(pluginsFullPathFilter);

				PluginList & pl = nppParams.getPluginList();
				pl.add(foundFileName, false);
			}
		}
		// get plugin folder
		while (::FindNextFile(hFindFolder, &foundData))
		{
			generic_string foundFileName2 = foundData.cFileName;
			if (foundFileName2 != TEXT(".") && foundFileName2 != TEXT("..") && generic_stricmp(foundFileName2.c_str(), TEXT("Config")) != 0)
			{
				generic_string pluginsFullPathFilter2 = pluginsFolder;
				PathAppendCompat(pluginsFullPathFilter2, foundFileName2);
				generic_string pluginsFolderPath2 = pluginsFullPathFilter2;
				generic_string  dllName2 = foundFileName2;
				dllName2 += TEXT(".dll");
				PathAppendCompat(pluginsFullPathFilter2, dllName2);

				// get plugin
				if (hFindDll)
				{
					::FindClose(hFindDll);
					hFindDll = INVALID_HANDLE_VALUE;
				}
				hFindDll = ::FindFirstFile(pluginsFullPathFilter2.c_str(), &foundData);
				if (hFindDll != INVALID_HANDLE_VALUE && !(foundData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					dllNames.push_back(pluginsFullPathFilter2);

					PluginList & pl = nppParams.getPluginList();
					pl.add(foundFileName2, false);
				}
			}
		}

	}
	::FindClose(hFindFolder);
	::FindClose(hFindDll);

	for (size_t i = 0, len = dllNames.size(); i < len; ++i)
	{
		loadPlugin(dllNames[i].c_str());
	}

	return true;
}

// return true if cmdID found and its shortcut is enable
// false otherwise
bool PluginsManager::getShortcutByCmdID(int cmdID, ShortcutKey *sk)
{
	if (cmdID == 0 || !sk)
		return false;

	const vector<PluginCmdShortcut> & pluginCmdSCList = (NppParameters::getInstance()).getPluginCommandList();

	for (size_t i = 0, len = pluginCmdSCList.size(); i < len ; ++i)
	{
		if (pluginCmdSCList[i].getID() == (unsigned long)cmdID)
		{
			const KeyCombo & kc = pluginCmdSCList[i].getKeyCombo();
			if (kc._key == 0x00)
				return false;

			sk->_isAlt = kc._isAlt;
			sk->_isCtrl = kc._isCtrl;
			sk->_isShift = kc._isShift;
			sk->_key = kc._key;
			return true;
		}
	}
	return false;
}

// returns false if cmdID not provided, true otherwise
bool PluginsManager::removeShortcutByCmdID(int cmdID)
{
	if (cmdID == 0)
		return false;

	NppParameters& nppParam = NppParameters::getInstance();
	vector<PluginCmdShortcut> & pluginCmdSCList = nppParam.getPluginCommandList();

	for (size_t i = 0, len = pluginCmdSCList.size(); i < len; ++i)
	{
		if (pluginCmdSCList[i].getID() == (unsigned long)cmdID)
		{
			//remove shortcut
			pluginCmdSCList[i].clear();

			// inform accelerator instance
			nppParam.getAccelerator()->updateShortcuts();

			// set dirty flag to force writing shortcuts.xml on shutdown
			nppParam.setShortcutDirty();
			break;
		}
	}
	return true;
}

void PluginsManager::installPluginMenuAt(int i)
{
    vector<PluginCmdShortcut> & pluginCmdSCList = (NppParameters::getInstance()).getPluginCommandList();
	PluginInfo & plug = *_pluginInfos[i];
	if (plug._pluginMenu!=0)
	{
		// todo first remve the old menu and the old PluginCommands
	}
	::InsertMenu(_hPluginsMenu, i, MF_BYPOSITION | MF_POPUP, (UINT_PTR)plug._pluginMenu, plug._funcName.c_str());
	_plugin_module_table[(long)plug._hLib] = i;
	_plugin_module_name_table[(TCHAR*)plug._moduleName.data()] = i;

    unsigned short j = 0;
	for ( ; j < plug._nbFuncItem ; ++j)
	{
		if (plug._funcItems[j]._pFunc == NULL)
		{
			::InsertMenu(plug._pluginMenu, j, MF_BYPOSITION | MF_SEPARATOR, 0, TEXT(""));
			continue;
		}

        _pluginsCommands.push_back(PluginCommand(plug._moduleName.c_str(), j, plug._funcItems[j]._pFunc));

		int cmdID = ID_PLUGINS_CMD + static_cast<int32_t>(_pluginsCommands.size() - 1);
		plug._funcItems[j]._cmdID = cmdID;
		generic_string itemName = plug._funcItems[j]._itemName;

		if (plug._funcItems[j]._pShKey)
		{
			ShortcutKey & sKey = *(plug._funcItems[j]._pShKey);
            PluginCmdShortcut pcs(Shortcut(itemName.c_str(), sKey._isCtrl, sKey._isAlt, sKey._isShift, sKey._key), cmdID, plug._moduleName.c_str(), j);
			pluginCmdSCList.push_back(pcs);
			itemName += TEXT("\t");
			itemName += pcs.toString();
		}
		else
		{	//no ShortcutKey is provided, add an disabled shortcut (so it can still be mapped, Paramaters class can still index any changes and the toolbar wont funk out
            Shortcut sc(itemName.c_str(), false, false, false, 0x00);
            PluginCmdShortcut pcs(sc, cmdID, plug._moduleName.c_str(), j);	//VK_NULL and everything disabled, the menu name is left alone
			pluginCmdSCList.push_back(pcs);
		}
		::InsertMenu(plug._pluginMenu, j, MF_BYPOSITION, cmdID, itemName.c_str());

		_plugin_cid_table[cmdID]=i;

		if (plug._funcItems[j]._init2Check)
			::CheckMenuItem(_hPluginsMenu, cmdID, MF_BYCOMMAND | MF_CHECKED);
	}
}

HMENU PluginsManager::getMenuForCommand(int cmdID) 
{
	auto id = _plugin_cid_table.find(cmdID);
	if(id!=_plugin_cid_table.end()) {
		int pluginid=id->second;
		if(pluginid>=0&&pluginid<_pluginInfos.size()) 
		{
			return _pluginInfos[pluginid]->_pluginMenu;
		}
	}
	return 0;
}

PluginInfo* PluginsManager::getInfoForCommand(int cmdID) 
{
	auto id = _plugin_cid_table.find(cmdID);
	if(id!=_plugin_cid_table.end()) {
		int pluginid=id->second;
		if(pluginid>=0&&pluginid<_pluginInfos.size()) {
			return _pluginInfos[pluginid];
		}
	}
	return 0;
}

PluginInfo* PluginsManager::getInfoForDynCommand(int cmdID) 
{
	if (cmdID<_regstrDynCmdMax)
	{
		for (size_t i = 0, len = _pluginInfos.size(); i < len ; ++i)
		{
			if (cmdID>=_pluginInfos[i]->_regstrDynCmdSt 
				&& (i==len-1||cmdID<_pluginInfos[i+1]->_regstrDynCmdSt)
				)
			{
				return _pluginInfos[i];
			}
		}
	}
	return NULL;
}

int PluginsManager::getIdForCommand(int cmdID) 
{
	auto id = _plugin_cid_table.find(cmdID);
	if(id!=_plugin_cid_table.end()) {
		int pluginid=id->second;
		if(pluginid>=0&&pluginid<_pluginInfos.size()) {
			return pluginid;
		}
	}
	return -1;
}

HMENU PluginsManager::getMenuForModule(HINSTANCE moduleID) 
{
	auto id = _plugin_module_table.find((long)moduleID);
	if(id!=_plugin_module_table.end()) {
		int pluginid=id->second;
		if(pluginid>=0&&pluginid<_pluginInfos.size()) {
			return _pluginInfos[pluginid]->_pluginMenu;
		}
	}
	return 0;
}


HMENU PluginsManager::setMenu(HMENU hMenu, const TCHAR *menuName, bool enablePluginAdmin)
{
	const TCHAR *nom_menu = (menuName && menuName[0])?menuName:TEXT("插件(&P)");

	if (!_hPluginsMenu)
	{
		_hPluginsMenu = ::CreateMenu();
		::InsertMenu(hMenu,  MENUINDEX_PLUGINS, MF_BYPOSITION | MF_POPUP, (UINT_PTR)_hPluginsMenu, nom_menu);

		int i = 1;

		::InsertMenu(_hPluginsMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, TEXT(""));

		::InsertMenu(_hPluginsMenu, i++, MF_BYPOSITION, IDM_SETTING_PLUGINADM, TEXT("插件管理..."));
		::InsertMenu(_hPluginsMenu, i++, MF_BYPOSITION | MF_SEPARATOR, 0, TEXT(""));

		::InsertMenu(_hPluginsMenu, i, MF_BYPOSITION, IDM_SETTING_OPENPLUGINSDIR, TEXT("打开插件文件夹..."));
	}
	return _hPluginsMenu;
}


void PluginsManager::runPluginCommand(size_t i)
{
	if (i < _pluginsCommands.size())
	{
		if (_pluginsCommands[i]._pFunc != NULL)
		{
			try
			{
				_pluginsCommands[i]._pFunc();
			}
			catch (std::exception& e)
			{
				::MessageBoxA(NULL, e.what(), "PluginsManager::runPluginCommand Exception", MB_OK);
			}
			catch (...)
			{
				TCHAR funcInfo[128];
				generic_sprintf(funcInfo, TEXT("runPluginCommand(size_t i : %zd)"), i);
				pluginCrashAlert(_pluginsCommands[i]._pluginName.c_str(), funcInfo);
			}
		}
	}
}


void PluginsManager::runPluginCommand(const TCHAR *pluginName, int commandID)
{
	for (size_t i = 0, len = _pluginsCommands.size() ; i < len ; ++i)
	{
		if (!generic_stricmp(_pluginsCommands[i]._pluginName.c_str(), pluginName))
		{
			if (_pluginsCommands[i]._funcID == commandID)
			{
				try
				{
					_pluginsCommands[i]._pFunc();
				}
				catch (std::exception& e)
				{
					pluginExceptionAlert(_pluginsCommands[i]._pluginName.c_str(), e);
				}
				catch (...)
				{
					TCHAR funcInfo[128];
					generic_sprintf(funcInfo, TEXT("runPluginCommand(const TCHAR *pluginName : %s, int commandID : %d)"), pluginName, commandID);
					pluginCrashAlert(_pluginsCommands[i]._pluginName.c_str(), funcInfo);
				}
			}
		}
	}
}

// send the notification to a specific plugin
void PluginsManager::notify(size_t indexPluginInfo, const SCNotification *notification)
{
	if (indexPluginInfo >= _pluginInfos.size())
		return;

	if (_pluginInfos[indexPluginInfo]->_hLib)
	{
		SCNotification scNotif = *notification;
		try
		{
			if (indexPluginInfo>0 && memcmp(&scNotif, notification, sizeof(SCNotification))!=0)
			{
#ifdef _DEBUG
				LogIs("%s 不准改！", _pluginInfos[indexPluginInfo]->_moduleName.c_str());
#endif
				scNotif = *notification; 
			}
			_pluginInfos[indexPluginInfo]->_pBeNotified(&scNotif);
		}
		catch (std::exception& e)
		{
			pluginExceptionAlert(_pluginInfos[indexPluginInfo]->_moduleName.c_str(), e);
		}
		catch (...)
		{
			TCHAR funcInfo[256];
			generic_sprintf(funcInfo, TEXT("notify(SCNotification *notification) : \r notification->nmhdr.code == %d\r notification->nmhdr.hwndFrom == %p\r notification->nmhdr.idFrom == %" PRIuPTR), \
				scNotif.nmhdr.code, scNotif.nmhdr.hwndFrom, scNotif.nmhdr.idFrom);
			pluginCrashAlert(_pluginInfos[indexPluginInfo]->_moduleName.c_str(), funcInfo);
		}
	}
}

int cc=0;
// broadcast the notification to all plugins
void PluginsManager::notify(const SCNotification *notification)
{
	if(notification->nmhdr.code == NPPN_SHUTDOWN) {
		_noMoreNotification = true;
	}

	if (_noMoreNotification) // this boolean should be enabled after NPPN_SHUTDOWN has been sent
		return;

	int isTBMOD = notification->nmhdr.code==NPPN_TBMODIFICATION;

	for (size_t i = 0, len = _pluginInfos.size() ; i < len ; ++i)
	{
		if (isTBMOD)
		{
			_pluginInfos[i]->_regstrDynCmdSt = _dynamicIDAlloc.tellNextId();
		}

		notify(i, notification);

		if (isTBMOD && i==len-1)
		{
			_regstrDynCmdMax = _dynamicIDAlloc.tellNextId();
		}
	}


#if 0
	int ModifyType = notification->modificationType;
	if ((notification->nmhdr.code == SCN_MODIFIED))
	{
		cc++;
		if(cc>=85) {
			//::MessageBox(NULL, TEXT("notify"), TEXT("  notify "), MB_OK);

			TCHAR buffer[128];

			wsprintf(buffer,TEXT("MT=%d | NC=%d | PT=%d"), ModifyType, cc, notification);

			::MessageBox(NULL, buffer, TEXT(""), MB_OK);
		}

	}
#endif

}


void PluginsManager::relayNppMessages(UINT Message, WPARAM wParam, LPARAM lParam)
{
	for (size_t i = 0, len = _pluginInfos.size(); i < len ; ++i)
	{
        if (_pluginInfos[i]->_hLib)
		{
			try
			{
				_pluginInfos[i]->_pMessageProc(Message, wParam, lParam);
			}
			catch (std::exception& e)
			{
				pluginExceptionAlert(_pluginInfos[i]->_moduleName.c_str(), e);
			}
			catch (...)
			{
				TCHAR funcInfo[128];
				generic_sprintf(funcInfo, TEXT("relayNppMessages(UINT Message : %u, WPARAM wParam : %" PRIuPTR ", LPARAM lParam : %" PRIiPTR ")"), Message, wParam, lParam);
				pluginCrashAlert(_pluginInfos[i]->_moduleName.c_str(), funcInfo);
			}
		}
	}
}

bool PluginsManager::relayPluginMessages(UINT Message, WPARAM wParam, LPARAM lParam)
{
	const TCHAR * moduleName = (const TCHAR *)wParam;
	if (!moduleName || !moduleName[0] || !lParam)
		return false;

	for (size_t i = 0, len = _pluginInfos.size() ; i < len ; ++i)
	{
        if (_pluginInfos[i]->_moduleName == moduleName)
		{
            if (_pluginInfos[i]->_hLib)
			{
				try
				{
					_pluginInfos[i]->_pMessageProc(Message, wParam, lParam);
				}
				catch (std::exception& e)
				{
					pluginExceptionAlert(_pluginInfos[i]->_moduleName.c_str(), e);
				}
				catch (...)
				{
					TCHAR funcInfo[128];
					generic_sprintf(funcInfo, TEXT("relayPluginMessages(UINT Message : %u, WPARAM wParam : %" PRIuPTR ", LPARAM lParam : %" PRIiPTR ")"), Message, wParam, lParam);
					pluginCrashAlert(_pluginInfos[i]->_moduleName.c_str(), funcInfo);
				}
				return true;
            }
		}
	}
	return false;
}


bool PluginsManager::allocateCmdID(int numberRequired, int *start)
{
	bool retVal = true;

	*start = _dynamicIDAlloc.allocate(numberRequired);

	if (-1 == *start)
	{
		*start = 0;
		retVal = false;
	}
	return retVal;
}

bool PluginsManager::allocateMarker(int numberRequired, int *start)
{
	bool retVal = true;
	*start = _markerAlloc.allocate(numberRequired);
	if (-1 == *start)
	{
		*start = 0;
		retVal = false;
	}
	return retVal;
}

generic_string PluginsManager::getLoadedPluginNames() const
{
	generic_string pluginPaths;
	for (size_t i = 0; i < _pluginInfos.size(); ++i)
	{
		pluginPaths += _pluginInfos[i]->_moduleName;
		pluginPaths += TEXT(" ");
	}
	return pluginPaths;
}

