/*
 *  Copyright (c) 2020 Jeremy HU <jeremy-at-dust3d dot org>. All rights reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */
#ifndef DUST3D_SPINNABLE_AWESOME_BUTTON_H
#define DUST3D_SPINNABLE_AWESOME_BUTTON_H
#include "waitingspinnerwidget.h"
#include <QPushButton>
#include <QWidget>

class SpinnableAwesomeButton : public QWidget {
public:
    SpinnableAwesomeButton(QWidget* parent = nullptr);
    void setAwesomeIcon(QChar c);
    void showSpinner(bool showSpinner = true);
    bool isSpinning();
    QPushButton* button();

private:
    QPushButton* m_button = nullptr;
    WaitingSpinnerWidget* m_spinner = nullptr;
};

#endif
