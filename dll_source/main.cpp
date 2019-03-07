// Copyright (c) 2017-2019 hors<horsicq@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#include <windows.h>
#include <winternl.h>
#include <Tlhelp32.h>

#ifdef _WIN64
#define __X64 1
#endif

typedef struct _CLIENT_ID
{
	PVOID UniqueProcess;
	PVOID UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

typedef enum _KWAIT_REASON
{
	Executive = 0,
	FreePage = 1,
	PageIn = 2,
	PoolAllocation = 3,
	DelayExecution = 4,
	Suspended = 5,
	UserRequest = 6,
	WrExecutive = 7,
	WrFreePage = 8,
	WrPageIn = 9,
	WrPoolAllocation = 10,
	WrDelayExecution = 11,
	WrSuspended = 12,
	WrUserRequest = 13,
	WrEventPair = 14,
	WrQueue = 15,
	WrLpcReceive = 16,
	WrLpcReply = 17,
	WrVirtualMemory = 18,
	WrPageOut = 19,
	WrRendezvous = 20,
	Spare2 = 21,
	Spare3 = 22,
	Spare4 = 23,
	Spare5 = 24,
	WrCalloutStack = 25,
	WrKernel = 26,
	WrResource = 27,
	WrPushLock = 28,
	WrMutex = 29,
	WrQuantumEnd = 30,
	WrDispatchInt = 31,
	WrPreempted = 32,
	WrYieldExecution = 33,
	WrFastMutex = 34,
	WrGuardedMutex = 35,
	WrRundown = 36,
	MaximumWaitReason = 37
} KWAIT_REASON;

typedef struct _THREAD_BASIC_INFORMATION {
	NTSTATUS                ExitStatus;
	PVOID                   TebBaseAddress;
	CLIENT_ID               ClientId;
	KAFFINITY               AffinityMask;
	LONG                    Priority;
	LONG                    BasePriority;

} THREAD_BASIC_INFORMATION, *PTHREAD_BASIC_INFORMATION;


typedef struct _SYSTEM_THREAD {
	LARGE_INTEGER           KernelTime;
	LARGE_INTEGER           UserTime;
	LARGE_INTEGER           CreateTime;
	ULONG                   WaitTime;
	PVOID                   StartAddress;
	CLIENT_ID               ClientId;
	LONG	                Priority;
	LONG                    BasePriority;
	ULONG                   ContextSwitchCount;
	ULONG                   State;
	KWAIT_REASON            WaitReason;
} SYSTEM_THREAD, *PSYSTEM_THREAD;

#define OSVERSION_UNKNOWN 0
#define OSVERSION_WIN7_SP1 0x00000010
#define OSVERSION_WIN7_RTM 2
#define OSVERSION_WIN7_RC 3
#define OSVERSION_WIN7_BETA 4

#define IOCTL_OPENPROCESS			CTL_CODE(FILE_DEVICE_UNKNOWN,0x801,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_CLOSEPROCESS			CTL_CODE(FILE_DEVICE_UNKNOWN,0x802,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_READPROCESSMEMORY		CTL_CODE(FILE_DEVICE_UNKNOWN,0x803,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_WRITEPROCESSMEMORY	CTL_CODE(FILE_DEVICE_UNKNOWN,0x804,METHOD_BUFFERED,FILE_ANY_ACCESS)

#define IOCTL_GETNUMBEROFTHREADS	CTL_CODE(FILE_DEVICE_UNKNOWN,0x805,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_GETEPROCESS			CTL_CODE(FILE_DEVICE_UNKNOWN,0x806,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_GETKPCR				CTL_CODE(FILE_DEVICE_UNKNOWN,0x807,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_GETETHREADS			CTL_CODE(FILE_DEVICE_UNKNOWN,0x808,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_GETETHREAD			CTL_CODE(FILE_DEVICE_UNKNOWN,0x809,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_GETKPCRS				CTL_CODE(FILE_DEVICE_UNKNOWN,0x80A,METHOD_BUFFERED,FILE_ANY_ACCESS)

struct PROCESSMEMORY
{
	void *pProcessHandle;
	void *pMemoryAddress;
	int nMemorySize;
};

typedef NTSTATUS (NTAPI *pfnNtQueryInformationProcess)(
	IN  HANDLE ProcessHandle,
	IN  PROCESSINFOCLASS ProcessInformationClass,
	OUT PVOID ProcessInformation,
	IN  ULONG ProcessInformationLength,
	OUT PULONG ReturnLength    OPTIONAL
	);

typedef NTSTATUS (NTAPI *pfnNtQuerySystemInformation)(
	__in       SYSTEM_INFORMATION_CLASS SystemInformationClass,
	__inout    PVOID SystemInformation,
	__in       ULONG SystemInformationLength,
	__out_opt  PULONG ReturnLength
	);

typedef NTSTATUS (NTAPI *pfnNtQueryInformationThread)(
	__in       HANDLE ThreadHandle,
	__in       THREADINFOCLASS ThreadInformationClass,
	__inout    PVOID ThreadInformation,
	__in       ULONG ThreadInformationLength,
	__out_opt  PULONG ReturnLength
	);

typedef bool (*pfnIsWow64Process)(
	_In_   HANDLE hProcess,
	_Out_  PBOOL Wow64Process
	);

struct __PROCESSINFO
{
	int nPID;
	int nParentPID;
	WCHAR wszExeName[256];
};

SC_HANDLE hSCManager=0;
SC_HANDLE hService=0;
HANDLE hDevice=0;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
extern "C" BOOL __declspec(dllexport) __cdecl _SetPrivilege(char* SeNamePriv, BOOL EnableTF)
{
	HANDLE hToken;
	LUID SeValue;
	TOKEN_PRIVILEGES tp;

	if (!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,&hToken))
	{
		return FALSE;
	}

	if (!LookupPrivilegeValueA(NULL, SeNamePriv, &SeValue)) 
	{
		CloseHandle(hToken);
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = SeValue;
	tp.Privileges[0].Attributes = EnableTF ? SE_PRIVILEGE_ENABLED : 0;

	AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);

	CloseHandle(hToken);
	return TRUE;
}
extern "C" int __declspec(dllexport) __cdecl __GetProcesses(char* pBuffer, int nBufferSize)
{
	int nResult=0;
	__PROCESSINFO *pi=(__PROCESSINFO *)pBuffer;
	PROCESSENTRY32W pe32;
	HANDLE hProcesses=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);

	if(hProcesses!=INVALID_HANDLE_VALUE)
	{
		pe32.dwSize=sizeof(PROCESSENTRY32W);

		if(Process32FirstW(hProcesses,&pe32))
		{
			// number of processes
			do 
			{
				if((nResult+(int)sizeof(__PROCESSINFO))<=nBufferSize)
				{
					pi->nPID=pe32.th32ProcessID;
					pi->nParentPID=pe32.th32ParentProcessID;
					lstrcpyW(pi->wszExeName,pe32.szExeFile);
					
					pi++;
					nResult+=sizeof(__PROCESSINFO);
				}
				else
				{
					nResult=0;
					break;
				}
				//nResult+=sizeof(__PROCESSINFO);
			} 
			while (Process32NextW(hProcesses,&pe32));
		}

		CloseHandle(hProcesses);
	}

	return nResult;
}
extern "C" PVOID __declspec(dllexport) __cdecl um_GetPEB(void *nProcessHandle)
{
	int nTemp;
	_PROCESS_BASIC_INFORMATION pbi;
	pfnNtQueryInformationProcess gNtQueryInformationProcess;

	HMODULE hNtDll=LoadLibrary(TEXT("ntdll.dll"));
	if(hNtDll==NULL)return NULL;

	gNtQueryInformationProcess=(pfnNtQueryInformationProcess)GetProcAddress(hNtDll,"NtQueryInformationProcess");


	if(gNtQueryInformationProcess((HANDLE)nProcessHandle,ProcessBasicInformation,&pbi,sizeof(pbi),(PULONG)&nTemp)==ERROR_SUCCESS)
	{
		return (HANDLE)pbi.PebBaseAddress;
	}

	return 0;

}
//extern "C" int __declspec(dllexport) __cdecl um_GetNumberOfThreads(int nProcessID)
//{
//	int nTemp,nNumberOfThreads=0;
//	pfnNtQuerySystemInformation gNtQuerySystemInformation;
//	SYSTEM_PROCESS_INFORMATION *pOffset;
//	void *pMemory;
//
//	if(GetCurrentProcessId()==nProcessID)
//	{
//		return 1;
//	}
//
//	pMemory=GlobalAlloc(GMEM_FIXED+GMEM_ZEROINIT,200000);
//	pOffset=(SYSTEM_PROCESS_INFORMATION *)pMemory;
//
//	HMODULE hNtDll=LoadLibrary(TEXT("ntdll.dll"));
//	if(hNtDll==NULL)
//	{
//		return NULL;
//	}
//
//	
//
//	gNtQuerySystemInformation=(pfnNtQuerySystemInformation)GetProcAddress(hNtDll,"NtQuerySystemInformation");
//
//
//	if(gNtQuerySystemInformation(SystemProcessInformation,(void *)pMemory,200000,(PULONG)&nTemp)==ERROR_SUCCESS)
//	{
//		while(pOffset->NextEntryOffset)
//		{
//			if(pOffset->UniqueProcessId==(HANDLE)nProcessID)
//			{
//				nNumberOfThreads=*(int *)((int)pOffset+4);
//				GlobalFree(pMemory);
//
//				return nNumberOfThreads;
//			}
//			pOffset=(SYSTEM_PROCESS_INFORMATION *)((int)pOffset+pOffset->NextEntryOffset);
//		}
//	}
//
//	GlobalFree(pMemory);
//
//	return 0;
//
//}
extern "C" PVOID __declspec(dllexport) __cdecl um_GetTEB(void *nThreadID)
{
	THREAD_BASIC_INFORMATION tbi;
	HANDLE hThread;
	pfnNtQueryInformationThread gNtQueryInformationThread;
	int nTemp;
	void *nResult=0;

	HMODULE hNtDll=LoadLibrary(TEXT("ntdll.dll"));
	if(hNtDll==NULL)
	{
		return NULL;
	}
	gNtQueryInformationThread=(pfnNtQueryInformationThread)GetProcAddress(hNtDll,"NtQueryInformationThread");

	hThread=OpenThread(THREAD_QUERY_INFORMATION ,0,(DWORD)nThreadID);

	if(hThread)
	{
		gNtQueryInformationThread(hThread,(THREADINFOCLASS)0,&tbi,sizeof(tbi),(PULONG)&nTemp);
		nResult=tbi.TebBaseAddress;

		CloseHandle(hThread);
	}

	return nResult;
}

