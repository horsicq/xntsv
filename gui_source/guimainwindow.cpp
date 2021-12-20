/* Copyright (c) 2017-2021 hors<horsicq@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
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

    ui->widgetProcesses->setGlobal(&g_xShortcuts,&g_xOptions);

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
//    ui->widgetProcesses->setShortcuts(&g_xShortcuts);

//    XProcessWidget::OPTIONS options={};
//    options.sStructsPath=g_xOptions.getStructsPath();
//    options.sSearchSignaturesPath=g_xOptions.getSearchSignaturesPath();

//    ui->widgetProcesses->setOptions(options);
}

void GuiMainWindow::on_actionExit_triggered()
{
    this->close();
}

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
