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
  Q_OBJECT;

 public:

  enum Actions { About = 1, Help = 2, Preferences = 4 };
  enum Flags { Stretch = 1, TopLevel = 2 };
  enum Position { Left = 0, Right, Top, Bottom };

  /**
   * Construct a KApplet widget just like any other widget.
   **/
  KPanelApplet( QWidget* parent = 0, const char* name = 0 );
  
  /**
   * Destructor
   **/
  virtual ~KPanelApplet() {}
  
  /**
   * Initialize the applet according to the passed command line
   * parameters. Call this function after instantiating the applet
   * widget in your main() function.
   *
   * Evalutate some command line arguments and get docked by the panel.
   **/
  void init( int& argc, char ** argv );
  
  /**
   * Set the applet flags.
   *
   * Supported flags:
   *
   * Stretch:
   * --------
   * Most applets might want to have a fixed size.
   * Applets like a taskbar however can set the stretch flag to get
   * all space available on the panel between the two surrounding applets
   * and/or the panel borders.
   *
   * TopLevel:
   * ---------
   * Some applets do not want to dock into the panel but map toplevel windows only.
   * A example is the kasbar applet.
   *
   * @see KPanelApplet::flags
   **/
  void setFlags(int f);

  /**
   * @returns int indicating the applet flags.
   **/
  bool flags() { return _flags; }

  /**
   * Set the RMB menu actions supported by the applet
   *
   * The panel supports 3 default actions besides 'Move' and 'Remove' in the
   * applets RMB menu:
   *
   * About - Launch about dialog.
   * Help - Show applet manual/help.
   * Preferences - Launch preferences dialog.
   *
   * Not all of these make sense/are supported by all applets, so you
   * can use this method to enable the actions supported by your applet.
   *
   * Example: setActions( About | Help | Preferences );
   *
   * @see KPanelApplet::actions
   */
  void setActions( int a);

  /**
   * @returns a int indicating the supported RMB menu actions.
   **/
  int actions() { return _actions; }
  
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
   * Is called when the applet is removed from the panel.
   *
   * The default implementation will simply call "kapp->quit()".
   * If the KPanelApplet widget is only a part of a bigger application
   * you might want to reimplement this to avoid the app shutdown and do
   * something else like simply deleting the applet widget instead.
   **/
  virtual void removedFromPanel();

  /**
   * Is called when 'About' is selcted from the applets RMB menu.
   *
   * There is no default implementation.
   * Reimplement this to launch a about dialog in for your applet.
   **/
  virtual void about() {}

  /**
   * Is called when 'Help' is selcted from the applets RMB menu.
   *
   * There is no default implementation.
   * Reimplement this to launch the applet-manual / help for your applet.
   **/
  virtual void help() {}

  /**
   * Is called when 'Preferences' is selcted from the applets RMB menu.
   *
   * There is no default implementation.
   * Reimplement this to launch a preferences dialog for your applet.
   **/
  virtual void preferences() {}

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
  int _actions, _flags;
  Orientation _orient;
  Position _pos;

};

#endif
