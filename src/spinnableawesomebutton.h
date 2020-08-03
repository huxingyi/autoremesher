#ifndef DUST3D_SPINNABLE_AWESOME_BUTTON_H
#define DUST3D_SPINNABLE_AWESOME_BUTTON_H
#include <QWidget>
#include <QPushButton>
#include "waitingspinnerwidget.h"

class SpinnableAwesomeButton : public QWidget
{
public:
    SpinnableAwesomeButton(QWidget *parent=nullptr);
    void setAwesomeIcon(QChar c);
    void showSpinner(bool showSpinner=true);
    bool isSpinning();
    QPushButton *button();
private:
    QPushButton *m_button = nullptr;
    WaitingSpinnerWidget *m_spinner = nullptr;
};

#endif
