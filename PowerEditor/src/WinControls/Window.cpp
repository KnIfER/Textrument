#include "Window.h"


void Window::reSizeTo(RECT & rc){
	::MoveWindow(_hSelf, rc.left, rc.top, rc.right, rc.bottom, 1);
	//redraw(0);
	UpdateWindow(_hSelf);
}