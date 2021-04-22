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

#include <map>

extern const TCHAR* configFileName;

__declspec(selectany) std::map<std::string, std::string> Profile;

__declspec(selectany) std::map<std::string, std::string> localizefile;

__declspec(selectany) bool ProfDirty=false;

__declspec(selectany) TCHAR			g_IniFilePath[MAX_PATH]{0};

std::string* GetProfString(char* name);

std::string* GetLocalText(std::map<std::string, std::string> & m, char* name);

int GetProfInt(char* name, int defVal);

bool PutProfString(char* name, char* data);

bool PutProfInt(char* name, int val);

bool loadProf(TCHAR* path, const TCHAR* name);

void loadLanguge(TCHAR* path);

void saveProf(TCHAR* path, const TCHAR* name);
