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

/////////////////// KDateTable widget class //////////////////////
//
// Copyright (C) 1997 Tim D. Gilman
//           (C) 1998, 1999 Mirko Sucker
// Written using Qt (http://www.troll.no) for the
// KDE project (http://www.kde.org)
//
// This is a support class for the KDatePicker class.  It just
// draws the calender table without titles, but could theoretically
// be used as a standalone.
//
// When a date is selected by the user, it emits a signal: 
//      dateSelected(QDate)

#ifndef KDATETBL_H
#define KDATETBL_H

#include <qvalidator.h>
#include <qtableview.h>
#include <qlineedit.h>
#include <qdatetime.h>
#include <qsize.h>

/** This class defines a table containing all month names. 
    It is used to pick a month directly. */
class KDateInternalMonthPicker : public QTableView
{
  Q_OBJECT
protected:
  /** Store the month that has been clicked [1..12]. */
  int result;
  /** Contains the largest rectangle needed by the month names. */
  QRect max;
signals: 
  /** This is send from the mouse click event handler. */
  void closeMe(int);
public:
  /** The constructor. */
  KDateInternalMonthPicker(int fontsize, QWidget* parent, const char* name=0);
  /** The size hint. */
  QSize sizeHint() const;
  /** Return the result. 0 means no selection (reject()), 1..12 are the
      months. */
  int getResult();
protected:
  /** Set up the painter. */
  void setupPainter(QPainter *p);
  /** The resize event. */
  void resizeEvent(QResizeEvent*);
  /** Paint a cell. This simply draws the month names in it. */
  void paintCell(QPainter* painter, int row, int col);
  /** Catch mouse click events. 
      Emit monthSelected(int) when a cell has been clicked. */
  void mousePressEvent(QMouseEvent *e);
};

/** This class implements a widget to select a year by entering it. */
class KDateInternalYearSelector : public QLineEdit
{
  Q_OBJECT
protected:
  QIntValidator *val;
  int result;
public slots:
  void yearEnteredSlot();
signals:
  void closeMe(int);
public:
  KDateInternalYearSelector(int fontsize, 
			    QWidget* parent=0, 
			    const char* name=0);
  int getYear();
  void setYear(int year);
};

/** This class implements a frame that behaves like a popup menu
    and shows the main widget in its client rectangle. */
class QPopupFrame : public QFrame
{
  Q_OBJECT
protected:
  /** The result. It is returned from exec() when the popup window closes. */
  int result;
  /** Catch key press events. */
  void keyPressEvent(QKeyEvent* e);
  /** The only subwidget that uses the whole dialog window. */
  QWidget *main;
public slots:
  /** Close the popup window. This is called from the main widget, usually. 
      r is the result returned from exec(). */
  void close(int r);
public:
  /** The contructor. Creates a dialog without buttons. */
  QPopupFrame(QWidget* parent=0, const char*  name=0);
  /** Set the main widget. You cannot set the main widget from the constructor,
      since it must be a child of the frame itselfes. 
      Be careful: the size is set to the main widgets size. It is up to you to 
      set the main widgets correct size before setting it as the main 
      widget. */
  void setMainWidget(QWidget* m);
  /** The resize event. Simply resizes the main widget to the whole 
      widgets client size. */
  void resizeEvent(QResizeEvent*);
  /** Execute the popup window. */
  int exec(QPoint p);
  /** Dito. */
  int exec(int x, int y);
};

/** KDateValidator validates the values entered into the line 
    edit. */
class KDateValidator : public QValidator
{
public:
  KDateValidator(QWidget* parent=0, const char* name=0);
  State validate(QString&, int&) const;
  State date(const QString&, QDate&) const;
};

class KDateTable : public QTableView
{
  Q_OBJECT
public:
  /** The constructor. */
  KDateTable(QWidget *parent=0,
	     QDate date=QDate::currentDate(),
	     const char* name=0, WFlags f=0);
  /** Returns a recommended size for the widget.
      To save some time, the size of the largest used cell content is
      calculated in each paintCell() call, since all calculations have
      to be done there anyway. The size is stored in maxCell. The
      sizeHint() simply returns a multiple of maxCell. */
  QSize sizeHint() const;
  /** Set the font size of the date table. */
  void setFontSize(int size);  
  /** Select and display this date. */
  bool setDate(const QDate&);
  const QDate& getDate();
protected:
  /** Paint a cell. */
  void paintCell(QPainter*, int, int);
  /** Handle the resize events. */
  void resizeEvent(QResizeEvent *);
  /** React on mouse clicks that select a date. */
  void mousePressEvent(QMouseEvent *);
  /** The font size of the displayed text. */
  int fontsize;
  /** The currently selected date. */
  QDate date;
  /** The day of the first day in the month [1..7]. */
  int firstday;
  /** The number of days in the current month. */
  int numdays;
  /** The number of days in the previous month. */
  int numDaysPrevMonth;
  /** Whether something has been selected or not. */
  bool hasSelection;
  /** Save the size of the largest used cell content. */
  QRect maxCell;
  /** The day names. */
  QString Days[7];
signals:
  /** The selected date changed. */
  void dateChanged(QDate);
  /** A date has been selected by clicking on the table. */
  void tableClicked();
};

#endif // KDATETBL_H
