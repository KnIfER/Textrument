#include "NppShell.h"
#include "Bitmap.h"
#include "resource.h"
#include <shellapi.h>
#include <algorithm>

#ifndef RGBA
#define RGBA(r,g,b,a)        ((COLORREF)( (((DWORD)(BYTE)(a))<<24) |     RGB(r,g,b) ))
#endif



//---------------------------------------------------------------------------
//  Global variables
//---------------------------------------------------------------------------
UINT _cRef = 0; // COM Reference count.
HINSTANCE _hModule = NULL; // DLL Module.

//Some global default values for registering the DLL

//Menu
TCHAR szDefaultMenutext[] = TEXT("Edit with &Textrument");

#define szHelpTextA "Edits the selected file(s) with Textrument"
#define szHelpTextW L"Edits the selected file(s) with Textrument"
TCHAR szMenuTitle[TITLE_SIZE];
TCHAR szDefaultCustomcommand[] = TEXT("");
//Icon
DWORD isDynamic = 1;
DWORD maxText = 25;
DWORD showIcon = 1;

//Forward function declarations
extern "C" int APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved);
STDAPI DllRegisterServer(void){ return S_OK; };
STDAPI DllUnregisterServer(void){ return S_OK; };
STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine){ return S_OK; };

INT_PTR CALLBACK DlgProcSettings(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InvalidateIcon(HICON * iconSmall, HICON * iconLarge);

#ifdef UNICODE
#define _ttoi _wtoi
#else
#define _ttoi atoi
#endif


//---------------------------------------------------------------------------
// DllMain
//---------------------------------------------------------------------------
int APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/) {
	if (dwReason == DLL_PROCESS_ATTACH) {
		_hModule = hInstance;
	}
	return TRUE;
}

//---------------------------------------------------------------------------
// DllCanUnloadNow
//---------------------------------------------------------------------------
STDAPI DllCanUnloadNow(void) {
	return (_cRef == 0 ? S_OK : S_FALSE);
}

//---------------------------------------------------------------------------
// DllGetClassObject
//---------------------------------------------------------------------------
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppvOut) {
	*ppvOut = NULL;
	if (IsEqualIID(rclsid, CLSID_ShellExtension)) {
		CShellExtClassFactory *pcf = new CShellExtClassFactory;
		return pcf->QueryInterface(riid, ppvOut);
	}
	return CLASS_E_CLASSNOTAVAILABLE;
}

//---------------------------------------------------------------------------
// MsgBoxError
//---------------------------------------------------------------------------
void MsgBoxError(LPCTSTR lpszMsg) {
	MessageBox(NULL,
		lpszMsg,
		TEXT("Textrument Error:"),
		MB_OK | MB_ICONWARNING);
}
// --- CShellExtClassFactory ---
CShellExtClassFactory::CShellExtClassFactory() :
	m_cRef(0L)
{
	_cRef++;
}

CShellExtClassFactory::~CShellExtClassFactory() {
	_cRef--;
}

// *** IUnknown methods ***
STDMETHODIMP CShellExtClassFactory::QueryInterface(REFIID riid, LPVOID FAR *ppv) {
	*ppv = NULL;
	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory)) {
		*ppv = (LPCLASSFACTORY)this;
		AddRef();
		return NOERROR;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CShellExtClassFactory::AddRef() {
	return ++m_cRef;
}

STDMETHODIMP_(ULONG) CShellExtClassFactory::Release()
{
	if (--m_cRef)
		return m_cRef;
	delete this;
	return 0L;
}

// *** IClassFactory methods ***
STDMETHODIMP CShellExtClassFactory::CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, LPVOID *ppvObj) {
	*ppvObj = NULL;
	if (pUnkOuter)
		return CLASS_E_NOAGGREGATION;
	CShellExt * pShellExt = new CShellExt();
	if (!pShellExt)
		return E_OUTOFMEMORY;
	return pShellExt->QueryInterface(riid, ppvObj);
}

STDMETHODIMP CShellExtClassFactory::LockServer(BOOL /*fLock*/) {
	return NOERROR;
}

