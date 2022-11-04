#pragma once

#include <Windows.h>
#include <psapi.h> // For access to GetModuleFileNameEx
#include <strsafe.h>
#include <tchar.h>
#include <string>
#include <algorithm>
#include <iostream>

#ifdef UNICODE
#define tstring std::wstring
#else
#define tstring std::string
#endif

BOOL IfDirExists(TCHAR* szDirPath);
BOOL GetPathByProcessId(DWORD dwProcessId, TCHAR* szFilePath, DWORD dwBufLen);
BOOL GetNameByProcessId(DWORD dwProcessId, TCHAR* szFilePath, DWORD dwBufLen);
BOOL GetFileNameFromHandle(HANDLE hFile, TCHAR* pszFilename, DWORD dwBufLen);
std::wstring toLowercase(std::wstring s);
std::string getAnsiString(TCHAR* szEvent);
tstring Escape4Csv(tstring s);

