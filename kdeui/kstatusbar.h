/* This file is part of the KDE libraries
   Copyright (C) 1997 Mark Donohoe (donohoe@kde.org)
   Copyright (C) 1997, 1998 1998 Sven Radej (sven@lisa.exp.univie.ac.at)

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

#include <qstatusbar.h>
#include <QHash>
#include <qlabel.h>
#include <kdelibs_export.h>

class KStatusBar;

/**
 *  Internal label class for use in KStatusBar
 *  @internal
 */
class KDEUI_EXPORT KStatusBarLabel : public QLabel
{
  Q_OBJECT

public:


  KStatusBarLabel( const QString& text, int _id, KStatusBar* parent = 0 );
  ~KStatusBarLabel () {}

protected:

  void mousePressEvent (QMouseEvent* _event);
  void mouseReleaseEvent (QMouseEvent* _event);

private:

  int id;

signals:

  void itemPressed (int id);
  void itemReleased (int id);
};

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
 *  specified time, or until you call the slot QStatusBar::clear(). See
 *  QStatusBar::message for details.
 *
 *  It is useful to connect the KActionCollection signals to the
 *  QStatusBar::message slots.
 *
 *  KStatusBar inherits QStatusBar, you can freely use all QStatusBar
 *  methods.
 *
 *  Empty text items are not visible. They will become visible when you change
 *  (add) text.
 *
 *  @author Mark Donohoe (donohoe@kde.org) Maintained by Sven Radej <radej@kde.org>

 *  @see KActionCollection
 */
class KDEUI_EXPORT KStatusBar : public QStatusBar
{
  Q_OBJECT

public:
  /**
   *  Constructs a status bar. @p parent is usually KMainWindow.
   */
  KStatusBar( QWidget* parent = 0 );

  /**
   *  Destructor.
   *
   *  Deletes all internal objects.
   */
  ~KStatusBar();

  /**
   *  Inserts a text label into the status bar.
   *  Parameters @p stretch and  @p permanent are passed to
   * QStatusBar::addWidget .
   *
   *  If @p permanent is true, then item will be placed on the far right of
   *  the statusbar and will never be hidden by QStatusBar::message.
   *
   *  @param text The label's text string.
   *  @param id id of item
   *  @param stretch stretch passed to QStatusBar::addWidget
   *  @param permanent is item permanent or not (passed to QStatusBar::addWidget )
   *
   *  @see QStatusbar::addWidget
   *
   */
  void insertItem(const QString& text, int id, int stretch=0, bool permanent=false );

  /**
   *  Inserts a fixed width text label into status bar. The width will be set
   *  according to @p text, but will remain fixed even if you change text.
   *  You can change fixed width by calling setItemFixed.
   *
   *  @param text The label's text string
   *  @param id id of item
   *  @param permanent permanent flag passed to QStatusBar::addWidget
   */
  inline void insertFixedItem(const QString& text, int id, bool permanent=false)
               { insertItem(text, id, 0, permanent); setItemFixed(id); }

  /**
   *  Removes an item.
   *
   * @param id The item to remove.
   */
  void removeItem( int id );

  /**
   *  @since 3.2
   *
   *  Returns true if an item with @p id exists already in KStatusBar,
   *  otherwise returns false.
   *
   *  @param id id of the item
   */
  bool hasItem( int id ) const;

  /**
   * Changes the text in a status bar field.
   *
   * The item will be resized to fit the text. If you change text to be empty,
   * item will not be visible (untill you add some text).
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
  void setItemAlignment(int id, int align);

  /**
   * Sets item @p id to have fixed width. This cannot be undone, but you can
   * always set new fixed width.
   *
   * @param id id of item
   * @param width fixed width in pixels. Default -1 is to adapt to text width.
   */
  void setItemFixed(int id, int width=-1);

signals:

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

private:
  QHash<int, KStatusBarLabel*> items;
  class KStatusBarPrivate* d;
};

#endif // KSTATUSBAR_H

