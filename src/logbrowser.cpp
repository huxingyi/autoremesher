/*
 *  Copyright (c) 2020 Jeremy HU <jeremy-at-dust3d dot org>. All rights reserved. 
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:

 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.

 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */
#include <QMetaType>
#include <QDir>
#include <stdio.h>
#include "logbrowser.h"
#include "logbrowserdialog.h"

#if AUTO_REMESHER_DEV
bool LogBrowser::m_enableOutputToFile = true;
#else
bool LogBrowser::m_enableOutputToFile = false;
#endif

LogBrowser::LogBrowser(QObject *parent) :
    QObject(parent)
{
    qRegisterMetaType<QtMsgType>("QtMsgType");
    m_browserDialog = new LogBrowserDialog;
    connect(this, &LogBrowser::sendMessage, m_browserDialog, &LogBrowserDialog::outputMessage, Qt::QueuedConnection);
    
    if (m_enableOutputToFile) {
        QString filePath = "autoremesher.log";
        m_outputTo = fopen(filePath.toUtf8().constData(), "w");
        
        freopen("autoremesher-stderr.log", "w", stderr);
        freopen("autoremesher-stdout.log", "w", stdout);
    }
}

LogBrowser::~LogBrowser()
{
    delete m_browserDialog;
    if (m_outputTo)
        fclose(m_outputTo);
}

void LogBrowser::showDialog()
{
    m_browserDialog->show();
    m_browserDialog->activateWindow();
    m_browserDialog->raise();
}

void LogBrowser::hideDialog()
{
    m_browserDialog->hide();
}

bool LogBrowser::isDialogVisible()
{
    return m_browserDialog->isVisible();
}

void LogBrowser::outputMessage(QtMsgType type, const QString &msg, const QString &source, int line)
{
    if (m_outputTo) {
        fprintf(m_outputTo, "[%s:%d]: %s\n", source.toUtf8().constData(), line, msg.toUtf8().constData());
        fflush(m_outputTo);
    }
    emit sendMessage(type, msg, source, line);
}
