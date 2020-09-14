#ifndef DUST3D_FLOAT_NUMBER_WIDGET_H
#define DUST3D_FLOAT_NUMBER_WIDGET_H
#include <QToolButton>

QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QSlider)

class FloatNumberWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FloatNumberWidget(QWidget *parent = nullptr, bool singleLine=true);
    void setRange(float min, float max);
    float value() const;
    void setItemName(const QString &name);

public slots:
    void increaseValue();
    void descreaseValue();
    void setValue(float value);

signals:
    void valueChanged(float value);
    
private:
    void updateValueLabel(float value);

private:
    QLabel *m_label = nullptr;
    QSlider *m_slider = nullptr;
    QString m_itemName;
};

#endif
