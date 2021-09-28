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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLibrary>
#include <QMessageBox>
#include <QTextBrowser>
#include <QUrl>
#include <QSqlError>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QDir>
#include <QFileDialog>
#include <QScrollBar>

#include "htmltable.h"
#include "history.h"
#include "about.h"
#include "prototype.h"
//#include "../../qxhexview.h"
#include "_version.h"
#include "dialogmemorymap.h"

#include "xprocess.h"

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
    struct __PROCESSINFO
    {
        int nPID;
        int nParentPID;
        wchar_t  wszExeName[256];
    };
    struct _UNICODE_STRING32 // Size=8
    {
        unsigned short Length; //Size=2 Offset=0
        unsigned short MaximumLength; //Size=2 Offset=2
        unsigned short * Buffer; //Size=4 Offset=4
    };
    struct _STRING32 // Size=8
    {
        unsigned short Length; //Size=2 Offset=0
        unsigned short MaximumLength; //Size=2 Offset=2
        char * Buffer; //Size=4 Offset=4
    };
    struct _LIST_ENTRY32 // Size=8
    {
        struct _LIST_ENTRY32 * Flink; //Size=4 Offset=0
        struct _LIST_ENTRY32 * Blink; //Size=4 Offset=4
    };

    struct _IMAGE_DOS_HEADER
    {
        quint16 e_magic;      /* 00: MZ Header signature */
        quint16 e_cblp;       /* 02: Bytes on last page of file */
        quint16 e_cp;         /* 04: Pages in file */
        quint16 e_crlc;       /* 06: Relocations */
        quint16 e_cparhdr;    /* 08: Size of header in paragraphs */
        quint16 e_minalloc;   /* 0a: Minimum extra paragraphs needed */
        quint16 e_maxalloc;   /* 0c: Maximum extra paragraphs needed */
        quint16 e_ss;         /* 0e: Initial (relative) SS value */
        quint16 e_sp;         /* 10: Initial SP value */
        quint16 e_csum;       /* 12: Checksum */
        quint16 e_ip;         /* 14: Initial IP value */
        quint16 e_cs;         /* 16: Initial (relative) CS value */
        quint16 e_lfarlc;     /* 18: File address of relocation table */
        quint16 e_ovno;       /* 1a: Overlay number */
        quint16 e_res[4];     /* 1c: Reserved words */
        quint16 e_oemid;      /* 24: OEM identifier (for e_oeminfo) */
        quint16 e_oeminfo;    /* 26: OEM information; e_oemid specific */
        quint16 e_res2[10];   /* 28: Reserved words */
        qint32 e_lfanew;     /* 3c: Offset to extended header */
    };

    struct _IMAGE_FILE_HEADER
    {
        quint16 Machine;
        quint16 NumberOfSections;
        quint32 TimeDateStamp;
        quint32 PointerToSymbolTable;
        quint32 NumberOfSymbols;
        quint16 SizeOfOptionalHeader;
        quint16 Characteristics;
    };

    struct _IMAGE_DATA_DIRECTORY
    {
        quint32 VirtualAddress;
        quint32 Size;
    };

    struct _IMAGE_OPTIONAL_HEADER32
    {
        //
        // Standard fields.
        //
        quint16 Magic;
        quint8  MajorLinkerVersion;
        quint8  MinorLinkerVersion;
        quint32 SizeOfCode;
        quint32 SizeOfInitializedData;
        quint32 SizeOfUninitializedData;
        quint32 AddressOfEntryPoint;
        quint32 BaseOfCode;
        quint32 BaseOfData;
        //
        // NT additional fields.
        //
        quint32 ImageBase;
        quint32 SectionAlignment;
        quint32 FileAlignment;
        quint16 MajorOperatingSystemVersion;
        quint16 MinorOperatingSystemVersion;
        quint16 MajorImageVersion;
        quint16 MinorImageVersion;
        quint16 MajorSubsystemVersion;
        quint16 MinorSubsystemVersion;
        quint32 Win32VersionValue;
        quint32 SizeOfImage;
        quint32 SizeOfHeaders;
        quint32 CheckSum;
        quint16 Subsystem;
        quint16 DllCharacteristics;
        quint32 SizeOfStackReserve;
        quint32 SizeOfStackCommit;
        quint32 SizeOfHeapReserve;
        quint32 SizeOfHeapCommit;
        quint32 LoaderFlags;
        quint32 NumberOfRvaAndSizes;
        _IMAGE_DATA_DIRECTORY DataDirectory[16];
    };

    struct _IMAGE_OPTIONAL_HEADER64
    {
        //
        // Standard fields.
        //
        quint16 Magic;
        quint8  MajorLinkerVersion;
        quint8  MinorLinkerVersion;
        quint32 SizeOfCode;
        quint32 SizeOfInitializedData;
        quint32 SizeOfUninitializedData;
        quint32 AddressOfEntryPoint;
        quint32 BaseOfCode;
        //
        // NT additional fields.
        //
        qint64 ImageBase;
        quint32 SectionAlignment;
        quint32 FileAlignment;
        quint16 MajorOperatingSystemVersion;
        quint16 MinorOperatingSystemVersion;
        quint16 MajorImageVersion;
        quint16 MinorImageVersion;
        quint16 MajorSubsystemVersion;
        quint16 MinorSubsystemVersion;
        quint32 Win32VersionValue;
        quint32 SizeOfImage;
        quint32 SizeOfHeaders;
        quint32 CheckSum;
        quint16 Subsystem;
        quint16 DllCharacteristics;
        qint64 SizeOfStackReserve;
        qint64 SizeOfStackCommit;
        qint64 SizeOfHeapReserve;
        qint64 SizeOfHeapCommit;
        quint32 LoaderFlags;
        quint32 NumberOfRvaAndSizes;
        _IMAGE_DATA_DIRECTORY DataDirectory[16];
    };

    struct _IMAGE_NT_HEADERS64
    {
        quint32 Signature;
        _IMAGE_FILE_HEADER FileHeader;
        _IMAGE_OPTIONAL_HEADER64 OptionalHeader;
    };

    struct _IMAGE_NT_HEADERS32
    {
        quint32 Signature;
        _IMAGE_FILE_HEADER FileHeader;
        _IMAGE_OPTIONAL_HEADER32 OptionalHeader;
    };

    struct _IMAGE_SECTION_HEADER
    {
        quint8  Name[8];
        union
        {
            quint32 PhysicalAddress;
            quint32 VirtualSize;
        } Misc;
        quint32 VirtualAddress;
        quint32 SizeOfRawData;
        quint32 PointerToRawData;
        quint32 PointerToRelocations;
        quint32 PointerToLinenumbers;
        quint16 NumberOfRelocations;
        quint16 NumberOfLinenumbers;
        quint32 Characteristics;
    };

