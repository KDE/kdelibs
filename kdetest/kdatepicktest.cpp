/* This file is part of the KDE libraries
    Copyright (C) 1997 Tim D. Gilman (tdgilman@best.org)

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
#include <kapp.h>
#include <qpixmap.h>
#include "kdatepicktestdlg.h"
#include "kdatepik.h"

int main(int argc, char **argv)
{
   KApplication a(argc, argv);
//   a.setStyle(WindowsStyle); // comment out for Motif
   
//   CMainDlg *w = new CMainDlg;
   KDatePicker *w = new KDatePicker;

   a.setMainWidget(w);
   w->resize(220,200);
   w->show();
   return a.exec();
}
   
