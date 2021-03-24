#pragma once
#include "windows.h"

class DirectGripper
{
public:
	DirectGripper(HWND hParent){
		_hParent = hParent;
	}

	void show(bool doshow);

	void create();


//private:
	HWND _hSelf=0;
	HWND _hParent=0;

};