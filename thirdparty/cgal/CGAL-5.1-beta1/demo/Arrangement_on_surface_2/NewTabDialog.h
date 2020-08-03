// Copyright (c) 2012  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Arrangement_on_surface_2/demo/Arrangement_on_surface_2/NewTabDialog.h $
// $Id: NewTabDialog.h 254d60f 2019-10-19T15:23:19+02:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Alex Tsui <alextsui05@gmail.com>

#ifndef NEW_TAB_DIALOG_H
#define NEW_TAB_DIALOG_H

#include <QDialog>

class QButtonGroup;
namespace Ui
{
  class NewTabDialog;
}

class NewTabDialog : public QDialog
{
public:
  NewTabDialog( QWidget* parent = 0, Qt::WindowFlags f = 0 );
  int checkedId( ) const;

protected:
  Ui::NewTabDialog* ui;
  QButtonGroup* buttonGroup;
};
#endif // NEW_TAB_DIALOG_H
