#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <Psapi.h>
#include <Tlhelp32.h>

using namespace std;


int write_memory(string information) {
	HANDLE hData = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 1024 * 1024, "data");
	if (hData == NULL) {
		printf("Cannot create file mapping. Error code: %d", GetLastError());
		return 0;
	}

	unsigned char* pData = (unsigned char*)MapViewOfFile(hData, FILE_MAP_WRITE, 0, 0, 0);
	if (pData == NULL) {
		printf("Cannot get pointer to file mapping. Error code: %d", GetLastError());
		CloseHandle(hData);
		return 0;
	}
	
	memcpy(pData, information.c_str(), information.length()+1);
	printf("Scriere in memorie terminata, te rog nu inchide procesul.");
	getchar();
	CloseHandle(hData);
}



void ps_command()
{
	HANDLE process_handle;
	string information = string();
	PROCESSENTRY32 infos;
	infos.dwSize = sizeof(PROCESSENTRY32);
	process_handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (process_handle == INVALID_HANDLE_VALUE)
	{
		printf("Nu se poate executa ps.Eroare: %d.\n", GetLastError());
		return;
	}
	bool ok;
	ok = Process32First(process_handle, &infos);
	if (ok == 0)
	{
		printf("Eroare la ProcessFirst: %d.\n", GetLastError());
		CloseHandle(process_handle);
		return;
	}

	do
	{
		char procInfo[100];
		sprintf(procInfo, "%d-%d-%s|", infos.th32ProcessID,infos.th32ParentProcessID,infos.szExeFile);
		information.append(procInfo);
	} while (Process32Next(process_handle, &infos) != 0);

	write_memory(information);
	CloseHandle(process_handle);
}

int main()
{
	ps_command();
	return 0;
}