// Copyright (c) 2012  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Arrangement_on_surface_2/demo/Arrangement_on_surface_2/OverlayDialog.h $
// $Id: OverlayDialog.h 254d60f 2019-10-19T15:23:19+02:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Alex Tsui <alextsui05@gmail.com>

#ifndef OVERLAY_DIALOG_H
#define OVERLAY_DIALOG_H

#include <QDialog>
#include <vector>
#include <CGAL/Object.h>

class ArrangementDemoWindow;
class QListWidgetItem;
namespace Ui { class OverlayDialog; }

class OverlayDialog : public QDialog
{
  Q_OBJECT

  public:
  typedef enum OverlayDialogRole {
    ARRANGEMENT = 32
  } OverlayDialogRole;

  OverlayDialog( ArrangementDemoWindow* parent, Qt::WindowFlags f = 0 );

  std::vector< CGAL::Object > selectedArrangements( ) const;

public Q_SLOTS:
  void on_pickPushButton_pressed( );
  void on_unpickPushButton_pressed( );

protected:
  void restrictSelection( QListWidgetItem* item );
  void unrestrictSelection( );

  Ui::OverlayDialog* ui;
};

#endif // OVERLAY_DIALOG_H
