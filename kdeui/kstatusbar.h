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

#include <qtimer.h>
#include <qframe.h>
#include <qstring.h> 
#include <qlist.h>
#include <qlabel.h> 

/**
 *  Internal item class for use in KStatusBar
 *
 *  @short Internal class for use in KStatusBar
 */
class KStatusBarItem
{
public:

  KStatusBarItem( QWidget* w, int i, bool m = false )
  { item = w; id = i; mine = m; };
  ~KStatusBarItem()
  { if( mine ) delete item; };
  void setGeometry (int x, int y, int w, int h)
  { item->setGeometry( x, y, w, h ); };
  int width()
  { return item->width(); };
  int height()
  { return item->height(); };
  void show()
  { item->show(); };
  void hide()
  { item->hide(); };
  int ID()
  {return id;};
  QWidget *getItem()
  { return item; };
  
private:

  int id;
  QWidget *item;
  bool mine;
};


/**
 *  Internal label class for use in KStatusBar
 *
 *  @short Internal class for use in KStatusBar
 */
class KStatusBarLabel : public QLabel
{
  Q_OBJECT

public:

  KStatusBarLabel( const QString& text, int ID, QWidget* parent = 0L, const char *name=0L );
  ~KStatusBarLabel () {};

  int w;
  int h;
  
protected:

  void mousePressEvent( QMouseEvent* _event );
  void mouseReleaseEvent( QMouseEvent* _event );

private:

  int id;

signals:

  void Pressed( int );
  void Released( int );
};

/**
 *  KStatusBar is widget for displaying status messages. You can insert
 *  text labels or custom widgets. Managing of items is internal. KStatusBar
 *  resizes itself, but positioning is left to KTMainWindow (or to you, if
 *  you don't use KTMainWindow).
 *
 *  Special type of item is a message, a temporary text-message or custom
 *  widget which is displayed on top of other items in full-width. Messages
 *  are visible for specified time, or until you call slot clear().
 *
 *  STILL UNIMPLEMENTED:
 *
 *  It is also possible to replace one item by another, keeping the same
 *  size and position.
 *
 *  @short KDE statusbar widget
 *  @author Mark Donohoe (donohoe@kde.org) Maintained by Sven Radej <sven@lisa.exp.univie.ac.at>
 */
class KStatusBar : public QFrame
{
  Q_OBJECT
    
public:

  enum BarStatus{ Toggle, Show, Hide };
  enum Position{ Top, Left, Bottom, Right, Floating };
  enum InsertOrder{ LeftToRight, RightToLeft };

  /**
   *  Constructs KStatusBar object.
   */
  KStatusBar( QWidget* parent = 0L, const char* name = 0L );

  /**
   *  Destructor. Deletes all internal objects.
   */
  ~KStatusBar();

  /**
   *  Insert text label into the status bar. When inserting the item send the
   *  longest text you expect to go into the field as the first argument.
   *  The field is sized to accomodate this text. However, the last field
   *  inserted is always stretched to fit the window width.
   *
   *  @see #insertWidget
   */
  int insertItem( const QString& text, int ID );

  /**
   *  Insert custom widget into the status bar. The widget must have statusbar
   *  as parent. The size is the width of the widget. However, the last item
   *  inserted is always stretched to fit the window width.
   *
   *  @see #insertItem
   */
  int insertWidget( QWidget* _widget, int size, int id );

  /**
   *  Removes item id. If that was your custom widget it's hidden
   *  but not deleted.
   */
  void removeItem( int id );

  /**
   *  NOT YET IMPLEMENTED!
   *  Replaces item id with new label wich has text new_text. New
   *  label will have the same position and size as old. If old item was
   *  your custom widget it is not deleted. Note that it is rather pointless
   *  to replace one label by another; use @ref #changeItem for that.
   */
  void replaceItem( int _id, const QString& new_text );

