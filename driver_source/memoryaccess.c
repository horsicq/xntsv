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

#include <Ntifs.h>
#include <ntddk.h>

#ifdef _WIN64
#define __X64 1
#endif

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

typedef NTSTATUS (NTAPI *pfnZwQuerySystemInformation)(
	__in       int SystemInformationClass,
	__inout    PVOID SystemInformation,
	__in       ULONG SystemInformationLength,
	__out_opt  PULONG ReturnLength
	);

struct PROCESSMEMORY
{
	void *nProcessHandle;
	void *nMemoryAddress;
	int nMemorySize;
};

struct SYSTEM_PROCESS_INFORMATION {
	ULONG NextEntryOffset;
	ULONG NumberOfThreads;
	UCHAR Reserved1[48];
	PVOID Reserved2[3];
	HANDLE UniqueProcessId;
	PVOID Reserved3;
	ULONG HandleCount;
	UCHAR Reserved4[4];
	PVOID Reserved5[11];
	SIZE_T PeakPagefileUsage;
	SIZE_T PrivatePageCount;
	LARGE_INTEGER Reserved6[6];
};

#define SystemProcessInformation 5

UNICODE_STRING	usSymbolLinkName;

pfnZwQuerySystemInformation ZwQuerySystemInformation;

HANDLE addrKPCR[32];

VOID dpcfunction(IN struct _KDPC *Dpc, IN PVOID DeferredContext, IN PVOID SystemArgument1, IN PVOID SystemArgument2)
{
	
    unsigned int currentCPU;
    HANDLE currentKPCR;
    
    currentCPU = KeGetCurrentProcessorNumber();
#ifndef __X64
	_asm
	{
		mov eax,fs:DWORD PTR[1Ch]
		mov currentKPCR,eax
	}
#else
	currentKPCR=(HANDLE)__readgsqword(24);
#endif
    
    //DbgPrint("KPCR %08x%08x\n",*(int *)((char *)currentKPCR+4),(int)currentKPCR);
    //DbgPrint("currentCPU %d\n",currentCPU);
    //DbgPrint("KPCR %p\n",currentKPCR);
    
    addrKPCR[currentCPU] = currentKPCR;
}

NTSTATUS DriverInit(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING TheRegistryPath)
{
	//DbgPrint("Init Driver");
	
	return STATUS_SUCCESS;
}

NTSTATUS DriverUnload(IN PDRIVER_OBJECT DriverObject)
{
	//DbgPrint("Init Unload");
	//DbgPrint("Delete SymbolicLink");
	IoDeleteSymbolicLink(&usSymbolLinkName);
	//DbgPrint("Delete Device");
	IoDeleteDevice(DriverObject->DeviceObject);

	return STATUS_SUCCESS;
}
NTSTATUS DispatchCreate(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
	pIrp->IoStatus.Status=STATUS_SUCCESS;
	pIrp->IoStatus.Information=0;

	//DbgPrint("Dispatch Create");

	IofCompleteRequest(pIrp,IO_NO_INCREMENT);

	return pIrp->IoStatus.Status;
};

NTSTATUS DispatchClose(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
	pIrp->IoStatus.Status=STATUS_SUCCESS;
	pIrp->IoStatus.Information=0;

	//DbgPrint("Dispatch Close");

	IofCompleteRequest(pIrp,IO_NO_INCREMENT);

	return pIrp->IoStatus.Status;
};

