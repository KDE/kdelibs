/*
   This file is part of the KDE libraries

   Copyright (c) 2000 Matthias Elter   <elter@kde.org>
   base on code written 1999 by Matthias Ettrich <ettrich@kde.org>
                 
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

#ifndef __kpanelapplet_h__
#define __kpanelapplet_h__

#include <qwidget.h>
#include <dcopobject.h>

class KPanelAppletData;

/**
* KDE Panel Applet class
*
* This class implements panel applets.
*
* @author Matthias Elter <elter@kde.org>
* @short KDE Panel Applet class
*/
class KPanelApplet : public QWidget, DCOPObject
{
  enum Position { Left = 0, Right, Top, Bottom };

  Q_OBJECT;

 public:
  /**
   * Construct a KApplet widget just like any other widget.
   **/
  KPanelApplet( QWidget* parent = 0, const char* name = 0 );
  
  /**
   * Destructor
   **/
  virtual ~KPanelApplet();
  
  /**
   * Initialize the applet according to the passed command line
   * parameters. Call this function after instantiating the applet
   * widget in your main() function.
   *
   * Evalutate some command line arguments and get docked by the panel.
   **/
  void init( int& argc, char ** argv );
  
  /**
   * Set the applet to be fixed size or stretchable.
   *
   * Most applets might want to have a fixed size.
   * Applets like a taskbar however can set the stretch flag to get
   * all space available on the panel between the two surrounding applets
   * and/or the panel borders.
   **/
  void setStretch(bool s);

  /**
   * @returns Bool indicating whether the applet is fixed size or stretchable.
   **/
  bool stretch() { return _stretch; }
  
  /**
   * @returns A suggested width for a given height.
   *
   * Applets should reimplement this function.
   *
   * Depending on the panel orientation the height (horizontal panel) or the
   * width (vertical panel) of the applets is fixed.
   * The exact values of the fixed size component depend on the panel size.
   *
   * On a horizontal panel the applet height is fixed, the panel will
   * call widthForHeight(int height) with height == 'the fixed applet height'
   * when layouting the applets.
   *
   * The applet can now choose the other size component (width)
   * based on the given height.
   *
   * The width you return is guaranteed.
   **/
  virtual int widthForHeight(int height);

  /**
   * @returns A suggested height for a given width.
   *
   * Applets should reimplement this function.
   *
   * Depending on the panel orientation the height (horizontal panel) or the
   * width (vertical panel) of the applets is fixed.
   * The exact values of the fixed size component depend on the panel size.
   *
   * On a vertical panel the applet width is fixed, the panel will
   * call heightForWidth(int width) with width == 'the fixed applet width'
   * when layouting the applets.
   *
   * The applet can now choose the other size component (height)
   * based on the given width.
   *
   * The height you return is guaranteed.
   **/
  virtual int heightForWidth(int width);

  /**
   * Notify the panel about a new applet layout.
   *
   * Call this to make the panel relayout all applets, when
   * you want to change your width (horizontal panel) or
   * height (vertical panel).
   *
   * The panel is going to relayout all applets based on their
   * widthForHeight(int height) (horizontal panel) or 
   * heightForWidth(int width) (vertical panel).
   *
   * Please note that the panel may change the applets location
   * if the new widthForHeight(int height) (horizontal panel) or
   * heightForWidth(int width) (vertical panel) does not fit into the
   * current panel layout.
   **/
  void updateLayout();

  /**
   * @returns The panel orientation. You may need this if you want to popup menus at the
   * right position.
   **/
  Orientation orientation() const { return _orient; }

  /**
   * @returns The panel position. You may need this if you want to popup menus at the
   * right position.
   **/
  Position position() const { return _pos; }

  // dcop internal
  bool process(const QCString &fun, const QByteArray &data,
               QCString& replyType, QByteArray &replyData);
 private:
  KPanelAppletData *d;
  bool _stretch;
  Orientation _orient;
  Position _pos;

};

#endif
