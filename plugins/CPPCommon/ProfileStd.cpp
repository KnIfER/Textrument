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

extern HWND mainAppWnd;

string* GetProfString(char* name)
{
	auto idx = Profile.find(name);
	if(idx!=Profile.end())
	{		
		return &(*idx).second;
	}
	return 0;
}

string* GetLocalText(std::map<std::string, std::string> & m, char* name)
{
	if(!m.size())
	{
		return NULL;
	}
	auto idx = m.find(name);
	if(idx!=m.end())
	{		
		return &(*idx).second;
	}
	return NULL;
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

void loadProfile(TCHAR* path, std::map<std::string, std::string> & m, bool skipSpace)
{
	char buffer[256]; int len, i;
	ifstream in(path, ios::in);
	int error_code=0;
	int curr=-1;
	if (in.is_open())
	{
		while (!in.eof())
		{
			if(in.getline(buffer,255).fail())
			{
				// break dead loop
				if(!in.eof()) error_code=-1;
				break;
			}
			curr = in.cur;
			buffer[255] = '\0';
			if(buffer[0]!='[')
			{
				for(int i=0, len = strlen(buffer);i<len;i++) {
					if(buffer[i]=='=')
					{
						buffer[i]='\0';
						if(skipSpace)
						{
							for(int j=i-1;j>=0;j--) {
								if(buffer[j]==' ')
									buffer[j]='\0';
								else break;
							}
							for(int j=i+1;j<len;j++) {
								if (j==i+1 && buffer[j]=='\\')
								{
									i++;
									break;
								} 
								if(buffer[j]==' ')
									i++;
								else break;
							}
						}
						m[buffer]=buffer+i+1;
						break;
					}
				}
			}
		}
		in.close();
	}
	if(error_code==-1) 
	{
		int len = WideCharToMultiByte(CP_ACP, 0, configFileName, lstrlen(configFileName), buffer, 255, 0, 0);
		buffer[len]='\0';
		::MessageBoxA(mainAppWnd, buffer, ("INI Parse Error!"), MB_OK);
	}
}

bool loadProf(TCHAR* path, const TCHAR* name)
{
	if(Profile.size()==0)
	{
		lstrcpy(g_IniFilePath, path);
		PathAppend(g_IniFilePath, name);
		if (PathFileExists(g_IniFilePath))
		{
			loadProfile(g_IniFilePath, Profile, false);
			return true;
		}
		//auto len=lstrlen(g_IniFilePath);
		//lstrcpy(g_IniFilePath+len, TEXT(".update"));
		//if (PathFileExists(g_IniFilePath))
		//{
		//	TCHAR* pathTmp=new TCHAR[MAX_PATH];
		//	lstrcpy(pathTmp, g_IniFilePath);
		//	loadProfile(pathTmp);
		//	PathRenameExtension(pathTmp, TEXT(".done"));
		//	MoveFile(g_IniFilePath, pathTmp);
		//	delete[] pathTmp;
		//	ProfDirty=true;
		//}
	}
	return false;
}

void loadLanguge(TCHAR* path)
{
	localizefile.clear();
	if (PathFileExists(path))
	{
		loadProfile(path, localizefile, true);
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