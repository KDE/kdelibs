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
 *  Display status messages.
 *
 *  You can insert
 *  text labels or custom widgets. Their geometry is Managed internally.
 *  KStatusBar
 *  resizes itself, but positioning is left to @ref KTMainWindow (or to you, if
 *  you don't use @ref KTMainWindow).
 *
 *  A special type of item is a message which is a temporary text-message
 *  or custom
 *  widget which is displayed on top of other items in full-width. Messages
 *  are visible for specified time, or until you call the slot @ref clear().
 *
 *  @sect STILL UNIMPLEMENTED:
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

  /**
   * Possible states of the status bar.
   **/
  enum BarStatus{ Toggle, Show, Hide };
  /**
   * Possible positions of the status bar.
   **/
  enum Position{ Top, Left, Bottom, Right, Floating };
  /**
   * Ways to inset widgets.
   **/
  enum InsertOrder{ LeftToRight, RightToLeft };

  /**
   *  Construct KStatusBar object.
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
   * 
   *  When inserting the item send the
   *  longest text you expect to go into the field as the first argument.
   *  The field is sized to accomodate this text. However, the last field
   *  inserted is always stretched to fit the window width.
   *
   *  @see insertWidget()
   */
  int insertItem( const QString& text, int ID );

  /**
   *  Insert custom widget into the status bar.
   *
   *  The widget must have this statusbar
   *  as it's parent. The size is the width of the widget.
   *  However, the last item
   *  inserted is always stretched to fit the window width.
   *
   *  @see insertItem()
   */
  int insertWidget( QWidget* _widget, int size, int id );

  /**
   *  Remove an item.
   *
   *  If @p id corresponds to your custom widget, then it's hidden
   *  but not deleted.
   *
   * @param id The item to remove.
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
   *  Change the text in a status bar field.
   *
   *  The field is not resized!
   *  This is useful only for labels.
   */
  void changeItem( const QString& text, int id );

  /** 
   * Change the order of widget insertion.
   *
   *  If order is KStatusBar::LeftToRight the fields are inserted from left
   *  to right, in particular the last field is streched to the right
   *  border of the window. If order is KStatusBar::RightToLeft the fields
   *  are inserted from the right.
   */
  void setInsertOrder( InsertOrder order );

  /**
   *  Sets the alignment of a field.
   *
   *  By default all fields are aligned left.
   *  Useful only for labels.
   *
   *  Valid values for align are:
   *
   *  @li AlignLeft Aligns to the left border. 
   *  @li AlignRight Aligns to the right border. 
   *  @li AlignHCenter Aligns horizontally centered. 
   *  @li AlignTop Aligns to the top border. 
   *  @li AlignBottom Aligns to the bottom border. 
   *  @li AlignVCenter Aligns vertically centered 
   *  @li AlignCenter Same as @p AlignHCenter | @p AlignVCenter
   */
  void setAlignment( int id, int align );

  /**
   *  Set the height of the StatusBar.
   *
   *  Default height is computed from
   *  default font height.
   */
  void setHeight( int );

  /**
   *  Set the border width of the status bar seperators and frame.
   */
  void setBorderWidth( int );

  /**
   *  Enable/ disable status bar.
   *
   *  You can get the same effect with @ref @QWidget::show()
   *  or @ref QWidget::hide(), but if you do that the signal
   *  @ref moved() won't be emitted.
   */
  bool enable( BarStatus stat );

  /**
   *  Hide all items and display temporary text message over entire statusbar.
   *
   *  The message will be removed (and old items redisplayed) after a time @p time has elapsed (in ms).
   *  If @p time is 0 (default) the message will remain until you call
   * @ref clear().
   *  You can remove the message by calling @ref clear() at any time.
   */
  void message( const QString& text, int time = 0 );

  /**
   *  Hide all items and display temporary custom widget over entire statusbar.
   *
   *  The widget must have this statusbar as its parent.
   *  The widget will be removed (and old items redisplayed) after time @p time has elapsed (in ms).
   *  If @p time is 0 (default) widget will remain until you call @ref clear().
   *  You can remove the message by calling @ref clear() any time. Upon
   *  calling @ref clear() your widget will be hidden, not deleted.
   */
  void message( QWidget* wiiidget, int time = 0 );

  /**
   * For now returns @ref QWidget::height() and @ref QWidget::width().
   */
  QSize sizeHint() const;

  /**
   * Retrieve a statusbar item.
   * @internal
   **/
  KStatusBarItem *item( int id );
  
public slots:

  /**
   *  Clear the message (if any), and return to regular state.
   *
   *  This method is a slot so you can connect to it. It does nothing if
   *  @ref message() was not called before. 
   *  If the message was your custom widget it will be hidden but not deleted, so you have to clean it up.
   */
  void clear();
   
signals:

  /**
   *  Emitted when mouse is pressed over statusbar item @p id.
   *
   *  Connect to this signal
   *  if you want to respond to mouse press events. If you want to catch this
   *  signal for your custom widgets, the widgets must not catch mouse press
   *  events.
   */
  void pressed( int );

  /**
   *  Emitted when mouse is released over statusbar item @p id.
   *
   * Connect to
   *  this signal if you want to respond to mouse clicks. If you want to catch
   *  this signal for your custom widgets, the widgets must not catch mouse release
   *  events.
   */
  void released( int );

  /**
   *  Emitted when statusbar changes its position (not implemented!), or when
   *  item is removed from statusbar.
   *
   *  This is normaly connected to
   *  @ref KTMainWindow::updateRects().
   *  If you subclass @ref KTMainWindow and reimplement
   *  @ref KTMainWindow::resizeEvent() or
   *  @ref KTMainWindow::updateRects(), be sure to connect to
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

