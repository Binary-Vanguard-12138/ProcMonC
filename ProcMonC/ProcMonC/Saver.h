#pragma once

#include <Windows.h>
#include <sysinfoapi.h>
#include <tchar.h>
#include <string>

extern TCHAR g_szProcessDirPath[MAX_PATH], g_szFileDirPath[MAX_PATH];


void SaveProcessEvent(TCHAR* szEvent);
void SaveFileEvent(TCHAR* szEvent);
