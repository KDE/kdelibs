/*  -*- C++ -*-
    This file is part of the KDE libraries
    Copyright (C) 1997 Tim D. Gilman (tdgilman@best.org)
              (C) 1998 Mirko Sucker (mirko.sucker@unibw-hamburg.de)
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
//           (C) 1998 Mirko Sucker
// I also documented protected members as this is a library 
// reference.
// Original header from Tim:
// This version of the class is the first release.  Please
// send comments/suggestions/bug reports to <tdgilman@best.com>
//
// Written using Qt (http://www.troll.no) for the 
// KDE project (http://www.kde.org)
//
//
// Use this class to make a date picker widget
// When a date is selected by the user, it emits a signal: 
//    dateSelected(QDate)
//
// Required header for use:
//    kdatepik.h
// 
// Required files for compilation:
//
//    kdatepik.h kdatepik.cpp kdatetbl.h kdatetbl.cpp


#include <qframe.h>
#include <qdatetime.h>
#include <qsize.h>

class KDateTable;
class QLabel;
class QPushButton;
class QDate;

/** Use this class to make a date picker widget.
  *
  * When a date is selected by the user, it emits a signal: 
  *    dateSelected(QDate)
  *
  * @short A widget for selecting dates.
  * @author Mirko Sucker (mirko.sucker@unibw-hamburg.de)
  * @version $Id$
  */

class KDatePicker : public QFrame {
  Q_OBJECT
public:
  /** The usual constructor, the given date will be displayed 
    * initially.
    */
  KDatePicker(QWidget *parent=0, 
	      QDate=QDate::currentDate(), 
	      const char *name=0);
  // Mirko, Mar 17 1998:
  /** Returns a recommended size for the widget.
    * The recommended size is calculated so that the widget "looks good", 
    * that means everything fits in it and there is some additional space
    * between the rows. Also the buttons and every (!) month name 
    * fit in the headline.
    */
  QSize sizeHint() const;
  // Mirko, Aug 21 1998:
  /** Set the size of the contents of the datepicker widget using 
    * setFontSize.
    * The days are displayed with the font size, the headline gets a size
    * of fontsize + 2 by default. The default is 12pt.
    * Important: this changes the headline size, so adjust it later!
    */
  void setFontSize(int size);
  int fontSize(); // return it
  /** Set the font size of the headline, independent from the rest of the widget.
    * The default size is fontsize + 2.
    */
  void setHeadlineSize(int size);
  int headlineSize();
  // ^^^^^^^^^^^^^^^^^^^
private:
  KDateTable *m_tbl;
  QLabel *m_header;
  QLabel *m_footer;
  QPushButton *m_back;
  QPushButton *m_forward;
  // highstick: added May 13 1998
  QPushButton *m_up;
  QPushButton *m_down;
  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^
protected:
  void resizeEvent(QResizeEvent *);
  void sizeElements();
  /// The size that lets the button look best.
  static const int PreferredButtonSize;
  /// The minimum space between the button and the border.
  static const int MinimumFrameAroundButtons;
  /// The number of month in a year, usually 12 I suppose :-).
  static const int NoOfMonth;
  /// The names of the 12 month translated to the user language.
  QString Month[12];
  int fontsize;
signals:
  /** This signal is emmitted when the user picked a date.
    */
  void dateSelected(QDate);
private slots:
public slots:
  void updateHeader(QDate dt);
  // Mirko, Mar 17 1998:
  void setDate(QDate);
  // ^^^^^^^^^^^^^^^^^^^
};

#endif // _KDATEPIK_H
