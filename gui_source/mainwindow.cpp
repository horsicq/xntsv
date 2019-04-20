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
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QString sDataBaseName;

    ui->setupUi(this);

    setWindowTitle(QString("XNTSV v%1").arg(XNTSV_APPLICATIONVERSION));
    //    ui->textBrowserResult->setStyleSheet("background-color:rgb(150,147,88);");

    hPID=nullptr;
    bKernel=false;
    //bIsProcess64=false;

    connect(ui->textBrowserResult,SIGNAL(anchorClicked(const QUrl &)),this,SLOT(anchor(const QUrl &)));

    lib=new QLibrary;

#ifdef _WIN64
#define __X64 1
#endif

#ifndef __X64
    lib->setFileName("gpm32.dll");
    //   lib->setFileName("F:\\prepare\\cpp\\gpm32\\Debug\\gpm32.dll");
#else
    lib->setFileName("gpm64.dll");
#endif
    kmUnloadDriver=0;

    if(lib->load())
    {
#ifndef __X64
        addLog("Load gpm32.dll");
#else
        addLog("Load gpm64.dll");
#endif
        ui->radioButtonUserMode->setChecked(true);

        GetBuildNumber=(_GetBuildNumber)lib->resolve("_GetBuildNumber");
        GetSPNumber=(_GetBuildNumber)lib->resolve("_GetSPNumber");
        umOpenProcess=(um_OpenProcess)lib->resolve("um_OpenProcess");
        kmOpenProcess=(km_OpenProcess)lib->resolve("km_OpenProcess");
        umGetPEB=(um_GetPEB)lib->resolve("um_GetPEB");
        umCloseProcess=(um_CloseProcess)lib->resolve("um_CloseProcess");
        kmCloseProcess=(km_CloseProcess)lib->resolve("km_CloseProcess");
        SetPrivilege=(_SetPrivilege)lib->resolve("_SetPrivilege");
        umReadProcessMemory=(um_ReadProcessMemory)lib->resolve("um_ReadProcessMemory");
        kmReadProcessMemory=(km_ReadProcessMemory)lib->resolve("km_ReadProcessMemory");
        //umGetNumberOfThreads=(um_GetNumberOfThreads)lib->resolve("um_GetNumberOfThreads");
        //kmGetNumberOfThreads=(km_GetNumberOfThreads)lib->resolve("km_GetNumberOfThreads");
        umGetTEBs=(um_GetTEBs)lib->resolve("um_GetTEBs");
        kmLoadDriver=(km_LoadDriver)lib->resolve("km_LoadDriver");
        kmUnloadDriver=(km_UnloadDriver)lib->resolve("km_UnloadDriver");
        kmGetEPROCESS=(km_GetEPROCESS)lib->resolve("km_GetEPROCESS");
        kmGetKPCRS=(km_GetKPCRS)lib->resolve("km_GetKPCRS");
        //kmGetETHREADs=(km_GetETHREADs)lib->resolve("km_GetETHREADs");
        kmGetETHREAD=(km_GetETHREAD)lib->resolve("km_GetETHREAD");
        umGetThreadIDs=(um_GetThreadIDs)lib->resolve("um_GetThreadIDs");
        umGetTEB=(um_GetTEB)lib->resolve("um_GetTEB");
        umMemoryQuery=(um_MemoryQuery)lib->resolve("um_MemoryQuery");

#ifdef __X64
        //umIsProcess64=(um_IsProcess64)lib->resolve("um_IsProcess64");;
#endif
        if(SetPrivilege((char *)("SeDebugPrivilege"),true))
        {
            addLog("Set Debug Privilege");

            getProcesses();
        }
        else
        {
            QMessageBox::critical(0, "Need Admin Privs", "!");
        }

#ifdef __X64
        sDataBaseName=QString("%1_%2_64.db3").arg(GetBuildNumber()).arg(GetSPNumber());
#else
        sDataBaseName=QString("%1_%2_32.db3").arg(GetBuildNumber()).arg(GetSPNumber());
#endif

        Base=QSqlDatabase::addDatabase("QSQLITE","Structurs");
        Base.setDatabaseName(QCoreApplication::applicationDirPath().replace("/","\\")+"\\db\\"+sDataBaseName);
        //StructursBase.setDatabaseName(QDir::currentPath()+"\\db\\"+sDataBaseName);
        QFile dbfile;
        dbfile.setFileName(QCoreApplication::applicationDirPath().replace("/","\\")+"\\db\\"+sDataBaseName);

        if(dbfile.exists())
        {
            if(Base.open())
            {
                addLog("Open DataBase: "+Base.databaseName());
            }
            else
            {
                addLog("Cannot Open Database: "+sDataBaseName);
                addLog(Base.lastError().text());
            }
        }
        else
        {
            addLog("Cannot Open Database: "+sDataBaseName);
        }

        ui->comboBoxType->addItem("");
        ui->comboBoxType->addItem("Image::PE");

        QSqlQuery query(Base);
        query.exec("SELECT * FROM PROTOS");

        while(query.next())
        {
            // get all fields
            ui->comboBoxType->addItem(query.value(0).toString());
        }

#ifndef __X64
        QString sDriverName=QCoreApplication::applicationDirPath().replace("/","\\")+"\\memoryaccess32.sys";
#else
        QString sDriverName=QCoreApplication::applicationDirPath().replace("/","\\")+"\\memoryaccess64.sys";
#endif
        if(kmLoadDriver(sDriverName.toLatin1().data()))
        {
            addLog("Load Kernel Driver: "+sDriverName);
            ui->radioButtonKernelMode->setEnabled(true);
        }
        else
        {
            addLog("Cannot Load Kernel Driver: "+sDriverName);
            ui->radioButtonKernelMode->setEnabled(false);
        }
    }
    else
    {
        QMessageBox::critical(nullptr, "Error!", "Cannot load "+lib->fileName());
        exit(0);
    }

    ui->tableProcesses->setColumnWidth(0,165);
    ui->tableProcesses->setColumnWidth(1,50);

    //    ui->pushButtonHex->setEnabled(false);
}

MainWindow::~MainWindow()
{
    if(kmUnloadDriver)
    {
        kmUnloadDriver();
    }

    delete ui;
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}

void MainWindow::on_pushButtonRefresh_clicked()
{
    getProcesses();
}

void MainWindow::addLog(const QString &sText)
{
    ui->textEditLog->append(sText);
}

