/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>

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
#ifndef KICONVIEW_H
#define KICONVIEW_H

#include <qcursor.h>
#include <qiconview.h>

/**
 * This Widget extends the functionality of QIconView to honor the system
 * wide settings for Single Click/Double Click mode, Auto Selection and
 * Change Cursor over Link.
 *
 * There is a new signal executed(). It gets connected to either
 * @ref QIconView::clicked() or @ref QIconView::doubleClicked() depending on the KDE
 * wide Single Click/Double Click settings. It is strongly recomended that
 * you use this signal instead of the above mentioned. This way you don´t
 * need to care about the current settings.
 * If you want to get informed when the user selects something connect to the
 * @ref QIconView::selectionChanged() signal.
 *
 * @short A variant of QIconView that honors KDE's system-wide settings.
 **/
class KIconView : public QIconView
{
  Q_OBJECT
  Q_ENUMS( Mode )
  Q_PROPERTY( Mode mode READ mode WRITE setMode )

public:
  KIconView( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );

  ~KIconView();

  /**
   * KIconView has two different operating modes. Execute mode is depending
   * on the configuration of single-click or double-click where the signal
   * executed() will be emitted upon click/double-click.
   * In Select mode, this signal will not be emitted.
   *
   * Default is Execute mode.
   */
  enum Mode { Execute, Select };

  /**
   * Sets the mode to Execute or Select.
   * @li In Execute mode, the signal executed()
   * will be emitted when the user clicks/double-clicks an item.
   * @li Select mode is
   * the normal QIconView mode.
   *
   * Default is Execute.
   */
  void setMode( Mode m );

  /**
   * @returns the current Mode, either Execute or Select.
   */
  Mode mode() const;

  /**
   * Returns the index of @p item or -1 if @p item doesn't exist
   * in this icon view.
   */
  //BCI: Remove this function. The bug(crash for nonexistent items) will hopefully be fixed then.
  int itemIndex( const QIconViewItem *item ) const;

signals:

  /**
   * This signal is emitted whenever the user executes an iconview item.
   * That means depending on the KDE wide Single Click/Double Click
   * setting the user clicked or double clicked on that item.
   * @param item is the pointer to the executed iconview item.
   *
   * Note that you may not delete any @ref QIconViewItem objects in slots
   * connected to this signal.
   */
  void executed( QIconViewItem *item );

  /**
   * This signal is emitted whenever the user executes an iconview item.
   * That means depending on the KDE wide Single Click/Double Click
   * setting the user clicked or double clicked on that item.
   * @param item is the pointer to the executed iconview item.
   * @param pos is the position where the user has clicked
   *
   * Note that you may not delete any @ref QIconViewItem objects in slots
   * connected to this signal.
   */
  void executed( QIconViewItem *item, const QPoint &pos );

  /**
   * This signal gets emitted whenever the user double clicks into the
   * iconview.
   * @param item is the pointer to the clicked iconview item.
   * @param pos is the position where the user has clicked, and
   *
   * Note that you may not delete any @ref QIconViewItem objects in slots
   * connected to this signal.
   *
   * This signal is more or less here for the sake of completeness.
   * You should normally not need to use this. In most cases it´s better
   * to use @ref #executed() instead.
   */
  void doubleClicked( QIconViewItem *item, const QPoint &pos );

protected slots:
  void slotOnItem( QIconViewItem *item );
  void slotOnViewport();

  void slotSettingsChanged(int);

  /**
   * Auto selection happend.
   */
  void slotAutoSelect();

protected:
  void emitExecute( QIconViewItem *item, const QPoint &pos );

  virtual void focusOutEvent( QFocusEvent *fe );
  virtual void leaveEvent( QEvent *e );
  virtual void contentsMousePressEvent( QMouseEvent *e );
  virtual void contentsMouseDoubleClickEvent ( QMouseEvent * e );
  virtual void contentsMouseReleaseEvent( QMouseEvent *e );
    
  QCursor oldCursor;
  bool m_bUseSingle;
  bool m_bChangeCursorOverItem;

  QIconViewItem* m_pCurrentItem;

  QTimer* m_pAutoSelect;
  int m_autoSelectDelay;

private slots:
  void slotMouseButtonClicked( int btn, QIconViewItem *item, const QPoint &pos );

private:
  class KIconViewPrivate;
  KIconViewPrivate *d;
};

#endif
