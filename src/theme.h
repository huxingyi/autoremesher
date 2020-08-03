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
#ifndef AUTO_REMESHER_THEME_H
#define AUTO_REMESHER_THEME_H
#include <QColor>
#include <QString>
#include <map>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include "QtAwesome.h"

class Theme
{
public:
    static QColor red;
    static QColor green;
    static QColor blue;
    static QColor white;
    static QColor black;
    static QColor dark;
    static QColor altDark;
    static QColor broken;
    static float normalAlpha;
    static float checkedAlpha;
    static float branchAlpha;
    static float fillAlpha;
    static float edgeAlpha;
    static int skeletonNodeBorderSize;
    static int skeletonEdgeWidth;
    static QString tabButtonSelectedStylesheet;
    static QString tabButtonStylesheet;
    static std::map<QString, QString> nextSideColorNameMap;
    static std::map<QString, QColor> sideColorNameToColorMap;
    static QtAwesome *awesome();
    static QWidget *createHorizontalLineWidget();
    static QWidget *createVerticalLineWidget();
    static int toolIconFontSize;
    static int toolIconSize;
    static int materialPreviewImageSize;
    static int cutFacePreviewImageSize;
    static int posePreviewImageSize;
    static int partPreviewImageSize;
    static int motionPreviewImageSize;
    static int miniIconFontSize;
    static int miniIconSize;
    static int sidebarPreferredWidth;
    static int normalButtonSize;
public:
    static void initAwesomeButton(QPushButton *button);
    static void initAwesomeLabel(QLabel *label);
    static void initAwesomeSmallButton(QPushButton *button);
    static void initAwesomeMiniButton(QPushButton *button);
    static void updateAwesomeMiniButton(QPushButton *button, QChar icon, bool highlighted, bool enabled, bool unnormal=false);
    static void initAwesomeToolButton(QPushButton *button);
    static void initAwesomeToolButtonWithoutFont(QPushButton *button);
    static void initAwsomeBaseSizes();
    static void initToolButton(QPushButton *button);
    static void initCheckbox(QCheckBox *checkbox);
};

#endif