void MainWindow::getProcesses()
{
    __PROCESSINFO *pPI;
    int nDataSize;
    QByteArray baProcesses;
    QTableWidgetItem *newItem;
    QString sBuffer;

    __GetProcesses _GetProcesses=(__GetProcesses)lib->resolve("__GetProcesses");

    //    ui->tableProcesses->clearContents();
    baProcesses.resize(200000);
    baProcesses.fill(0);
    nDataSize=_GetProcesses(baProcesses.data(),baProcesses.size());

    if(nDataSize)
    {
        addLog("Get Processes");

        pPI=(__PROCESSINFO *)baProcesses.data();

        for(int i=0; i<(nDataSize/(int)sizeof(__PROCESSINFO)); i++)
        {
            ui->tableProcesses->setRowCount(i+1);
            // Name
            newItem = new QTableWidgetItem;
            sBuffer=QString::fromWCharArray(pPI->wszExeName);
            newItem->setText(sBuffer);
            ui->tableProcesses->setItem(i,0,newItem);
            // PID
            newItem = new QTableWidgetItem;
            newItem->setData(Qt::EditRole,pPI->nPID);
            ui->tableProcesses->setItem(i,1,newItem);
            ui->tableProcesses->item(i,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

            pPI++;
        }
    }
    else
    {
        addLog("Error!");
    }
}

void MainWindow::on_tableProcesses_itemSelectionChanged()
{
    QString sPID=ui->tableProcesses->selectedItems().at(1)->data(0).toString();
    ui->lineEditPID->setText(sPID);
}

void MainWindow::getInfo(void *nPID)
{
    HtmlTable ht;
    QByteArray baData;
    int nNumberOfThreads=0;
    void *pAddress=nullptr;
    void *hThread=nullptr;
    QString sLink;

    ui->textBrowserResult->setHtml("");

    history.clear();

    if(hPID)
    {
        addLog("Close process");

        if(bKernel)
        {
            kmCloseProcess(hPID);
        }
        else
        {
            umCloseProcess(hPID);
        }

        hPID=nullptr;
        //bIsProcess64=false;
    }

    bKernel=ui->radioButtonKernelMode->isChecked();

    if(bKernel)
    {
        hPID=kmOpenProcess(nPID);
    }
    else
    {
        hPID=umOpenProcess(nPID);
    }

    ui->pushButtonMemoryMap->setEnabled(hPID!=nullptr);
    ui->pushButtonReload->setEnabled(hPID!=nullptr);

    if(hPID)
    {
        ht.addTableBegin();

        pAddress=nullptr;

        if(bKernel)
        {
            pAddress=kmGetEPROCESS(nPID);
            sLink=QString("%1#%2#%3.html").arg("struct _EPROCESS").arg(QString("Process(ID=%1)").arg(hexToString(nPID))).arg(addressToString(pAddress));
        }
        else
        {
            pAddress=umGetPEB(hPID);
            sLink=QString("%1#%2#%3.html").arg("struct _PEB").arg(QString("Process(ID=%1)").arg(hexToString(nPID))).arg(addressToString(pAddress));
        }

        ht.addRecord("",QString("Process(ID=%1)").arg(hexToString(nPID)),makeLink(addressToString(pAddress),sLink),"","","","");

        pAddress=nullptr;

        if(bKernel)
        {
            void *kpcrs[32];
            int result=kmGetKPCRS((char *)kpcrs,sizeof(kpcrs));
            QString sKPCR;

            for(int i=0; i<result/sizeof(void *); i++)
            {
                sKPCR=QString("Kernel Processor Control Region(%1)").arg(i);
                sLink=QString("%1#%2#%3.html").arg("struct _KPCR").arg(sKPCR).arg(addressToString(kpcrs[i]));
                ht.addRecord("",sKPCR,makeLink(addressToString(kpcrs[i]),sLink),"","","","");
            }

            //            pAddress=kmGetKPCR(hPID);
            //            sLink=QString("%1#%2#%3.html").arg("struct _KPCR").arg(QString("Kernel Processor Control Region(0)")).arg(addressToString(pAddress));
            //            ht.addRecord("",QString("Kernel Processor Control Region(0)"),makeLink(addressToString(pAddress),sLink),"","","","");
        }

        addLog(QString("Open process(PID=%1)").arg((int)nPID));
        baData.resize(4*1000);
        nNumberOfThreads=umGetThreadIDs(nPID,(void **)baData.data(),baData.size());

        for(int i=0; i<nNumberOfThreads; i++)
        {

#ifndef __X64
            hThread=(void *)(*(int *)(baData.data()+sizeof(void *)*i));
#else
            hThread=(void *)(*(long long *)(baData.data()+sizeof(void *)*i));
#endif
            pAddress=nullptr;

            if(bKernel)
            {
                pAddress=kmGetETHREAD(hThread);
                sLink=QString("%1#%2#%3.html").arg("struct _ETHREAD").arg(QString("Thread(ID=%1)").arg(hexToString(hThread))).arg(addressToString(pAddress));
            }
            else
            {
                pAddress=umGetTEB(hThread);
                sLink=QString("%1#%2#%3.html").arg("struct _TEB").arg(QString("Thread(ID=%1)").arg(hexToString(hThread))).arg(addressToString(pAddress));
            }

            ht.addRecord(hexToString((void *)i),QString("Thread(ID=%1)").arg(hexToString(hThread)),makeLink(addressToString(pAddress),sLink),"","","","");
        }

        ht.addTableEnd();

        ui->lineEditName->setText("Start");
        ui->lineEditAddress->setText("");

        ui->textBrowserResult->setHtml(ht.toString());
        ui->comboBoxType->setItemText(0,"");
        ui->comboBoxType->setCurrentIndex(0);

        history.setHome("Start&"+ht.toString());
        history.addRecord("Start&"+ht.toString());
    }
    else
    {
        addLog(QString("Cannot open process(PID=%1)").arg((int)nPID));
    }
}


void MainWindow::anchor(const QUrl &link)
{
    int nIndex=0;
    QString sTitle;
    QString sResult;
    QString sElementType;
    QString sElementName;
    QString sAddress;
    void *pAddress;
    int nElementSize=0;
    QByteArray baData;
    //StructursBase

    sTitle=link.toString();
    sTitle=sTitle.section(".",0,0);
    sElementType=sTitle.section("#",0,0);
    sElementName=sTitle.section("#",1,1);
    sAddress=sTitle.section("#",2,2);

    //    qDebug(sAddress.toAscii().data());

    bool ok;

#ifndef __X64
    pAddress=(void *)sAddress.toUInt(&ok,16);
#else
    pAddress=(void *)sAddress.toULongLong(&ok,16);
#endif

    if(!ok)
    {
        addLog(QString("Cannot recognize address(%1)").arg(sAddress));
    }

    // Get element size
    if(sElementType=="Image::PE")
    {
        if(pAddress)
        {
            //            qDebug(QString("test1 %1").arg((int)pAddress,0,16).toAscii().data());
            nElementSize=64;
            nIndex=1;
            sResult=handlePE(pAddress);
        }
    }
    //    else if(sElementType=="HEX")
    //    {
    //        //        qDebug("HEX");

    //        int nPosition=ui->textBrowserResult->verticalScrollBar()->value();

    //        ui->textBrowserResult->setSource(QUrl());


    //        ui->textBrowserResult->verticalScrollBar()->setValue(nPosition);

    //        hex(pAddress,0,"");
    //        //        ui->textBrowserResult->scrollToAnchor("#"+sAddress);

    //        return;
    //    }
    else
    {
        nElementSize=getElementSize(sElementType);

        if(nElementSize!=-1)
        {
            nIndex=getIndex(sElementName);

            baData.resize(nElementSize*nIndex);

            if(readProcessMemory(baData.data(),baData.size(),(void *)pAddress,nElementSize*nIndex))
            {
                sResult=addTable(pAddress,0,sElementType,sElementName,baData.data(),baData.size());
            }
        }
    }

    ui->lineEditName->setText(sElementName);
    ui->lineEditAddress->setText(sAddress);
    ui->lineEditSize->setText(QString("0x%1").arg((unsigned int)(nElementSize*nIndex),8,16,QChar('0')));
    ui->comboBoxType->setItemText(0,sElementType);
    ui->comboBoxType->setCurrentIndex(0);

    ui->textBrowserResult->setHtml(sResult);
    history.addRecord(sTitle+"&"+sResult);

    ui->pushButtonBack->setEnabled(history.isBackAvailable());
    ui->pushButtonForward->setEnabled(history.isForwardAvailable());
    //    qDebug(sResult.toAscii().data());
}

void MainWindow::on_pushButtonStart_clicked()
{
    QString sPID=ui->lineEditPID->text();

    getInfo((void *)sPID.toUInt());

//    if(ui->radioButtonDecimal->isChecked())
//    {
//        getInfo((void *)sPID.toUInt());
//    }
//    else if(ui->radioButtonHeximal->isChecked())
//    {
//        bool bTemp;
//        getInfo((void *)sPID.toUInt(&bTemp,16));
//    }

    ui->pushButtonBack->setEnabled(false);
    ui->pushButtonForward->setEnabled(false);
}

void MainWindow::on_pushButtonBack_clicked()
{
    QString sTitle;
    QString sElementType;
    QString sElementName;
    QString sAddress;

    history.decrement();

    QString sRecord=history.current();
    sTitle=sRecord.section("&",0,0);

    sElementType=sTitle.section("#",0,0);
    sElementName=sTitle.section("#",1,1);
    sAddress=sTitle.section("#",2,2);

    QString sElementSize;
    int nElementSize=getElementSize(sElementType);
    int nIndex=getIndex(sElementName);
    sElementSize=QString("0x%1").arg((unsigned int)(nElementSize*nIndex),8,16,QChar('0'));
    ui->lineEditSize->setText(sElementSize);

    ui->lineEditName->setText(sElementName);
    ui->lineEditAddress->setText(sAddress);
    ui->comboBoxType->setItemText(0,sElementType);
    ui->comboBoxType->setCurrentIndex(0);

    ui->textBrowserResult->setHtml(sRecord.section("&",1,1));

    ui->pushButtonBack->setEnabled(history.isBackAvailable());
    ui->pushButtonForward->setEnabled(history.isForwardAvailable());
}

void MainWindow::on_pushButtonForward_clicked()
{
    QString sTitle;
    QString sElementType;
    QString sElementName;
    QString sAddress;

    history.increment();

    QString sRecord=history.current();
    sTitle=sRecord.section("&",0,0);

    sElementType=sTitle.section("#",0,0);
    sElementName=sTitle.section("#",1,1);
    sAddress=sTitle.section("#",2,2);

    QString sElementSize;
    int nElementSize=getElementSize(sElementType);
    int nIndex=getIndex(sElementName);
    sElementSize=QString("0x%1").arg((unsigned int)(nElementSize*nIndex),8,16,QChar('0'));
    ui->lineEditSize->setText(sElementSize);

    ui->lineEditName->setText(sElementName);
    ui->lineEditAddress->setText(sAddress);
    ui->comboBoxType->setItemText(0,sElementType);
    ui->comboBoxType->setCurrentIndex(0);

    ui->textBrowserResult->setHtml(sRecord.section("&",1,1));

    ui->pushButtonBack->setEnabled(history.isBackAvailable());
    ui->pushButtonForward->setEnabled(history.isForwardAvailable());
}

void MainWindow::on_pushButtonReload_clicked()
{
    // Change . Load from edits;
    QString sRecord=QString("%1#%2#%3.html").arg(ui->comboBoxType->currentText()).arg(ui->lineEditName->text()).arg(ui->lineEditAddress->text());

    QUrl link;

    if(ui->lineEditName->text()=="Start")
    {
        on_pushButtonStart_clicked();
    }
    else
    {
        int nPosition=ui->textBrowserResult->verticalScrollBar()->value();
        link.setUrl(sRecord);
        anchor(link);
        ui->textBrowserResult->verticalScrollBar()->setValue(nPosition);
    }
}

int MainWindow::getElementSize(const QString &sElementType)
{
    if(sElementType=="")
    {
        return -1;
    }

    int nElementSize=0;

    if(sElementType.contains("*"))
    {
#ifndef __X64
        nElementSize=4;
#else
        nElementSize=8;
#endif
    }
    else if(sElementType.contains("enum "))
    {
        nElementSize=4;
    }
    else
    {
        QSqlDatabase DataBase;

        DataBase=Base;

        QSqlQuery query(DataBase);

        query.exec(QString("SELECT SIZE FROM TYPES WHERE NAME=\"%1\"").arg(sElementType));
        query.next();

        if(query.value(0).isValid())
        {
            nElementSize=query.value(0).toInt();

            //addLog(QString("Database: Get %1 Size:%2").arg(sElementType).arg(nElementSize));
        }
        else
        {
            nElementSize=-1;

            addLog(QString("Database: Error(%1)(Size)").arg(sElementType));
            addLog(query.lastError().text());
        }
    }

    return nElementSize;
}

int MainWindow::getElementShow(const QString &sElementType)
{
    int nElementShow=0;

    if(sElementType.contains("*"))
    {
        if(getElementShow(sElementType.section(" *",0,0))==0)
        {
            nElementShow=2;
        }
        else if(getElementShow(sElementType.section(" *",0,0))==1)
        {
            nElementShow=1;
        }
    }
    else if(sElementType.contains("enum "))
    {
        nElementShow=1;
    }
    else
    {
        QSqlDatabase DataBase;
        DataBase=Base;

        QSqlQuery query(DataBase);

        query.exec(QString("SELECT SHOW FROM TYPES WHERE NAME=\"%1\"").arg(sElementType));
        query.next();

        if(query.value(0).isValid())
        {
            nElementShow=query.value(0).toInt();

            //addLog(QString("Database: Get %1 Size:%2").arg(sElementType).arg(nElementSize));
        }
        else
        {
            nElementShow=-1;

            addLog(QString("Database: Error(%1)(Show)").arg(sElementType));
            addLog(query.lastError().text());
        }
    }

    return nElementShow;
}

QString MainWindow::addTable(void *pAddress,int nStartOffset, const QString sElementType, const QString sElementName, char *pData, int nDataSize)
{
    int nIndex;
    QString sDBTable;
    int nOffset;
    QString sType;
    QString sName;
    QString sComment;
    int nSize;
    int nShow;
    QString sValue;
    QString sLink;
    QString sIndex;
    QString sOffset;
    unsigned char ucValue;
    unsigned short usValue;
    unsigned int unValue;
    unsigned long long un64Value;
    int nBitCount;
    int nBitOffset=0;
    long long nMask;
    HtmlTable ht;
    QString sResult;
    int nElementSize;
    void *pCurrentAddress=nullptr;

    QSqlDatabase DataBase;

    DataBase=Base;

    QSqlQuery query(DataBase);

    nIndex=getIndex(sElementName);
    nElementSize=getElementSize(sElementType);
    ht.addTableBegin();

    if(nIndex==1)
    {
        sDBTable=sElementType;
        sDBTable.replace(" ","$");

        query.exec(QString("SELECT * FROM %1").arg(sDBTable));

        while(query.next())
        {
            // get all fields
            nOffset=query.value(0).toInt();
            sType=query.value(1).toString();
            sName=query.value(2).toString();
            sComment=query.value(3).toString();

#ifndef __X64
            pCurrentAddress=(void *)((int)pAddress+nOffset);
#else
            pCurrentAddress=(void *)((long long)pAddress+nOffset);
#endif
            nSize=getElementSize(sType);
            nShow=getElementShow(sType);

            if(sComment.contains("#LE_LDTE0"))
            {
                sComment.replace("#LE_LDTE0",getListEntryLinks(pCurrentAddress,"struct _LDR_DATA_TABLE_ENTRY",0));
            }

            if(sComment.contains("#LE_LDTE1"))
            {
                sComment.replace("#LE_LDTE1",getListEntryLinks(pCurrentAddress,"struct _LDR_DATA_TABLE_ENTRY",-sizeof(_LIST_ENTRY32)));
            }

            if(sComment.contains("#LE_LDTE2"))
            {
                sComment.replace("#LE_LDTE2",getListEntryLinks(pCurrentAddress,"struct _LDR_DATA_TABLE_ENTRY",-sizeof(_LIST_ENTRY32)*2));
            }

            if(sComment.contains("#LE_YS"))
            {
                sComment.replace("#LE_YS",getListEntryLinks(pCurrentAddress,sElementType,-nOffset));
            }

            if(sComment.contains("#UNICODE_STRING"))
            {
                sComment.replace("#UNICODE_STRING",QString("Unicode String=\"%1\"").arg(getUnicodeString(pCurrentAddress)));
            }

            if(sComment.contains("#ANSI_STRING"))
            {
                sComment.replace("#ANSI_STRING",QString("Ansi String=\"%1\"").arg(getUnicodeString(pCurrentAddress)));
            }

            if(sComment.contains("#32"))
            {
                sComment.replace("#32","");
                nSize=4;
            }

            if(sComment.contains("#PE"))
            {
                sComment.replace("#PE",getPELink(pCurrentAddress));
            }

            // Ubrat 3 i 4, a vmesto ih ispolzovat kommentarii v db
            if((nShow==0)||(nShow==3)||(nShow==4))
            {
                sLink=QString("%1#%2#%3").arg(sType).arg(sName).arg(addressToString(pCurrentAddress));

                sValue="...";
                //                if(nShow==3)        // Unicode
                //                {
                //                    sComment.append(QString("Unicode String=\"%1\"").arg(getUnicodeString(pCurrentAddress)));
                //                }
                //                else if(nShow==4)   // Ansi
                //                {
                //                    sComment.append(QString("Ansi String=\"%1\"").arg(getAnsiString(pCurrentAddress)));
                //                }
            }
            else
            {
                nBitCount=0;

                if(sName.contains(":"))
                {
                    nBitCount=sName.section(":",1,1).toInt();
                }
                else
                {
                    nBitOffset=0;
                }

                if(nSize==1)
                {
                    ucValue=*(unsigned char *)(pData+nOffset);

                    if(nBitCount)
                    {
                        nMask=-1;

                        for(int i=0; i<nBitOffset; i++)
                        {
                            ucValue=ucValue>>1;
                        }

                        for(int i=0; i<nBitCount; i++)
                        {
                            nMask=nMask<<1;
                        }

                        ucValue=ucValue&(~nMask);
                    }

                    sValue=QString("0x%1").arg(ucValue,2,16,QChar('0'));
                }
                else if(nSize==2)
                {
                    usValue=*(unsigned short *)(pData+nOffset);

                    if(nBitCount)
                    {
                        nMask=-1;

                        for(int i=0; i<nBitOffset; i++)
                        {
                            usValue=usValue>>1;
                        }

                        for(int i=0; i<nBitCount; i++)
                        {
                            nMask=nMask<<1;
                        }

                        usValue=usValue&(~nMask);
                    }

                    sValue=QString("0x%1").arg(usValue,4,16,QChar('0'));
                }
                else if(nSize==4)
                {
                    unValue=*(unsigned int *)(pData+nOffset);

                    if(nBitCount)
                    {
                        nMask=-1;

                        for(int i=0; i<nBitOffset; i++)
                        {
                            unValue=unValue>>1;
                        }

                        for(int i=0; i<nBitCount; i++)
                        {
                            nMask=nMask<<1;
                        }

                        unValue=unValue&(~nMask);
                    }

                    sValue=QString("0x%1").arg(unValue,8,16,QChar('0'));
                }
                else if(nSize==8)
                {
                    un64Value=*(unsigned long long *)(pData+nOffset);

                    if(nBitCount)
                    {
                        nMask=-1;

                        for(int i=0; i<nBitOffset; i++)
                        {
                            un64Value=un64Value>>1;
                        }

                        for(int i=0; i<nBitCount; i++)
                        {
                            nMask=nMask<<1;
                        }

                        un64Value=un64Value&(~nMask);
                    }

                    sValue=QString("0x%1").arg(un64Value,16,16,QChar('0'));
                }
                else
                {
                    addLog(QString("Unknown size(%1)").arg(nSize));
                }

                if(sName.contains(":"))
                {
                    nBitOffset+=nBitCount;

                    if(nSize==1)
                    {
                        nBitOffset=nBitOffset%8;
                    }
                    else if(nSize==2)
                    {
                        nBitOffset=nBitOffset%16;
                    }
                    else if(nSize==4)
                    {
                        nBitOffset=nBitOffset%32;
                    }
                    else if(nSize==8)
                    {
                        nBitOffset=nBitOffset%64;
                    }
                }
                else
                {
                    // not bitset

                    //                    if(nSize==sizeof(void *))
                    //                    {
                    //                        if(nSize==4)
                    //                        {
                    //                            if(checkAddress((void *)unValue))
                    //                            {
                    //                                sComment+=QString(" <a href=\"%1#%2#%3\">HEX</a>").arg("HEX").arg(addressToString((void *)unValue)).arg(addressToString((void *)unValue));
                    //                            }
                    //                        }
                    //                        else if(nSize==8)
                    //                        {
                    //                            if(checkAddress((void *)un64Value))
                    //                            {
                    //                                sComment+=QString(" <a href=\"%1#%2#%3\">HEX</a>").arg("HEX").arg(addressToString((void *)un64Value)).arg(addressToString((void *)un64Value));
                    //                            }
                    //                        }
                    //                    }
                }

                if(nShow==1)
                {
                    sLink="";
                }
                else if(nShow==2)
                {
                    sLink=sType;
                    sLink.remove(" *");
                    sLink=QString("%1#%2#%3.html").arg(sLink).arg(sName).arg(sValue);
                }

                // zhirniy shrift?
                if(sValue=="0x00000000")
                {
                    sLink="";
                }
                else if(sValue=="0xffffffff")
                {
                    sLink="";
                }
                else if(sValue=="0x0000000000000000")
                {
                    sLink="";
                }
            }

            if(sName.contains("["))
            {
                sIndex=sName;
                sIndex=sIndex.section("[",1,1);
                sIndex=sIndex.section("]",0,0);
                nIndex=sIndex.toInt();

                if(nIndex!=1)
                {
                    sLink=QString("%1#%2#%3").arg(sType).arg(sName).arg(addressToString(pCurrentAddress));
                    sValue="...";

                    if(sComment.contains("#SZ"))
                    {
                        sComment.replace("#SZ",QString("szString=\"%1\"").arg(getSZ(pCurrentAddress,nIndex)));
                    }
                }
            }

            if(sValue=="0x00")
            {

            }
            else if(sValue=="0x0000")
            {
            }
            else if(sValue=="0x00000000")
            {
            }
            else if(sValue=="0x0000000000000000")
            {
            }
            else
            {
                sValue=QString("<b>%1</b>").arg(sValue);
            }

            sOffset=addressToString((void *)(nOffset+nStartOffset));
            ht.addRecord(addressToString(pCurrentAddress),sOffset,sType,sName,makeLink(sValue,sLink),sComment,"");
        }
    }
    else // Array
    {
        sType=sElementType;
        sName=sElementName;
        sName=sName.section("[",0,0);

        nSize=getElementSize(sType);
        nShow=getElementShow(sType);

        for(int i=0; i<nIndex; i++)
        {
            nOffset=nElementSize*i;
#ifndef __X64
            pCurrentAddress=(void *)((int)pAddress+nOffset);
#else
            pCurrentAddress=(void *)((long long)pAddress+nOffset);
#endif
            if(nShow==0)
            {
                sLink=QString("%1#%2#%3.html").arg(sType).arg(sName).arg(addressToString(pCurrentAddress));
                sValue="...";
            }
            else
            {
                if(nSize==1)
                {
                    sValue=QString("0x%1").arg(*(unsigned char *)(pData+nOffset),2,16,QChar('0'));
                }
                else if(nSize==2)
                {
                    sValue=QString("0x%1").arg(*(unsigned short *)(pData+nOffset),4,16,QChar('0'));
                }
                else if(nSize==4)
                {
                    sValue=QString("0x%1").arg(*(unsigned int *)(pData+nOffset),8,16,QChar('0'));
                }
                else if(nSize==8)
                {
                    sValue=QString("0x%1").arg(*(unsigned long long *)(pData+nOffset),16,16,QChar('0'));
                }
                else
                {
                    addLog("Unknown size");
                }

                if(nShow==1)
                {
                    sLink="";
                }
                else if(nShow==2)
                {
                    sLink=sType;
                    sLink.remove(" *");
                    sLink=QString("%1#%2#%3").arg(sLink).arg(sName).arg(sValue);
                }

                if(sValue=="0x00000000")
                {
                    sLink="";
                }
                else if(sValue=="0xffffffff")
                {
                    sLink="";
                }
                else if(sValue=="0x0000000000000000")
                {
                    sLink="";
                }
            }

            if(sValue=="0x00")
            {

            }
            else if(sValue=="0x0000")
            {
            }
            else if(sValue=="0x00000000")
            {
            }
            else if(sValue=="0x0000000000000000")
            {
            }
            else
            {
                sValue=QString("<b>%1</b>").arg(sValue);
            }

            sOffset=QString("0x%1").arg((unsigned int)(nOffset+nStartOffset),8,16,QChar('0'));
            ht.addRecord(addressToString(pCurrentAddress),sOffset,sElementType,QString("%1[%2]").arg(sName).arg(i),makeLink(sValue,sLink),sComment,"");
        }
    }

    ht.addTableEnd();
    sResult=ht.toString();

    return sResult;
}

int MainWindow::getIndex(const QString sElementName)
{
    int nIndex=1;
    QString sIndex;

    if(sElementName.contains("["))
    {
        sIndex=sElementName;
        sIndex=sIndex.section("[",1,1);
        sIndex=sIndex.section("]",0,0);
        nIndex=sIndex.toInt();
    }

    return nIndex;
}

bool MainWindow::readProcessMemory(char *pBuffer, int nBufferSize, void *pMemoryAddress, int nMemorySize)
{
    bool bSuccess=true;

    if(pMemoryAddress&&nMemorySize)
    {
        if(bKernel)
        {
            bSuccess=kmReadProcessMemory(hPID,pBuffer,nBufferSize,pMemoryAddress,nMemorySize);
        }
        else
        {
            bSuccess=umReadProcessMemory(hPID,pBuffer,nBufferSize,pMemoryAddress,nMemorySize);
        }
    }
    else
    {
        bSuccess=false;
    }

    if(bSuccess)
    {
        addLog(QString("Read process memory(Address=%1 Size:%2)").arg(addressToString(pMemoryAddress)).arg(nMemorySize));

        return true;
    }
    else
    {
        addLog(QString("Cannot read process memory(Address=%1 Size:%2)").arg(addressToString(pMemoryAddress)).arg(nMemorySize));

        return false;
    }
}

QString MainWindow::getUnicodeString(void *pAddress)
{
    QString sResult;
    _UNICODE_STRING32 us32;
    QByteArray baBuffer;

    sResult="";

    if(readProcessMemory((char *)&us32,sizeof(us32),pAddress,sizeof(us32)))
    {
        baBuffer.resize(sizeof(short)*us32.MaximumLength);
        baBuffer.fill(0);

        if(us32.Length)
        {
            if(readProcessMemory(baBuffer.data(),sizeof(short)*us32.MaximumLength,(void *)us32.Buffer,sizeof(short)*us32.Length))
            {
                sResult=QString::fromWCharArray((wchar_t *)baBuffer.data());
            }
        }
    }

    return sResult;
}

QString MainWindow::getAnsiString(void *pAddress)
{
    QString sResult;
    _STRING32 s32;
    QByteArray baBuffer;

    sResult="";

    if(readProcessMemory((char *)&s32,sizeof(s32),pAddress,sizeof(s32)))
    {
        baBuffer.resize(sizeof(char)*s32.MaximumLength);
        baBuffer.fill(0);

        if(s32.Length)
        {
            if(readProcessMemory(baBuffer.data(),sizeof(char)*s32.MaximumLength,(void *)s32.Buffer,sizeof(char)*s32.Length))
            {
                sResult=QString::fromLatin1(baBuffer.data());
            }
        }
    }

    return sResult;
}
QString MainWindow::getSZ(void *pAddress,int nMemorySize)
{
    QString sResult;
    QByteArray baBuffer;

    baBuffer.resize(nMemorySize);
    baBuffer.fill(0);

    sResult="";

    if(readProcessMemory(baBuffer.data(),baBuffer.size(),pAddress,nMemorySize))
    {
        sResult=baBuffer;
    }

    return sResult;
}

QString MainWindow::getListEntryLinks(void *pAddress, const QString &sStructur, int nOffset)
{
    QString sResult;
    _LIST_ENTRY32 le32;

    sResult="";

    if(readProcessMemory((char *)&le32,sizeof(le32),pAddress,sizeof(le32)))
    {
        //addLog(QString("%1").arg((long long)le32.Flink,0,16));
        //addLog(QString("%1").arg((long long)le32.Blink,0,16));
        //addLog(QString("%1").arg(nOffset));

        sResult=QString("<a href=\"%1#%2#%3\">Flink</a> <a href=\"%4#%5#%6\">Blink</a>").arg(sStructur).arg(addressToString((char *)le32.Flink+nOffset)).arg(addressToString((char *)le32.Flink+nOffset)).arg(sStructur).arg(addressToString((char *)le32.Blink+nOffset)).arg(addressToString((char *)le32.Blink+nOffset));
        sResult.append(" ");
    }

    return sResult;
}

QString MainWindow::getPELink(void *pAddress)
{
    QString sResult;
    void *pImage;

    if(readProcessMemory((char *)&pImage,sizeof(pImage),pAddress,sizeof(pImage)))
    {
        sResult=QString("<a href=\"%1#%2#%3\">PE</a>").arg("Image::PE").arg(addressToString(pImage)).arg(addressToString(pImage));

        sResult.append(" ");
    }

    return sResult;
}
QString MainWindow::addressToString(void *pAddress)
{
    QString sResult;

#ifndef __X64
    sResult=QString("0x%1").arg((unsigned int)pAddress,8,16,QChar('0'));
#else
    sResult=QString("0x%1").arg((unsigned long long)pAddress,16,16,QChar('0'));
#endif

    return sResult;
}
QString MainWindow::hexToString(void *pValue)
{
    QString sResult;

#ifndef __X64
    sResult=QString("0x%1").arg((unsigned int)pValue,0,16,QChar('0'));
#else
    sResult=QString("0x%1").arg((unsigned long long)pValue,0,16,QChar('0'));
#endif

    return sResult;
}
QString MainWindow::makeLink(const QString &sValue,const QString &sLink)
{
    QString sResult;

    if(sLink!="")
    {
        sResult=QString("<a href=\"%1\">%2</a>").arg(sLink).arg(sValue);
    }
    else
    {
        sResult=sValue;
    }

    return sResult;
}

void MainWindow::on_pushButtonPrototype_clicked()
{
    QString sIndex=ui->comboBoxType->currentText();

    Prototype prototype(this);

    prototype.setPrototype(getPrototype(sIndex));
    prototype.setTitle(sIndex);

    prototype.exec();
}

void MainWindow::on_pushButtonSave_clicked()
{
    QFile file;
    QString sFileName;
    QString sInfo;
    QString sTitle;

    QString sRecord=history.current();

    sTitle=sRecord.section("&",0,0);
    sInfo=sRecord.section("&",1,1);

    if(sInfo!="")
    {
        if(sTitle=="Start")
        {
            sTitle="index";
        }

        sFileName=QString("%1.html").arg(sTitle);

        sFileName=QFileDialog::getSaveFileName(this, tr("Save File..."),sFileName, tr("html-Files (*.html);;All Files (*)"));

        if(!sFileName.isEmpty())
        {
            file.setFileName(sFileName);
            file.open(QIODevice::ReadWrite);
            file.resize(0);
            file.write(sInfo.toLatin1());
            file.close();
        }
    }
}
QString MainWindow::getPrototype(const QString &sElementType)
{
    QString sResult;

    if((sElementType!="")&&(sElementType!="Image::PE"))
    {
        QByteArray baBuffer;

        QSqlDatabase DataBase;

        DataBase=Base;

        QSqlQuery query(DataBase);

        query.exec(QString("SELECT PROTO FROM PROTOS WHERE NAME=\"%1\"").arg(sElementType));
        query.next();

        if(query.value(0).isValid())
        {
            sResult=query.value(0).toString();
            baBuffer.append(sResult);
            sResult=QString(baBuffer.fromBase64(baBuffer));

            //addLog(QString("Database: Get %1 Size:%2").arg(sElementType).arg(nElementSize));
        }
        else
        {
            sResult="";

            addLog(QString("Database: Error(%1)").arg(sElementType));
            addLog(query.lastError().text());
        }
    }

    return sResult;
}

void MainWindow::on_actionAbout_triggered()
{
    About about(this);
    about.exec();
}

//void MainWindow::on_pushButtonHex_clicked()
//{
//    bool ok;
//    DialogHex dialoghex;
//    void *pAddress;

//#ifndef __X64
//    pAddress=(void *)(ui->lineEditAddress->text().toUInt(&ok,16));
//#else
//    pAddress=(void *)(ui->lineEditAddress->text().toULongLong(&ok,16));
//#endif

//    int nSize=ui->lineEditSize->text().toUInt(&ok,16);

//    if(nSize)
//    {
//        QByteArray baData;
//        baData.resize(nSize);

//        if(readProcessMemory(baData.data(),baData.size(),(void *)pAddress,nSize))
//        {
//            dialoghex.setData(baData,pAddress,nSize);
//        }
//    }

//    dialoghex.exec();
//}

//void MainWindow::on_radioButtonDecimal_toggled(bool checked)
//{
//    if(checked)
//    {
//        QString sTemp;
//        unsigned int nTemp;
//        bool bTemp;
//        sTemp=ui->lineEditPID->text();
//        nTemp=sTemp.toUInt(&bTemp,16);
//        sTemp=QString("%1").arg(nTemp);
//        ui->lineEditPID->setText(sTemp);

//        for(int i=0; i<ui->tableProcesses->rowCount(); i++)
//        {
//            sTemp=ui->tableProcesses->item(i,1)->text();
//            nTemp=sTemp.toUInt(&bTemp,16);
//            sTemp=QString("%1").arg(nTemp);
//            ui->tableProcesses->item(i,1)->setText(sTemp);
//        }
//    }
//}

//void MainWindow::on_radioButtonHeximal_toggled(bool checked)
//{

//    if(checked)
//    {
//        QString sTemp;
//        unsigned int nTemp;

//        sTemp=ui->lineEditPID->text();
//        nTemp=sTemp.toUInt();
//        sTemp=QString("%1").arg(nTemp,0,16);
//        ui->lineEditPID->setText(sTemp);

//        for(int i=0; i<ui->tableProcesses->rowCount(); i++)
//        {
//            sTemp=ui->tableProcesses->item(i,1)->text();
//            nTemp=sTemp.toUInt();
//            sTemp=QString("%1").arg(nTemp,0,16);
//            ui->tableProcesses->item(i,1)->setText(sTemp);
//        }
//    }
//}

void MainWindow::on_lineEditAddress_textChanged(const QString &arg1)
{
    //    ui->pushButtonHex->setEnabled(arg1!="");
}

QString MainWindow::handlePE(void *pAddress)
{
    QString sResult;
    HtmlTable htIDH;
    QByteArray baData;
    int nSize=0;
    char *pData;
    int nNumberOfSections=0;
    char *pSectionsTable;

    nSize=0x1000;
    baData.resize(nSize);
    pData=baData.data();

    if(readProcessMemory(baData.data(),baData.size(),pAddress,nSize))
    {
        //!!!
        if(*(unsigned short *)pData!=0x5A4D) // IMAGE_DOS_SIGNATURE
        {
            return "Invalid IMAGE_DOS_SIGNATURE";
        }

        htIDH.addString("IMAGE_DOS_HEADER");
        htIDH.addBR();

        htIDH.addTableBegin();
        htIDH.addRecord("e_magic",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_magic)),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_cblp",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_cblp)),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_cp",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_cp)),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_crlc",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_crlc)),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_cparhdr",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_cparhdr)),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_minalloc",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_minalloc)),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_maxalloc",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_maxalloc)),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_ss",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_ss)),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_sp",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_sp)),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_csum",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_csum)),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_ip",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_ip)),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_cs",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_cs)),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_lfarlc",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_lfarlc)),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_ovno",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_ovno)),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_res[0]",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_res)),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_res[1]",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_res)+2),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_res[2]",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_res)+4),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_res[3]",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_res)+6),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_oemid",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_oemid)),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_oeminfo",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_oeminfo)),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_res2[0]",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_res2)),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_res2[1]",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_res2)+2),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_res2[2]",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_res2)+4),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_res2[3]",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_res2)+6),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_res2[4]",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_res2)+8),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_res2[5]",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_res2)+10),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_res2[6]",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_res2)+12),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_res2[7]",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_res2)+14),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_res2[8]",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_res2)+16),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_res2[9]",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_DOS_HEADER,e_res2)+18),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("e_lfanew",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_DOS_HEADER,e_lfanew)),8,16,QChar('0')),"","","","","");
        htIDH.addTableEnd();

        htIDH.addBR();
        htIDH.addBR();

        if(*(unsigned int *)(pData+offsetof(_IMAGE_DOS_HEADER,e_lfanew))>0x500)
        {
            return "Invalid e_lfanew";
        }

        pData+=*(unsigned int *)(pData+offsetof(_IMAGE_DOS_HEADER,e_lfanew));

        if(*(unsigned int *)(pData+offsetof(_IMAGE_NT_HEADERS32,Signature))!=0x00004550) // IMAGE_NT_SIGNATURE
        {
            return "Invalid IMAGE_NT_SIGNATURE";
        }

        htIDH.addString("IMAGE_NT_HEADERS");
        htIDH.addBR();


        htIDH.addTableBegin();
        htIDH.addRecord("Signature",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_NT_HEADERS32,Signature)),8,16,QChar('0')),"","","","","");
        htIDH.addTableEnd();
        htIDH.addBR();
        htIDH.addBR();

        pData+=4;

        htIDH.addString("IMAGE_FILE_HEADER");
        htIDH.addBR();

        htIDH.addTableBegin();
        htIDH.addRecord("Machine",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_FILE_HEADER,Machine)),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("NumberOfSections",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_FILE_HEADER,NumberOfSections)),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("TimeDateStamp",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_FILE_HEADER,TimeDateStamp)),8,16,QChar('0')),"","","","","");
        htIDH.addRecord("PointerToSymbolTable",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_FILE_HEADER,PointerToSymbolTable)),8,16,QChar('0')),"","","","","");
        htIDH.addRecord("NumberOfSymbols",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_FILE_HEADER,NumberOfSymbols)),8,16,QChar('0')),"","","","","");
        htIDH.addRecord("SizeOfOptionalHeader",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_FILE_HEADER,SizeOfOptionalHeader)),4,16,QChar('0')),"","","","","");
        htIDH.addRecord("Characteristics",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_FILE_HEADER,Characteristics)),4,16,QChar('0')),"","","","","");
        htIDH.addTableEnd();
        htIDH.addBR();
        htIDH.addBR();

        pSectionsTable=pData+*(unsigned short *)(pData+offsetof(_IMAGE_FILE_HEADER,SizeOfOptionalHeader))+sizeof(_IMAGE_FILE_HEADER);
        nNumberOfSections=*(unsigned short *)(pData+offsetof(_IMAGE_FILE_HEADER,NumberOfSections));

        pData+=sizeof(_IMAGE_FILE_HEADER);

        htIDH.addString("IMAGE_OPTIONAL_HEADER");
        htIDH.addBR();

        htIDH.addTableBegin();

        if(*(unsigned short *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,Magic))==0x010b)
        {
            // PE32
            htIDH.addRecord("Magic",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,Magic)),4,16,QChar('0')),"","","","","");
            htIDH.addRecord("MajorLinkerVersion",QString("0x%1").arg(*(unsigned char *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,MajorLinkerVersion)),2,16,QChar('0')),"","","","","");
            htIDH.addRecord("MinorLinkerVersion",QString("0x%1").arg(*(unsigned char *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,MinorLinkerVersion)),2,16,QChar('0')),"","","","","");
            htIDH.addRecord("SizeOfCode",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,SizeOfCode)),8,16,QChar('0')),"","","","","");
            htIDH.addRecord("SizeOfInitializedData",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,SizeOfInitializedData)),8,16,QChar('0')),"","","","","");
            htIDH.addRecord("SizeOfUninitializedData",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,SizeOfUninitializedData)),8,16,QChar('0')),"","","","","");
            htIDH.addRecord("AddressOfEntryPoint",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,AddressOfEntryPoint)),8,16,QChar('0')),"","","","","");
            htIDH.addRecord("BaseOfCode",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,BaseOfCode)),8,16,QChar('0')),"","","","","");
            //            htIDH.addRecord("BaseOfData",QString("0x%1").arg( *(unsigned int *)(pData+offsetof(IMAGE_OPTIONAL_HEADER,BaseOfData)),8,16,QChar('0')),"","","","","");

            htIDH.addRecord("ImageBase",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,ImageBase)),8,16,QChar('0')),"","","","","");
            htIDH.addRecord("SectionAlignment",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,SectionAlignment)),8,16,QChar('0')),"","","","","");
            htIDH.addRecord("FileAlignment",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,FileAlignment)),8,16,QChar('0')),"","","","","");

            htIDH.addRecord("MajorOperatingSystemVersion",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,MajorOperatingSystemVersion)),4,16,QChar('0')),"","","","","");
            htIDH.addRecord("MinorOperatingSystemVersion",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,MinorOperatingSystemVersion)),4,16,QChar('0')),"","","","","");
            htIDH.addRecord("MajorImageVersion",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,MajorImageVersion)),4,16,QChar('0')),"","","","","");
            htIDH.addRecord("MinorImageVersion",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,MinorImageVersion)),4,16,QChar('0')),"","","","","");
            htIDH.addRecord("MajorSubsystemVersion",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,MajorSubsystemVersion)),4,16,QChar('0')),"","","","","");
            htIDH.addRecord("MinorSubsystemVersion",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,MinorSubsystemVersion)),4,16,QChar('0')),"","","","","");

            htIDH.addRecord("Win32VersionValue",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,Win32VersionValue)),8,16,QChar('0')),"","","","","");
            htIDH.addRecord("SizeOfImage",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,SizeOfImage)),8,16,QChar('0')),"","","","","");
            htIDH.addRecord("SizeOfHeaders",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,SizeOfHeaders)),8,16,QChar('0')),"","","","","");
            htIDH.addRecord("CheckSum",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,CheckSum)),8,16,QChar('0')),"","","","","");

            htIDH.addRecord("Subsystem",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,Subsystem)),4,16,QChar('0')),"","","","","");
            htIDH.addRecord("DllCharacteristics",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,DllCharacteristics)),4,16,QChar('0')),"","","","","");

            htIDH.addRecord("SizeOfStackReserve",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,SizeOfStackReserve)),8,16,QChar('0')),"","","","","");
            htIDH.addRecord("SizeOfStackCommit",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,SizeOfStackCommit)),8,16,QChar('0')),"","","","","");
            htIDH.addRecord("SizeOfHeapReserve",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,SizeOfHeapReserve)),8,16,QChar('0')),"","","","","");
            htIDH.addRecord("SizeOfHeapCommit",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,SizeOfHeapCommit)),8,16,QChar('0')),"","","","","");
            htIDH.addRecord("LoaderFlags",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,LoaderFlags)),8,16,QChar('0')),"","","","","");
            htIDH.addRecord("NumberOfRvaAndSizes",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER32,NumberOfRvaAndSizes)),8,16,QChar('0')),"","","","","");

            pData+=sizeof(_IMAGE_OPTIONAL_HEADER32);
        }
        else if(*(unsigned short *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,Magic))==0x020b)
        {
            // PE64
            htIDH.addRecord("Magic",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,Magic)),4,16,QChar('0')),"","","","","");
            htIDH.addRecord("MajorLinkerVersion",QString("0x%1").arg(*(unsigned char *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,MajorLinkerVersion)),2,16,QChar('0')),"","","","","");
            htIDH.addRecord("MinorLinkerVersion",QString("0x%1").arg(*(unsigned char *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,MinorLinkerVersion)),2,16,QChar('0')),"","","","","");
            htIDH.addRecord("SizeOfCode",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,SizeOfCode)),8,16,QChar('0')),"","","","","");
            htIDH.addRecord("SizeOfInitializedData",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,SizeOfInitializedData)),8,16,QChar('0')),"","","","","");
            htIDH.addRecord("SizeOfUninitializedData",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,SizeOfUninitializedData)),8,16,QChar('0')),"","","","","");
            htIDH.addRecord("AddressOfEntryPoint",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,AddressOfEntryPoint)),8,16,QChar('0')),"","","","","");
            htIDH.addRecord("BaseOfCode",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,BaseOfCode)),8,16,QChar('0')),"","","","","");


            htIDH.addRecord("ImageBase",QString("0x%1").arg(*(unsigned long long *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,ImageBase)),16,16,QChar('0')),"","","","","");
            htIDH.addRecord("SectionAlignment",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,SectionAlignment)),8,16,QChar('0')),"","","","","");
            htIDH.addRecord("FileAlignment",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,FileAlignment)),8,16,QChar('0')),"","","","","");

            htIDH.addRecord("MajorOperatingSystemVersion",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,MajorOperatingSystemVersion)),4,16,QChar('0')),"","","","","");
            htIDH.addRecord("MinorOperatingSystemVersion",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,MinorOperatingSystemVersion)),4,16,QChar('0')),"","","","","");
            htIDH.addRecord("MajorImageVersion",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,MajorImageVersion)),4,16,QChar('0')),"","","","","");
            htIDH.addRecord("MinorImageVersion",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,MinorImageVersion)),4,16,QChar('0')),"","","","","");
            htIDH.addRecord("MajorSubsystemVersion",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,MajorSubsystemVersion)),4,16,QChar('0')),"","","","","");
            htIDH.addRecord("MinorSubsystemVersion",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,MinorSubsystemVersion)),4,16,QChar('0')),"","","","","");

            htIDH.addRecord("Win32VersionValue",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,Win32VersionValue)),8,16,QChar('0')),"","","","","");
            htIDH.addRecord("SizeOfImage",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,SizeOfImage)),8,16,QChar('0')),"","","","","");
            htIDH.addRecord("SizeOfHeaders",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,SizeOfHeaders)),8,16,QChar('0')),"","","","","");
            htIDH.addRecord("CheckSum",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,CheckSum)),8,16,QChar('0')),"","","","","");

            htIDH.addRecord("Subsystem",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,Subsystem)),4,16,QChar('0')),"","","","","");
            htIDH.addRecord("DllCharacteristics",QString("0x%1").arg(*(unsigned short *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,DllCharacteristics)),4,16,QChar('0')),"","","","","");

            htIDH.addRecord("SizeOfStackReserve",QString("0x%1").arg(*(unsigned long long *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,SizeOfStackReserve)),16,16,QChar('0')),"","","","","");
            htIDH.addRecord("SizeOfStackCommit",QString("0x%1").arg(*(unsigned long long *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,SizeOfStackCommit)),16,16,QChar('0')),"","","","","");
            htIDH.addRecord("SizeOfHeapReserve",QString("0x%1").arg(*(unsigned long long *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,SizeOfHeapReserve)),16,16,QChar('0')),"","","","","");
            htIDH.addRecord("SizeOfHeapCommit",QString("0x%1").arg(*(unsigned long long *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,SizeOfHeapCommit)),16,16,QChar('0')),"","","","","");
            htIDH.addRecord("LoaderFlags",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,LoaderFlags)),8,16,QChar('0')),"","","","","");
            htIDH.addRecord("NumberOfRvaAndSizes",QString("0x%1").arg(*(unsigned int *)(pData+offsetof(_IMAGE_OPTIONAL_HEADER64,NumberOfRvaAndSizes)),8,16,QChar('0')),"","","","","");

            pData+=sizeof(_IMAGE_OPTIONAL_HEADER64);
        }

        htIDH.addTableEnd();
        htIDH.addBR();
        htIDH.addBR();

        pData-=16*sizeof(_IMAGE_DATA_DIRECTORY);

        htIDH.addString("Directories");
        htIDH.addBR();

        htIDH.addTableBegin();

        htIDH.addRecord("EXPORT",QString("0x%1").arg(*(unsigned int *)(pData+0*sizeof(_IMAGE_DATA_DIRECTORY)),8,16,QChar('0')),QString("0x%1").arg(*(unsigned int *)(pData+0*sizeof(_IMAGE_DATA_DIRECTORY)+4),8,16,QChar('0')),"","","","");
        htIDH.addRecord("IMPORT",QString("0x%1").arg(*(unsigned int *)(pData+1*sizeof(_IMAGE_DATA_DIRECTORY)),8,16,QChar('0')),QString("0x%1").arg(*(unsigned int *)(pData+1*sizeof(_IMAGE_DATA_DIRECTORY)+4),8,16,QChar('0')),"","","","");
        htIDH.addRecord("RESOURCE",QString("0x%1").arg(*(unsigned int *)(pData+2*sizeof(_IMAGE_DATA_DIRECTORY)),8,16,QChar('0')),QString("0x%1").arg(*(unsigned int *)(pData+2*sizeof(_IMAGE_DATA_DIRECTORY)+4),8,16,QChar('0')),"","","","");
        htIDH.addRecord("EXCEPTION",QString("0x%1").arg(*(unsigned int *)(pData+3*sizeof(_IMAGE_DATA_DIRECTORY)),8,16,QChar('0')),QString("0x%1").arg(*(unsigned int *)(pData+3*sizeof(_IMAGE_DATA_DIRECTORY)+4),8,16,QChar('0')),"","","","");
        htIDH.addRecord("SECURITY",QString("0x%1").arg(*(unsigned int *)(pData+4*sizeof(_IMAGE_DATA_DIRECTORY)),8,16,QChar('0')),QString("0x%1").arg(*(unsigned int *)(pData+4*sizeof(_IMAGE_DATA_DIRECTORY)+4),8,16,QChar('0')),"","","","");
        htIDH.addRecord("BASERELOC",QString("0x%1").arg(*(unsigned int *)(pData+5*sizeof(_IMAGE_DATA_DIRECTORY)),8,16,QChar('0')),QString("0x%1").arg(*(unsigned int *)(pData+5*sizeof(_IMAGE_DATA_DIRECTORY)+4),8,16,QChar('0')),"","","","");
        htIDH.addRecord("DEBUG",QString("0x%1").arg(*(unsigned int *)(pData+6*sizeof(_IMAGE_DATA_DIRECTORY)),8,16,QChar('0')),QString("0x%1").arg(*(unsigned int *)(pData+6*sizeof(_IMAGE_DATA_DIRECTORY)+4),8,16,QChar('0')),"","","","");
        htIDH.addRecord("COPYRIGHT",QString("0x%1").arg(*(unsigned int *)(pData+7*sizeof(_IMAGE_DATA_DIRECTORY)),8,16,QChar('0')),QString("0x%1").arg(*(unsigned int *)(pData+7*sizeof(_IMAGE_DATA_DIRECTORY)+4),8,16,QChar('0')),"","","","");
        htIDH.addRecord("GLOBALPTR",QString("0x%1").arg(*(unsigned int *)(pData+8*sizeof(_IMAGE_DATA_DIRECTORY)),8,16,QChar('0')),QString("0x%1").arg(*(unsigned int *)(pData+8*sizeof(_IMAGE_DATA_DIRECTORY)+4),8,16,QChar('0')),"","","","");
        htIDH.addRecord("TLS",QString("0x%1").arg(*(unsigned int *)(pData+9*sizeof(_IMAGE_DATA_DIRECTORY)),8,16,QChar('0')),QString("0x%1").arg(*(unsigned int *)(pData+9*sizeof(_IMAGE_DATA_DIRECTORY)+4),8,16,QChar('0')),"","","","");
        htIDH.addRecord("LOAD_CONFIG",QString("0x%1").arg(*(unsigned int *)(pData+10*sizeof(_IMAGE_DATA_DIRECTORY)),8,16,QChar('0')),QString("0x%1").arg(*(unsigned int *)(pData+10*sizeof(_IMAGE_DATA_DIRECTORY)+4),8,16,QChar('0')),"","","","");
        htIDH.addRecord("BOUND_IMPORT",QString("0x%1").arg(*(unsigned int *)(pData+11*sizeof(_IMAGE_DATA_DIRECTORY)),8,16,QChar('0')),QString("0x%1").arg(*(unsigned int *)(pData+11*sizeof(_IMAGE_DATA_DIRECTORY)+4),8,16,QChar('0')),"","","","");
        htIDH.addRecord("IAT",QString("0x%1").arg(*(unsigned int *)(pData+12*sizeof(_IMAGE_DATA_DIRECTORY)),8,16,QChar('0')),QString("0x%1").arg(*(unsigned int *)(pData+12*sizeof(_IMAGE_DATA_DIRECTORY)+4),8,16,QChar('0')),"","","","");
        htIDH.addRecord("DELAY_IMPORT",QString("0x%1").arg(*(unsigned int *)(pData+13*sizeof(_IMAGE_DATA_DIRECTORY)),8,16,QChar('0')),QString("0x%1").arg(*(unsigned int *)(pData+13*sizeof(_IMAGE_DATA_DIRECTORY)+4),8,16,QChar('0')),"","","","");
        htIDH.addRecord("COM_DESCRIPTOR",QString("0x%1").arg(*(unsigned int *)(pData+14*sizeof(_IMAGE_DATA_DIRECTORY)),8,16,QChar('0')),QString("0x%1").arg(*(unsigned int *)(pData+14*sizeof(_IMAGE_DATA_DIRECTORY)+4),8,16,QChar('0')),"","","","");
        htIDH.addRecord("Reserved",QString("0x%1").arg(*(unsigned int *)(pData+15*sizeof(_IMAGE_DATA_DIRECTORY)),8,16,QChar('0')),QString("0x%1").arg(*(unsigned int *)(pData+15*sizeof(_IMAGE_DATA_DIRECTORY)+4),8,16,QChar('0')),"","","","");

        htIDH.addTableEnd();
        htIDH.addBR();
        htIDH.addBR();

        for(int i=0; i<nNumberOfSections; i++)
        {
            htIDH.addString(QString("Section(%1)").arg(i));
            htIDH.addBR();

            htIDH.addTableBegin();

            htIDH.addRecord("Name",QString(pSectionsTable),"","","","","");
            htIDH.addRecord("VirtualSize",QString("0x%1").arg(*(unsigned int *)(pSectionsTable+offsetof(_IMAGE_SECTION_HEADER,Misc.VirtualSize)),8,16,QChar('0')),"","","","","");
            htIDH.addRecord("VirtualAddress",QString("0x%1").arg(*(unsigned int *)(pSectionsTable+offsetof(_IMAGE_SECTION_HEADER,VirtualAddress)),8,16,QChar('0')),"","","","","");
            htIDH.addRecord("SizeOfRawData",QString("0x%1").arg(*(unsigned int *)(pSectionsTable+offsetof(_IMAGE_SECTION_HEADER,SizeOfRawData)),8,16,QChar('0')),"","","","","");
            htIDH.addRecord("PointerToRawData",QString("0x%1").arg(*(unsigned int *)(pSectionsTable+offsetof(_IMAGE_SECTION_HEADER,PointerToRawData)),8,16,QChar('0')),"","","","","");
            htIDH.addRecord("PointerToRelocations",QString("0x%1").arg(*(unsigned int *)(pSectionsTable+offsetof(_IMAGE_SECTION_HEADER,PointerToRelocations)),8,16,QChar('0')),"","","","","");
            htIDH.addRecord("PointerToLinenumbers",QString("0x%1").arg(*(unsigned int *)(pSectionsTable+offsetof(_IMAGE_SECTION_HEADER,PointerToLinenumbers)),8,16,QChar('0')),"","","","","");
            htIDH.addRecord("NumberOfRelocations",QString("0x%1").arg(*(unsigned char *)(pSectionsTable+offsetof(_IMAGE_SECTION_HEADER,NumberOfRelocations)),4,16,QChar('0')),"","","","","");
            htIDH.addRecord("NumberOfLinenumbers",QString("0x%1").arg(*(unsigned char *)(pSectionsTable+offsetof(_IMAGE_SECTION_HEADER,NumberOfLinenumbers)),4,16,QChar('0')),"","","","","");
            htIDH.addRecord("Characteristics",QString("0x%1").arg(*(unsigned int *)(pSectionsTable+offsetof(_IMAGE_SECTION_HEADER,Characteristics)),8,16,QChar('0')),"","","","","");

            htIDH.addTableEnd();
            htIDH.addBR();
            htIDH.addBR();

            pSectionsTable+=sizeof(_IMAGE_SECTION_HEADER);
        }
    }

    sResult.append(htIDH.toString());

    return sResult;
}

