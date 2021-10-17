// Copyright (c) 2017-2021 hors<horsicq@gmail.com>
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
#include "guimainwindow.h"
#include "ui_guimainwindow.h"

GuiMainWindow::GuiMainWindow(QWidget *pParent) :
    QMainWindow(pParent),
    ui(new Ui::GuiMainWindow)
{
    ui->setupUi(this);

    if(!XProcess::setDebugPrivilege(true))
    {
        QMessageBox::critical(this,tr("Error"),tr("Please run the program as an administrator"));

        exit(1);
    }

    setWindowTitle(XOptions::getTitle(X_APPLICATIONDISPLAYNAME,X_APPLICATIONVERSION));

    g_xOptions.setName(X_OPTIONSFILE);

    QList<XOptions::ID> listIDs;

    listIDs.append(XOptions::ID_STYLE);
    listIDs.append(XOptions::ID_QSS);
    listIDs.append(XOptions::ID_LANG);
    listIDs.append(XOptions::ID_STAYONTOP);
    listIDs.append(XOptions::ID_SEARCHSIGNATURESPATH);
    listIDs.append(XOptions::ID_STRUCTSPATH);

    g_xOptions.setValueIDs(listIDs);
    g_xOptions.load();

    g_xShortcuts.setName(X_SHORTCUTSFILE);

    g_xShortcuts.addGroup(XShortcuts::ID_PROCESS);
    g_xShortcuts.addGroup(XShortcuts::ID_DISASM);
    g_xShortcuts.addGroup(XShortcuts::ID_HEX);
    g_xShortcuts.addGroup(XShortcuts::ID_STRINGS);
    g_xShortcuts.addGroup(XShortcuts::ID_SIGNATURES);

    g_xShortcuts.load();

    ui->widgetProcesses->setActive(true);

    adjust();
}

GuiMainWindow::~GuiMainWindow()
{
    g_xOptions.save();
    g_xShortcuts.save();

    delete ui;
}

void GuiMainWindow::adjust()
{
    g_xOptions.adjustStayOnTop(this);
    ui->widgetProcesses->setShortcuts(&g_xShortcuts);

    XProcessWidget::OPTIONS options={};
    options.sStructsPath=g_xOptions.getStructsPath();
    options.sSearchSignaturesPath=g_xOptions.getSearchSignaturesPath();

    ui->widgetProcesses->setOptions(options);
}

void GuiMainWindow::on_actionExit_triggered()
{
    this->close();
}

//QString GuiMainWindow::addTable(void *pAddress,int nStartOffset, const QString sElementType, const QString sElementName, char *pData, int nDataSize)
//{
//    int nIndex;
//    QString sDBTable;
//    int nOffset;
//    QString sType;
//    QString sName;
//    QString sComment;
//    int nSize;
//    int nShow;
//    QString sValue;
//    QString sLink;
//    QString sIndex;
//    QString sOffset;
//    unsigned char ucValue;
//    unsigned short usValue;
//    unsigned int unValue;
//    unsigned long long un64Value;
//    int nBitCount;
//    int nBitOffset=0;
//    long long nMask;
//    HtmlTable ht;
//    QString sResult;
//    int nElementSize;
//    void *pCurrentAddress=nullptr;

//    QSqlDatabase DataBase;

//    DataBase=Base;

//    QSqlQuery query(DataBase);

//    nIndex=getIndex(sElementName);
//    nElementSize=getElementSize(sElementType);
//    ht.addTableBegin();

//    if(nIndex==1)
//    {
//        sDBTable=sElementType;
//        sDBTable.replace(" ","$");

//        query.exec(QString("SELECT * FROM %1").arg(sDBTable));

//        while(query.next())
//        {
//            // get all fields
//            nOffset=query.value(0).toInt();
//            sType=query.value(1).toString();
//            sName=query.value(2).toString();
//            sComment=query.value(3).toString();

//#ifndef __X64
//            pCurrentAddress=(void *)((int)pAddress+nOffset);
//#else
//            pCurrentAddress=(void *)((long long)pAddress+nOffset);
//#endif
//            nSize=getElementSize(sType);
//            nShow=getElementShow(sType);

//            if(sComment.contains("#LE_LDTE0"))
//            {
//                sComment.replace("#LE_LDTE0",getListEntryLinks(pCurrentAddress,"struct _LDR_DATA_TABLE_ENTRY",0));
//            }

