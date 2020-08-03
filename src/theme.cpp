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
#include <QApplication>
#include <QGuiApplication>
#include <QDebug>
#include <QFontMetrics>
#include "theme.h"

// Red
// 0xfc, 0x66, 0x21
// 252, 102, 33
// 0.99, 0.4, 0.13

// Green
// 0xaa, 0xeb, 0xc4

// Blue
// 0x2a, 0x5a, 0xac
// 0x0d, 0xa9, 0xf1

// White
// 0xf7, 0xd9, 0xc8

QColor Theme::red = QColor(0xfc, 0x66, 0x21);
QColor Theme::green = QColor(0xaa, 0xeb, 0xc4);
//QColor Theme::blue = QColor(0x2a, 0x5a, 0xac);
QColor Theme::blue = QColor(0x0d, 0xa9, 0xf1);
QColor Theme::white = QColor(0xf7, 0xd9, 0xc8);
QColor Theme::black = QColor(0x25,0x25,0x25);
QColor Theme::dark = QColor(0x19,0x19,0x19);
QColor Theme::altDark = QColor(0x16,0x16,0x16);
QColor Theme::broken = QColor(0xff,0xff,0xff);
float Theme::normalAlpha = 96.0 / 255;
float Theme::branchAlpha = 64.0 / 255;
float Theme::checkedAlpha = 1.0;
float Theme::edgeAlpha = 1.0;
float Theme::fillAlpha = 50.0 / 255;
int Theme::skeletonNodeBorderSize = 0;
int Theme::skeletonEdgeWidth = 0;
int Theme::toolIconFontSize = 0;
int Theme::toolIconSize = 0;
int Theme::miniIconFontSize = 0;
int Theme::miniIconSize = 0;
int Theme::partPreviewImageSize = 0;
int Theme::materialPreviewImageSize = 0;
int Theme::cutFacePreviewImageSize = 0;
int Theme::posePreviewImageSize = 0;
int Theme::motionPreviewImageSize = 0;
int Theme::sidebarPreferredWidth = 0;
int Theme::normalButtonSize = 0;

void Theme::initAwsomeBaseSizes()
{
    QFontMetrics fontMetrics(QApplication::font());
    Theme::toolIconFontSize = fontMetrics.height();
    
    Theme::toolIconSize = (int)(Theme::toolIconFontSize * 1.5);
    Theme::miniIconFontSize = (int)(Theme::toolIconFontSize * 0.7);
    Theme::miniIconSize = (int)(Theme::miniIconFontSize * 1.67);
    Theme::partPreviewImageSize = (Theme::miniIconSize * 3);
    Theme::sidebarPreferredWidth = Theme::partPreviewImageSize * 4; //3.7;
    Theme::posePreviewImageSize = Theme::sidebarPreferredWidth * 0.4;
    Theme::materialPreviewImageSize = Theme::posePreviewImageSize;
    Theme::cutFacePreviewImageSize = Theme::posePreviewImageSize;
    Theme::motionPreviewImageSize = Theme::posePreviewImageSize;
    Theme::normalButtonSize = Theme::toolIconSize * 2;
}

QtAwesome *Theme::awesome()
{
    static QtAwesome *s_awesome = nullptr;
    if (nullptr == s_awesome) {
        s_awesome = new QtAwesome();
        s_awesome->initFontAwesome();
        s_awesome->setDefaultOption("color", Theme::white);
        s_awesome->setDefaultOption("color-disabled", QColor(0xcc, 0xcc, 0xcc));
        s_awesome->setDefaultOption("color-active", Theme::white);
        s_awesome->setDefaultOption("color-selected", Theme::white);
    }
    return s_awesome;
}

std::map<QString, QString> createSideColorNameMap() {
    std::map<QString, QString> map;
    map["red"] = "green";
    map["green"] = "red";
    return map;
}

std::map<QString, QColor> createSideColorNameToColorMap() {
    std::map<QString, QColor> map;
    map["red"] = Theme::red;
    map["green"] = Theme::green;
    return map;
}

std::map<QString, QString> Theme::nextSideColorNameMap = createSideColorNameMap();
std::map<QString, QColor> Theme::sideColorNameToColorMap = createSideColorNameToColorMap();

