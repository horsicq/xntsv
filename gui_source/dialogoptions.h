/* Copyright (c) 2017-2025 hors<horsicq@gmail.com>
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
#ifndef DIALOGOPTIONS_H
#define DIALOGOPTIONS_H

#include <QDialog>

#include "../global.h"
#include "searchsignaturesoptionswidget.h"
#include "nfdoptionswidget.h"
#include "xbinary.h"
#include "xdisasmviewoptionswidget.h"
#include "xdynstructsoptionswidget.h"
#include "xhexviewoptionswidget.h"
#include "xoptions.h"

namespace Ui {
class DialogOptions;
}

class DialogOptions : public QDialog {
    Q_OBJECT

public:
    explicit DialogOptions(QWidget *pParent, XOptions *pOptions);
    ~DialogOptions();

private slots:
    void on_pushButtonOK_clicked();
    void on_pushButtonCancel_clicked();

private:
    Ui::DialogOptions *ui;
    NFDOptionsWidget *g_pNFDOptionsWidget;
    SearchSignaturesOptionsWidget *g_pSearchSignaturesOptionsWidget;
    XHexViewOptionsWidget *g_pXHexViewOptionsWidget;
    XDisasmViewOptionsWidget *g_pXDisasmViewOptionsWidget;
    XDynStructsOptionsWidget *g_pXDynStructsOptionsWidget;
    XOptions *g_pOptions;
};

#endif  // DIALOGOPTIONS_H
