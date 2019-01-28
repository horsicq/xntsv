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
#include "dialogmemorymap.h"
#include "ui_dialogmemorymap.h"

DialogMemoryMap::DialogMemoryMap(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogMemoryMap)
{
    ui->setupUi(this);

    actHex=new QAction("Hex", this);

    connect(actHex,SIGNAL(triggered()),this,SLOT(hex()));

    bIs64=(sizeof(void *)==8);
}

DialogMemoryMap::~DialogMemoryMap()
{
    delete ui;
}

void DialogMemoryMap::setData(int (*umMemoryQuery)(void *, void *, char *, int),QByteArray(* ReadFromMemory)(QVariant, unsigned long long, unsigned long long, bool *),void *hPID)
{
    this->umMemoryQuery=umMemoryQuery;
    this->ReadFromMemory=ReadFromMemory;
    this->hPID=hPID;

    reload();
}

void DialogMemoryMap::reload()
{
    QList<_MEMORY_BASIC_INFORMATION> listMemoryMap;
    QString sText;

    _MEMORY_BASIC_INFORMATION mbi;
    QTableWidgetItem *newItem;

    unsigned long long pAddress=0;

    while(true)
    {
        if(umMemoryQuery(hPID,(void *)pAddress,(char *)&mbi,sizeof(_MEMORY_BASIC_INFORMATION))==0)
        {
            break;
        }

        listMemoryMap.append(mbi);

        pAddress+=mbi.RegionSize;
    }

    ui->tableWidgetMemoryMap->setRowCount(listMemoryMap.count());
    ui->tableWidgetMemoryMap->setColumnCount(7);


    QStringList labels;
    labels.append("BaseAddress");
    labels.append("AllocationBase");
    labels.append("AllocationProtect");
    labels.append("RegionSize");
    labels.append("State");
    labels.append("Protect");
    labels.append("Type");

    ui->tableWidgetMemoryMap->setHorizontalHeaderLabels(labels);

    for(int i=0; i<listMemoryMap.count(); i++)
    {
        newItem = new QTableWidgetItem;
        newItem->setText(QString("%1").arg((unsigned long long)listMemoryMap.at(i).BaseAddress,bIs64?16:8,16,QChar('0')));
        ui->tableWidgetMemoryMap->setItem(i,0,newItem);
        ui->tableWidgetMemoryMap->item(i,0)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

        newItem = new QTableWidgetItem;
        newItem->setText(QString("%1").arg((unsigned long long)listMemoryMap.at(i).AllocationBase,bIs64?16:8,16,QChar('0')));
        ui->tableWidgetMemoryMap->setItem(i,1,newItem);
        ui->tableWidgetMemoryMap->item(i,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

        newItem = new QTableWidgetItem;
        newItem->setText(getProtect(listMemoryMap.at(i).AllocationProtect));
        ui->tableWidgetMemoryMap->setItem(i,2,newItem);
        ui->tableWidgetMemoryMap->item(i,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

        newItem = new QTableWidgetItem;
        newItem->setText(QString("%1").arg((unsigned long long)listMemoryMap.at(i).RegionSize,8,16,QChar('0')));
        ui->tableWidgetMemoryMap->setItem(i,3,newItem);
        ui->tableWidgetMemoryMap->item(i,3)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

        newItem = new QTableWidgetItem;

        if((unsigned long long)listMemoryMap.at(i).State==0x1000)
        {
            sText="COMMIT";
        }
        else if((unsigned long long)listMemoryMap.at(i).State==0x10000)
        {
            sText="FREE";
        }
        else if((unsigned long long)listMemoryMap.at(i).State==0x2000)
        {
            sText="RESERVE";
        }

        newItem->setText(sText);
        ui->tableWidgetMemoryMap->setItem(i,4,newItem);
        ui->tableWidgetMemoryMap->item(i,4)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

        newItem = new QTableWidgetItem;
        newItem->setText(getProtect(listMemoryMap.at(i).Protect));
        ui->tableWidgetMemoryMap->setItem(i,5,newItem);
        ui->tableWidgetMemoryMap->item(i,5)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

        newItem = new QTableWidgetItem;

        if((unsigned long long)listMemoryMap.at(i).Type==0)
        {
            sText="0";
        }
        else if((unsigned long long)listMemoryMap.at(i).Type==0x1000000)
        {
            sText="IMAGE";
        }
        else if((unsigned long long)listMemoryMap.at(i).Type==0x40000)
        {
            sText="MAPPED";
        }
        else if((unsigned long long)listMemoryMap.at(i).Type==0x20000)
        {
            sText="PRIVATE";
        }

        newItem->setText(sText);
        ui->tableWidgetMemoryMap->setItem(i,6,newItem);
        ui->tableWidgetMemoryMap->item(i,6)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

        ui->tableWidgetMemoryMap->setRowHeight(i,20);
    }

    ui->tableWidgetMemoryMap->setColumnWidth(0,115);
    ui->tableWidgetMemoryMap->setColumnWidth(1,115);
    ui->tableWidgetMemoryMap->setColumnWidth(2,115);
    ui->tableWidgetMemoryMap->setColumnWidth(3,115);
    ui->tableWidgetMemoryMap->setColumnWidth(4,70);
    ui->tableWidgetMemoryMap->setColumnWidth(5,115);
    ui->tableWidgetMemoryMap->setColumnWidth(6,70);
}

void DialogMemoryMap::on_pushButtonClose_clicked()
{
    this->close();
}

QString DialogMemoryMap::getProtect(unsigned int nValue)
{
    QString sResult=QString::number(nValue,16);

    if(nValue&0x10)
    {
        sResult="exec";
    }
    else if(nValue&0x20)
    {
        sResult="exec+read";
    }
    else if(nValue&0x40)
    {
        sResult="exec+readwrite";
    }
    else if(nValue&0x80)
    {
        sResult="exec+writecopy";
    }
    else if(nValue&0x01)
    {
        sResult="no access";
    }
    else if(nValue&0x02)
    {
        sResult="readonly";
    }
    else if(nValue&0x04)
    {
        sResult="readwrite";
    }
    else if(nValue&0x08)
    {
        sResult="writecopy";
    }

    if(nValue&0x100)
    {
        sResult+="+guard";
    }
    else if(nValue&0x200)
    {
        sResult+="+nocache";
    }
    else if(nValue&0x400)
    {
        sResult+="+writecombine";
    }

    return sResult;
}

void DialogMemoryMap::on_pushButtonReload_clicked()
{
    reload();
}
