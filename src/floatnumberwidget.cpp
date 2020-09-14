#include <QtWidgets>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "theme.h"
#include "floatnumberwidget.h"

FloatNumberWidget::FloatNumberWidget(QWidget *parent, bool singleLine) :
    QWidget(parent)
{
    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setRange(0, 100);
    m_slider->setFixedWidth(240);

    m_label = new QLabel(this);
    m_label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_label->setAlignment(Qt::AlignRight);

    connect(m_slider, &QAbstractSlider::valueChanged, [=](int value) {
        float fvalue = value / 100.0;
        updateValueLabel(fvalue);
        emit valueChanged(fvalue);
    });
    
    QBoxLayout *layout = nullptr;
    if (singleLine) {
        layout = new QHBoxLayout(this);
        layout->setMargin(2);
        layout->addWidget(m_label);
        layout->addWidget(m_slider);
    } else {
        layout = new QVBoxLayout(this);
        layout->setMargin(2);
        layout->addWidget(m_label);
        layout->addWidget(m_slider);
    }
    
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

void FloatNumberWidget::updateValueLabel(float value)
{
    QString valueString = QString().sprintf("%.2f", value);
    if (m_itemName.isEmpty())
        m_label->setText(valueString);
    else
        m_label->setText(m_itemName + ": " + valueString);
}

void FloatNumberWidget::setItemName(const QString &name)
{
    m_itemName = name;
    updateValueLabel(value());
}

void FloatNumberWidget::setRange(float min, float max)
{
    m_slider->setRange(min * 100, max * 100);
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
    m_slider->setValue(value * 100);
}