extern "C" int __declspec(dllexport) __cdecl um_GetThreadIDs(void *nProcessID,void **pBuffer, int nBufferSize)
{
	int nTemp,nNumberOfThreads=0;
	HANDLE hThreadID;
	pfnNtQuerySystemInformation gNtQuerySystemInformation;
	SYSTEM_PROCESS_INFORMATION *pOffset;
	char *pOffset2;
	void *pMemory;

	if(GetCurrentProcessId()==(DWORD)nProcessID)
	{
		if(nBufferSize>=sizeof(HANDLE))
		{
			hThreadID=(HANDLE)GetCurrentThreadId();
			*(HANDLE *)pBuffer=hThreadID;

			return 1;
		}
		else
		{
			return 0;
		}
	}

	pMemory=GlobalAlloc(GMEM_FIXED+GMEM_ZEROINIT,200000);
	pOffset=(SYSTEM_PROCESS_INFORMATION *)pMemory;

	HMODULE hNtDll=LoadLibrary(TEXT("ntdll.dll"));
	if(hNtDll==NULL)
	{
		return NULL;
	}

	gNtQuerySystemInformation=(pfnNtQuerySystemInformation)GetProcAddress(hNtDll,"NtQuerySystemInformation");

	if(gNtQuerySystemInformation(SystemProcessInformation,(void *)pMemory,200000,(PULONG)&nTemp)==ERROR_SUCCESS)
	{
		while(pOffset->NextEntryOffset)
		{
			if(pOffset->UniqueProcessId==(HANDLE)nProcessID)
			{
				nNumberOfThreads=*(int *)((int)pOffset+4);
				if(nNumberOfThreads*((int)sizeof(int))<=nBufferSize)
				{
					pOffset2=(char *)((int)pOffset+sizeof(SYSTEM_PROCESS_INFORMATION));

					for(int i=0;i<nNumberOfThreads;i++)
					{
						hThreadID=((_SYSTEM_THREAD *)pOffset2)->ClientId.UniqueThread;

						*(HANDLE *)pBuffer=hThreadID;

						pBuffer++;
						pOffset2+=sizeof(_SYSTEM_THREAD);
					}

					GlobalFree(pMemory);
					return nNumberOfThreads;
				}
				else
				{
					GlobalFree(pMemory);
					return 0;
				}
			}
			pOffset=(SYSTEM_PROCESS_INFORMATION *)((int)pOffset+pOffset->NextEntryOffset);
		}
	}

	GlobalFree(pMemory);

	return 0;
}

