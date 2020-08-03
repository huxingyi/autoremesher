/**
 * MIT Licensed
 *
 * Copyright 2011-2015 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "QtAwesome.h"

#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QMainWindow w;

    QtAwesome* awesome = new QtAwesome(&w);
    awesome->initFontAwesome();

    QVBoxLayout* layout = new QVBoxLayout();

    // a simple beer button
    //=====================
    {
        QPushButton* beerButton = new QPushButton( "Cheers!");

        QVariantMap options;
        options.insert("anim", qVariantFromValue( new QtAwesomeAnimation(beerButton) ) );
        beerButton->setIcon( awesome->icon( fa::beer, options  ) );

        layout->addWidget(beerButton);
    }

    // a simple beer checkbox button
    //==============================
    {
        QPushButton* toggleButton = new QPushButton("Toggle Me");
        toggleButton->setCheckable(true);

        QVariantMap options;
        options.insert("color", QColor(Qt::green) );
        options.insert("text-off", QString(fa::squareo) );
        options.insert("color-off", QColor(Qt::red) );
        toggleButton->setIcon( awesome->icon( fa::checksquareo, options ));


        layout->addWidget(toggleButton);
    }


    // add the samples
    QWidget* samples = new QWidget();
    samples->setLayout(layout);
    w.setCentralWidget(samples);

    w.show();
    
    return app.exec();
}
