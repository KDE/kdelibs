/* This file is part of the KDE libraries
   Copyright (C) 1999 Toivo Pedaste <toivo@ucs.uwa.edu.au>

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
#ifndef KACCELMENU_H
#define KACCELMENU_H

// Qt Headers
#include <qintdict.h>
#include <kpopupmenu.h>
#include <kstdaccel.h>

class KAccelMenuPrivate;
//////////////////////////////////////////////////////////////////////////////
/**
 * Simplify the use of @ref KAccel and @ref KKeyDialog with menus.
 *
 * It also provides for the changing of menu item shortcuts by
 * typing the key when the menu item is selected.
 *
 * The exact method of setting shortcuts is the subject of some debate,
 * currently the ' key will put up a message indicating the next key
 * hit will change the shortcut.
 * The Delete key removes the accelerator associated with the menu item.
 *
 * Previously: The quote key was required only for keys that
 * are used for menu accelerators.
 *
 * @sect Usage:
 * <pre>
 * keys = new KAccel(this);
 *
 * filemenu = new KAccelMenu(keys);
 * filemenu->insItem(i18n("&Open"), "&Open", kp ,SLOT(fileOpen()),
 *                   KStdAccel::Open);
 * filemenu->insItem(i18n("Open &URL"), "Open &URL", kp, SLOT(fileOpenUrl()),
 *                   "CTRL+X");
 *
 * keys->readSettings();
 * </pre>
 *
 * Calling the KKeyDialog:
 * <pre>
 * KKeyDialog::configureKeys( keys );
 * </pre>
 *
 * On exit you need:
 * <pre>
 * keys->writeSettings();
 * </pre>
 *
 **/

class KAccelMenu : public KPopupMenu
{
  Q_OBJECT

public:
  /**
   *   KAccelMenu constructor.
   *
   *   @param k The @ref KAccel object associated with this menu
   **/
  KAccelMenu(KAccel *k, QWidget * parent=0, const char * name=0 );

  /**
   * Destructor.
   **/
  ~KAccelMenu();

  /**
   *  Insert a menu item with accelerator.
   *
   *  @param pixmap Image attached to menu item.
   *  @param text  The interationalized version of the menu item text.
   *  @param action The untranslated version of the menu item text.
   *  @param receiver Object with action slot.
   *  @param member Menu item action slot.
   *  @param accel The accelerator in text format.
   *
   * @return Id of the menu item.
   **/

  int insItem (const QPixmap & pixmap, const char* text,
	       const char * action, const QObject * receiver,
	       const char * member, const char * accel = 0);

  /**
   *  Insert a menu item with accelerator.
   *
   *  @param text  The interationalized version of the menu item text.
   *  @param action The untranslated version of the menu item text.
   *  @param receiver Object with action slot.
   *  @param member Menu item action slot.
   *  @param accel The accelerator in text format.
   *
   * @return Id of the menu item.
   **/
  int insItem (const char* text, const char * action,
	       const QObject * receiver,
	       const char * member, const char * accel = 0);

  /**
   *   Insert a menu item with a standard accelerator
   *
   *  @param pixmap Image attached to menu item.
   *  @param text  The interationalized version of the menu item text.
   *  @param action The untranslated version of the menu item text.
   *  @param receiver Object with action slot.
   *  @param member Menu item action slot.
   *  @param accel Code for a standard accelerator
   *
   *  @return Id of the menu item.
   **/
  int insItem ( const QPixmap & pixmap, const char * text,
		const char * action, const QObject * receiver,
 	    const char * member, KStdAccel::StdAccel accel );

  /**
   *   Insert a menu item with a standard accelerator
   *
   *  @param text  The interationalized version of the menu item text.
   *  @param action The untranslated version of the menu item text.
   *  @param receiver Object with action slot.
   *  @param member Menu item action slot.
   *  @param accel code for a standard accelerator
   *
   *  @return Id of the menu item.
   **/
  int insItem ( const char * text,  const char * action,
		const QObject * receiver,
		const char * member, KStdAccel::StdAccel accel );


protected:
  // Strip the & characters from the menu tedt
  char *stripAnd(const char *str);
  // Insert the key string in the menu entry.
  void changeMenuAccel (int id, const char * action);
  // Handles key presses
  void keyPressEvent ( QKeyEvent * );
  // Popup information window
  void popMsg ();

  // the KAccel object associated with this menu
  KAccel *keys;
  // id of currently selected menu item
  int cid;
  // maps untranslated text to item id
  QIntDict<char> actions;
  // if the quote character has been typed
  bool quote;

protected slots:
  // called when selected menu item changes
  void highl(int);
  // called befaore a menu is displayed
  void aboutTS();

private:
  KAccelMenuPrivate *d;
};

#endif 