extern "C" int __declspec(dllexport) __cdecl um_GetTEBs(void *nProcessID,void **pBuffer, int nBufferSize)
{
	int nTemp,nNumberOfThreads=0;
	HANDLE UniqueThreadId;
	pfnNtQuerySystemInformation gNtQuerySystemInformation;
	pfnNtQueryInformationThread gNtQueryInformationThread;
	SYSTEM_PROCESS_INFORMATION *pOffset;
	char *pOffset2;
	void *pMemory;
	HANDLE hThread;

	THREAD_BASIC_INFORMATION tbi;

	if(GetCurrentProcessId()==(DWORD)nProcessID)
	{
#ifndef __X64
		if(nBufferSize>=4)
		{

			_asm
			{
				mov eax,fs:[18h]
				mov nTemp,eax
			}
			(*(int *)pBuffer)=nTemp;

			return 1;
		}
		else
		{
			return 0;
		}
#else
		return 0;
#endif
	}

	pMemory=GlobalAlloc(GMEM_FIXED+GMEM_ZEROINIT,200000);
	pOffset=(SYSTEM_PROCESS_INFORMATION *)pMemory;

	HMODULE hNtDll=LoadLibraryA("ntdll.dll");
	if(hNtDll==NULL)
	{
		return NULL;
	}

	gNtQuerySystemInformation=(pfnNtQuerySystemInformation)GetProcAddress(hNtDll,"NtQuerySystemInformation");
	gNtQueryInformationThread=(pfnNtQueryInformationThread)GetProcAddress(hNtDll,"NtQueryInformationThread");

	if(gNtQuerySystemInformation(SystemProcessInformation,(void *)pMemory,200000,(PULONG)&nTemp)==ERROR_SUCCESS)
	{
		while(pOffset->NextEntryOffset)
		{
			if(pOffset->UniqueProcessId==(HANDLE)nProcessID)
			{
				nNumberOfThreads=*(int *)((int)pOffset+4);
				if(nNumberOfThreads*((int)sizeof(int))<=nBufferSize)
				{
					pOffset2=(char *)((int)pOffset+sizeof(SYSTEM_PROCESS_INFORMATION));

					for(int i=0;i<nNumberOfThreads;i++)
					{
						UniqueThreadId=((_SYSTEM_THREAD *)pOffset2)->ClientId.UniqueThread;

						hThread=OpenThread(THREAD_QUERY_INFORMATION ,0,(DWORD)UniqueThreadId);

						gNtQueryInformationThread(hThread,(THREADINFOCLASS)0,&tbi,sizeof(tbi),(PULONG)&nTemp);

						*pBuffer=(void *)(tbi.TebBaseAddress);

						CloseHandle(hThread);

						pBuffer++;
						pOffset2+=sizeof(_SYSTEM_THREAD);
					}

					GlobalFree(pMemory);
					return nNumberOfThreads;
				}
				else
				{
					GlobalFree(pMemory);
					return 0;
				}
			}
			pOffset=(SYSTEM_PROCESS_INFORMATION *)((int)pOffset+pOffset->NextEntryOffset);
		}
	}

	GlobalFree(pMemory);

	return 0;

}
extern "C" PVOID __declspec(dllexport) __cdecl um_OpenProcess(int nProcessID)
{
	return (void *)OpenProcess(PROCESS_ALL_ACCESS,0,nProcessID);
}
extern "C" bool __declspec(dllexport) __cdecl um_ReadProcessMemory(void *pProcessHandle,char* pBuffer, int nBufferSize,void *pMemoryAddress,int nMemorySize)
{
	SIZE_T nTemp;

	if(nMemorySize<=nBufferSize)
	{
		ReadProcessMemory((HANDLE)pProcessHandle,(void *)pMemoryAddress,pBuffer,nMemorySize,&nTemp);
		
		return (nTemp==nMemorySize);
	}

	return false;
}
extern "C" void __declspec(dllexport) __cdecl um_CloseProcess(void *pProcessHandle)
{
	CloseHandle((HANDLE)pProcessHandle);
}
//extern "C" int __declspec(dllexport) __cdecl _GetOSVersion()
//{
//	OSVERSIONINFOEXA ovi;
//
//	ovi.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEXA);
//
//	GetVersionExA((OSVERSIONINFOA *)&ovi);
//
//	if(ovi.dwMajorVersion==6)
//	{
//		if(ovi.dwMinorVersion==1)
//		{
//			if((ovi.wServicePackMajor==1)&&(ovi.wServicePackMinor==0))
//			{
//				return OSVERSION_WIN7_SP1;
//			}
//		}
//	}
//
//	return OSVERSION_UNKNOWN;
//
//}
extern "C" int __declspec(dllexport) __cdecl _GetBuildNumber()
{
	OSVERSIONINFOEXA ovi;

	ovi.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEXA);

	GetVersionExA((OSVERSIONINFOA *)&ovi);

	return ovi.dwBuildNumber;
}
extern "C" int __declspec(dllexport) __cdecl _GetSPNumber()
{
	OSVERSIONINFOEXA ovi;

	ovi.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEXA);

	GetVersionExA((OSVERSIONINFOA *)&ovi);

	return ovi.wServicePackMajor;
}

