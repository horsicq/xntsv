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
#include "htmltable.h"

HtmlTable::HtmlTable()
{
}

void HtmlTable::addTableBegin()
{
    //    sTable.append("<table border=\"1\">");
    sTable.append("<table border=\"1\" style=\"border-style:solid;\" cellspacing=\"-1\" cellpadding=\"5\">");
    //    table border="1" style="border-style:solid" cellspacing="-1" cellpadding="0"
}

void HtmlTable::addTableEnd()
{
    sTable.append("</table>");
}

void HtmlTable::addBR()
{
    sTable.append("<br />");
}

void HtmlTable::addRecord(const QString &s1,const QString &s2=QString(),const QString &s3=QString(),const QString &s4=QString(),const QString &s5=QString(),const QString &s6=QString(),const QString &s7=QString())
{
    //	QString sReference;
    //    sTable.append(QString("<a name=\"%1\">").arg(s1));

    sTable.append("<tr>");
    sTable.append("<td align=\"center\">");
    sTable.append(s1);
    sTable.append("</td>");

    if(s2!="")
    {
        sTable.append("<td align=\"center\">");
        sTable.append(s2);
        sTable.append("</td>");
    }

    if(s3!="")
    {
        sTable.append("<td align=\"center\">");
        sTable.append(s3);
        sTable.append("</td>");
    }

    if(s4!="")
    {
        sTable.append("<td align=\"center\">");
        sTable.append(s4);
        sTable.append("</td>");
    }

    if(s5!="")
    {
        sTable.append("<td align=\"center\">");
        sTable.append(s5);
        sTable.append("</td>");
    }

    if(s6!="")
    {
        sTable.append("<td align=\"center\">");
        sTable.append(s6);
        sTable.append("</td>");
    }

    if(s7!="")
    {
        sTable.append("<td align=\"center\">");
        sTable.append(s7);
        sTable.append("</td>");
    }

    sTable.append("</tr>");

    //    sTable.append("</a>");
}

QString HtmlTable::toString()
{
    QString sResult=sTable;
    return sResult;
}

void HtmlTable::addString(const QString &sString)
{
    sTable.append(sString);
}

void HtmlTable::addTab(const QString &sString)
{
    sTable.append("<table><tr><td width=\"75\" align=\"center\">");
    sTable.append("</td><td>");
    sTable.append(sString);
    sTable.append("</td></tr></table>");
}

void HtmlTable::addHeader(const QString &s1,const QString &s2=QString(),const QString &s3=QString(),const QString &s4=QString(),const QString &s5=QString(),const QString &s6=QString(),const QString &s7=QString())
{
    QString sReference;

    sTable.append("<tr>");
    sTable.append("<th align=\"center\">");
    sTable.append(s1);
    sTable.append("</th>");

    if(s2!="")
    {
        sTable.append("<th align=\"center\">");
        sTable.append(s2);
        sTable.append("</th>");
    }

    if(s3!="")
    {
        sTable.append("<th align=\"center\">");
        sTable.append(s3);
        sTable.append("</th>");
    }

    if(s4!="")
    {
        sTable.append("<th align=\"center\">");
        sTable.append(s4);
        sTable.append("</th>");
    }

    if(s5!="")
    {
        sTable.append("<th align=\"center\">");
        sTable.append(s5);
        sTable.append("</th>");
    }

    if(s6!="")
    {
        sTable.append("<th align=\"center\">");
        sTable.append(s6);
        sTable.append("</th>");
    }

    if(s7!="")
    {
        sTable.append("<th align=\"center\">");
        sTable.append(s7);
        sTable.append("</th>");
    }

    sTable.append("</tr>");
}
