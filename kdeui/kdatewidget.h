/* This file is part of the KDE libraries
    Copyright (C) 2001 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation. 

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef __KDATECTL_H__
#define __KDATECTL_H__

#include <qwidget.h>
#include <qdatetime.h>

/**
* This widget can be used to display or allow user selection of a date.
*
* @see KDatePicker
*
* @short A pushbutton to display or allow user selection of a date.
* @version $Id$
*/
class KDateWidget : public QWidget
{
  Q_OBJECT

public:
  /**
   * Constructs a date selection widget.
   */
  KDateWidget( QWidget *parent=0, const char *name=0 );

  /**
   * Constructs a date selection widget with the initial date set to @p date.
   */
  KDateWidget( QDate date, QWidget *parent=0, const char *name=0 );

  /**
   * Destructs the date selection widget.
   */
  virtual ~KDateWidget();

  /**
   * Returns the currently selected date.
   */
  QDate date() const;

  /**
   * Changes the selected date to @p date.
   */
  void setDate(QDate date);


signals:
  /**
   * Emitted whenever the date of the widget
   * is changed, either with @ref #setDate() or via user selection.
   */
   void changed(QDate);

protected:
   void init();

protected slots:
  void slotDateChanged();

private:
   class KDateWidgetPrivate;
   KDateWidgetPrivate *d;
};

#endif

