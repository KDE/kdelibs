/* This file is part of the KDE libraries
   Copyright (C) 2002 Hans Petter bieker <bieker@kde.org>

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

#ifndef __KDATETIMEWIDGET__
#define __KDATETIMEWIDGET__

#include <qwidget.h>
#include <qdatetime.h>

/**
 * This widget can be used to display or allow user selection of date and time.
 *
 * @see KDateWidget
 *
 * @short A combination of a date and a time selection widget.
 * @author Hans Petter Bieker <bieker@kde.org>
 * @version $Id$
 * @since 3.2
 */
class KDateTimeWidget : public QWidget
{
  Q_OBJECT

public:
  /**
   * Constructs a date and time selection widget.
   */
  KDateTimeWidget(QWidget * parent = 0, const char * name = 0);

  /**
   * Constructs a date and time selection widget with the initial date and
   * time set to @p datetime.
   */
  KDateTimeWidget(const QDateTime & datetime,
                  QWidget * parent = 0, const char * name = 0 );

  /**
   * Destructs the date and time selection widget.
   */
  virtual ~KDateTimeWidget();

  /**
   * Returns the currently selected date and time.
   */
  QDateTime dateTime() const;

public slots:
  /**
   * Changes the selected date and time to @p datetime.
   */
  void setDateTime(const QDateTime & datetime);

signals:
  /**
   * Emitted whenever the date or time of the widget
   * is changed, either with @ref #setDateTime() or via user selection.
   */
  void valueChanged(const QDateTime & datetime);

private:
  void init();

private slots:
  void slotValueChanged();

private:
  class KDateTimeWidgetPrivate;
  KDateTimeWidgetPrivate *d;
};

#endif
