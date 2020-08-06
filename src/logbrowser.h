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
#ifndef AUTO_REMESHER_LOG_BROWSER_H
#define AUTO_REMESHER_LOG_BROWSER_H
#include <QObject>
#include <cstdio>

class LogBrowserDialog;

class LogBrowser : public QObject
{
    Q_OBJECT
public:
    explicit LogBrowser(QObject *parent=0);
    ~LogBrowser();

public slots:
    void outputMessage(QtMsgType type, const QString &msg, const QString &source, int line);
    void showDialog();
    void hideDialog();
    bool isDialogVisible();

signals:
    void sendMessage(QtMsgType type, const QString &msg, const QString &source, int line);

private:
    LogBrowserDialog *m_browserDialog = nullptr;
    FILE *m_outputTo = nullptr;
    static bool m_enableOutputToFile;
};

#endif
