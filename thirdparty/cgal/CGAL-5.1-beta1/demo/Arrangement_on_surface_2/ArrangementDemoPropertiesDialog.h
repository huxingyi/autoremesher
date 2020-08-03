// Copyright (c) 2012  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Arrangement_on_surface_2/demo/Arrangement_on_surface_2/ArrangementDemoPropertiesDialog.h $
// $Id: ArrangementDemoPropertiesDialog.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Alex Tsui <alextsui05@gmail.com>

#ifndef ARRANGEMENT_DEMO_PROPERTIES_DIALOG_H
#define ARRANGEMENT_DEMO_PROPERTIES_DIALOG_H

#include <QDialog>

class ArrangementDemoWindow;

namespace Ui
{
  class ArrangementDemoPropertiesDialog;
}

class ArrangementDemoPropertiesDialog : public QDialog
{
  Q_OBJECT
  public:
  // keep this in order with the ui layout
  enum PropertyKey {
    EDGE_COLOR_KEY,
    EDGE_WIDTH_KEY,
    VERTEX_COLOR_KEY,
    VERTEX_RADIUS_KEY,
    ENVELOPE_EDGE_COLOR_KEY,
    ENVELOPE_EDGE_WIDTH_KEY,
    ENVELOPE_VERTEX_COLOR_KEY,
    ENVELOPE_VERTEX_RADIUS_KEY,
    VERTICAL_RAY_EDGE_COLOR_KEY,
    VERTICAL_RAY_EDGE_WIDTH_KEY,
    DELETE_CURVE_MODE_KEY,
    GRID_SIZE_KEY,
    GRID_COLOR_KEY
  };

  ArrangementDemoPropertiesDialog( ArrangementDemoWindow* parent_ = 0,
                                   Qt::WindowFlags f = 0 );
  QVariant property( int index );

protected:
  void setupUi( );
  void updateUi( );

  ArrangementDemoWindow* parent;
  Ui::ArrangementDemoPropertiesDialog* ui;
}; // class ArrangementDemoPropertiesDialog

#endif // ARRANGEMENT_DEMO_PROPERTIES_DIALOG_H
