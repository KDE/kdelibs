/* This file is part of the KDE libraries
    Copyright (C) 1997 Sven Radej (sven.radej@iname.com)

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
#ifndef _KSTATUSBAR_H
#define _KSTATUSBAR_H

#include <qframe.h>
#include <qstring.h> 
#include <qlist.h>
#include <qlabel.h> 

/**
       * Internal class for use in KStatusBar
       * @short Internal class for use in KStatusBar
       */
class KStatusBarItem : public QLabel {
  Q_OBJECT

public:

  KStatusBarItem( const char *text, int ID, QWidget *parent=NULL,
                  const char *name=NULL );

  /**
	* Returns id of item. Internal
	*/
  int ID();

  /**
	* Store width and height of the KStatusBarItem
	*/
  int w,h;
  
protected:

  /**
	* Internal. emits signal @ref pressed
	*/
  void mousePressEvent(QMouseEvent *);

  /**
	* Internal. emits signal @ref released
	*/
  void mouseReleaseEvent(QMouseEvent *);

private:

  /**
	* Stores id
	*/
  int id;

  signals:

  /**
	* Internal. Emits when mouse press occures
	*/
  void Pressed(int);

  /**
	* Internal. Emits when mouse press occures
	*/
  void Released(int);

};
/**
           * @short KDE statusbar with signals pressed and released
           */
class KStatusBar : public QFrame {
  Q_OBJECT
    
public:
  enum BarStatus{ Toggle, Show, Hide };
  enum Position{Top, Left, Bottom, Right, Floating};
  enum InsertOrder{LeftToRight, RightToLeft};

  KStatusBar(QWidget *parent = NULL, const char *name = NULL );
  ~KStatusBar();

  /**
	* Enable disable status bar
	*/
  bool enable( BarStatus stat );
	
  /**
	* Insert field into the status bar. When inserting the item send the
	* longest text you expect to go into the field as the first argument.
	* The field is sized to accomodate this text. However, the last field
	* inserted is always stretched to fit the window width.
	* @short Insert field into the status bar.
	*/
  int insertItem( const char *text, int ID );
	
  /**
	* Change the text in a status bar field. The field is not resized !!!
	* @short Change the text in a status bar field.
	*/
  void changeItem( const char *text, int id );

  /** 
	* If order is KStatusBar::LeftToRight the field are inserted from left
	* to right, in particular the last field ist streched to the right
	* border of the app. If order is KStatusBar::RightToLeft the fields
	* are inserted from the right.
	* @short Sets inserting order
	*/
  void setInsertOrder(InsertOrder order);

  /**
	* Sets the alignment of a field. By default all fields are aligned left.
	* @short Sets the alignment of a field.
	*/
  void setAlignment(int id, int align);

  /**
	* Sets the Height of the StatusBar
	* @short Sets the Height of the StatusBar
	*/
  void setHeight(int);

  /**
	* Sets the border width of the status bar seperators and frame.
	* @short Sets width of the seperators and frame.
	*/
  void setBorderWidth(int);

private:
  QList <KStatusBarItem> labels;
  InsertOrder insert_order;
  int fieldheight, borderwidth;

protected:
  void drawContents ( QPainter * );
  void resizeEvent( QResizeEvent* );
  void init();
  void updateRects( bool resize = FALSE );

 protected slots:
 void slotPressed(int);
  void slotReleased(int);

  signals:
  /**
	* Emits when mouse is pressed over item id. Connect to this signal
	* if you want to notice mouse press events
	*/
  void pressed(int);

  /**
	* Emits when mouse is released over item id. Conect to
	* this signal if you want to receive mouse click
	*/
  void released(int);
};


#endif
