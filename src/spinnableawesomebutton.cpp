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
#include "spinnableawesomebutton.h"
#include "theme.h"

SpinnableAwesomeButton::SpinnableAwesomeButton(QWidget *parent) :
    QWidget(parent)
{
    setFixedSize(Theme::toolIconSize, Theme::toolIconSize);

    m_button = new QPushButton(this);
    Theme::initAwesomeButton(m_button);
    
    m_spinner = new WaitingSpinnerWidget(this);
    m_spinner->setColor(Theme::white);
    m_spinner->setInnerRadius(Theme::toolIconSize / 8);
    m_spinner->setLineLength(Theme::toolIconSize / 4);
    m_spinner->setNumberOfLines(9);
    m_spinner->hide();
}

void SpinnableAwesomeButton::setAwesomeIcon(QChar c)
{
    m_button->setText(c);
}

void SpinnableAwesomeButton::showSpinner(bool showSpinner)
{
    if (showSpinner) {
        m_spinner->start();
        m_spinner->show();
        m_button->hide();
    } else {
        m_spinner->stop();
        m_spinner->hide();
        m_button->show();
    }
}

QPushButton *SpinnableAwesomeButton::button()
{
    return m_button;
}


bool SpinnableAwesomeButton::isSpinning()
{
    return m_spinner->isVisible();
}
