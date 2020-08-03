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
#ifndef AUTO_REMESHER_UPDATES_CHECK_WIDGET_H
#define AUTO_REMESHER_UPDATES_CHECK_WIDGET_H
#include <QDialog>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>

class UpdatesChecker;

class UpdatesCheckWidget : public QDialog
{
    Q_OBJECT
public:
    UpdatesCheckWidget();
    ~UpdatesCheckWidget();
public slots:
    void check();
    void checkFinished();
    void viewUpdates();
private:
    UpdatesChecker *m_updatesChecker = nullptr;
    QStackedWidget *m_stackedWidget = nullptr;
    QLabel *m_infoLabel = nullptr;
    QPushButton *m_viewButton = nullptr;
    QString m_viewUrl;
};

#endif
