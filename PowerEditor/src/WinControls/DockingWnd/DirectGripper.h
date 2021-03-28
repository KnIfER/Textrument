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

private:
	bool isRegistered=false;

};