/*  -*- C++ -*-
    This file is part of the KDE libraries
    Copyright (C) 1997 Tim D. Gilman (tdgilman@best.org)
              (C) 1998 Mirko Sucker (mirko.sucker@hamburg.netsurf.de)
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
#ifndef _KDATETBL_H 
#define _KDATETBL_H

/////////////////// KDateTable widget class //////////////////////
//
// Copyright (C) 1997 Tim D. Gilman
//           (C) 1998 Mirko Sucker
// Written using Qt (http://www.troll.no) for the
// KDE project (http://www.kde.org)
//
// This is a support class for the KDatePicker class.  It just
// draws the calender table without titles, but could theoretically
// be used as a standalone.
//
// When a date is selected by the user, it emits a signal: 
//      dateSelected(QDate)

#include <qtablevw.h>
#include <qdatetm.h>
#include <qsize.h>

/**
  * Draws a calendar table.
  * @author Tim D. Gilman
  * @version $Id$
  */

class KDateTable: public QTableView {
  Q_OBJECT
public:
  KDateTable(QWidget *parent=0, 
	     QDate date=QDate::currentDate(), 
	     const char *name=0, WFlags f=0);
  ~KDateTable();
  // Mirko, Mar 17 1998:
  /** Returns a recommended size for the widget.
    */
  QSize sizeHint() const;
  // ^^^^^^^^^^^^^^^^^^^
public slots:
  void goForward();
  void goBackward();
  // highstick: added May 13 1998
  void goDown();
  void goUp();
  // Mirko, Mar 17 1998:
  /** Sets the currently selected date. The date is
    * only set if the given date is a valid one.
    */
  void setDate(QDate);
  // ^^^^^^^^^^^^^^^^^^^
signals:
  void monthChanged(QDate);
  void yearChanged(QDate);	// highstick: added May 13 1998
  void dateSelected(QDate);
protected:
  void paintCell( QPainter *p, int row, int col );
  void resizeEvent( QResizeEvent * );
  void mousePressEvent(QMouseEvent *e);
  const char* Days[7];
private:
  QDate m_date;
  int m_firstDayOfWeek;
  int m_daysInPrevMonth;
  int m_oldRow;
  int m_oldCol;
  bool m_bSelection;
  int m_selRow;
  int m_selCol;

  void setSelection(int row, int col);
  void getPrevMonth(QDate dtnow, QDate &dtprv);
  void getNextMonth(QDate dtnow, QDate &dtnxt);
  /** Returns the number of the day at position (row,col).
    * The number is a negative one if the day is in the 
    * previous month, and it is bigger than m_date.daysInMonth()
    * if the day is in the next month.
    */
  int dayNum(int row, int col);
  // highstick: added May 13 1998
  void getPrevYear(QDate dtnow, QDate &dtprv);
  void getNextYear(QDate dtnow, QDate &dtprv);
private:  // Disabled copy constructor and operator=
  KDateTable(const KDateTable & ) : QTableView() {}
  KDateTable &operator=(const KDateTable &) { return *this; }
};

#endif // _KDATETBL_H
