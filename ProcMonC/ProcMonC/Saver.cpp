#include "Saver.h"
#include "FileUtils.h"

#define RECORD_INTERVAL_TICKS (60 * 1000)

tstring GetTimeString() {
	TCHAR szBuffer[0x40] = {};
	SYSTEMTIME st = {};
	GetLocalTime(&st);
	_sntprintf_s(szBuffer, _countof(szBuffer) - 1, _T("%02d%02d%02d_%02d%02d"), st.wYear - 2000, st.wMonth, st.wDay, st.wHour, st.wMinute);
	return szBuffer;
}

void SaveProcessEvent(TCHAR* szEvent) {
	static HANDLE s_hProcessEventFile = INVALID_HANDLE_VALUE;
	static ULONGLONG s_ullLastFileCreateTime = 0;
	static std::string sBanner = "EventName,ProcessId,ProcessName,Path,CommandLine,ParentProcessId,ParentProcessName,ParentPath\n";
	std::string sEvent = getAnsiString(szEvent);
	printf(sEvent.c_str());
	ULONGLONG ullNowTick = GetTickCount64();
	if (ullNowTick > s_ullLastFileCreateTime + RECORD_INTERVAL_TICKS) {
		if (INVALID_HANDLE_VALUE != s_hProcessEventFile) {
			CloseHandle(s_hProcessEventFile);
			s_hProcessEventFile = INVALID_HANDLE_VALUE;
		}
	}
	DWORD dwWrittenBytes = 0;
	if (INVALID_HANDLE_VALUE == s_hProcessEventFile) {
		TCHAR szFilePath[MAX_PATH] = {};
		_sntprintf_s(szFilePath, _countof(szFilePath) - 1, _T("%s\\process_%s.csv"), g_szProcessDirPath, GetTimeString().c_str());
		s_hProcessEventFile = CreateFile(szFilePath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == s_hProcessEventFile) {
			_tprintf(_T("Faild to create file %s, errno = %d\n"), szFilePath, GetLastError());
			return;
		}
		s_ullLastFileCreateTime = ullNowTick;
		if (!WriteFile(s_hProcessEventFile, sBanner.c_str(), (DWORD)sBanner.length(), &dwWrittenBytes, NULL)) {
			_tprintf_s(_T("Failed to write %llu bytes, errno=%d\n"), sBanner.length(), GetLastError());
			return;
		}
	}
	dwWrittenBytes = 0;
	if (!WriteFile(s_hProcessEventFile, sEvent.c_str(), (DWORD)sEvent.length(), &dwWrittenBytes, NULL)) {
		_tprintf_s(_T("Failed to write %llu bytes, errno=%d\n"), sEvent.length(), GetLastError());
		return;
	}
}

void SaveFileEvent(TCHAR* szEvent) {
	static HANDLE s_hFileEventFile = INVALID_HANDLE_VALUE;
	static ULONGLONG s_ullLastFileCreateTime = 0;
	static std::string sBanner = "EventName,FilePath,ProcessId,ProcessPath\n";
	std::string sEvent = getAnsiString(szEvent);
	// printf(sEvent.c_str());
	ULONGLONG ullNowTick = GetTickCount64();
	if (ullNowTick > s_ullLastFileCreateTime + RECORD_INTERVAL_TICKS) {
		if (INVALID_HANDLE_VALUE != s_hFileEventFile) {
			CloseHandle(s_hFileEventFile);
			s_hFileEventFile = INVALID_HANDLE_VALUE;
		}
	}
	DWORD dwWrittenBytes = 0;
	if (INVALID_HANDLE_VALUE == s_hFileEventFile) {
		TCHAR szFilePath[MAX_PATH] = {};
		_sntprintf_s(szFilePath, _countof(szFilePath) - 1, _T("%s\\file_%s.csv"), g_szFileDirPath, GetTimeString().c_str());
		s_hFileEventFile = CreateFile(szFilePath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == s_hFileEventFile) {
			_tprintf(_T("Faild to create file %s, errno = %d\n"), szFilePath, GetLastError());
			return;
		}
		s_ullLastFileCreateTime = ullNowTick;
		if (!WriteFile(s_hFileEventFile, sBanner.c_str(), (DWORD)sBanner.length(), &dwWrittenBytes, NULL)) {
			_tprintf_s(_T("Failed to write %llu bytes, errno=%d\n"), sBanner.length(), GetLastError());
			return;
		}
	}
	dwWrittenBytes = 0;
	if (!WriteFile(s_hFileEventFile, sEvent.c_str(), (DWORD)sEvent.length(), &dwWrittenBytes, NULL)) {
		_tprintf_s(_T("Failed to write %llu bytes, errno=%d\n"), sEvent.length(), GetLastError());
		return;
	}
}