/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)

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
#ifndef _KCONFIG_TEST_H
#define _KCONFIG_TEST_H

#include <kapp.h>
#include <qdialog.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <kdebug.h>
#include <ksimpleconfig.h>

// Standard Qt widgets

#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>

#include <kconfig.h>

//
// KConfigTestView contains lots of Qt widgets.
//

class KConfigTestView : public QDialog
{
  Q_OBJECT
public:
  KConfigTestView( QWidget *parent=0, const char *name=0 );
  ~KConfigTestView();

private slots:
  void appConfigEditReturnPressed();
  void groupEditReturnPressed();
  void keyEditReturnPressed();
  void writeButtonClicked();

private:
  QLabel* pAppFileLabel;
  QLineEdit* pAppFileEdit;
  QLabel* pGroupLabel;
  QLineEdit* pGroupEdit;
  QLineEdit* pKeyEdit;
  QLabel* pEqualsLabel;
  QLineEdit* pValueEdit;
  QPushButton* pWriteButton;
  QLabel* pInfoLabel1, *pInfoLabel2;
  QPushButton* pQuitButton;

  KConfig* pConfig;
  QFile* pFile;
  QTextStream* pStream;
};

#endif