QByteArray MainWindow::ReadFromMemory(QVariant parameter, unsigned long long nOffset, unsigned long long nSize, bool *pbIsReadOnly)
{
    QByteArray baResult;

    *pbIsReadOnly=true;

    baResult.resize(nSize);
    baResult.fill(0);

    //    unsigned int nG=parameter.toUInt();
    //    nG++;
    bool bResult;

    if(bKernel)
    {
        bResult=kmReadProcessMemory((void *)(parameter.toUInt()),baResult.data(),baResult.size(),(void *)nOffset,nSize);
    }
    else
    {
        bResult=umReadProcessMemory((void *)(parameter.toUInt()),baResult.data(),baResult.size(),(void *)nOffset,nSize);
    }

    if(!bResult)
    {
        baResult.resize(0);
    }

    return baResult;
}

MainWindow::um_ReadProcessMemory MainWindow::umReadProcessMemory=nullptr;
MainWindow::km_ReadProcessMemory MainWindow::kmReadProcessMemory=nullptr;
bool MainWindow::bKernel=false;

void MainWindow::on_pushButtonMemoryMap_clicked()
{
    DialogMemoryMap dmm;
    dmm.setData(umMemoryQuery,&MainWindow::ReadFromMemory,hPID);
    dmm.exec();
}

bool MainWindow::getMemoryInformation(void *pAddress, MainWindow::_MEMORY_BASIC_INFORMATION *pMBI)
{
    return (umMemoryQuery(hPID,pAddress,(char *)pMBI,sizeof(_MEMORY_BASIC_INFORMATION))!=0);
}

bool MainWindow::checkAddress(void *pAddress)
{
    bool bResult=false;

    if(pAddress)
    {
        _MEMORY_BASIC_INFORMATION mbi;

        if(getMemoryInformation(pAddress,&mbi))
        {
            if(mbi.Type!=0)
            {
                bResult=true;
            }
        }
    }

    return bResult;
}

void MainWindow::on_radioButtonKernelMode_toggled(bool checked)
{
    bKernel=ui->radioButtonKernelMode->isChecked();
}

void MainWindow::on_radioButtonUserMode_toggled(bool checked)
{
    bKernel=ui->radioButtonKernelMode->isChecked();
}
