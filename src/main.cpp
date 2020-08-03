#include <QApplication>
#include <QDesktopWidget>
#include <QStyleFactory>
#include <QFontDatabase>
#include <QDebug>
#include <QtGlobal>
#include <QSurfaceFormat>
#include <QSettings>
#include <QTranslator>
#include "mainwindow.h"
#include "theme.h"
#include "version.h"
#include "preferences.h"

void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static FILE *s_file = fopen("autoremesher.log", "w");
    fprintf(s_file, "[%s:%u]: %s\n", context.file, context.line, msg.toUtf8().constData());
    fflush(s_file);
}

int main(int argc, char ** argv)
{
    qInstallMessageHandler(outputMessage);
    QApplication app(argc, argv);

    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setProfile(QSurfaceFormat::OpenGLContextProfile::CoreProfile);
    format.setVersion(3, 3);
    QSurfaceFormat::setDefaultFormat(format);
    
    qApp->setStyle(QStyleFactory::create("Fusion"));
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, Theme::black);
    darkPalette.setColor(QPalette::WindowText, Theme::white);
    darkPalette.setColor(QPalette::Base, QColor(25,25,25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53,53,53));
    //darkPalette.setColor(QPalette::ToolTipBase, Theme::white);
    //darkPalette.setColor(QPalette::ToolTipText, Theme::white);
    darkPalette.setColor(QPalette::Text, Theme::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, Theme::black);
    darkPalette.setColor(QPalette::Button, QColor(53,53,53));
    darkPalette.setColor(QPalette::ButtonText, Theme::white);
    darkPalette.setColor(QPalette::BrightText, Theme::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, Theme::red);
    darkPalette.setColor(QPalette::HighlightedText, Theme::black);    
    qApp->setPalette(darkPalette);
    //qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #fc6621; border: 1px solid white; }");
    
    QCoreApplication::setApplicationName(APP_NAME);
    QCoreApplication::setOrganizationName(APP_COMPANY);
    QCoreApplication::setOrganizationDomain(APP_HOMEPAGE_URL);
    
    QFont font;
    font.setWeight(QFont::Light);
    //font.setPixelSize(11);
    font.setBold(false);
    QApplication::setFont(font);
    
    Theme::initAwsomeBaseSizes();
    
    MainWindow *mainWindow = new MainWindow();
    mainWindow->setAttribute(Qt::WA_DeleteOnClose);
    QSize size = Preferences::instance().mainWindowSize();
    if (size.isValid()) {
        mainWindow->resize(size);
        mainWindow->show();
    } else {
        mainWindow->showMaximized();
    }
    
    return app.exec();
}
