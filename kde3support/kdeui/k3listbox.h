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
#ifndef K3LISTBOX_H
#define K3LISTBOX_H

#include <kde3support_export.h>
#include <Qt3Support/Q3ListBox>

/**
 * @short A variant of QListBox that honors KDE's system-wide settings.
 *
 * Extends the functionality of QListBox to honor the system
 * wide settings for Single Click/Double Click mode, Auto Selection and
 * Change Cursor over Link.
 *
 * There is a new signal executed(). It gets connected to either
 * QListBox::clicked() or QListBox::doubleClicked()
 * depending on the KDE wide Single Click/Double Click settings. It is
 * strongly recommended that you use this signal instead of the above
 * mentioned. This way you don't need to care about the current
 * settings.  If you want to get informed when the user selects
 * something connect to the QListBox::selectionChanged() signal.
 **/
class KDE3SUPPORT_EXPORT K3ListBox : public Q3ListBox
{
    Q_OBJECT

public:
  explicit K3ListBox( QWidget *parent = 0, const char *name = 0, Qt::WFlags f = 0 );

Q_SIGNALS:

  /**
   * Emitted whenever the user executes an listbox item.
   *
   * That means depending on the KDE wide Single Click/Double Click
   * setting the user clicked or double clicked on that item.
   * @param item is the pointer to the executed listbox item.
   *
   * Note that you may not delete any QListBoxItem objects in slots
   * connected to this signal.
   */
  void executed( Q3ListBoxItem *item );

  /**
   * Emitted whenever the user executes an listbox item.
   *
   * That means depending on the KDE wide Single Click/Double Click
   * setting the user clicked or double clicked on that item.
   * @param item is the pointer to the executed listbox item.
   * @param pos is the position where the user has clicked
   *
   * Note that you may not delete any QListBoxItem objects in slots
   * connected to this signal.
   */
  void executed( Q3ListBoxItem *item, const QPoint &pos );

  /**
   * This signal gets emitted whenever the user double clicks into the
   * listbox.
   *
   * @param item The pointer to the clicked listbox item.
   * @param pos The position where the user has clicked.
   *
   * Note that you may not delete any QListBoxItem objects in slots
   * connected to this signal.
   *
   * This signal is more or less here for the sake of completeness.
   * You should normally not need to use this. In most cases it's better
   * to use executed() instead.
   */
  void doubleClicked( Q3ListBoxItem *item, const QPoint &pos );

protected Q_SLOTS:
  void slotOnItem( Q3ListBoxItem *item );
  void slotOnViewport();

  void slotSettingsChanged(int);

  /**
   * Auto selection happend.
   */
  void slotAutoSelect();

protected:
  void emitExecute( Q3ListBoxItem *item, const QPoint &pos );

  virtual void keyPressEvent(QKeyEvent *e);
  virtual void focusOutEvent( QFocusEvent *fe );
  virtual void leaveEvent( QEvent *e );
  virtual void contentsMousePressEvent( QMouseEvent *e );
  virtual void contentsMouseDoubleClickEvent ( QMouseEvent *e );

  bool m_bUseSingle;
  bool m_bChangeCursorOverItem;

  Q3ListBoxItem* m_pCurrentItem;

  QTimer* m_pAutoSelect;
  int m_autoSelectDelay;

private Q_SLOTS:
  void slotMouseButtonClicked( int btn, Q3ListBoxItem *item, const QPoint &pos );

private:
  class K3ListBoxPrivate;
  K3ListBoxPrivate* const d;
};

#endif
