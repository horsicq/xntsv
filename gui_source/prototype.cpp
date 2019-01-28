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
#include "prototype.h"
#include "ui_prototype.h"

Prototype::Prototype(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Prototype)
{
    ui->setupUi(this);
}

Prototype::~Prototype()
{
    delete ui;
}

void Prototype::on_pushButtonOK_clicked()
{
    this->close();
}

void Prototype::setPrototype(QString sText)
{
    sProto=sText;
    ui->textEditPrototype->setText(sText);
}

void Prototype::setTitle(QString sText)
{
    this->setWindowTitle(sText);
}

void Prototype::on_checkBoxShowComments_stateChanged(int arg1)
{
    if(!(ui->checkBoxShowComments->isChecked()))
    {
        ui->textEditPrototype->setText(removeComments(ui->textEditPrototype->toPlainText()));

    }
    else
    {
        ui->textEditPrototype->setText(sProto);

        if(ui->checkBoxOffsetAsHEX->isChecked())
        {
            ui->textEditPrototype->setText(offsetAsHEX(ui->textEditPrototype->toPlainText()));
        }
    }

    ui->checkBoxOffsetAsHEX->setEnabled(ui->checkBoxShowComments->isChecked());
}

void Prototype::on_checkBoxOffsetAsHEX_stateChanged(int arg1)
{
    if(ui->checkBoxOffsetAsHEX->isChecked())
    {
        ui->textEditPrototype->setText(offsetAsHEX(ui->textEditPrototype->toPlainText()));

    }
    else
    {
        ui->textEditPrototype->setText(sProto);
    }
}

QString Prototype::removeComments(QString sText)
{
    QString sResult;
    QString sLine;

    QTextStream ts(&sText);

    do
    {
        sLine=ts.readLine();
        sLine=sLine.section("//",0,0);

        if(sLine!="")
        {
            sResult+=sLine+"\r\n";
        }

    }
    while(!sLine.isNull());


    return sResult;
}

QString Prototype::offsetAsHEX(QString sText)
{
    QString sResult;
    QString sLine;
    QString sDec;
    QString sHEX;

    QTextStream ts(&sText);

    do
    {
        sLine=ts.readLine();

        if(sLine.contains("Offset="))
        {
            sDec=sLine.section("Offset=",1,1);
            sDec=sDec.section(" ",0,0);
            sHEX=QString("0x%1").arg(sDec.toInt(),0,16);
            sLine.replace("Offset="+sDec,"Offset="+sHEX);
        }

        if(sLine.contains("Size="))
        {
            sDec=sLine.section("Size=",1,1);
            sDec=sDec.section(" ",0,0);
            sHEX=QString("0x%1").arg(sDec.toInt(),0,16);
            sLine.replace("Size="+sDec,"Size="+sHEX);
        }

        if(sLine.contains("Count="))
        {
            sDec=sLine.section("Count=",1,1);
            sDec=sDec.section(" ",0,0);
            sHEX=QString("0x%1").arg(sDec.toInt(),0,16);
            sLine.replace("Count="+sDec,"Count="+sHEX);
        }

        if(sLine!="")
        {
            sResult+=sLine+"\r\n";
        }

    }
    while(!sLine.isNull());


    return sResult;
}
