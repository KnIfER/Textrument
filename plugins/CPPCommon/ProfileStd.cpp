/*
* Copyright 2020 Simple 'ini' Profile Parser.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <shlwapi.h>
#include "ProfileStd.h"

using namespace std;

string* GetProfString(char* name)
{
	auto idx = Profile.find(name);
	if(idx!=Profile.end())
	{		
		return &(*idx).second;
	}
	return 0;
}

int GetProfInt(char* name, int defVal)
{
	auto data=GetProfString(name);
	if(data)
	{
		auto & text=*data;
		size_t len = text.length();
		int number=0;
		bool intOpened=false;
		bool Negative=false;
		for(int i=0;i<len;i++) {
			int intVal = text[i]-'0';
			int valval = intVal>=0&&intVal<=9;
			if(!intOpened)
			{
				if(intOpened=valval)
				{
					Negative=i>0&&text[i-1]=='-';
				}
			}
			else if(!valval)
			{
				break;
			}
			if(intOpened)
			{
				number = number*10+intVal;
			}
		}
		if(intOpened)
		{
			if(Negative)
				number*=-1;
			return number;
		}
	}
	return defVal;
}

bool PutProfString(char* name, char* data)
{
	auto idx = Profile.find(name);
	if(idx!=Profile.end())
	{		
		auto & item=(*idx).second;
		if(item==data)
		{
			return 0;
		}
	}
	ProfDirty=1;
	Profile[name]=data;
	return 1;
}

bool PutProfInt(char* name, int val)
{
	char buffer[64]={0};
	itoa(val, buffer, 10);
	return PutProfString(name, buffer);
}

void loadProfile(TCHAR* path)
{
	char buffer[256]; int len, i;
	ifstream in(path, ios::in);
	if (in.is_open())
	{
		while (!in.eof())
		{
			in.getline(buffer,255);
			if(buffer[0]!='[')
			{
				for(int i=0, len = strlen(buffer);i<len;i++) {
					if(buffer[i]=='=')
					{
						buffer[i]='\0';
						Profile[buffer]=buffer+i+1;
					}
				}
			}
		}
		in.close();
	}
}

void loadProf(TCHAR* path, const TCHAR* name)
{
	if(Profile.size()==0)
	{
		lstrcpy(g_IniFilePath, path);
		PathAppend(g_IniFilePath, name);
		if (PathFileExists(g_IniFilePath))
		{
			loadProfile(g_IniFilePath);
		}
		auto len=lstrlen(g_IniFilePath);
		lstrcpy(g_IniFilePath+len, TEXT(".update"));
		if (PathFileExists(g_IniFilePath))
		{
			TCHAR* pathTmp=new TCHAR[MAX_PATH];
			lstrcpy(pathTmp, g_IniFilePath);
			loadProfile(pathTmp);
			PathRenameExtension(pathTmp, TEXT(".done"));
			MoveFile(g_IniFilePath, pathTmp);
			delete[] pathTmp;
			ProfDirty=true;
		}
	}
}

void saveProf(TCHAR* path, const TCHAR* name)
{
	if(ProfDirty)
	{
		lstrcpy(g_IniFilePath, path);
		PathAppend(g_IniFilePath, name);
		ofstream out(g_IniFilePath, ios::out);
		if (out.is_open())
		{
			for(auto val:Profile)
			{
				out << val.first << "=" << val.second << endl;
			}
		}
		out.flush();
		out.close();
		ProfDirty=false;
	}
}