extern "C" bool __declspec(dllexport) __cdecl km_LoadDriver(char *pszDriverPathName)
{
	int nError;
	char *pszServiceName="xntsv";
	//char *pszDriverPathName="Q:\\prepare\\drivers\\ma\\obg\\amd64\\memoryaccess.sys";
	//char szBuffer[256];
	char *pszDeviceName="\\\\.\\DriverForXNTSV";

	//GetCurrentDirectoryA(sizeof(szBuffer),szBuffer);
	//lstrcatA(szBuffer,pszDriverPathName);
	hSCManager=OpenSCManagerW(0,0,SC_MANAGER_ALL_ACCESS);

	if(hSCManager)
	{
		//hService=CreateServiceA(hSCManager,pszServiceName,pszServiceName,SERVICE_ALL_ACCESS,SERVICE_KERNEL_DRIVER,SERVICE_DEMAND_START,SERVICE_ERROR_IGNORE,szBuffer,0,0,0,0,0);
		hService=CreateServiceA(hSCManager,pszServiceName,pszServiceName,SERVICE_ALL_ACCESS,SERVICE_KERNEL_DRIVER,SERVICE_DEMAND_START,SERVICE_ERROR_IGNORE,pszDriverPathName,0,0,0,0,0);
		
		nError=GetLastError();

		if(nError==ERROR_SERVICE_EXISTS)
		{
			hService=OpenServiceA(hSCManager,pszServiceName,SERVICE_ALL_ACCESS);
		}

		if(hService)
		{
			if(StartService(hService,0,0))
			{
				hDevice=CreateFileA(pszDeviceName,GENERIC_READ+GENERIC_WRITE,0,0,OPEN_EXISTING,0,0);

				if(hDevice!=INVALID_HANDLE_VALUE)
				{
					return true;
				}
				else
				{
					hDevice=0;
					return false;
				}
			}
		}
	}

	return 0;
}
//extern "C" void __declspec(dllexport) __cdecl km_DeviceIoControl(int nCode,char *pInBuffer,int nInBufferSize,char *pOutBuffer,int nOutBufferSize,int *pnReturned)
//{
//
//}