  /**
   *  NOT YET IMPLEMENTED!
   *  Replaces item id with new widget new_widget. New widget will have the
   *  same position and size as old item. If old item was your custom widget
   *  it is not deleted.
   */
  void replaceItem( int _id, QWidget *new_widget );
  
  /**
   *  Change the text in a status bar field. The field is not resized !!!
   *  Usefull only for labels.
   */
  void changeItem( const QString& text, int id );

  /** 
   *  If order is KStatusBar::LeftToRight the fields are inserted from left
   *  to right, in particular the last field ist streched to the right
   *  border of the window. If order is KStatusBar::RightToLeft the fields
   *  are inserted from the right.
   */
  void setInsertOrder( InsertOrder order );

  /**
   *  Sets the alignment of a field. By default all fields are aligned left.
   *  Usefull only for labels.
   */
  void setAlignment( int id, int align );

  /**
   *  Sets the Height of the StatusBar. Default height is computed from
   *  default font height.
   */
  void setHeight( int );

  /**
   *  Sets the border width of the status bar seperators and frame.
   */
  void setBorderWidth( int );

  /**
   *  Enable disable status bar. You can get the same effect with show
   *  or hide, but if you do that the signal @ref #moved won't be emitted.
   */
  bool enable( BarStatus stat );

  /**
   *  Hides all items and displays temporary text message in whole statusbar.
   *  Message will be removed (and old items redisplayed) after time (in ms).
   *  If time is 0 (default) message will remain untill you call @ref #clear.
   *  You can remove the message by calling @ref #clear any time.
   */
  void message( const QString& text, int time = 0 );

  /**
   *  Hides all items and displays temporary custom widget in whole statusbar.
   *  Widget must have statusbar for it's parent.
   *  Widget will be removed (and old items redisplayed) after time (in ms).
   *  If time is 0 (default) widget will remain untill you call @ref #clear.
   *  You can remove the message by calling @ref #clear any time. Upon
   *  @ref #clear your widget will be hidden, not deleted.
   */
  void message( QWidget* wiiidget, int time = 0 );

  /**
   *  SizeHint. For now returns height() and width().
   */
  QSize sizeHint() const;

  KStatusBarItem *item( int id );
  
public slots:

  /**
   *  Clears the message (if any), and shows back old
   *  state. This method is slot, you can connect to it. Does nothing if
   *  @ref #message was not called before. Message is hidden, not deleted.
   *  If message was your custom widget you have to clean it up.
   */
  void clear();
   
signals:

  /**
   *  Emits when mouse is pressed over label id. Connect to this signal
   *  if you want to notice mouse press events. If you want to catch this
   *  signal for your custom widgets, they must not catch mouse press
   *  events.
   */
  void pressed( int );

  /**
   *  Emits when mouse is released over item id. Conect to
   *  this signal if you want to receive mouse click. If you want to catch
   *  this signal for your custom widgets, they must not catch mouse release
   *  events.
   */
  void released( int );

  /**
   *  Emits when toolbar changes its position (not implemented!), or when
   *  item is removed from toolbar. This is normaly connected to
   *  @ref KTMainWindow::updateRects.
   *  If you subclass @ref KTMainWindow and reimplement
   *  @ref KTMainWindow::resizeEvent or
   *  @ref KTMainWindow::updateRects, be sure to connect to
   *  this signal.
   */
  void moved();

protected:

  void drawContents( QPainter* );
  void resizeEvent( QResizeEvent* );
  void init();
  void updateRects( bool resize = FALSE );

protected slots:

  void slotPressed( int );
  void slotReleased( int );

private:

  QList<KStatusBarItem> items;
  InsertOrder insert_order;
  int fieldheight;
  int borderwidth;
  KStatusBarLabel *tempMessage;
  QWidget *tempWidget;
  QTimer *tmpTimer; //for future bugfix

  //fut:
  bool bull;
  int tni;
};

#endif // __KSTATUSBAR_H__