#ifndef _WIN64
    struct _MEMORY_BASIC_INFORMATION
    {
        void *BaseAddress;
        void *AllocationBase;
        unsigned int AllocationProtect;
        unsigned int RegionSize;
        unsigned int State;
        unsigned int Protect;
        unsigned int Type;
    };
#else
    struct _MEMORY_BASIC_INFORMATION
    {
        void *BaseAddress;
        void *AllocationBase;
        unsigned int AllocationProtect;
        unsigned int __alignment1;
        unsigned long long RegionSize;
        unsigned int State;
        unsigned int Protect;
        unsigned int Type;
        unsigned int __alignment2;
    };
#endif

    typedef bool (*_SetPrivilege)(char *,bool);
    typedef int (* um_MemoryQuery)(void *,void *,char *,int);
    typedef int (*__GetProcesses)(char *,int);
    typedef void * (*um_GetPEB)(void *);
    typedef void * (*km_GetEPROCESS)(void *);
    //typedef void * (*km_GetKPCR)(void *);
    typedef void * (*um_OpenProcess)(void *);
    typedef void * (*km_OpenProcess)(void *);
    typedef void (*um_CloseProcess)(void *);
    typedef void (*km_CloseProcess)(void *);
    //typedef int (*_GetOSVersion)();
    typedef int (*_GetBuildNumber)();
    typedef int (*_GetSPNumber)();
    typedef bool (*um_ReadProcessMemory)(void *,char*,int,void *,int);
    typedef bool (*km_ReadProcessMemory)(void *,char*,int,void *,int);
    //typedef int (*um_GetNumberOfThreads)(int);
    //typedef int (*km_GetNumberOfThreads)(int);
    typedef int (*um_GetTEBs)(void *,void **,int);
    //typedef int (*km_GetETHREADs)(int,int*,int);
    typedef void * (*km_GetETHREAD)(void *);
    typedef int (*um_GetThreadIDs)(void *,void **,int);
    typedef void * (*um_GetTEB)(void *);
    typedef int (*km_GetKPCRS)(char *,int);

    typedef bool (*km_LoadDriver)(char *);
    typedef void (*km_UnloadDriver)();

