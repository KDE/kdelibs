/*  -*- C++ -*-
    This file is part of the KDE libraries
    Copyright (C) 1997 Tim D. Gilman (tdgilman@best.org)
              (C) 1998, 1999 Mirko Sucker (mirko@kde.org)
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

/////////////////// KDatePicker widget class //////////////////////
//
// Copyright (C) 1997 Tim D. Gilman
//           (C) 1998, 1999 Mirko Sucker
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


#ifndef KDATEPICKER_H
#define KDATEPICKER_H
#include <qdatetime.h>
#include <qrect.h>
#include <qframe.h>
// #include "kdatetbl.h"

class QLineEdit;
class QToolButton;
class KDateValidator;
class KDateTable;

/** Widget for selecting dates. 

    Different from the
    previous versions, it now emits two types of signals, either
    dateSelected() or dateEntered() (see documentation for both
    signals). 

    A line edit has been added in the newer versions to allow the user 
    to select a date directly by entering numbers like 19990101
    or 990101. 

    @version $Id$
    @author Tim Gilman, Mirko Sucker
*/
class KDatePicker: public QFrame {
  Q_OBJECT
public:
  /** The usual constructor, the given date will be displayed 
   * initially.
   */
  KDatePicker(QWidget *parent=0, 
	      QDate=QDate::currentDate(), 
	      const char *name=0);
  /** The destructor. */
  virtual ~KDatePicker();
  /** The size hint for KDatePickers. The size hint recommends the
      minimum size of the widget so that all elements may be placed
      without clipping. This sometimes looks ugly, so when using the
      size hint, try adding 28 to each of the reported numbers of
      pixels. */
  QSize sizeHint() const; 
  /** Set the date. Returns false and does not change anything 
      if the date given is invalid. */
  bool setDate(const QDate&);
  /** Get the date. */
  const QDate& getDate();
  /** Enable or disable the widget. */
  void setEnabled(bool);
  /** Set the font size of the widgets elements. */
  void setFontSize(int);
  /// the month names
  static QString *Month[12];
protected:
  /// the resize event
  void resizeEvent(QResizeEvent*);
  /// the font size for the widget
  int fontsize;
  /// the year forward button
  QToolButton *yearForward;
  /// the year backward button
  QToolButton *yearBackward;
  /// the month forward button
  QToolButton *monthForward;
  /// the month backward button
  QToolButton *monthBackward;
  /// the button for selecting the month directly
  QToolButton *selectMonth;
  /// the button for selecting the year directly
  QToolButton *selectYear;
  /// the line edit to enter the date directly
  QLineEdit *line;
  /// the validator for the line edit:
  KDateValidator *val;
  /// the date table 
  KDateTable *table;
  /// the size calculated during resize events
  QSize sizehint;
  /// KDatePicker reference counter
  static int KDatePickers;
  /// the widest month string in pixels:
  QSize maxMonthRect;
protected slots:
  void dateChangedSlot(QDate);
  void tableClickedSlot();
  void monthForwardClicked();
  void monthBackwardClicked();
  void yearForwardClicked();
  void yearBackwardClicked();
  void selectMonthClicked();
  void selectYearClicked();
  void lineEnterPressed();
signals:
  /** This signal is emitted each time the selected date is changed. 
      Usually, this does not mean that the date has been entered,
      since the date also changes, for example, when another month is
      selected. 
      @see dateSelected  */
  void dateChanged(QDate);
  /** This signal is emitted each time a day has been selected by
      clicking on the table (hitting a day in the current month). It
      has the same meaning as dateSelected() in older versions of
      KDatePicker. */
  void dateSelected(QDate);
  /** This signal is emitted when enter is pressed and a VALID date
      has been entered before into the line edit. Connect to both
      dateEntered() and dateSelected() to receive all events where the 
      user really enters a date. */
  void dateEntered(QDate);
  /** This signal is emitted when the day has been selected by
      clicking on it in the table. */
  void tableClicked();
};

#endif //  KDATEPICKER_H
