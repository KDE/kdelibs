/* This file is part of the KDE libraries
    Copyright (C) 1997 Tim D.Gilman (tdgilman@best.com)

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
#include <stdlib.h>

#include <qapplication.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qdatetime.h>

#include "kdatepicktestdlg.h"
#include "kdatepicktestdlg.h"

#include "kdatepik.h"

int getRand(int nLower, int nUpper);

CMainDlg::CMainDlg(QWidget *parent, const char *name)
       :QDialog(parent,name)
{
   initMetaObject();
   
   setCaption("KDatePicker Demonstration");
   setFixedSize(2*285,2*152);

   // buttons
   QPushButton *pb;

   pb = new QPushButton("Quit",this);
   pb->setGeometry(2*195,2*130,2*50,2*14);
   connect(pb, SIGNAL(clicked()), SLOT(quitapp()));

   pb = new QPushButton("About",this);
   pb->setGeometry(2*30,2*130,2*50,2*14);
   connect(pb,SIGNAL(clicked()),SLOT(about()));

   KDatePicker *dp;
   dp = new KDatePicker(this);
   dp->setGeometry(10,10,222,200);
   
   m_lab = new QLabel(this);
   m_lab->setGeometry(270, 110, 150, 28);
   
   connect(dp, SIGNAL(dateSelected(QDate)), SLOT(printDate(QDate)));
   
}

void CMainDlg::quitapp()
{
   qApp->quit();
}

void CMainDlg::about()
{
   QMessageBox mb;
   mb.setText("Send comments/suggestions/bugs to\nTim Gilman  <tdgilman@best.com>");
   mb.show();
}

void CMainDlg::printDate(QDate dt)
{
   m_lab->setText((const char*)dt.toString());
}
#include "kdatepicktestdlg.moc"

