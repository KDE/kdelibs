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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __KSTATUSBAR_H__
#define __KSTATUSBAR_H__

#include <qstatusbar.h>
#include <qintdict.h>
#include <qlabel.h> 

class KStatusBar;

/**
 *  Internal label class for use in KStatusBar
 *  @internal
 */
class KStatusBarLabel : public QLabel
{
  Q_OBJECT

public:


  KStatusBarLabel( const QString& text, int _id, KStatusBar* parent = 0L, const char *name=0L );
  ~KStatusBarLabel () {};

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
 *  Display status messages.
 *
 *  You can insert text labels or custom widgets. Their geometry is managed
 *  internally. KStatusBar resizes itself, but positioning is left to
 *  @ref KMainWindow (or to you, if you don't use @ref KMainWindow ).
 *
 *  A special type of item is a message which is a temporary text-message
 *  displayed on top of other items in full-width. Messages are visible for
 *  specified time, or until you call the slot @ref QStatusBar::clear(). See
 *  @ref QStatusBar::message for details.
 *
 *  KStatusBar inherits @ref QStatusBar, you can freely use all @ref QStatusBar
 *  methods.
 *
 *  Empty text items are not visible. They will become visible when you change
 *  (add) text.
 *
 *  @short KDE statusbar widget
 *  @author Mark Donohoe (donohoe@kde.org) Maintained by Sven Radej <radej@kde.org>
 *  @version $Id$
 */
class KStatusBar : public QStatusBar
{
  Q_OBJECT
    
public:
  /**
   *  WARNING: This enum is only for backward compatibility and it may be removed.
   *  No method here uses it.
   */
  enum BarStatus{ Toggle, Show, Hide };
  
  /**
   *  Construct KStatusBar object. @p parent is usually @ref KMainWindow.
   */
  KStatusBar( QWidget* parent = 0L, const char* name = 0L );

  /**
   *  Destructor.
   *
   *  Deletes all internal objects.
   */
  ~KStatusBar();

  /**
   *  Insert text label into the status bar.
   *  Paremeters @p stretch and  @p permanent are passed to
   *  @ref QStatusBar::addWidget .
   *
   *  If @p permanent is true, then item will be placed on the far right of
   *  the statusbar and will never be hidden by @ref QStatusBar::message.
   *
   *  @param id id of item
   *  @param stretch stretch passed to @ref QStatusBar::addWidget
   *  @param permanent is item permanent or not (passed to @ref QStatusBar::addWidget )
   *
   *  @see QStatusbar::addWidget
   * 
   */
  void insertItem(const QString& text, int id, int stretch=0, bool permanent=false );

  /**
   *  Insert fixed width text label into status bar. The width will be set
   *  according to @p text, but will remain fixed even if you change text.
   *  You can change fixed width by calling @ref setItemFixed.
   *  @param id id ov item
   *  @param permanent permanent flag passed to QStatusBar::addWidget
   */
  inline void insertFixedItem(const QString& text, int id, bool permanent=false)
               { insertItem(text, id, 0, permanent); setItemFixed(id); }
  
  /**
   *  Remove an item.
   *
   * @param id The item to remove.
   */
  void removeItem( int id );

  /**
   * Change the text in a status bar field.
   *
   * The item will be resized to fit the text. If you change text to be empty,
   * item will not be visible (untill you add some text).
   *
   * @param id The id of item.
   */
  void changeItem( const QString& text, int id );

  /**
   * Sets the alignment of item @p id. By default all fields are aligned
   * @p AlignHCenter | @p AlignVCenter. See @ref QLabel::setAlignment for details.
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
  QIntDict<KStatusBarLabel> items;
};

#endif // __KSTATUSBAR_H__