extern "C" void __declspec(dllexport) __cdecl km_UnloadDriver()
{
	SERVICE_STATUS ServiceStatus;

	if(hDevice)
	{
		CloseHandle(hDevice);
		hDevice=0;
	}
	// Stop
	if(hService)
	{
		ControlService(hService,SERVICE_CONTROL_STOP,&ServiceStatus);
		DeleteService(hService);
		CloseServiceHandle(hService);
		hService=0;
	}
	if(hSCManager)
	{
		CloseServiceHandle(hSCManager);
		hSCManager=0;
	}
}

extern "C" PVOID __declspec(dllexport) __cdecl km_OpenProcess(void *nProcessID)
{
	long long nTemp=0;
	HANDLE nProcessHandle;

	if(DeviceIoControl(hDevice,IOCTL_OPENPROCESS,&nProcessID,sizeof(PVOID),&nProcessHandle,sizeof(PVOID),(LPDWORD)&nTemp,0))
	{
		if(nTemp)
		{
			return nProcessHandle;
		}
	}
	
	return 0;
}
extern "C" void __declspec(dllexport) __cdecl km_CloseProcess(void *nProcessHandle)
{
	long long nTemp=0;

	DeviceIoControl(hDevice,IOCTL_CLOSEPROCESS,&nProcessHandle,sizeof(PVOID),0,0,(LPDWORD)&nTemp,0);
}
extern "C" bool __declspec(dllexport) __cdecl km_ReadProcessMemory(void *pProcessHandle,char* pBuffer, int nBufferSize,void *pMemoryAddress,int nMemorySize)
{
	long long nTemp=0;
	PROCESSMEMORY pm;

	if(nMemorySize<=nBufferSize)
	{
		pm.pProcessHandle=pProcessHandle;
		pm.pMemoryAddress=pMemoryAddress;
		pm.nMemorySize=nMemorySize;

		if(DeviceIoControl(hDevice,IOCTL_READPROCESSMEMORY,&pm,sizeof(pm),pBuffer,nMemorySize,(LPDWORD)&nTemp,0))
		{
			return (nTemp==nMemorySize);
		}
	}

	return false;
}
extern "C" PVOID __declspec(dllexport) __cdecl km_GetEPROCESS(void *nProcessID)
{
	long long nTemp=0;
	HANDLE nEPROCESS=0;

	if(DeviceIoControl(hDevice,IOCTL_GETEPROCESS,&nProcessID,sizeof(HANDLE),&nEPROCESS,sizeof(HANDLE),(LPDWORD)&nTemp,0))
	{
		if(nTemp)
		{
			return nEPROCESS;
		}
	}

	return 0;
}
//extern "C" int __declspec(dllexport) __cdecl km_GetKPCR(int nProcessID)
//{
//	int nKPCR,nTemp;
//
//	if(DeviceIoControl(hDevice,IOCTL_GETKPCR,&nProcessID,sizeof(int),&nKPCR,sizeof(int),(LPDWORD)&nTemp,0))
//	{
//		if(nTemp)
//		{
//			return nKPCR;
//		}
//	}
//
//	return 0;
//}
//extern "C" int __declspec(dllexport) __cdecl km_GetETHREADs(int nProcessID,int* pnBuffer, int nBufferSize)
//{
//	int nTemp;
//
//	if(DeviceIoControl(hDevice,IOCTL_GETETHREADS,&nProcessID,sizeof(int),(char *)pnBuffer,nBufferSize,(LPDWORD)&nTemp,0))
//	{
//		return nTemp/4;
//	}
//
//	return 0;
//}
//extern "C" int __declspec(dllexport) __cdecl km_GetNumberOfThreads(int nProcessID)
//{
//	int nGetNumberOfThreads,nTemp;
//
//	if(DeviceIoControl(hDevice,IOCTL_GETNUMBEROFTHREADS,&nProcessID,sizeof(int),&nGetNumberOfThreads,sizeof(int),(LPDWORD)&nTemp,0))
//	{
//		if(nTemp)
//		{
//			return nGetNumberOfThreads;
//		}
//	}
//
//	return 0;
//}
extern "C" PVOID __declspec(dllexport) __cdecl km_GetETHREAD(void *nThreadID)
{
	long long nTemp=0;
	HANDLE nETHREAD=0;

	if(DeviceIoControl(hDevice,IOCTL_GETETHREAD,&nThreadID,sizeof(HANDLE),&nETHREAD,sizeof(HANDLE),(LPDWORD)&nTemp,0))
	{
		if(nTemp)
		{
			return nETHREAD;
		}
	}

	return 0;
}

