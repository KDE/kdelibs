//////////////////////////////////////////////////////////////////////
//      $Id$ 
// File  : kpopupmenu.h
// Author: Toivo Pedaste
//
//////////////////////////////////////////////////////////////////////

#ifndef KACCELMENU_H
#define KACCELMENU_H

// Standard Headers
#include <stdio.h>

// Qt Headers
#include <qintdict.h> 
#include <qpopupmenu.h>

#include <kapp.h>
#include <kconfig.h>
#include <kaccel.h>

class KQuickHelpWindow;

//////////////////////////////////////////////////////////////////////////////
/**
 * This class simplifies the use of KAccel and KKeyDialog with menus.
 *
 * It also  provides for the changing of menu item shortcuts by
 * typing the key when the menu item is selected. 
 *
 * The exact method of setting shortcuts is the subject of some debate,
 * currently the ' key will put up a message indicating the next key
 * hit will change the shortcut.
 * The Delete key removes the accelerator associated with the menu item.
 *
 * Previously: the quote key was only required for keys that
 * are used for menu accellerators
 *
 * Usage:
 <pre>
 *  keys = new KAccel(this);
 *
 * filemenu = new KAccelMenu(keys);
 * filemenu->insItem(i18n("&Open"), "&Open", kp ,SLOT(fileOpen()), KAccel::Open);
 * filemenu->insItem(i18n("Open &URL"), "Open &URL", kp, SLOT(fileOpenUrl()),"CTRL+X");
 *
 * keys->readSettings();
 </pre>
 *
 * Calling the KKeyDialog
 <pre>
 * KKeyDialog::configureKeys( keys ); 
 </pre>
 *
 * On exit you need
 <pre>
 * keys->writeSettings();
 </pre>

 **/

class KAccelMenu : public QPopupMenu
{
  Q_OBJECT;

public:
  /**
   *   KAccelMenu constructor
   *   Arguments:
   *   @param k the Kacccel object associated with this menu
   **/
  KAccelMenu(KAccel *k, QWidget * parent=0, const char * name=0 );

  /**
   * Destructor
   **/
  ~KAccelMenu();

  /**
   *  Insert a menu item with accelerator
   *  Arguments:
   *
   *  @param pixmap image attached to menu item
   *  @param text  the interationalized version of the menu item text
   *  @param action the untranslated version of the menu item text
   *  @param receiver object with action slot
   *  @param member menu item action slot
   *  @param accel the accelerator in text format
   *
   **/

  int insItem (const QPixmap & pixmap, const char* text,
	       const char * action, const QObject * receiver,
	       const char * member, const char * accel = 0);

  int insItem (const char* text, const char * action,
	       const QObject * receiver,
	       const char * member, const char * accel = 0);

  /**
   *   Insert a menu item with a standard accelerator
   *   Arguments:
   *
   *   The same as above apart from
   *   @param accel code for a standard accelerator
   */
  int insItem ( const QPixmap & pixmap, const char * text,
		const char * action, const QObject * receiver,
		const char * member, KAccel::StdAccel accel );

  int insItem ( const char * text,  const char * action,
		const QObject * receiver,
		const char * member, KAccel::StdAccel accel );

  
protected:
  // Strip the & characters from the menu tedt
  char *stripAnd(const char *str);
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
  // information window
  KQuickHelpWindow *kq;
  // trap "hide" calls
  void hide();

protected slots:
  // called when selected menu item changes
  void highl(int);
  // called befaore a menu is displayed
  void aboutTS();
};

#endif 