//            if(sComment.contains("#LE_LDTE1"))
//            {
//                sComment.replace("#LE_LDTE1",getListEntryLinks(pCurrentAddress,"struct _LDR_DATA_TABLE_ENTRY",-sizeof(_LIST_ENTRY32)));
//            }

//            if(sComment.contains("#LE_LDTE2"))
//            {
//                sComment.replace("#LE_LDTE2",getListEntryLinks(pCurrentAddress,"struct _LDR_DATA_TABLE_ENTRY",-sizeof(_LIST_ENTRY32)*2));
//            }

//            if(sComment.contains("#LE_YS"))
//            {
//                sComment.replace("#LE_YS",getListEntryLinks(pCurrentAddress,sElementType,-nOffset));
//            }

//            if(sComment.contains("#UNICODE_STRING"))
//            {
//                sComment.replace("#UNICODE_STRING",QString("Unicode String=\"%1\"").arg(getUnicodeString(pCurrentAddress)));
//            }

//            if(sComment.contains("#ANSI_STRING"))
//            {
//                sComment.replace("#ANSI_STRING",QString("Ansi String=\"%1\"").arg(getUnicodeString(pCurrentAddress)));
//            }

//            if(sComment.contains("#32"))
//            {
//                sComment.replace("#32","");
//                nSize=4;
//            }

//            if(sComment.contains("#PE"))
//            {
//                sComment.replace("#PE",getPELink(pCurrentAddress));
//            }

//            if((nShow==0)||(nShow==3)||(nShow==4))
//            {
//                sLink=QString("%1#%2#%3").arg(sType).arg(sName).arg(addressToString(pCurrentAddress));

//                sValue="...";
//                //                if(nShow==3)        // Unicode
//                //                {
//                //                    sComment.append(QString("Unicode String=\"%1\"").arg(getUnicodeString(pCurrentAddress)));
//                //                }
//                //                else if(nShow==4)   // Ansi
//                //                {
//                //                    sComment.append(QString("Ansi String=\"%1\"").arg(getAnsiString(pCurrentAddress)));
//                //                }
//            }
//                else
//                {
//                    // not bitset

//                    //                    if(nSize==sizeof(void *))
//                    //                    {
//                    //                        if(nSize==4)
//                    //                        {
//                    //                            if(checkAddress((void *)unValue))
//                    //                            {
//                    //                                sComment+=QString(" <a href=\"%1#%2#%3\">HEX</a>").arg("HEX").arg(addressToString((void *)unValue)).arg(addressToString((void *)unValue));
//                    //                            }
//                    //                        }
//                    //                        else if(nSize==8)
//                    //                        {
//                    //                            if(checkAddress((void *)un64Value))
//                    //                            {
//                    //                                sComment+=QString(" <a href=\"%1#%2#%3\">HEX</a>").arg("HEX").arg(addressToString((void *)un64Value)).arg(addressToString((void *)un64Value));
//                    //                            }
//                    //                        }
//                    //                    }
//                }

//                if(nShow==1)
//                {
//                    sLink="";
//                }
//                else if(nShow==2)
//                {
//                    sLink=sType;
//                    sLink.remove(" *");
//                    sLink=QString("%1#%2#%3.html").arg(sLink).arg(sName).arg(sValue);
//                }

//                // zhirniy shrift?
//                if(sValue=="0x00000000")
//                {
//                    sLink="";
//                }
//                else if(sValue=="0xffffffff")
//                {
//                    sLink="";
//                }
//                else if(sValue=="0x0000000000000000")
//                {
//                    sLink="";
//                }
//            }

//            if(sName.contains("["))
//            {
//                sIndex=sName;
//                sIndex=sIndex.section("[",1,1);
//                sIndex=sIndex.section("]",0,0);
//                nIndex=sIndex.toInt();

//                if(nIndex!=1)
//                {
//                    sLink=QString("%1#%2#%3").arg(sType).arg(sName).arg(addressToString(pCurrentAddress));
//                    sValue="...";

//                    if(sComment.contains("#SZ"))
//                    {
//                        sComment.replace("#SZ",QString("szString=\"%1\"").arg(getSZ(pCurrentAddress,nIndex)));
//                    }
//                }
//            }

//            if(sValue=="0x00")
//            {

//            }
//            else if(sValue=="0x0000")
//            {
//            }
//            else if(sValue=="0x00000000")
//            {
//            }
//            else if(sValue=="0x0000000000000000")
//            {
//            }
//            else
//            {
//                sValue=QString("<b>%1</b>").arg(sValue);
//            }

