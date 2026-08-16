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
#include "floatnumberwidget.h"
#include "theme.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QtWidgets>
#include <cmath>

FloatNumberWidget::FloatNumberWidget(QWidget* parent, bool singleLine)
    : QWidget(parent)
{
    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setRange(0, 100);
    m_slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_spinBox = new QDoubleSpinBox(this);
    m_spinBox->setDecimals(2);
    m_spinBox->setSingleStep(0.01);
    m_spinBox->setRange(0.0, 1.0);
    Theme::initNumberInput(m_spinBox);

    m_label = new QLabel(this);
    m_label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_label->setAlignment(Qt::AlignLeft);

    connect(m_slider, &QAbstractSlider::valueChanged, [=](int value) {
        if (m_syncing)
            return;
        float fvalue = value / 100.0;
        syncSpinBoxFromSlider();
        emit valueChanged(fvalue);
    });

    connect(m_spinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double value) {
        if (m_syncing)
            return;
        m_syncing = true;
        m_slider->setValue(std::round(value * 100.0));
        m_syncing = false;
        emit valueChanged((float)value);
    });

    QBoxLayout* layout = nullptr;
    if (singleLine) {
        layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(m_label);
        layout->addWidget(m_slider);
        layout->addWidget(m_spinBox);
    } else {
        QHBoxLayout* inputLayout = new QHBoxLayout;
        inputLayout->setContentsMargins(0, 0, 0, 0);
        inputLayout->addWidget(m_slider);
        inputLayout->addWidget(m_spinBox);

        layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(m_label);
        layout->addLayout(inputLayout);
    }

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

void FloatNumberWidget::syncSpinBoxFromSlider()
{
    m_syncing = true;
    m_spinBox->setValue(value());
    m_syncing = false;
}

void FloatNumberWidget::setItemName(const QString& name)
{
    m_itemName = name;
    m_label->setText(m_itemName);
}

void FloatNumberWidget::setRange(float min, float max)
{
    m_slider->setRange(std::round(min * 100.0), std::round(max * 100.0));
    m_spinBox->setRange(min, max);
    syncSpinBoxFromSlider();
}

void FloatNumberWidget::increaseValue()
{
    m_slider->triggerAction(QSlider::SliderPageStepAdd);
}

void FloatNumberWidget::descreaseValue()
{
    m_slider->triggerAction(QSlider::SliderPageStepSub);
}

float FloatNumberWidget::value() const
{
    return m_slider->value() / 100.0;
}

void FloatNumberWidget::setValue(float value)
{
    m_slider->setValue(std::round(value * 100.0));
    syncSpinBoxFromSlider();
}
