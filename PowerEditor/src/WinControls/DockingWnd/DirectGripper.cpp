/* Copyright 2014 bapijun
*
* Licensed under the Apache License, Version 2.0 (the "License"); 
* you may not use this file except in compliance with the License. 
* You may obtain a copy of the License at
* 
* http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, 
* software distributed under the License is distributed on an "AS IS" BASIS, 
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
* See the License for the specific language governing permissions and limitations under the License.
* 
* from 
* // draw a transparent window
* // https://blog.csdn.net/u011822516/article/details/41946631
*/

#include "DirectGripper.h"


#define WS_EX_LAYERED           0x00080000
#define LWA_COLORKEY            0x00000001
#define LWA_ALPHA               0x00000002

#define kClassWindow L"DockingPrev"

typedef BOOL (WINAPI *TRANFUNC)(HWND,COLORREF,BYTE,DWORD);

class CTransparent  
{
public:
    static BOOL SetTransparentWnd(HWND hWnd, COLORREF crKey, UINT cAlpha, BOOL dwFlags);
    static BOOL FreeUser32DLL();
    static BOOL ChangeTransparency(HWND hWnd,COLORREF crKey, UINT bAlpha, DWORD dwFlags);
    static BOOL LoadUser32DLL();

    static TRANFUNC m_pFunc;
    static HINSTANCE m_hInstance;
    static BOOL initialized;

};

HINSTANCE CTransparent::m_hInstance = NULL;
TRANFUNC  CTransparent::m_pFunc = NULL;
BOOL CTransparent::initialized = FALSE;

BOOL CTransparent::LoadUser32DLL()
{
    BOOL bRet = FALSE;
    if (!initialized)
    {
        m_hInstance = LoadLibrary(L"User32.DLL");
        if(m_hInstance ) 
        { 
            //取得SetLayeredWindowAttributes函数指针 
            m_pFunc=(TRANFUNC)GetProcAddress(m_hInstance, "SetLayeredWindowAttributes");
            if(m_pFunc != NULL)
            {
                initialized = TRUE;
                bRet = TRUE;
            }
        }
    }
    return bRet ;
}

BOOL CTransparent::FreeUser32DLL()
{   
    if(m_hInstance != NULL)
    {  
        FreeLibrary(m_hInstance); 
        if(m_pFunc != NULL) //置空
            m_pFunc = NULL ;
        if(initialized != NULL)       //置空
            initialized  = NULL ;
    }
    return TRUE;
}

BOOL CTransparent::ChangeTransparency(HWND hWnd, COLORREF crKey, UINT bAlpha, DWORD dwFlags)
{
    //透明度Sp的值是0-255 
    //其中0为完全透明应该避免出现
    BOOL bRet = FALSE;
    if (m_pFunc == NULL)
        LoadUser32DLL();    

    if(m_pFunc != NULL)
    {
        m_pFunc(hWnd, crKey, bAlpha, dwFlags); 
        bRet = TRUE;   
    }
    return bRet;
}

BOOL CTransparent::SetTransparentWnd(HWND hWnd, COLORREF crKey, UINT cAlpha, BOOL dwFlags)
{
    BOOL bRet = TRUE;   

    //要使使窗体拥有透明效果,首先要有WS_EX_LAYERED扩展属性 WS_EX_LAYERED = 0x80000 
    //SetWindowLong(this->GetSafeHwnd(),GWL_EXSTYLE,GetWindowLong(this->GetSafeHwnd(),GWL_EXSTYLE)^0x80000);
    //SetWindowLong(hWnd,  GWL_EXSTYLE , GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);  

    if(dwFlags)
    {
        ChangeTransparency(hWnd, crKey, cAlpha, LWA_ALPHA); //设置cAlpha值0-255
    } 
    else
    {
        ChangeTransparency(hWnd, crKey, cAlpha, LWA_COLORKEY);    //crkey相同色变成透明
    }

    return bRet;
}


LRESULT WINAPI transparentWndProc(
	__in HWND hWnd,
	__in UINT msg,
	__in WPARAM wParam,
	__in LPARAM lParam)
{
    if (msg==WM_NCCREATE)
    {
        return true;
    }
    if (msg==WM_ACTIVATE )
    {
        return true;
    }
    if (msg==WM_PAINT)
    {
        //if (WS_EX_LAYERED == (WS_EX_LAYERED & GetWindowLong(hWnd, GWL_EXSTYLE))) break;;

        RECT rcClient;
        ::GetClientRect(hWnd, &rcClient);

        PAINTSTRUCT ps = { 0 };
        HDC hdc = ::BeginPaint(hWnd, &ps);

        RECT rect = rcClient;  

        //rect.right = rect.left+(rect.right-rect.left)/2;

       HBRUSH hbrush = CreateSolidBrush(0xafafaf);
        


        FillRect(hdc, &rect, hbrush);


        ::EndPaint(hWnd, &ps);
        return 1;
    }
    if (msg==WM_NCHITTEST)
    {
        return HTTRANSPARENT;
    }
    //if (msg==WM_NCACTIVATE)
    //{
    //
    //    return (wParam == 0) ? TRUE : FALSE;
    //}
    //return true;
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

void DirectGripper::show(bool doshow)
{
    if(!IsWindow(_hSelf))
        create();
    ::ShowWindow(_hSelf, doshow?SW_SHOWNA:SW_HIDE);
}

void DirectGripper::create()
{
    if (!isRegistered)
    {
        LPCTSTR lpcsClassName = kClassWindow;
        DWORD dwStyle = CS_HREDRAW | CS_VREDRAW;
        // regWndClass
        WNDCLASS wndclass = { 0 };

        wndclass.style = dwStyle;
        wndclass.lpfnWndProc = transparentWndProc;
        wndclass.cbClsExtra = 200;
        wndclass.cbWndExtra = 200;
        wndclass.hInstance = ::GetModuleHandle(NULL);
        wndclass.hIcon = NULL;
        //wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
        wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wndclass.lpszMenuName = NULL;
        wndclass.lpszClassName = lpcsClassName;

        ::RegisterClass(&wndclass);
        isRegistered = true;
    }

    // Windows of style WS_EX_NOACTIVATE still require focus when it's first created.

    _hSelf = ::CreateWindowEx(WS_EX_NOACTIVATE|WS_EX_LAYERED , kClassWindow , NULL
        , WS_CHILD |WS_POPUP|WS_VISIBLE , 0 , 0 , 0 , 0 , _hParent , NULL , ::GetModuleHandle(NULL), NULL);

    //SetWindowLongPtr(_hSelf, GWLP_USERDATA, (LONG_PTR)this);

    //在这里注册一个窗口第三个参数表示窗口的类型
    CTransparent::SetTransparentWnd((HWND)_hSelf, 0xffff3333, 100, 1); //设置窗口透明
}