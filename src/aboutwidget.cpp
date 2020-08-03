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
#include <QTextEdit>
#include <QVBoxLayout>
#include <QOpenGLFunctions>
#include "aboutwidget.h"
#include "version.h"
#include "util.h"

AboutWidget::AboutWidget()
{
    QTextEdit *versionInfoLabel = new QTextEdit;
    versionInfoLabel->setText(QString("%1 %2 (version: %3 build: %4 %5)\nopengl: %6 shader: %7 core: %8").arg(APP_NAME).arg(APP_HUMAN_VER).arg(APP_VER).arg(__DATE__).arg(__TIME__).arg((char *)glGetString(GL_VERSION)).arg((char *)glGetString(GL_SHADING_LANGUAGE_VERSION)).arg(QSurfaceFormat::defaultFormat().profile() == QSurfaceFormat::CoreProfile ? "true" : "false"));
    versionInfoLabel->setReadOnly(true);
    
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(versionInfoLabel);
    
    setLayout(mainLayout);
    setFixedSize(QSize(640, 380));
    
    setWindowTitle(unifiedWindowTitle(tr("About")));
}
