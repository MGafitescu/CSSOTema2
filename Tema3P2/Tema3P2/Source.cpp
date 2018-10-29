#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <string>
#include <Psapi.h>
#include <Tlhelp32.h>

using namespace std;

struct InfoProces {
	DWORD pid;
	DWORD ppid;
	char  exeName[256];
};

struct InfoProces processes[20000];
int processesNumber;
int pids[20000];
int tree_pid[1024];

void add_process(string process, int count) {
	string delimiter = "-";
	size_t pos = 0;
	string name;
	int pid;
	int ppid;

	pos = process.find(delimiter);
	pid = stoi(process.substr(0, pos));
	process.erase(0, pos + delimiter.length());

	pos = process.find(delimiter);
	ppid = stoi(process.substr(0, pos));
	process.erase(0, pos + delimiter.length());

	pos = process.find(delimiter);
	name = process.substr(0, pos);
	process.erase(0, pos + delimiter.length());
	processes[count].pid = pid;
	processes[count].ppid = ppid;
	strcpy(processes[count].exeName, name.c_str());
	
}

void parse_data(string data) {
	string delimiter = "|";
	size_t pos = 0;
	string token;
	int count = -1;
	while ((pos = data.find(delimiter)) != string::npos) {
		count++;
		token = data.substr(0, pos);
		add_process(token, count);
		data.erase(0, pos + delimiter.length());
	}
	processesNumber = count;
}


void print_command() {
	HANDLE hData = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "data");
		if (hData == NULL) {
			printf("Cannot open file mapping. Error code: %d", GetLastError());
			return;
		}

		char* pData = (char*)MapViewOfFile(hData, FILE_MAP_WRITE, 0, 0, 0);
		if (pData == NULL) {
			printf("Cannot get pointer to file mapping. Error code: %d", GetLastError());
			CloseHandle(hData);
			return;
		}
		parse_data(string(pData));
}

bool pid_in_list(int pid) {
	for (int i = 0; i < processesNumber; i++) {
		if (processes[i].pid == pid)
			return true;
	}
	return false;
}

void parent(int pid, int indent)
{
	for(int i = 0; i < processesNumber; i++)
	{
		if (pid == processes[i].ppid && pids[processes[i].pid] == 0)
		{
			for (int i = 0; i < indent; i++)
				printf("\t");
			printf("PID: %d\t PPID:%d\t%s\n", processes[i].pid, processes[i].ppid, processes[i].exeName);
			pids[processes[i].pid] = 1;
			if (processes[i].pid != processes[i].ppid)
				parent(processes[i].pid, indent + 1);
		}
	} 

}

void ps_tree_command()
{
	int count = -1;

	for (int i = 0; i < processesNumber; i++) {
		if (pids[processes[i].pid] == 0) {
			count++;
			tree_pid[count] = processes[i].pid;
			printf("\n[Arbore nr. %d]\nPID: %d\t PPID:%d\t%s\n",count, processes[i].pid, processes[i].ppid, processes[i].exeName);
			parent(processes[i].pid, 1);
		}
		
	}

}

void killp_command(int pid)
{
	HANDLE process_handle;
	process_handle = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
	if (process_handle == NULL)
	{
		printf("Nu se poate deschide procesul %d.Eroare:%d \n", pid, GetLastError());
		return;
	}
	bool terminated;
	terminated = TerminateProcess(process_handle, 0);
	if (terminated == 0)
	{
		printf("Nu se poate termina procesul %d.Eroare:%d \n", pid, GetLastError());
		CloseHandle(process_handle);
		return;
	}
	printf("Procesul %d a fost terminat cu succes.\n", pid);
	CloseHandle(process_handle);
}

void subtree(int pid)
{
	for (int i = 0; i < processesNumber; i++)
	{
		if (pid == processes[i].ppid)
		{
			subtree(processes[i].pid);
		}
	}
	killp_command(pid);
}


void killt_command(int pid)
{

	for (int i = 0; i < processesNumber; i++)
	{
		if (pid == processes[i].pid)
		{
			subtree(pid);
		}

	} 

	printf("Arbore terminat cu succes.\n");
}


int main()
{
	print_command();
	ps_tree_command();
	printf("Tasteaza numarul unui arbore:");
	int id;
	scanf("%d", &id);
	killt_command(tree_pid[id]);
	Sleep(1000);
	getchar();
	return 0;
}