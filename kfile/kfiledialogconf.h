// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
    
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KFILEDIALOGCONF_H
#define KFILEDIALOGCONF_H

#include <qchkbox.h>
#include <qtabdlg.h>
#include <qradiobt.h>
#include <qlabel.h>
#include <qslider.h>

/**
 * KFileDialog configuration widget.
 *
 * @short This widget allows users to configure KFileDialog
 * @author Richard Moore (rich@kde.org)
 * @version $Id$
 */
class KFileDialogConfigure : public QWidget
{
  Q_OBJECT

public:
  KFileDialogConfigure(QWidget *parent= 0, const char *name= 0);

  enum ViewStyle { ShortView, DetailView };

public slots:
  void saveConfiguration();

protected:  
  QRadioButton *myDetailView;
  QRadioButton *myShortView;
  
  QCheckBox *myShowFilter;
  QCheckBox *myShowHidden;
  QCheckBox *myShowStatusLine;
  QCheckBox *useSingleClick;
  QCheckBox *myShowListLabels;
  QCheckBox *myMixDirsAndFiles;
  QCheckBox *myKeepDirsFirst;

};

class KFileDialogConfigureDlg : public QTabDialog
{
  Q_OBJECT

public:
  KFileDialogConfigureDlg(QWidget *parent, 
			  const char *name);

};

#endif // KFILEDIALOGCONF_H
