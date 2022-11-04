#include "FileUtils.h"

BOOL IfDirExists(TCHAR* szDirPath)
{
	DWORD ftyp = GetFileAttributes(szDirPath);
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return FALSE;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return TRUE;   // this is a directory!

	return FALSE;    // this is not a directory!
}

BOOL GetPathByProcessId(DWORD dwProcessId, TCHAR *szFilePath, DWORD dwBufLen) {
	HANDLE hProcess = NULL;

	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId);
	if (NULL == hProcess) {
		return FALSE;
	}

	if (GetModuleFileNameEx(hProcess, NULL, szFilePath, dwBufLen) == 0) {
		CloseHandle(hProcess);
		return FALSE;
	}

	CloseHandle(hProcess);
	return TRUE;
}

BOOL GetNameByProcessId(DWORD dwProcessId, TCHAR* szFileName, DWORD dwBufLen) {
	HANDLE hProcess = NULL;

	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId);
	if (NULL == hProcess) {
		return FALSE;
	}

	if (GetModuleBaseName(hProcess, NULL, szFileName, dwBufLen) == 0) {
		CloseHandle(hProcess);
		return FALSE;
	}

	CloseHandle(hProcess);
	return TRUE;
}

BOOL GetFileNameFromHandle(HANDLE hFile, TCHAR *pszFilename, DWORD dwBufLen)
{
#define BUFSIZE 512
	BOOL bSuccess = FALSE;
	// TCHAR pszFilename[MAX_PATH + 1];
	HANDLE hFileMap;

	// Get the file size.
	DWORD dwFileSizeHi = 0;
	DWORD dwFileSizeLo = GetFileSize(hFile, &dwFileSizeHi);

	if (dwFileSizeLo == 0 && dwFileSizeHi == 0)
	{
		_tprintf(TEXT("Cannot map a file with a length of zero.\n"));
		return FALSE;
	}

	// Create a file mapping object.
	hFileMap = CreateFileMapping(hFile,
		NULL,
		PAGE_READONLY,
		0,
		1,
		NULL);

	if (hFileMap)
	{
		// Create a file mapping to get the file name.
		void* pMem = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 1);

		if (pMem)
		{
			if (GetMappedFileName(GetCurrentProcess(),
				pMem,
				pszFilename,
				MAX_PATH))
			{

				// Translate path with device name to drive letters.
				TCHAR szTemp[BUFSIZE];
				szTemp[0] = '\0';

				if (GetLogicalDriveStrings(BUFSIZE - 1, szTemp))
				{
					TCHAR szName[MAX_PATH];
					TCHAR szDrive[3] = TEXT(" :");
					BOOL bFound = FALSE;
					TCHAR* p = szTemp;

					do
					{
						// Copy the drive letter to the template string
						*szDrive = *p;

						// Look up each device name
						if (QueryDosDevice(szDrive, szName, MAX_PATH))
						{
							size_t uNameLen = _tcslen(szName);

							if (uNameLen < MAX_PATH)
							{
								bFound = _tcsnicmp(pszFilename, szName, uNameLen) == 0
									&& *(pszFilename + uNameLen) == _T('\\');

								if (bFound)
								{
									// Reconstruct pszFilename using szTempFile
									// Replace device path with DOS path
									TCHAR szTempFile[MAX_PATH];
									StringCchPrintf(szTempFile,
										MAX_PATH,
										TEXT("%s%s"),
										szDrive,
										pszFilename + uNameLen);
									StringCchCopyN(pszFilename, MAX_PATH + 1, szTempFile, _tcslen(szTempFile));
								}
							}
						}

						// Go to the next NULL character.
						while (*p++);
					} while (!bFound && *p); // end of string
				}
			}
			bSuccess = TRUE;
			UnmapViewOfFile(pMem);
		}

		CloseHandle(hFileMap);
	}
	_tprintf(TEXT("File name is %s\n"), pszFilename);
	return(bSuccess);
}

std::wstring toLowercase(std::wstring s) {
	// copy string
	std::wstring sInput = s;
	// transform
	std::transform(sInput.begin(), sInput.end(), sInput.begin(),
		[](TCHAR c) { return std::tolower(c); });
	return sInput;
}

std::string getAnsiString(TCHAR* szEvent) {
	std::string sEvent;
	if (_tcslen(szEvent) == 0) {
		return sEvent;
	}
#ifdef UNICODE
	int dwAnsiLen = WideCharToMultiByte(CP_UTF8, 0, szEvent, (int)_tcslen(szEvent), NULL, 0, NULL, NULL);
	char* szAnsiBuf = (char*)calloc(dwAnsiLen + 1, sizeof(char));
	if (NULL == szAnsiBuf) {
		printf("Failed to alloc %d bytes\n", dwAnsiLen);
		return sEvent;
	}
	WideCharToMultiByte(CP_UTF8, 0, szEvent, (int)_tcslen(szEvent), szAnsiBuf, dwAnsiLen, NULL, NULL);
	sEvent = szAnsiBuf;
	free(szAnsiBuf);
#else
	sEvent = szEvent;
#endif
	return sEvent;
}