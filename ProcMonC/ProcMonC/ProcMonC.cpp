// ProcMonC.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <tdh.h>
#include <stdio.h>
#include <tchar.h>
#include "TraceManager.h"
#include <memory>
#include <map>
#include "EventData.h"
#include "FileUtils.h"
#include "Saver.h"

TCHAR g_szProcessDirPath[MAX_PATH] = {}, g_szFileDirPath[MAX_PATH] = {};

std::map<DWORD, DWORD> g_mapProcess2Parent;

void OnProcessEvent(std::shared_ptr<EventData> data) {
	auto& wsEventName = data->GetEventName();
	TCHAR szProcessPath[MAX_PATH] = {}, szProcessName[MAX_PATH] = {}, szParentProcessPath[MAX_PATH] = {}, szParentProcessName[MAX_PATH] = {};
	std::wstring wsProcessPath, wsCommandLine, wsParentProcessName, wsParentProcessPath;
	DWORD dwProcessId = data->GetProcessId();
	auto& wsProcessName = data->GetProcessName();
	if (0 < dwProcessId) {
		if (GetPathByProcessId(dwProcessId, szProcessPath, MAX_PATH - 1)) {
			// wsProcessPath = szProcessPath;
		}
	}
	DWORD dwParentProcessId = 0;
	auto eventProperty = data->GetProperty(L"CommandLine");
	if (eventProperty) {
		wsCommandLine = eventProperty->GetUnicodeString();
	}

	eventProperty = data->GetProperty(L"ParentId");
	if (eventProperty) {
		dwParentProcessId = eventProperty->GetValue<DWORD>();
		if (0 < dwParentProcessId) {
			g_mapProcess2Parent[dwProcessId] = dwParentProcessId;
		}
	}
	if (0 == dwParentProcessId) {
		if (g_mapProcess2Parent.find(dwProcessId) != g_mapProcess2Parent.end()) {
			dwParentProcessId = g_mapProcess2Parent[dwProcessId];
		}
	}
	if (0 < dwParentProcessId) {
		if (GetNameByProcessId(dwParentProcessId, szParentProcessName, MAX_PATH - 1)) {
			// wsParentProcessName = szParentProcessName;
		}
		if (GetPathByProcessId(dwParentProcessId, szParentProcessPath, MAX_PATH - 1)) {
			// wsParentProcessPath = szParentProcessPath;
		}
	}
	size_t dwBufLen = wsEventName.length() + wsProcessName.length() + _tcslen(szProcessPath) + wsCommandLine.length() + _tcslen(szParentProcessName) + _tcslen(szParentProcessPath) + 0x100;

	TCHAR *szLineBuffer = (TCHAR*)calloc(dwBufLen, sizeof(TCHAR));
	if (NULL == szLineBuffer) {
		printf("Failed to allocate %zu bytes\n", dwBufLen);
		return;
	}
	_sntprintf_s(szLineBuffer, dwBufLen, dwBufLen - 1, _T("%s,%u,%s,%s,%s,%u,%s,%s\n"),
		Escape4Csv(wsEventName).c_str(),
		dwProcessId, 
		Escape4Csv(wsProcessName).c_str(),
		Escape4Csv(szProcessPath).c_str(),
		Escape4Csv(wsCommandLine).c_str(),
		dwParentProcessId,
		Escape4Csv(szParentProcessName).c_str(),
		Escape4Csv(szParentProcessPath).c_str());
	SaveProcessEvent(szLineBuffer);
	free(szLineBuffer);
}

std::map<ULONG64, std::wstring> g_mapFileObject2Path;

