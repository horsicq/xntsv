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
#ifndef HTMLTABLE_H
#define HTMLTABLE_H

#include <QString>
#include <QRegExp>

class HtmlTable
{
private:
    QString sTable;
public:
    HtmlTable();
    void addTableBegin();
    void addTableEnd();
    void addBR();
    void addRecord(const QString &s1,const QString &s2,const QString &s3,const QString &s4,const QString &s5,const QString &s6,const QString &s7);
    void addHeader(const QString &s1,const QString &s2,const QString &s3,const QString &s4,const QString &s5,const QString &s6,const QString &s7);
    QString toString();
    void addString(const QString &sString);
    void addTab(const QString &sString);
};

#endif // HTMLTABLE_H