NTSTATUS DispatchControl(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
	OBJECT_ATTRIBUTES ObjectAttributes;
	CLIENT_ID ClientID;
	PIO_STACK_LOCATION pSL;
	void *pBuffer;
	PEPROCESS pEprocess;
	PETHREAD pEthread;
	KAPC_STATE Apc_State;
	int nTemp=0;
	void *pMemory;
	struct SYSTEM_PROCESS_INFORMATION *pOffset;
	int nNumberOfThreads;
	
	CCHAR cpunr;
	KAFFINITY cpus;

	ULONG cpucount;

	PKDPC dpc;
	int dpcnr;
	int maxCPU;
	int i=0;

	pIrp->IoStatus.Status=STATUS_SUCCESS;
	pIrp->IoStatus.Information=0;

	//DbgPrint("Dispatch Control");

	pBuffer=pIrp->AssociatedIrp.SystemBuffer;

	pSL=IoGetCurrentIrpStackLocation(pIrp);

	__try
	{
		if(pSL->Parameters.DeviceIoControl.IoControlCode==IOCTL_OPENPROCESS)
		{
			//DbgPrint("Open Process");
			RtlZeroMemory(&ObjectAttributes,sizeof(OBJECT_ATTRIBUTES));
			RtlZeroMemory(&ClientID,sizeof(CLIENT_ID));

			ClientID.UniqueProcess=*(HANDLE *)(pBuffer);

			//DbgPrint("Process ID %d",ClientID.UniqueProcess);

			if(ZwOpenProcess(pBuffer,PROCESS_ALL_ACCESS,&ObjectAttributes,&ClientID)==STATUS_SUCCESS)
			{
				pIrp->IoStatus.Status=STATUS_SUCCESS;
				pIrp->IoStatus.Information=sizeof(HANDLE);
			}
			else
			{
				pIrp->IoStatus.Status=STATUS_INVALID_CID;
				pIrp->IoStatus.Information=0;
			}
		}
		else if(pSL->Parameters.DeviceIoControl.IoControlCode==IOCTL_CLOSEPROCESS)
		{
			//DbgPrint("Close Process");

			ZwClose(*(HANDLE *)(pBuffer));

			pIrp->IoStatus.Information=0;
		}
		else if(pSL->Parameters.DeviceIoControl.IoControlCode==IOCTL_READPROCESSMEMORY)
		{
			//DbgPrint("Read Process Memory");

			if(NT_SUCCESS(ObReferenceObjectByHandle((HANDLE)((struct PROCESSMEMORY *)(pBuffer))->nProcessHandle, 0, NULL, UserMode, &pEprocess, NULL)))
			{
				KeStackAttachProcess(pEprocess,&Apc_State);
				
				__try
				{
					if(MmIsAddressValid(((struct PROCESSMEMORY *)pBuffer)->nMemoryAddress))
					{
						nTemp=((struct PROCESSMEMORY *)(pBuffer))->nMemorySize;
						memcpy(pBuffer,(char *)((struct PROCESSMEMORY *)(pBuffer))->nMemoryAddress,((struct PROCESSMEMORY *)(pBuffer))->nMemorySize);
					
						pIrp->IoStatus.Information=nTemp;
					}
					else
					{
						//DbgPrint("Invalid memory address");
						pIrp->IoStatus.Status=STATUS_EA_CORRUPT_ERROR;
						pIrp->IoStatus.Information=0;
					}
				}
				__except(EXCEPTION_EXECUTE_HANDLER)
				{
					//DbgPrint("Read Memory Error");
					pIrp->IoStatus.Status=STATUS_EA_CORRUPT_ERROR;
					pIrp->IoStatus.Information=0;
				}

				KeUnstackDetachProcess(&Apc_State);

				ObDereferenceObject(pEprocess);
			}
		}
		/*else if(pSL->Parameters.DeviceIoControl.IoControlCode==IOCTL_WRITEPROCESSMEMORY)
		{
			DbgPrint("Write Process Memory");
		}
		else if(pSL->Parameters.DeviceIoControl.IoControlCode==IOCTL_GETNUMBEROFTHREADS)
		{
			DbgPrint("Get Number Of Threads");
		} */
		else if(pSL->Parameters.DeviceIoControl.IoControlCode==IOCTL_GETETHREAD)
		{
			//DbgPrint("Get ETHREAD");
			ClientID.UniqueThread=*(HANDLE *)(pBuffer);
			PsLookupThreadByThreadId(ClientID.UniqueThread,&pEthread);

			ObDereferenceObject(pEthread);

			*(HANDLE*)(pBuffer)=(HANDLE)pEthread;
			pIrp->IoStatus.Information=sizeof(HANDLE);
		}
		else if(pSL->Parameters.DeviceIoControl.IoControlCode==IOCTL_GETEPROCESS)
		{
			//DbgPrint("Get EPROCESS");

			ClientID.UniqueProcess=*(HANDLE *)(pBuffer);
			PsLookupProcessByProcessId(ClientID.UniqueProcess,&pEprocess);

			ObDereferenceObject(pEprocess);

			*(HANDLE*)(pBuffer)=(HANDLE)pEprocess;
			pIrp->IoStatus.Information=sizeof(HANDLE);
		}
/*		else if(pSL->Parameters.DeviceIoControl.IoControlCode==IOCTL_GETKPCR)
		{
			//DbgPrint("Read Process Memory");

			if(NT_SUCCESS(ObReferenceObjectByHandle(*((HANDLE *)pBuffer), 0, NULL, UserMode, &pEprocess, NULL)))
			{
				KeStackAttachProcess(pEprocess,&Apc_State);
				
				__try
				{
#ifndef __X64
					_asm
					{
						mov eax,fs:DWORD PTR[1Ch]
						mov nTemp,eax
					}
					*(HANDLE*)(pBuffer)=(HANDLE)nTemp;
					pIrp->IoStatus.Information=sizeof(HANDLE);
					
#else
					*(HANDLE*)(pBuffer)=(HANDLE)__readgsqword(24);
					pIrp->IoStatus.Information=sizeof(HANDLE);
#endif
					
				}
				__except(EXCEPTION_EXECUTE_HANDLER)
				{
					//DbgPrint("Read Memory Error");
					pIrp->IoStatus.Status=STATUS_EA_CORRUPT_ERROR;
					pIrp->IoStatus.Information=0;
				}

				KeUnstackDetachProcess(&Apc_State);

				ObDereferenceObject(pEprocess);
			}
		} */
		else if(pSL->Parameters.DeviceIoControl.IoControlCode==IOCTL_GETKPCRS)
		{
			cpucount=0;
		    cpus=KeQueryActiveProcessors();

		    while (cpus)
		    {
		        if(cpus%2)
		            cpucount++;
		        cpus=cpus/2;
		    }
		    dpc=ExAllocatePool(NonPagedPool,sizeof(KDPC)*cpucount);
		    cpus=KeQueryActiveProcessors();
		    cpunr=0;
		    dpcnr=0;

		    while(cpus)
		    {
		        if(cpus%2)
		        {
		            
		            //DbgPrint("Calling dpc routine for cpunr %d\n", cpunr);
		            KeInitializeDpc(&dpc[dpcnr], dpcfunction, NULL);
		            KeSetTargetProcessorDpc (&dpc[dpcnr], cpunr);
		            KeInsertQueueDpc(&dpc[dpcnr], NULL, NULL);
		            
		            dpcnr++;
		        }
		        cpus=cpus/2;
		        cpunr++;
		    }
		    KeFlushQueuedDpcs();
		    ExFreePool(dpc);
		    maxCPU=cpucount;

		    /*for(i = 0;i < maxCPU;i++) //copy each entry into the output buffer
		    {
		    	DbgPrint("KPCR %p\n",addrKPCR[i]);
		    }*/
		    memcpy(pBuffer,(char *)addrKPCR,sizeof(HANDLE)*maxCPU);
		    pIrp->IoStatus.Information=sizeof(HANDLE)*maxCPU;
		}
		/*else if(pSL->Parameters.DeviceIoControl.IoControlCode==IOCTL_GETKPCR)
		{
			DbgPrint("Get KPCR");
			
			ClientID.UniqueProcess=*(HANDLE *)(pBuffer);
			PsLookupProcessByProcessId(ClientID.UniqueProcess,&pEprocess);

			KeStackAttachProcess(pEprocess,&Apc_State);
			_asm
			{
				mov eax,fs:DWORD PTR[1Ch]
				mov nTemp,eax
			}
			KeUnstackDetachProcess(&Apc_State);

			ObDereferenceObject(pEprocess);

			DbgPrint("KPCR %d",nTemp);

			*(int*)(pBuffer)=nTemp;
			pIrp->IoStatus.Information=sizeof(int);
		}
		else if(pSL->Parameters.DeviceIoControl.IoControlCode==IOCTL_GETETHREADS)
		{
			DbgPrint("Get ETHREADs");
			pIrp->IoStatus.Information=0;
		} */
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		//DbgPrint("Error!");
		pIrp->IoStatus.Status=STATUS_EA_CORRUPT_ERROR;
		pIrp->IoStatus.Information=0;
	}

	IofCompleteRequest(pIrp,IO_NO_INCREMENT);

	return pIrp->IoStatus.Status;
};

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING TheRegistryPath)
{
	UNICODE_STRING usDeviceName;
	PDEVICE_OBJECT pDeviceObject;
	UNICODE_STRING usZwQuerySystemInformation;

	//DbgPrint("Driver Entry");

	DriverObject->DriverInit=DriverInit;
	DriverObject->DriverUnload=DriverUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE]=DispatchCreate;
	DriverObject->MajorFunction[IRP_MJ_CLOSE]=DispatchClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]=DispatchControl;

	RtlInitUnicodeString(&usDeviceName,L"\\Device\\DriverForXNTSV");
	RtlInitUnicodeString(&usSymbolLinkName,L"\\??\\DriverForXNTSV");

	RtlInitUnicodeString(&usZwQuerySystemInformation,L"ZwQuerySystemInformation");
	
	ZwQuerySystemInformation=(pfnZwQuerySystemInformation)MmGetSystemRoutineAddress(&usZwQuerySystemInformation);
	//DbgPrint("QuerySystemInformation %d",(int)ZwQuerySystemInformation);

	if(IoCreateDevice(DriverObject,0,&usDeviceName,FILE_DEVICE_UNKNOWN,0,0,&pDeviceObject)==STATUS_SUCCESS)
	{
		//DbgPrint("Create Device");
		if(IoCreateSymbolicLink(&usSymbolLinkName,&usDeviceName)==STATUS_SUCCESS)
		{
			//DbgPrint("Create Symbolic Link");
		}
		else
		{
			//DbgPrint("Cannot Create Symbolic Link");
			IoDeleteDevice(DriverObject->DeviceObject);
		}
	}
	else
	{
		//DbgPrint("Cannot Create Device");
	}

	return STATUS_SUCCESS;
}