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
    m_spinBox->setDecimals(3);
    m_spinBox->setSingleStep(0.01);
    m_spinBox->setKeyboardTracking(false);
    m_spinBox->setMinimumWidth(80);
    m_spinBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    m_spinBox->setStyleSheet(
        "QDoubleSpinBox {"
        "  background-color: #333333;"
        "  color: #ffffff;"
        "  border: 1px solid #555555;"
        "  border-radius: 4px;"
        "  padding: 2px 4px;"
        "  font-size: 11px;"
        "  font-weight: bold;"
        "}"
        "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { width: 14px; background: #444; }"
    );

    m_label = new QLabel(this);
    m_label->setStyleSheet("color: #ffffff; font-size: 12px; font-weight: bold; margin-bottom: 2px;");
    m_label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_label->setAlignment(Qt::AlignLeft);

    connect(m_slider, &QAbstractSlider::valueChanged, [=](int value) {
        int decimals = m_spinBox ? m_spinBox->decimals() : 2;
        double factor = std::pow(10.0, (double)decimals);
        float fvalue = static_cast<float>(value / factor);
        {
            QSignalBlocker blocker(m_spinBox);
            m_spinBox->setValue(fvalue);
        }
        updateValueLabel(fvalue);
        emit valueChanged(fvalue);
    });
    connect(m_spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
        int decimals = m_spinBox ? m_spinBox->decimals() : 2;
        double factor = std::pow(10.0, (double)decimals);
        {
            QSignalBlocker blocker(m_slider);
            m_slider->setValue(qRound(value * factor));
        }
        emit valueChanged(static_cast<float>(value));
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

void FloatNumberWidget::updateValueLabel(float value)
{
    Q_UNUSED(value);
    m_label->setText(m_itemName);
}

void FloatNumberWidget::setItemName(const QString& name)
{
    m_itemName = name;
    updateValueLabel(value());
}

void FloatNumberWidget::setRange(float min, float max)
{
    float rangeSpan = max - min;
    int decimals = 3;
    if (rangeSpan <= 0.1f) decimals = 4;
    else if (rangeSpan > 10.0f) decimals = 2;

    m_spinBox->setDecimals(decimals);
    m_spinBox->setSingleStep(std::pow(10.0, -decimals));

    double factor = std::pow(10.0, (double)decimals);
    m_slider->setRange(qRound(min * factor), qRound(max * factor));

    QSignalBlocker blocker(m_spinBox);
    m_spinBox->setRange(min, max);
    m_spinBox->setValue(value());
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
    if (nullptr != m_spinBox)
        return static_cast<float>(m_spinBox->value());
    return static_cast<float>(m_slider->value() / 100.0);
}

void FloatNumberWidget::setValue(float value)
{
    int decimals = m_spinBox ? m_spinBox->decimals() : 2;
    double factor = std::pow(10.0, (double)decimals);
    m_slider->setValue(qRound(value * factor));
    if (nullptr != m_spinBox) {
        QSignalBlocker blocker(m_spinBox);
        m_spinBox->setValue(value);
    }
}