void OnFileEvent(std::shared_ptr<EventData> data) {
	auto& wsEventName = data->GetEventName();
	std::wstring wsFilePath;
	TCHAR szFilePath[MAX_PATH] = {}, szProcessPath[MAX_PATH] = {};
	ULONG64 hFileObject = 0, hFileKey = 0;
	DWORD dwProcessId = data->GetProcessId();
	if (GetPathByProcessId(dwProcessId, szProcessPath, MAX_PATH - 1)) {
	}

	EventProperty* eventProperty = NULL;
	auto& wsProcessName = data->GetProcessName();
	wsFilePath = data->GetFileName();
	if (wsFilePath.empty()) {
		eventProperty = (EventProperty*)data->GetProperty(L"OpenPath");
		if (eventProperty) {
			wsFilePath = eventProperty->GetUnicodeString();
		}
	}
	if (wsFilePath.empty()) {
		eventProperty = (EventProperty*)data->GetProperty(L"FileName");
		if (eventProperty) {
			wsFilePath = eventProperty->GetUnicodeString();
		}
	}
	eventProperty = (EventProperty*)data->GetProperty(L"FileObject");
	if (eventProperty) {
		hFileObject = eventProperty->GetValue<ULONG64>();
	}

	eventProperty = (EventProperty*)data->GetProperty(L"FileKey");
	if (eventProperty) {
		hFileKey = eventProperty->GetValue<ULONG64>();
	}

	// If succeed to get file name, store it in the dictionary.
	if (!wsFilePath.empty()) {
		if (0 < hFileObject) {
			g_mapFileObject2Path[hFileObject] = wsFilePath;
		}
		if (0 < hFileKey) {
			g_mapFileObject2Path[hFileKey] = wsFilePath;
		}
	}
	// If fail to get file name, load it from dictionary
	if (wsFilePath.empty()) {
		if (0 < hFileObject && g_mapFileObject2Path.find(hFileObject) != g_mapFileObject2Path.end()) {
			wsFilePath = g_mapFileObject2Path[hFileObject];
		}
		else if (0 < hFileKey && g_mapFileObject2Path.find(hFileKey) != g_mapFileObject2Path.end()) {
			wsFilePath = g_mapFileObject2Path[hFileKey];
		}
		else {
			if (0 < hFileObject) {
				if (0 < GetFinalPathNameByHandle((HANDLE)hFileObject, szFilePath, MAX_PATH - 1, FILE_NAME_NORMALIZED)) {
					// Always fails, don't know reason.
					wsFilePath = szFilePath;
				}
			}
			if (wsFilePath.empty()) {
				if (0 < hFileKey) {
					if (0 < GetFinalPathNameByHandle((HANDLE)hFileKey, szFilePath, MAX_PATH - 1, FILE_NAME_NORMALIZED)) {
						// Always fails, don't know reason.
						wsFilePath = szFilePath;
					}
				}
			}
		}
	}
	if (wsEventName == L"FileIo/QueryInfo"
		/*|| data->GetEventDescriptor().Opcode == 74*/) {
		if (4 < wsFilePath.length() && toLowercase(wsFilePath.substr(wsFilePath.length() - 4)) == L".dll") {
			// Except DLL files
			return;
		}
		size_t dwBufLen = wsEventName.length() + wsFilePath.length() + _tcslen(szProcessPath) + 0x100;
		TCHAR* szLineBuffer = (TCHAR*)calloc(dwBufLen, sizeof(TCHAR));
		if (NULL == szLineBuffer) {
			printf("Failed to allocate %zu bytes\n", dwBufLen);
			return;
		}
		if (wsFilePath.empty()) {
			_sntprintf_s(szLineBuffer, dwBufLen, dwBufLen - 1, _T("%s,%p,%u,%s\n"), Escape4Csv(wsEventName).c_str(), (HANDLE)hFileObject, dwProcessId, Escape4Csv(szProcessPath).c_str());
			// Not save into file which has no filename.
		}
		else {
			_sntprintf_s(szLineBuffer, dwBufLen, dwBufLen - 1, _T("%s,%s,%u,%s\n"), Escape4Csv(wsEventName).c_str(), Escape4Csv(wsFilePath).c_str(), dwProcessId, Escape4Csv(szProcessPath).c_str());
			SaveFileEvent(szLineBuffer);
		}
		free(szLineBuffer);
	}
}

void OnEvent(std::shared_ptr<EventData> data) {
	if (GetCurrentProcessId() == data->GetProcessId()) {
		return;
	}
	const GUID* pGuid = data->GetGUIDPtr();
	if (memcmp(pGuid, &ProcessGuid, sizeof(GUID)) == 0) {
		OnProcessEvent(data);
		return;
	}

	if (memcmp(pGuid, &DiskIoGuid, sizeof(GUID)) == 0 || memcmp(pGuid, &FileIoGuid, sizeof(GUID)) == 0) {
		OnFileEvent(data);
		return;
	}
}

TraceManager* g_pProcessMgr = NULL;
HANDLE g_hProcessEvent = INVALID_HANDLE_VALUE;

int _tmain(int argc, const TCHAR* argv[]) {
	TraceManager tmProcess, tmFile;
	TCHAR szModulePath[MAX_PATH] = {}, szOutputDirPath[MAX_PATH] = {};
	printf("Starting ProcessMonitorConsole...\nPress Ctrl+C to stop.\n");
	::GetModuleFileName(NULL, szModulePath, MAX_PATH - 1);
	*(_tcsrchr(szModulePath, _T('\\'))) = 0;
	_sntprintf_s(szOutputDirPath, MAX_PATH - 1, _T("%s\\output"), szModulePath);
	_sntprintf_s(g_szProcessDirPath, MAX_PATH - 1, _T("%s\\process"), szOutputDirPath);
	_sntprintf_s(g_szFileDirPath, MAX_PATH - 1, _T("%s\\file"), szOutputDirPath);
	if (!IfDirExists(szOutputDirPath)) {
		if (!CreateDirectory(szOutputDirPath, NULL)) {
			_tprintf(_T("Failed to create directory %s\n"), szOutputDirPath);
			return EXIT_FAILURE;
		}
	}
	if (!IfDirExists(g_szProcessDirPath)) {
		if (!CreateDirectory(g_szProcessDirPath, NULL)) {
			_tprintf(_T("Failed to create directory %s\n"), g_szProcessDirPath);
			return EXIT_FAILURE;
		}
	}
	if (!IfDirExists(g_szFileDirPath)) {
		if (!CreateDirectory(g_szFileDirPath, NULL)) {
			_tprintf(_T("Failed to create directory %s\n"), g_szFileDirPath);
			return EXIT_FAILURE;
		}
	}

	tmProcess.AddKernelEventTypes({ KernelEventTypes::Process | KernelEventTypes::FileIO | KernelEventTypes::DiskFileIO });

	g_pProcessMgr = &tmProcess;
	g_hProcessEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);

	if (!tmProcess.Start(OnEvent)) {
		printf("Failed to start process session");
		return EXIT_FAILURE;
	}

	::SetConsoleCtrlHandler([](auto type) {
		if (type == CTRL_C_EVENT) {
			g_pProcessMgr->Stop();
			::SetEvent(g_hProcessEvent);
			return TRUE;
		}
		return FALSE;
		}, TRUE);

	if (g_hProcessEvent) {
		::WaitForSingleObject(g_hProcessEvent, INFINITE);
		::CloseHandle(g_hProcessEvent);
	}

	return 0;
}