extern "C" PVOID __declspec(dllexport) __cdecl km_GetKPCR(void *nProcessID)
{
	long long nTemp=0;
	HANDLE nKPCR=0;

	if(DeviceIoControl(hDevice,IOCTL_GETKPCR,&nProcessID,sizeof(HANDLE),&nKPCR,sizeof(HANDLE),(LPDWORD)&nTemp,0))
	{
		if(nTemp)
		{
			return nKPCR;
		}
	}

	return 0;
}
extern "C" int __declspec(dllexport) __cdecl km_GetKPCRS(char *pBuffer, int nBufferSize)
{
	long long nTemp=0;
	HANDLE nKPCR=0;

	if(DeviceIoControl(hDevice,IOCTL_GETKPCRS,0,0,pBuffer,nBufferSize,(LPDWORD)&nTemp,0))
	{
		if(nTemp)
		{
			return (int)nTemp;
		}
	}

	return 0;
}

extern "C" int __declspec(dllexport) __cdecl um_MemoryQuery(void *pProcessHandle,void *pMemoryAddress,char* pBuffer, int nBufferSize)
{
	return VirtualQueryEx(pProcessHandle, pMemoryAddress, (PMEMORY_BASIC_INFORMATION)pBuffer, nBufferSize);
}

//extern "C" bool __declspec(dllexport) __cdecl um_IsProcess64(void *pHandle)
//{
//	BOOL bResult;
//	pfnIsWow64Process gIsWow64Process;
//
//	HMODULE hKernel32=LoadLibraryA("kernel32.dll");
//	if(hKernel32==NULL)
//	{
//		return NULL;
//	}
//
//	gIsWow64Process=(pfnIsWow64Process)GetProcAddress(hKernel32,"IsWow64Process");
//
//	if(gIsWow64Process)
//	{
//		gIsWow64Process(pHandle,(PBOOL)&bResult);
//
//		if(bResult)
//		{
//			return false;
//		}
//
//		return true;
//	}
//
//	return false;
//}