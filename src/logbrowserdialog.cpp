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
#include "logbrowserdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>
#include <QPushButton>
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QCloseEvent>
#include <QKeyEvent>
#include "version.h"
#include "util.h"
#include "mainwindow.h"

LogBrowserDialog::LogBrowserDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    m_browser = new QTextBrowser(this);
    layout->addWidget(m_browser);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    layout->addLayout(buttonLayout);

    buttonLayout->addStretch(10);

    m_clearButton = new QPushButton(this);
    m_clearButton->setText("Clear");
    buttonLayout->addWidget(m_clearButton);
    connect(m_clearButton, SIGNAL(clicked()), m_browser, SLOT(clear()));

    m_saveButton = new QPushButton(this);
    m_saveButton->setText("Save Output");
    buttonLayout->addWidget(m_saveButton);
    connect(m_saveButton, SIGNAL(clicked()), this, SLOT(save()));

    resize(640, 480);
    
    setWindowTitle(unifiedWindowTitle(tr("Debug")));
    
    hide();
}


LogBrowserDialog::~LogBrowserDialog()
{

}


void LogBrowserDialog::outputMessage(QtMsgType type, const QString &msg, const QString &source, int line)
{
    switch (type) {
    case QtDebugMsg:
        m_browser->append(tr("— DEBUG: %1 - %2:%3").arg(msg).arg(source).arg(QString::number(line)));
        break;

    case QtWarningMsg:
        m_browser->append(tr("— WARNING: %1 - %2:%3").arg(msg).arg(source).arg(QString::number(line)));
        break;

    case QtCriticalMsg:
        m_browser->append(tr("— CRITICAL: %1 - %2:%3").arg(msg).arg(source).arg(QString::number(line)));
        break;

    case QtInfoMsg:
        m_browser->append(tr("— INFO: %1 - %2:%3").arg(msg).arg(source).arg(QString::number(line)));
        break;

    case QtFatalMsg:
        m_browser->append(tr("— FATAL: %1 - %2:%3").arg(msg).arg(source).arg(QString::number(line)));
        break;
    }
}


void LogBrowserDialog::save()
{
    QString saveFileName = QFileDialog::getSaveFileName(this,
        tr("Save Log Output"),
        tr("%1/logfile.txt").arg(QDir::homePath()),
        tr("Text Files (*.txt);;All Files (*)"));

    if (saveFileName.isEmpty())
        return;

    QFile file(saveFileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this,
            tr("Error"),
            QString(tr("<nobr>File '%1'<br/>cannot be opened for writing.<br/><br/>"
            "The log output could <b>not</b> be saved!</nobr>"))
            .arg(saveFileName));
        return;
    }

    QTextStream stream(&file);
    stream << m_browser->toPlainText();
    file.close();
}

void LogBrowserDialog::closeEvent(QCloseEvent *e)
{
    if (0 == MainWindow::total()) {
        e->accept();
        return;
    }
    e->ignore();
    hide();
}

void LogBrowserDialog::keyPressEvent(QKeyEvent *e)
{
    // ignore all keyboard events
    // protects against accidentally closing of the dialog
    // without asking the user
    e->ignore();
}