//            sOffset=addressToString((void *)(nOffset+nStartOffset));
//            ht.addRecord(addressToString(pCurrentAddress),sOffset,sType,sName,makeLink(sValue,sLink),sComment,"");
//        }
//    }
//    else // Array
//    {
//        sType=sElementType;
//        sName=sElementName;
//        sName=sName.section("[",0,0);

//        nSize=getElementSize(sType);
//        nShow=getElementShow(sType);

//        for(int i=0; i<nIndex; i++)
//        {
//            nOffset=nElementSize*i;
//#ifndef __X64
//            pCurrentAddress=(void *)((int)pAddress+nOffset);
//#else
//            pCurrentAddress=(void *)((long long)pAddress+nOffset);
//#endif
//            if(nShow==0)
//            {
//                sLink=QString("%1#%2#%3.html").arg(sType).arg(sName).arg(addressToString(pCurrentAddress));
//                sValue="...";
//            }
//            else
//            {
//                if(nSize==1)
//                {
//                    sValue=QString("0x%1").arg(*(unsigned char *)(pData+nOffset),2,16,QChar('0'));
//                }
//                else if(nSize==2)
//                {
//                    sValue=QString("0x%1").arg(*(unsigned short *)(pData+nOffset),4,16,QChar('0'));
//                }
//                else if(nSize==4)
//                {
//                    sValue=QString("0x%1").arg(*(unsigned int *)(pData+nOffset),8,16,QChar('0'));
//                }
//                else if(nSize==8)
//                {
//                    sValue=QString("0x%1").arg(*(unsigned long long *)(pData+nOffset),16,16,QChar('0'));
//                }
//                else
//                {
//                    addLog("Unknown size");
//                }

//                if(nShow==1)
//                {
//                    sLink="";
//                }
//                else if(nShow==2)
//                {
//                    sLink=sType;
//                    sLink.remove(" *");
//                    sLink=QString("%1#%2#%3").arg(sLink).arg(sName).arg(sValue);
//                }

//                if(sValue=="0x00000000")
//                {
//                    sLink="";
//                }
//                else if(sValue=="0xffffffff")
//                {
//                    sLink="";
//                }
//                else if(sValue=="0x0000000000000000")
//                {
//                    sLink="";
//                }
//            }

//            if(sValue=="0x00")
//            {

//            }
//            else if(sValue=="0x0000")
//            {
//            }
//            else if(sValue=="0x00000000")
//            {
//            }
//            else if(sValue=="0x0000000000000000")
//            {
//            }
//            else
//            {
//                sValue=QString("<b>%1</b>").arg(sValue);
//            }

//            sOffset=QString("0x%1").arg((unsigned int)(nOffset+nStartOffset),8,16,QChar('0'));
//            ht.addRecord(addressToString(pCurrentAddress),sOffset,sElementType,QString("%1[%2]").arg(sName).arg(i),makeLink(sValue,sLink),sComment,"");
//        }
//    }

//    ht.addTableEnd();
//    sResult=ht.toString();

//    return sResult;
//}

//int GuiMainWindow::getIndex(const QString sElementName)
//{
//    int nIndex=1;
//    QString sIndex;

//    if(sElementName.contains("["))
//    {
//        sIndex=sElementName;
//        sIndex=sIndex.section("[",1,1);
//        sIndex=sIndex.section("]",0,0);
//        nIndex=sIndex.toInt();
//    }

//    return nIndex;
//}


//QString GuiMainWindow::getUnicodeString(void *pAddress)
//{
//    QString sResult;
//    _UNICODE_STRING32 us32;
//    QByteArray baBuffer;

//    sResult="";

//    if(readProcessMemory((char *)&us32,sizeof(us32),pAddress,sizeof(us32)))
//    {
//        baBuffer.resize(sizeof(short)*us32.MaximumLength);
//        baBuffer.fill(0);

//        if(us32.Length)
//        {
//            if(readProcessMemory(baBuffer.data(),sizeof(short)*us32.MaximumLength,(void *)us32.Buffer,sizeof(short)*us32.Length))
//            {
//                sResult=QString::fromWCharArray((wchar_t *)baBuffer.data());
//            }
//        }
//    }

//    return sResult;
//}

//QString GuiMainWindow::getAnsiString(void *pAddress)
//{
//    QString sResult;
//    _STRING32 s32;
//    QByteArray baBuffer;

