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
#ifndef _KDATEPIK_H
#define _KDATEPIK_H

/////////////////// KDatePicker widget class //////////////////////
//
// Copyright (C) 1997 Tim D. Gilman
// This version of the class is the first release.  Please
// send comments/suggestions/bug reports to <tdgilman@best.com>
//
// Written using Qt (http://www.troll.no) for the 
// KDE project (http://www.kde.org)
//
//
// Use this class to make a date picker widget
// When a date is selected by the user, it emits a signal: dateSelected(QDate)
//
// Required header for use:
//    kdatepik.h
// 
// Required files for compilation:
//
//    kdatepik.h kdatepik.cpp kdatetbl.h kdatetbl.cpp


#include <qframe.h>
#include <qdatetm.h>

class KDateTable;
class QLabel;
class QPushButton;
class QDate;

/** 
* Use this class to make a date picker widget.
*
* When a date is selected by the user, it emits a signal: dateSelected(QDate)
*
* @short A widget for selecting dates.
* @author Tim Gilman (tdgilman@best.com)
* @version $Id$
*/
class KDatePicker : public QFrame {
   Q_OBJECT
 public:
   KDatePicker(QWidget *parent=0, QDate=QDate::currentDate(), const char *name=0);

 private:
   KDateTable *m_tbl;
   QLabel *m_header;
   QLabel *m_footer;
   QPushButton *m_back;
   QPushButton *m_forward;

 protected:
   void resizeEvent(QResizeEvent *);
   void sizeElements();

 signals:
   void dateSelected(QDate);

 private slots:

 public slots:
   void updateHeader(QDate dt);

};

#endif // _KDATEPIK_H
