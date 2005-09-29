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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __KTIMEWIDGET__
#define __KTIMEWIDGET__

#include <qwidget.h>
#include <qdatetime.h>

#include <kdelibs_export.h>

/**
 * @short A time selection widget.
 *
 * This widget can be used to display or allow user selection of time.
 *
 * \image html ktimewidget.png "KDE Time Widget"
 *
 * @author Hans Petter Bieker <bieker@kde.org>
 * @since 3.2
 */
class KDEUI_EXPORT KTimeWidget : public QWidget
{
  Q_OBJECT
  Q_PROPERTY( QTime time READ time WRITE setTime )

public:
  /**
   * Constructs a time selection widget.
   */
  KTimeWidget(QWidget * parent = 0, const char * name = 0);

  /**
   * Constructs a time selection widget with the initial time set to
   * @p time.
   */
  KTimeWidget(const QTime & time,
              QWidget * parent = 0, const char * name = 0 );

  /**
   * Destructs the time selection widget.
   */
  virtual ~KTimeWidget();

  /**
   * Returns the currently selected time.
   */
  QTime time() const;

public slots:
  /**
   * Changes the selected time to @p time.
   */
  void setTime(const QTime & time);

signals:
  /**
   * Emitted whenever the time of the widget
   * is changed, either with setTime() or via user selection.
   */
  void valueChanged(const QTime & time);

private:
  void init();

private:
  class KTimeWidgetPrivate;
  KTimeWidgetPrivate *d;
};

#endif
