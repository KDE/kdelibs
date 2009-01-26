/* This file is part of the KDE libraries
   Copyright (C) 1997 Mark Donohoe (donohoe@kde.org)
   Copyright (C) 1997, 1998 1998 Sven Radej (sven@lisa.exp.univie.ac.at)
   Copyright (C) 2007 Aron Boström (aron.bostrom@gmail.com)

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

#ifndef KSTATUSBAR_H
#define KSTATUSBAR_H

#include <kdeui_export.h>
#include <QtGui/QStatusBar>

class QObject;
class QEvent;
class KStatusBarPrivate;

/**
 *  @short %KDE statusbar widget
 *
 *  Display status messages.
 *
 *  You can insert text labels or custom widgets. Their geometry is managed
 *  internally. KStatusBar resizes itself, but positioning is left to
 *  KMainWindow (or to you, if you don't use KMainWindow ).
 *
 *  A special type of item is a message which is a temporary text-message
 *  displayed on top of other items in full-width. Messages are visible for
 *  specified time, or until you call the slot QStatusBar::clearMessage(). See
 *  QStatusBar::showMessage for details.
 *
 *  It is useful to connect the KActionCollection signals to the
 *  QStatusBar::showMessage slots.
 *
 *  KStatusBar inherits QStatusBar, you can freely use all QStatusBar
 *  methods.
 *
 *  Empty text items are not visible. They will become visible when you change
 *  (add) text.
 *
 *  @author Mark Donohoe <donohoe@kde.org> and Sven Radej <radej@kde.org>

 *  @see KActionCollection
 */
class KDEUI_EXPORT KStatusBar : public QStatusBar
{
  Q_OBJECT

public:
  /**
   *  Constructs a status bar. @p parent is usually KMainWindow.
   */
  explicit KStatusBar( QWidget* parent = 0 );

  /**
   *  Destructor.
   *
   *  Deletes all internal objects.
   */
  ~KStatusBar();

  /**
   *  Inserts a temporary text label into the status bar.
   *  Parameter @p stretch is passed to QStatusBar::addWidget .
   *
   *  @param text The label's text string.
   *  @param id id of item
   *  @param stretch stretch passed to QStatusBar::addWidget
   *
   *  @see QStatusbar::addWidget
   *
   */
  void insertItem(const QString& text, int id, int stretch=0 );

  /**
   *  Inserts a permanent text label into the status bar.
   *  Parameter @p stretch is passed to QStatusBar::addWidget .
   *
   *  @param text The label's text string.
   *  @param id id of item
   *  @param stretch stretch passed to QStatusBar::addPermanentWidget
   *
   *  @see QStatusbar::addPermanentWidget
   *
   */
  void insertPermanentItem(const QString& text, int id, int stretch=0 );

  /**
   *  Inserts a fixed width temporary text label into status bar. The width
   *  will be set according to @p text, but will remain fixed even if you
   *  change text.  You can change fixed width by calling setItemFixed.
   *
   *  @param text The label's text string
   *  @param id id of item
   */
  void insertFixedItem(const QString& text, int id);

  /**
   *  Inserts a fixed width permanent text label into status bar. The width
   *  will be set according to @p text, but will remain fixed even if you
   *  change text.  You can change fixed width by calling setItemFixed.
   *
   *  @param text The label's text string
   *  @param id id of item
   */
  void insertPermanentFixedItem(const QString& text, int id);

  /**
   *  Removes an item.
   *
   * @param id The item to remove.
   */
  void removeItem( int id );

  /**
   *  Returns true if an item with @p id exists already in KStatusBar,
   *  otherwise returns false.
   *
   *  @param id id of the item
   */
  bool hasItem( int id ) const;

  /**
   * The text of an item, if it exists.
   */
  QString itemText( int id ) const;

  /**
   * Changes the text in a status bar field.
   *
   * The item will be resized to fit the text. If you change text to be empty,
   * item will not be visible (until you add some text).
   *
   * @param text The label's text string
   * @param id The id of item.
   */
  void changeItem( const QString& text, int id );

  /**
   * Sets the alignment of item @p id. By default all fields are aligned
   * @p AlignHCenter | @p AlignVCenter. See QLabel::setAlignment for details.
   *
  */
  void setItemAlignment(int id, Qt::Alignment alignment);

  /**
   * Sets item @p id to have fixed width. This cannot be undone, but you can
   * always set new fixed width.
   *
   * @param id id of item
   * @param width fixed width in pixels. Default -1 is to adapt to text width.
   */
  void setItemFixed(int id, int width=-1);

Q_SIGNALS:

  /**
   *  Emitted when mouse is pressed over item @p id.
   *
   *  Connect to this signal if you want to respond to mouse press events.
   *
   */
  void pressed( int );

  /**
   *  Emitted when mouse is released over item @p id.
   *
   *  Connect to this signal if you want to respond to mouse release events (clicks).
   */
  void released( int );

protected:
  bool eventFilter(QObject* object, QEvent *event);

private:
  KStatusBarPrivate* const d;
};

#endif // KSTATUSBAR_H

