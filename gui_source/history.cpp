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
#include "history.h"

History::History()
{
    nCurrent=0;
}

void History::addRecord(const QString &sRecord)
{
    while(isForwardAvailable())
    {
        lHistory.removeLast();
    }

    lHistory.append(sRecord);
    nCurrent=lHistory.count()-1;
}

QString History::home()
{
    return sHome;
}

void History::setHome(const QString &sHome)
{
    this->sHome=sHome;
}

void History::increment()
{
    nCurrent++;
}

void History::decrement()
{
    nCurrent--;
}

void History::replace(const QString &sRecord)
{
    lHistory.replace(nCurrent,sRecord);
}

bool History::isBackAvailable()
{
    return (nCurrent>0);
}

bool History::isForwardAvailable()
{
    return (nCurrent+1<lHistory.count());
}

void History::clear()
{
    lHistory.clear();
    nCurrent=0;
}

QString History::current()
{
    QString sResult;

    if(!lHistory.empty())
    {
        sResult=lHistory.at(nCurrent);
    }
    else
    {
        sResult="";
    }

    return sResult;
}

