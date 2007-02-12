/*  -*- C++ -*-
This file is part of the KDE libraries
Copyright (C) 1997 Tim D. Gilman (tdgilman@best.org)
(C) 1998-2001 Mirko Boehm (mirko@kde.org)
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
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/
#ifndef KDATETABLE_P_H
#define KDATETABLE_P_H

#include <QtGui/QLineEdit>
#include <Qt3Support/Q3GridView>

class QIntValidator;

/** Week selection widget.
* @internal
* @author Stephan Binner
*/
class KDateInternalWeekSelector : public QLineEdit
{
  Q_OBJECT
protected:
  QIntValidator *val;
  int result;
public Q_SLOTS:
  void weekEnteredSlot();
  void setMaxWeek(int max);
Q_SIGNALS:
  void closeMe(int);
public:
  KDateInternalWeekSelector( QWidget* parent=0);
  int getWeek();
  void setWeek(int week);

private:
  Q_DISABLE_COPY(KDateInternalWeekSelector)
};

/**
* A table containing month names. It is used to pick a month directly.
 * @internal
 * @author Tim Gilman, Mirko Boehm
 */
class KDateInternalMonthPicker : public Q3GridView
{
  Q_OBJECT
protected:
  int year;
  int month;
  int day;
  
  /**
  * Store the month that has been clicked [1..12].
   */
  int result;
  /**
  * the cell under mouse cursor when LBM is pressed
   */
  short int activeCol;
  short int activeRow;
  /**
    * Contains the largest rectangle needed by the month names.
   */
  QRect max;
Q_SIGNALS:
  /**
   * This is send from the mouse click event handler.
   */
  void closeMe(int);
public:
  /**
   * The constructor.
   */
  KDateInternalMonthPicker(const QDate& date, QWidget* parent);
  /**
    * The destructor.
   */
  ~KDateInternalMonthPicker();
  /**
    * The size hint.
   */
  QSize sizeHint() const;
  /**
    * Return the result. 0 means no selection (reject()), 1..12 are the
   * months.
   */
  int getResult() const;
protected:
  /**
   * Set up the painter.
   */
  void setupPainter(QPainter *p);
  /**
    * The resize event.
   */
  virtual void viewportResizeEvent(QResizeEvent*);
  /**
    * Paint a cell. This simply draws the month names in it.
   */
  virtual void paintCell(QPainter* painter, int row, int col);
  /**
    * Catch mouse click and move events to paint a rectangle around the item.
   */
  virtual void contentsMousePressEvent(QMouseEvent *e);
  virtual void contentsMouseMoveEvent(QMouseEvent *e);
  /**
    * Emit monthSelected(int) when a cell has been released.
   */
  virtual void contentsMouseReleaseEvent(QMouseEvent *e);
  
private:
  Q_DISABLE_COPY(KDateInternalMonthPicker)
};

/** Year selection widget.
* @internal
* @author Tim Gilman, Mirko Boehm
*/
class KDateInternalYearSelector : public QLineEdit
{
  Q_OBJECT
protected:
  QIntValidator *val;
  int result;
public Q_SLOTS:
  void yearEnteredSlot();
Q_SIGNALS:
  void closeMe(int);
public:
  KDateInternalYearSelector( QWidget* parent=0);
  int getYear();
  void setYear(int year);

private:
  Q_DISABLE_COPY(KDateInternalYearSelector)
};

#endif // KDATETABLE_P_H