//    sResult="";

//    if(readProcessMemory((char *)&s32,sizeof(s32),pAddress,sizeof(s32)))
//    {
//        baBuffer.resize(sizeof(char)*s32.MaximumLength);
//        baBuffer.fill(0);

//        if(s32.Length)
//        {
//            if(readProcessMemory(baBuffer.data(),sizeof(char)*s32.MaximumLength,(void *)s32.Buffer,sizeof(char)*s32.Length))
//            {
//                sResult=QString::fromLatin1(baBuffer.data());
//            }
//        }
//    }

//    return sResult;
//}
//QString GuiMainWindow::getSZ(void *pAddress,int nMemorySize)
//{
//    QString sResult;
//    QByteArray baBuffer;

//    baBuffer.resize(nMemorySize);
//    baBuffer.fill(0);

//    sResult="";

//    if(readProcessMemory(baBuffer.data(),baBuffer.size(),pAddress,nMemorySize))
//    {
//        sResult=baBuffer;
//    }

//    return sResult;
//}

//QString GuiMainWindow::getListEntryLinks(void *pAddress, const QString &sStructur, int nOffset)
//{
//    QString sResult;
//    _LIST_ENTRY32 le32;

//    sResult="";

//    if(readProcessMemory((char *)&le32,sizeof(le32),pAddress,sizeof(le32)))
//    {
//        //addLog(QString("%1").arg((long long)le32.Flink,0,16));
//        //addLog(QString("%1").arg((long long)le32.Blink,0,16));
//        //addLog(QString("%1").arg(nOffset));

//        sResult=QString("<a href=\"%1#%2#%3\">Flink</a> <a href=\"%4#%5#%6\">Blink</a>").arg(sStructur).arg(addressToString((char *)le32.Flink+nOffset)).arg(addressToString((char *)le32.Flink+nOffset)).arg(sStructur).arg(addressToString((char *)le32.Blink+nOffset)).arg(addressToString((char *)le32.Blink+nOffset));
//        sResult.append(" ");
//    }

//    return sResult;
//}

//QString GuiMainWindow::getPELink(void *pAddress)
//{
//    QString sResult;
//    void *pImage;

//    if(readProcessMemory((char *)&pImage,sizeof(pImage),pAddress,sizeof(pImage)))
//    {
//        sResult=QString("<a href=\"%1#%2#%3\">PE</a>").arg("Image::PE").arg(addressToString(pImage)).arg(addressToString(pImage));

//        sResult.append(" ");
//    }

//    return sResult;
//}
//QString GuiMainWindow::addressToString(void *pAddress)
//{
//    QString sResult;

//#ifndef __X64
//    sResult=QString("0x%1").arg((unsigned int)pAddress,8,16,QChar('0'));
//#else
//    sResult=QString("0x%1").arg((unsigned long long)pAddress,16,16,QChar('0'));
//#endif

//    return sResult;
//}
//QString GuiMainWindow::hexToString(void *pValue)
//{
//    QString sResult;

//#ifndef __X64
//    sResult=QString("0x%1").arg((unsigned int)pValue,0,16,QChar('0'));
//#else
//    sResult=QString("0x%1").arg((unsigned long long)pValue,0,16,QChar('0'));
//#endif

//    return sResult;
//}
//QString GuiMainWindow::makeLink(const QString &sValue,const QString &sLink)
//{
//    QString sResult;

//    if(sLink!="")
//    {
//        sResult=QString("<a href=\"%1\">%2</a>").arg(sLink).arg(sValue);
//    }
//    else
//    {
//        sResult=sValue;
//    }

//    return sResult;
//}

//void GuiMainWindow::on_pushButtonHex_clicked()
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

//void GuiMainWindow::on_radioButtonDecimal_toggled(bool checked)
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

//void GuiMainWindow::on_radioButtonHeximal_toggled(bool checked)
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

void GuiMainWindow::on_actionShortcuts_triggered()
{
    DialogShortcuts dialogShortcuts(this);

    dialogShortcuts.setData(&g_xShortcuts);

    dialogShortcuts.exec();

    adjust();
}

void GuiMainWindow::on_actionOptions_triggered()
{
    DialogOptions dialogOptions(this,&g_xOptions);
    dialogOptions.exec();

    adjust();
}

void GuiMainWindow::on_actionAbout_triggered()
{
    DialogAbout dialogAbout(this);
    dialogAbout.exec();
}
