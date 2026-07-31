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
#include "intnumberwidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QtWidgets>

IntNumberWidget::IntNumberWidget(QWidget* parent, bool singleLine)
    : QWidget(parent)
{
    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_spinBox = new QSpinBox(this);
    m_spinBox->setKeyboardTracking(false);
    m_spinBox->setMinimumWidth(75);
    m_spinBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    m_spinBox->setStyleSheet(
        "QSpinBox {"
        "  background-color: #333333;"
        "  color: #ffffff;"
        "  border: 1px solid #555555;"
        "  border-radius: 4px;"
        "  padding: 2px 4px;"
        "  font-size: 11px;"
        "  font-weight: bold;"
        "}"
        "QSpinBox::up-button, QSpinBox::down-button { width: 14px; background: #444; }"
    );

    m_label = new QLabel(this);
    m_label->setStyleSheet("color: #ffffff; font-size: 12px; font-weight: bold; margin-bottom: 2px;");
    m_label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_label->setAlignment(Qt::AlignLeft);

    connect(m_slider, &QAbstractSlider::valueChanged, [=](int value) {
        {
            QSignalBlocker blocker(m_spinBox);
            m_spinBox->setValue(value);
        }
        updateValueLabel(value);
        emit valueChanged(value);
    });
    connect(m_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
        {
            QSignalBlocker blocker(m_slider);
            m_slider->setValue(value);
        }
        emit valueChanged(value);
    });

    QBoxLayout* valueLayout = new QHBoxLayout;
    valueLayout->setContentsMargins(0, 0, 0, 0);
    valueLayout->setSpacing(6);
    valueLayout->addWidget(m_slider);
    valueLayout->addWidget(m_spinBox);

    QBoxLayout* layout = nullptr;
    if (singleLine) {
        layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(m_label);
        layout->addLayout(valueLayout);
    } else {
        layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(m_label);
        layout->addLayout(valueLayout);
    }

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

void IntNumberWidget::updateValueLabel(int value)
{
    Q_UNUSED(value);
    m_label->setText(m_itemName);
}

void IntNumberWidget::setItemName(const QString& name)
{
    m_itemName = name;
    updateValueLabel(value());
}

void IntNumberWidget::setSuffix(const QString& suffix)
{
    m_suffix = suffix;
    m_spinBox->setSuffix(suffix);
    updateValueLabel(value());
}

void IntNumberWidget::setRange(int min, int max)
{
    m_slider->setRange(min, max);
    QSignalBlocker blocker(m_spinBox);
    m_spinBox->setRange(min, max);
    m_spinBox->setValue(m_slider->value());
}

int IntNumberWidget::value() const
{
    if (nullptr != m_spinBox)
        return m_spinBox->value();
    return m_slider->value();
}

void IntNumberWidget::setValue(int value)
{
    m_slider->setValue(value);
    if (nullptr != m_spinBox) {
        QSignalBlocker blocker(m_spinBox);
        m_spinBox->setValue(value);
    }
}