// --- CShellExt ---
CShellExt::CShellExt() :
	m_cRef(0L),
	m_cbFiles(0),
	m_pDataObj(NULL),
	m_menuID(0),
	m_hMenu(NULL),
	m_showIcon(true),
	m_useCustom(false),
	m_nameLength(0),
	m_nameMaxLength(maxText),
	m_isDynamic(false),
	m_winVer(0),
	m_hBitmap(NULL)
{
	TCHAR szKeyTemp [MAX_PATH + GUID_STRING_SIZE];
	ZeroMemory(&m_stgMedium, sizeof(m_stgMedium));
	_cRef++;

	GetModuleFileName(_hModule, m_szModule, MAX_PATH);

	OSVERSIONINFOEX inf;
	ZeroMemory(&inf, sizeof(OSVERSIONINFOEX));
    inf.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx((OSVERSIONINFO *)&inf);
	m_winVer = MAKEWORD(inf.dwMinorVersion, inf.dwMajorVersion);

	if (m_winVer >= WINVER_VISTA) {
		InitTheming();
	}

	HKEY settingKey;
	LONG result;
	DWORD size = 0;
	DWORD dyn = 0, siz = 0, showicon = 0;

	wsprintf(szKeyTemp, TEXT("CLSID\\%s\\Settings"), szGUID);
	result = RegOpenKeyEx(HKEY_CLASSES_ROOT, szKeyTemp, 0, KEY_READ, &settingKey);
	if (result == ERROR_SUCCESS) {
		size = sizeof(TCHAR)*TITLE_SIZE;
		result = RegQueryValueEx(settingKey, TEXT("Title"), NULL, NULL, (LPBYTE)(m_szMenuTitle), &size);
		if (result != ERROR_SUCCESS) {
			lstrcpyn(m_szMenuTitle, szDefaultMenutext, TITLE_SIZE);
		}

		size = sizeof(DWORD);
		result = RegQueryValueEx(settingKey, TEXT("Dynamic"), NULL, NULL, (BYTE*)(&dyn), &size);
		if (result == ERROR_SUCCESS && dyn != 0) {
			m_isDynamic = true;
		}

		size = sizeof(DWORD);
		result = RegQueryValueEx(settingKey, TEXT("Maxtext"), NULL, NULL, (BYTE*)(&siz), &size);
		if (result == ERROR_SUCCESS) {
			m_nameMaxLength = std::max((DWORD)0,siz);
		}

		size = sizeof(DWORD);
		result = RegQueryValueEx(settingKey, TEXT("ShowIcon"), NULL, NULL, (BYTE*)(&showicon), &size);
		if (result == ERROR_SUCCESS) {
			m_showIcon = (showicon != 0);
		}

		result = RegQueryValueEx(settingKey, TEXT("CustomIcon"), NULL, NULL, NULL, NULL);
		if (result == ERROR_SUCCESS) {
			m_useCustom = true;
			size = MAX_PATH;
			RegQueryValueEx(settingKey, TEXT("CustomIcon"), NULL, NULL, (BYTE*)m_szCustomPath, &size);
		}

		RegCloseKey(settingKey);
	}
}