QString Theme::tabButtonSelectedStylesheet = "QPushButton { color: " + Theme::red.name() + "; background-color: #353535; border: 0px; padding-top: 2px; padding-bottom: 2px; padding-left: 25px; padding-right: 25px;}";
QString Theme::tabButtonStylesheet = "QPushButton { color: " + Theme::white.name() + "; background-color: transparent; border: 0px; padding-top: 2px; padding-bottom: 2px; padding-left: 25px; padding-right: 25px;}";

void Theme::initAwesomeButton(QPushButton *button)
{
    button->setFont(Theme::awesome()->font(Theme::toolIconFontSize));
    button->setFixedSize(Theme::toolIconSize, Theme::toolIconSize);
    button->setStyleSheet("QPushButton {color: " + Theme::white.name() + "}");
    button->setFocusPolicy(Qt::NoFocus);
}

void Theme::initAwesomeSmallButton(QPushButton *button)
{
    button->setFont(Theme::awesome()->font(Theme::toolIconFontSize * 0.7));
    button->setFixedSize(Theme::toolIconSize * 0.75, Theme::toolIconSize * 0.75);
    button->setStyleSheet("QPushButton {color: " + Theme::white.name() + "}");
    button->setFocusPolicy(Qt::NoFocus);
}

void Theme::initAwesomeLabel(QLabel *label)
{
    label->setFont(Theme::awesome()->font(Theme::toolIconFontSize));
    label->setStyleSheet("QLabel {color: " + Theme::white.name() + "}");
}

void Theme::initAwesomeMiniButton(QPushButton *button)
{
    button->setFont(Theme::awesome()->font(Theme::miniIconFontSize));
    button->setFixedSize(Theme::miniIconSize, Theme::miniIconSize);
    button->setFocusPolicy(Qt::NoFocus);
}

void Theme::updateAwesomeMiniButton(QPushButton *button, QChar icon, bool highlighted, bool enabled, bool unnormal)
{
    button->setText(icon);
    QColor color;
    bool needDesaturation = true;
    
    if (highlighted) {
        if (unnormal) {
            color = Theme::blue;
            needDesaturation = false;
        } else {
            color = Theme::red;
        }
    } else {
        color = QColor("#525252");
    }
    
    if (needDesaturation) {
        color = color.toHsv();
        color.setHsv(color.hue(), color.saturation() / 5, color.value() * 2 / 3);
        color = color.toRgb();
    }
    
    if (!enabled) {
        color = QColor(42, 42, 42);
    }

    button->setStyleSheet("QPushButton {border: none; background: none; color: " + color.name() + ";}");
}

void Theme::initAwesomeToolButtonWithoutFont(QPushButton *button)
{
    button->setFixedSize(Theme::toolIconSize / 2, Theme::toolIconSize / 2);
    button->setStyleSheet("QPushButton {color: " + Theme::white.name() + "}");
    button->setFocusPolicy(Qt::NoFocus);
}

void Theme::initAwesomeToolButton(QPushButton *button)
{
    button->setFont(Theme::awesome()->font(Theme::toolIconFontSize / 2));
    Theme::initAwesomeToolButtonWithoutFont(button);
}

void Theme::initToolButton(QPushButton *button)
{
    QFont font = button->font();
    font.setWeight(QFont::Light);
    font.setBold(false);
    button->setFont(font);
    button->setFixedHeight(Theme::toolIconSize * 0.75);
    button->setStyleSheet("QPushButton {color: " + Theme::white.name() + "}");
    button->setFocusPolicy(Qt::NoFocus);
}

void Theme::initCheckbox(QCheckBox *checkbox)
{
    QPalette palette = checkbox->palette();
    palette.setColor(QPalette::Background, Theme::white);
    checkbox->setPalette(palette);
}

QWidget *Theme::createHorizontalLineWidget()
{
    QWidget *hrLightWidget = new QWidget;
    hrLightWidget->setFixedHeight(1);
    hrLightWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    hrLightWidget->setStyleSheet(QString("background-color: #565656;"));
    hrLightWidget->setContentsMargins(0, 0, 0, 0);
    return hrLightWidget;
}

QWidget *Theme::createVerticalLineWidget()
{
    QWidget *hrLightWidget = new QWidget;
    hrLightWidget->setFixedWidth(1);
    hrLightWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    hrLightWidget->setStyleSheet(QString("background-color: #565656;"));
    hrLightWidget->setContentsMargins(0, 0, 0, 0);
    return hrLightWidget;
}
