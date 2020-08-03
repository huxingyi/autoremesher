/****************************************************************************

 Copyright (c) 2018  GeometryFactory Sarl (France).
 Copyright (C) 2002-2014 Gilles Debunne. All rights reserved.

 This file is part of a fork of the QGLViewer library version 2.7.0.

*****************************************************************************/
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/GraphicsView/include/CGAL/Qt/image_interface.h $
// $Id: image_interface.h 1ef976e 2019-10-19T16:09:56+02:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-only

#ifndef CGAL_IMAGE_INTERFACE_H
#define CGAL_IMAGE_INTERFACE_H
#include <CGAL/license/GraphicsView.h>

#include <QDialog>
#include <QWidget>
#include <QApplication>

#include "ui_ImageInterface.h"
class ImageInterface: public QDialog, public Ui::ImageInterface
{
  Q_OBJECT
  qreal ratio;
  QWidget *currentlyFocused;
public:
  ImageInterface(QWidget *parent, qreal ratio)
    : QDialog(parent), ratio(ratio)
  {
    currentlyFocused = nullptr;
    setupUi(this);
    connect(imgHeight, SIGNAL(valueChanged(int)),
            this, SLOT(imgHeightValueChanged(int)));

    connect(imgWidth, SIGNAL(valueChanged(int)),
            this, SLOT(imgWidthValueChanged(int)));

    connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)),
            this, SLOT(onFocusChanged(QWidget*, QWidget*)));
  }
private Q_SLOTS:
  void imgHeightValueChanged(int i)
  {
    if(currentlyFocused == imgHeight
       && ratioCheckBox->isChecked())
    {imgWidth->setValue(int(i*ratio));}
  }

  void imgWidthValueChanged(int i)
  {
    if(currentlyFocused == imgWidth
       && ratioCheckBox->isChecked())
    {imgHeight->setValue(int(i/ratio));}
  }

  void onFocusChanged(QWidget*, QWidget* now)
  {
    currentlyFocused = now;
  }
};
#endif // CGAL_IMAGE_INTERFACE_H
