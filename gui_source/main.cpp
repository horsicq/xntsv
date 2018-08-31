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
#include <QApplication>
#include "mainwindow.h"
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QCoreApplication::addLibraryPath("plugins");

    QCoreApplication::setOrganizationName(XNTSV_ORGANIZATIONNAME);
    QCoreApplication::setOrganizationDomain(XNTSV_ORGANIZATIONDOMAIN);
    QCoreApplication::setApplicationName(XNTSV_APPLICATIONNAME);
    QCoreApplication::setApplicationVersion(XNTSV_APPLICATIONVERSION);

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication a(argc, argv);

    a.setStyle(QStyleFactory::create("Fusion"));

    MainWindow w;
    w.show();

    return a.exec();
}
