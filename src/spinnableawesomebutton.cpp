#include "spinnableawesomebutton.h"
#include "theme.h"

SpinnableAwesomeButton::SpinnableAwesomeButton(QWidget *parent) :
    QWidget(parent)
{
    setFixedSize(Theme::toolIconSize, Theme::toolIconSize);

    m_button = new QPushButton(this);
    Theme::initAwesomeButton(m_button);
    
    m_spinner = new WaitingSpinnerWidget(this);
    m_spinner->setColor(Theme::white);
    m_spinner->setInnerRadius(Theme::toolIconSize / 8);
    m_spinner->setLineLength(Theme::toolIconSize / 4);
    m_spinner->setNumberOfLines(9);
    m_spinner->hide();
}

void SpinnableAwesomeButton::setAwesomeIcon(QChar c)
{
    m_button->setText(c);
}

void SpinnableAwesomeButton::showSpinner(bool showSpinner)
{
    if (showSpinner) {
        m_spinner->start();
        m_spinner->show();
        m_button->hide();
    } else {
        m_spinner->stop();
        m_spinner->hide();
        m_button->show();
    }
}

QPushButton *SpinnableAwesomeButton::button()
{
    return m_button;
}


bool SpinnableAwesomeButton::isSpinning()
{
    return m_spinner->isVisible();
}