CShellExt::~CShellExt() {
	if (m_winVer >= WINVER_VISTA) {
		DeinitTheming();
	}

	if (m_hBitmap != NULL && m_hBitmap != HBMMENU_CALLBACK)
		DeleteBitmap(m_hBitmap);

	if (m_pDataObj)
		m_pDataObj->Release();
	_cRef--;
}
// *** IUnknown methods ***
STDMETHODIMP CShellExt::QueryInterface(REFIID riid, LPVOID FAR *ppv) {
	*ppv = NULL;
	if (IsEqualIID(riid, IID_IUnknown)) {
		//*ppv = (LPUNKNOWN)this;
		*ppv = (LPSHELLEXTINIT)this;
	} else if (IsEqualIID(riid, IID_IShellExtInit)) {
		*ppv = (LPSHELLEXTINIT)this;
	} else if (IsEqualIID(riid, IID_IContextMenu)) {
		*ppv = (LPCONTEXTMENU)this;
	} else if (IsEqualIID(riid, IID_IContextMenu2)) {
		*ppv = (LPCONTEXTMENU2)this;
	} else if (IsEqualIID(riid, IID_IContextMenu3)) {
		*ppv = (LPCONTEXTMENU3)this;
	} else if (IsEqualIID(riid, IID_IPersistFile)) {
		*ppv = (LPPERSISTFILE)this;
	} else if (IsEqualIID(riid, IID_IExtractIcon)) {
		*ppv = (LPEXTRACTICON)this;
	}
	if (*ppv) {
		AddRef();
		return NOERROR;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CShellExt::AddRef() {
	return ++m_cRef;
}

STDMETHODIMP_(ULONG) CShellExt::Release() {
	if (--m_cRef)
		return m_cRef;
	delete this;
	return 0L;
}

// *** IShellExtInit methods ***
STDMETHODIMP CShellExt::Initialize(LPCITEMIDLIST /*pIDFolder*/, LPDATAOBJECT pDataObj, HKEY /*hRegKey*/) {
	if (m_pDataObj) {
		m_pDataObj->Release();
		m_pDataObj = NULL;
	}
	if (pDataObj) {
		m_pDataObj = pDataObj;
		pDataObj->AddRef();
	}
	return NOERROR;
}

// *** IContextMenu methods ***
STDMETHODIMP CShellExt::QueryContextMenu(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT /*idCmdLast*/, UINT /*uFlags*/) {
	UINT idCmd = idCmdFirst;

	FORMATETC fmte = {
		CF_HDROP,
		(DVTARGETDEVICE FAR *)NULL,
		DVASPECT_CONTENT,
		-1,
		TYMED_HGLOBAL
	};

	HRESULT hres = m_pDataObj->GetData(&fmte, &m_stgMedium);

	if (SUCCEEDED(hres)) {
		if (m_stgMedium.hGlobal)
			m_cbFiles = DragQueryFile((HDROP)m_stgMedium.hGlobal, (UINT)-1, 0, 0);
	}

	UINT nIndex = indexMenu++;

	InsertMenu(hMenu, nIndex, MF_STRING|MF_BYPOSITION, idCmd++, m_szMenuTitle);


	if (m_showIcon) {
		HBITMAP icon = NULL;
		if (m_winVer >= WINVER_VISTA) {
			icon = NULL;
			HICON hicon;
			DWORD menuIconWidth = GetSystemMetrics(SM_CXMENUCHECK);
			DWORD menuIconHeight = GetSystemMetrics(SM_CYMENUCHECK);
			HRESULT hr = LoadShellIcon(menuIconWidth, menuIconHeight, &hicon);
			if (SUCCEEDED(hr)) {
				icon = IconToBitmapPARGB32(hicon, menuIconWidth, menuIconHeight);
				DestroyIcon(hicon);
			}
		} else {
			icon = HBMMENU_CALLBACK;
		}

		if (icon != NULL) {
			MENUITEMINFO mii;
			ZeroMemory(&mii, sizeof(mii));
			mii.cbSize = sizeof(mii);
			mii.fMask = MIIM_BITMAP;
			mii.hbmpItem = icon;
			//mii.hbmpChecked = icon;
			//mii.hbmpUnchecked = icon;

			SetMenuItemInfo(hMenu, nIndex, MF_BYPOSITION, &mii);

			if (m_winVer >= WINVER_VISTA) {
				MENUINFO MenuInfo;
				MenuInfo.cbSize = sizeof(MenuInfo);
				MenuInfo.fMask = MIM_STYLE;
				MenuInfo.dwStyle = MNS_CHECKORBMP;

				SetMenuInfo(hMenu, &MenuInfo);
			}

		}
		m_hBitmap = icon;
	}

	m_hMenu = hMenu;
	m_menuID = idCmd;

	return ResultFromShort(idCmd-idCmdFirst);
}

STDMETHODIMP CShellExt::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi) {
	HRESULT hr = E_INVALIDARG;

	if (!HIWORD(lpcmi->lpVerb)) {
		UINT idCmd = LOWORD(lpcmi->lpVerb);
		switch(idCmd) {
			case 0:
				hr = InvokeNPP(lpcmi->hwnd, lpcmi->lpDirectory, lpcmi->lpVerb, lpcmi->lpParameters, lpcmi->nShow);
				break;
			default:
				break;
		}
	}
	return hr;
}

STDMETHODIMP CShellExt::GetCommandString(UINT_PTR, UINT uFlags, UINT FAR *, LPSTR pszName, UINT cchMax) {
	LPWSTR wBuffer = (LPWSTR) pszName;
	if (uFlags == GCS_HELPTEXTA) {
		lstrcpynA(pszName, szHelpTextA, cchMax);
		return S_OK;
	} else if (uFlags == GCS_HELPTEXTW) {
		lstrcpynW(wBuffer, szHelpTextW, cchMax);
		return S_OK;
	}
	return E_NOTIMPL;
}

STDMETHODIMP CShellExt::HandleMenuMsg2(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, LRESULT *plResult) {

	//Setup popup menu stuff (ownerdrawn)
	DWORD menuIconWidth = GetSystemMetrics(SM_CXMENUCHECK);
	DWORD menuIconHeight = GetSystemMetrics(SM_CYMENUCHECK);
	DWORD menuIconPadding = 2;	//+1 pixels on each side, is this fixed?

	switch(uMsg) {
		case WM_MEASUREITEM: {	//for owner drawn menu
			MEASUREITEMSTRUCT * lpdis = (MEASUREITEMSTRUCT*) lParam;

			if (lpdis == NULL)// || lpdis->itemID != m_menuID)
				break;

			if (m_showIcon) {
				lpdis->itemWidth = 0;	//0 seems to work for 98 and up
				if (lpdis->itemHeight < menuIconHeight)
					lpdis->itemHeight = menuIconHeight;
			}

			if (plResult)
				*plResult = TRUE;
			break; }
		case WM_DRAWITEM: {		//for owner drawn menu
			//Assumes proper font already been set
			DRAWITEMSTRUCT * lpdis = (DRAWITEMSTRUCT*) lParam;
			if ((lpdis == NULL) || (lpdis->CtlType != ODT_MENU))
				break;

			if (m_showIcon) {
				HICON nppIcon = NULL;

				HRESULT hr = LoadShellIcon(menuIconWidth, menuIconHeight, &nppIcon);

				if (SUCCEEDED(hr)) {
					DrawIconEx(lpdis->hDC, menuIconPadding, menuIconPadding, nppIcon, menuIconWidth, menuIconHeight, 0, NULL, DI_NORMAL);
					DestroyIcon(nppIcon);
				}
			}

			if (plResult)
				*plResult = TRUE;

			break; }
		default:
			break;
	}

	return S_OK;
}

// *** IPersistFile methods ***
HRESULT STDMETHODCALLTYPE CShellExt::Load(LPCOLESTR pszFileName, DWORD /*dwMode*/) {
	LPTSTR file[MAX_PATH];
#ifdef UNICODE
	lstrcpyn((LPWSTR)file, pszFileName, MAX_PATH);
#else
	WideCharToMultiByte(CP_ACP, 0, pszFileName, -1, (LPSTR)file, MAX_PATH, NULL, NULL);
#endif
	m_szFilePath[0] = 0;

	LPTSTR ext = PathFindExtension((LPTSTR)file);
	if (ext[0] == '.') {
		ext++;
	}
	int copySize = std::min(m_nameMaxLength+1, MAX_PATH);	//+1 to take zero terminator in account
	lstrcpyn(m_szFilePath, ext, copySize);
	m_nameLength = lstrlen(m_szFilePath);
	CharUpperBuff(m_szFilePath, m_nameLength);

	return S_OK;
}

// *** IExtractIcon methods ***
STDMETHODIMP CShellExt::GetIconLocation(UINT uFlags, LPTSTR szIconFile, UINT cchMax, int * piIndex, UINT * pwFlags) {
	*pwFlags = 0;
	if (uFlags & GIL_DEFAULTICON || m_szFilePath[0] == 0 || !m_isDynamic) {	//return regular N++ icon if requested OR the extension is bad OR static icon
		if (!m_useCustom) {
			lstrcpyn(szIconFile, m_szModule, cchMax);
			*piIndex = 0;
		} else {
			lstrcpyn(szIconFile, m_szCustomPath, cchMax);
			*piIndex = 0;
		}
		return S_OK;
	}

	if(cchMax > 0) {
		lstrcpyn(szIconFile, TEXT("NppShellIcon"), cchMax);
		int len = lstrlen(szIconFile);
		lstrcpyn(szIconFile, m_szFilePath, cchMax-len);
	}
	*piIndex = 0;
	*pwFlags |= GIL_NOTFILENAME;//|GIL_DONTCACHE|GIL_PERINSTANCE;

	return S_OK;
}

STDMETHODIMP CShellExt::Extract(LPCTSTR /*pszFile*/, UINT /*nIconIndex*/, HICON * phiconLarge
	, HICON * phiconSmall, UINT nIconSize) {
	WORD sizeSmall = HIWORD(nIconSize);
	WORD sizeLarge = LOWORD(nIconSize);
	ICONINFO iconinfo;
	BOOL res;
	HRESULT hrSmall = S_OK, hrLarge = S_OK;

	if (phiconSmall)
		hrSmall = LoadShellIcon(sizeSmall, sizeSmall, phiconSmall, IDI_ICON_SHELL);
	if (phiconLarge)
		hrLarge = LoadShellIcon(sizeLarge, sizeLarge, phiconLarge, IDI_ICON_SHELL);

	if (FAILED(hrSmall) || FAILED(hrLarge)) {
		InvalidateIcon(phiconSmall, phiconLarge);
		return S_FALSE;
	}

	//if (!m_isDynamic)	//No modifications required  || !phiconLarge || sizeLarge < 32
	return S_OK;

	//if(1) return S_OK;
	// here
}

void InvalidateIcon(HICON * iconSmall, HICON * iconLarge) {
	if (iconSmall && *iconSmall) {
		DestroyIcon(*iconSmall);
		*iconSmall = NULL;
	}
	if (iconLarge && *iconLarge) {
		DestroyIcon(*iconLarge);
		*iconLarge = NULL;
	}
}

// *** Private methods ***
STDMETHODIMP CShellExt::InvokeNPP(HWND /*hParent*/, LPCSTR /*pszWorkingDir*/, LPCSTR /*pszCmd*/, LPCSTR /*pszParam*/, int iShowCmd) {

	TCHAR szFilename[MAX_PATH];
	TCHAR szCustom[MAX_PATH];
	TCHAR szNotepadExecutableFilename[3 * MAX_PATH]; // Should be able to contain szFilename plus szCustom plus some additional characters.
	LPTSTR pszCommand;
	size_t bytesRequired = 1;

	memset(szNotepadExecutableFilename, 0, sizeof(TCHAR) * 3 * MAX_PATH);

	TCHAR szKeyTemp[MAX_PATH + GUID_STRING_SIZE];
	DWORD regSize = 0;
	DWORD pathSize = MAX_PATH;
	HKEY settingKey;
	LONG result;

	wsprintf(szKeyTemp, TEXT("CLSID\\%s\\Settings"), szGUID);
	result = RegOpenKeyEx(HKEY_CLASSES_ROOT, szKeyTemp, 0, KEY_READ, &settingKey);
	if (result != ERROR_SUCCESS) {
		MsgBoxError(TEXT("Unable to open registry key."));
		return E_FAIL;
	}

	result = RegQueryValueEx(settingKey, TEXT("Path"), NULL, NULL, NULL, &regSize);
	if (result == ERROR_SUCCESS) {
		bytesRequired += regSize+2;
	} else {
		MsgBoxError(TEXT("Cannot read path to executable."));
		RegCloseKey(settingKey);
		return E_FAIL;
	}

	result = RegQueryValueEx(settingKey, TEXT("Custom"), NULL, NULL, NULL, &regSize);
	if (result == ERROR_SUCCESS) {
		bytesRequired += regSize;
	}

	for (UINT i = 0; i < m_cbFiles; i++) {
		bytesRequired += DragQueryFile((HDROP)m_stgMedium.hGlobal, i, NULL, 0);
		bytesRequired += 3;
	}

	bytesRequired *= sizeof(TCHAR);
	pszCommand = (LPTSTR)CoTaskMemAlloc(bytesRequired);
	if (!pszCommand) {
		MsgBoxError(TEXT("Insufficient memory available."));
		RegCloseKey(settingKey);
		return E_FAIL;
	}
	*pszCommand = 0;

	regSize = (DWORD)MAX_PATH*sizeof(TCHAR);
	result = RegQueryValueEx(settingKey, TEXT("Path"), NULL, NULL, (LPBYTE)(szFilename), &regSize);
	szFilename[MAX_PATH-1] = 0;
	lstrcat(szNotepadExecutableFilename, TEXT("\""));
	lstrcat(szNotepadExecutableFilename, szFilename);
	lstrcat(szNotepadExecutableFilename, TEXT("\""));
	result = RegQueryValueEx(settingKey, TEXT("Custom"), NULL, NULL, (LPBYTE)(szCustom), &pathSize);
	if (result == ERROR_SUCCESS) {
		lstrcat(szNotepadExecutableFilename, TEXT(" "));
		lstrcat(szNotepadExecutableFilename, szCustom);
	}
	RegCloseKey(settingKey);

	// We have to open the files in batches. A command on the command-line can be at most
	// 2048 characters in XP and 32768 characters in Win7. In the degenerate case where all
	// paths are of length MAX_PATH, we can open at most x files at once, where:
	// 260 * (x + 2) = 2048 or 32768 <=> x = 5 or x = 124.
	// Note the +2 to account for the path to notepad++.exe.
	// http://stackoverflow.com/questions/3205027/maximum-length-of-command-line-string

	const UINT kiBatchSize = m_winVer > WINVER_XP ? 100 : 4;

	UINT iFileIndex = 0;
	while(iFileIndex < m_cbFiles) {
		memset(pszCommand, 0, bytesRequired);
		lstrcat(pszCommand, szNotepadExecutableFilename);
		for (UINT iBatchSizeCounter = 0; iFileIndex < m_cbFiles && iBatchSizeCounter < kiBatchSize; iBatchSizeCounter++) {
			DragQueryFile((HDROP)m_stgMedium.hGlobal, iFileIndex, szFilename, MAX_PATH);
			lstrcat(pszCommand, TEXT(" \""));
			lstrcat(pszCommand, szFilename);
			lstrcat(pszCommand, TEXT("\""));
			iFileIndex++;
		}

		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = (WORD)iShowCmd;	//SW_RESTORE;
		if (!CreateProcess (NULL, pszCommand, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
			DWORD errorCode = GetLastError();
			if (errorCode == ERROR_ELEVATION_REQUIRED) {	//Fallback to shellexecute
				CoInitializeEx(NULL, 0);
				HINSTANCE execVal = ShellExecute(NULL, TEXT("runas"), pszCommand, NULL, NULL, iShowCmd);
				CoUninitialize();
				if (execVal <= (HINSTANCE)32) {
					TCHAR * message = new TCHAR[512+bytesRequired];
					wsprintf(message, TEXT("ShellExecute failed (%d): Is this command correct?\r\n%s"), execVal, pszCommand);
					MsgBoxError(message);
					delete [] message;
				}
			} else {
				TCHAR * message = new TCHAR[512+bytesRequired];
				wsprintf(message, TEXT("Error in CreateProcess (%d): Is this command correct?\r\n%s"), errorCode, pszCommand);
				MsgBoxError(message);
				delete [] message;
			}
		}
	}



	CoTaskMemFree(pszCommand);
	return NOERROR;
}

STDMETHODIMP CShellExt::LoadShellIcon(int cx, int cy, HICON * phicon, int id) {
	HRESULT hr = E_OUTOFMEMORY;
	HICON hicon = NULL;

	if (m_useCustom) {
		hicon = (HICON)LoadImage(NULL, m_szCustomPath, IMAGE_ICON, cx, cy, LR_DEFAULTCOLOR|LR_LOADFROMFILE);
	}

	//Either no custom defined, or failed and use fallback
	if (hicon == NULL) {
		hicon = (HICON)LoadImage(_hModule, MAKEINTRESOURCE(id), IMAGE_ICON, cx, cy, LR_DEFAULTCOLOR);
	}

	if (hicon == NULL) {
		hr = E_OUTOFMEMORY;
		*phicon = NULL;
	} else {
		hr = S_OK;
		*phicon = hicon;
	}

	return hr;
}