// Copyright (c) 2017-2018 hors<horsicq@gmail.com>
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
#ifndef DIALOGMEMORYMAP_H
#define DIALOGMEMORYMAP_H

#include <QDialog>
#include <QVariant>
#include <QMenu>

namespace Ui
{
class DialogMemoryMap;
}

class DialogMemoryMap : public QDialog
{
    Q_OBJECT

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

public:
    explicit DialogMemoryMap(QWidget *parent = 0);
    ~DialogMemoryMap();
    void setData(int (* umMemoryQuery)(void *,void *,char *,int),QByteArray(* ReadFromMemory)(QVariant, unsigned long long, unsigned long long, bool *),void *hPID);
private slots:
    void reload();
    void on_pushButtonClose_clicked();
    QString getProtect(unsigned int nValue);

    void on_pushButtonReload_clicked();

private:
    Ui::DialogMemoryMap *ui;

    int (* umMemoryQuery)(void *,void *,char *,int);
    QByteArray(* ReadFromMemory)(QVariant, unsigned long long, unsigned long long, bool *);
    void *hPID;
    QAction *actHex;

    bool bIs64;
};

#endif // DIALOGMEMORYMAP_H
