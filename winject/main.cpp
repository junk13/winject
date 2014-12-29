#include <iostream>

#include <Windows.h>
#include <TlHelp32.h>

typedef HINSTANCE(*fpLoadLibrary)(char*);

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		return 1;
	}
	char* library = argv[1];
	char* process = argv[2];

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	DWORD pid = NULL;

	if (!Process32First(snapshot, &entry))
	{
		return 1;
	}

	do
	{
		if (strcmp(entry.szExeFile, process))
		{
			pid = entry.th32ProcessID;
			break;
		}
	} while (Process32Next(snapshot, &entry));

	CloseHandle(snapshot);

	HANDLE hProcess;

	HINSTANCE hDll = LoadLibraryA("KERNEL32");

	fpLoadLibrary LoadLibraryAddr = (fpLoadLibrary)GetProcAddress(hDll, "LoadLibraryA");

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, pid);

	LPVOID paramAddr = VirtualAllocEx(hProcess, 0, strlen(library) + 1, MEM_COMMIT, PAGE_READWRITE);
	bool memoryWritten = WriteProcessMemory(hProcess, paramAddr, library, strlen(library) + 1, NULL);

	CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryAddr, paramAddr, 0, 0);

	CloseHandle(hProcess);

	return 0;
}
