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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDATECTL_H
#define KDATECTL_H

#include <qwidget.h>

#include <kdelibs_export.h>

class QDate;


/**
* @short A pushbutton to display or allow user selection of a date.
*
* This widget can be used to display or allow user selection of a date.
*
* @see KDatePicker
*/
class KDEUI_EXPORT KDateWidget : public QWidget
{
  Q_OBJECT
  Q_PROPERTY( QDate date READ date WRITE setDate USER true )

public:
  /**
   * Constructs a date selection widget.
   */
  explicit KDateWidget( QWidget *parent=0 );

  /**
   * Constructs a date selection widget with the initial date set to @p date.
   */
  explicit KDateWidget( const QDate &date, QWidget *parent=0 );

  /**
   * Destructs the date selection widget.
   */
  virtual ~KDateWidget();

  /**
   * Returns the currently selected date.
   */
  const QDate& date() const;

  /**
   * Changes the selected date to @p date.
   */
  void setDate(const QDate &date);


signals:
  /**
   * Emitted whenever the date of the widget
   * is changed, either with setDate() or via user selection.
   */
   void changed(const QDate&);

protected:
   void init(const QDate&);

protected slots:
  void slotDateChanged();

protected:
  virtual void virtual_hook( int id, void* data );
private:
   class KDateWidgetPrivate;
   KDateWidgetPrivate *d;
};

#endif

