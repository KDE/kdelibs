/* This file is part of the KDE libraries
   Copyright (C) 2000 Reginald Stadlbauer <reggie@kde.org>

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
#ifndef KLISTWIDGET_H
#define KLISTWIDGET_H

#include <kdeui_export.h>

#include <QtGui/QListWidget>

/**
 * @short A variant of QListWidget that honors KDE's system-wide settings.
 *
 * Extends the functionality of QListWidget to honor the system
 * wide settings for Single Click/Double Click mode, Auto Selection and
 * Change Cursor over Link.
 *
 * There is a new signal executed(). It gets connected to either
 * QListWidget::itemClicked() or QListWidget::itemDoubleClicked()
 * depending on the KDE wide Single Click/Double Click settings. It is
 * strongly recommended that you use this signal instead of the above
 * mentioned. This way you don't need to care about the current
 * settings.  If you want to get informed when the user selects
 * something connect to the QListWidget::itemSelectionChanged() signal.
 **/
class KDEUI_EXPORT KListWidget : public QListWidget
{
    Q_OBJECT

public:
  explicit KListWidget( QWidget *parent = 0 );

  ~KListWidget();

Q_SIGNALS:

  /**
   * Emitted whenever the user executes an listbox item.
   *
   * That means depending on the KDE wide Single Click/Double Click
   * setting the user clicked or double clicked on that item.
   * @param item is the pointer to the executed listbox item.
   *
   * Note that you may not delete any QListWidgetItem objects in slots
   * connected to this signal.
   */
  void executed( QListWidgetItem *item );

  /**
   * Emitted whenever the user executes an listbox item.
   *
   * That means depending on the KDE wide Single Click/Double Click
   * setting the user clicked or double clicked on that item.
   * @param item is the pointer to the executed listbox item.
   * @param pos is the position where the user has clicked
   *
   * Note that you may not delete any QListWidgetItem objects in slots
   * connected to this signal.
   */
  void executed( QListWidgetItem *item, const QPoint &pos );

  /**
   * This signal gets emitted whenever the user double clicks into the
   * listbox.
   *
   * @param item The pointer to the clicked listbox item.
   * @param pos The position where the user has clicked.
   *
   * Note that you may not delete any QListWidgetItem objects in slots
   * connected to this signal.
   *
   * This signal is more or less here for the sake of completeness.
   * You should normally not need to use this. In most cases it's better
   * to use executed() instead.
   */
  void doubleClicked( QListWidgetItem *item, const QPoint &pos );

protected:
  virtual void keyPressEvent(QKeyEvent *e);
  virtual void focusOutEvent(QFocusEvent *e);
  virtual void leaveEvent(QEvent *e);
  virtual void mousePressEvent(QMouseEvent *e);
  virtual void mouseDoubleClickEvent (QMouseEvent *e);

private:
  class KListWidgetPrivate;
  KListWidgetPrivate* const d;

  Q_PRIVATE_SLOT(d, void _k_slotItemEntered(QListWidgetItem*))
  Q_PRIVATE_SLOT(d, void _k_slotOnViewport())
  Q_PRIVATE_SLOT(d, void _k_slotSettingsChanged(int))
  Q_PRIVATE_SLOT(d, void _k_slotAutoSelect())
};

#endif // KLISTWIDGET_H