#ifdef __X64
    //typedef bool (*um_IsProcess64)(void *);
#endif

public:
    explicit MainWindow(QWidget *parent=nullptr);
    ~MainWindow();

private slots:
    void on_actionExit_triggered();
    void on_pushButtonRefresh_clicked();
    void addLog(const QString &sText);
    void getProcesses();

    void on_tableProcesses_itemSelectionChanged();
    void getInfo(void *nPID);
    void anchor(const QUrl & link);

    void on_pushButtonStart_clicked();
    void on_pushButtonBack_clicked();
    void on_pushButtonForward_clicked();
    void on_pushButtonReload_clicked();

    int getElementSize(const QString &sElementType);
    int getElementShow(const QString &sElementType);
    QString addTable(void *pAddress,int nStartOffset,const QString sElementType,const QString sElementName,char *pData,int nDataSize);
    int getIndex(const QString sElementName);
    bool readProcessMemory(char *pBuffer,int nBufferSize,void *pMemoryAddress,int nMemorySize);
    QString getUnicodeString(void *pAddress);
    QString getAnsiString(void *pAddress);
    QString getSZ(void *pAddress,int nMemorySize);
    QString getListEntryLinks(void *pAddress,const QString &sStructur,int nOffset);
    QString getPELink(void *pAddress);
    QString addressToString(void *pAddress);
    QString hexToString(void *pValue);
    QString makeLink(const QString &sValue,const QString &sLink);
    void on_pushButtonPrototype_clicked();
    void on_pushButtonSave_clicked();
    QString getPrototype(const QString &sElementType);
    void on_actionAbout_triggered();
    //    void on_pushButtonHex_clicked();
//    void on_radioButtonDecimal_toggled(bool checked);
//    void on_radioButtonHeximal_toggled(bool checked);

    void on_lineEditAddress_textChanged(const QString &arg1);
    QString handlePE(void *pAddress);
    static QByteArray ReadFromMemory(QVariant parameter,unsigned long long nOffset,unsigned long long nSize,bool *pbIsReadOnly);
    void on_pushButtonMemoryMap_clicked();
    bool getMemoryInformation(void *pAddress,_MEMORY_BASIC_INFORMATION *pMBI);
    bool checkAddress(void *pAddress);
    void on_radioButtonKernelMode_toggled(bool checked);
    void on_radioButtonUserMode_toggled(bool checked);
    
private:
    Ui::MainWindow *ui;
    QLibrary *lib;
    void *hPID;
    static bool bKernel;
    //bool bIsProcess64;
    QSqlDatabase Base;

    History history;

    _GetBuildNumber GetBuildNumber;
    _GetBuildNumber GetSPNumber;
    um_MemoryQuery umMemoryQuery;
    um_OpenProcess umOpenProcess;
    km_OpenProcess kmOpenProcess;
    um_GetPEB umGetPEB;
    um_CloseProcess umCloseProcess;
    km_CloseProcess kmCloseProcess;
    _SetPrivilege SetPrivilege;
    static um_ReadProcessMemory umReadProcessMemory;
    static km_ReadProcessMemory kmReadProcessMemory;
    //um_GetNumberOfThreads umGetNumberOfThreads;
    //km_GetNumberOfThreads kmGetNumberOfThreads;
    um_GetTEBs umGetTEBs;
    km_LoadDriver kmLoadDriver;
    km_UnloadDriver kmUnloadDriver;
    km_GetEPROCESS kmGetEPROCESS;
    km_GetKPCRS kmGetKPCRS;
    //km_GetETHREADs kmGetETHREADs;
    km_GetETHREAD kmGetETHREAD;
    um_GetThreadIDs umGetThreadIDs;
    um_GetTEB umGetTEB;
#ifdef __X64
    //um_IsProcess64 umIsProcess64;
#endif
};

#endif // MAINWINDOW_H
