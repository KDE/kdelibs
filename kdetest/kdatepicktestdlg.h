/* This file is part of the KDE libraries
    Copyright (C) 1997 Tim D. Gilman (tdgilman@best.com)

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
#ifndef _MAINDLG_H
#define _MAINDLG_H

#include <qdialog.h>

class QLabel;
class QDate;

class CMainDlg : public QDialog {
   Q_OBJECT
 public:
   CMainDlg(QWidget *parent=0, const char *name=0);

 private:
   QLabel *m_lab;

 private slots:
   void about();

 public slots:
   void quitapp();
   void printDate(QDate dt);
};

#endif // _MAINDLG_H
