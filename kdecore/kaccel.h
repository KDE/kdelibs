/* This file is part of the KDE libraries
    Copyright (C) 2001,2002 Ellis Whitehead <ellis@kde.org>

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

#ifndef _KACCEL_H
#define _KACCEL_H

#include <qaccel.h>
#include <kshortcut.h>
#include <kstdaccel.h>

class QPopupMenu; // for obsolete insertItem() methods below
class QWidget;
class KAccelAction;
class KAccelActions;
class KConfigBase;

class KAccelPrivate;
/**
 * Handle shortcuts.
 *
 * Allow a user to configure shortcuts
 * through application configuration files or through the
 * @ref KKeyChooser GUI.
 *
 * A KAccel contains a list of @ref KAccelAction objects.
 *
 * For example, CTRL+Key_P could be a shortcut for printing a document. The key
 * codes are listed in qnamespace.h. "Print" could be the action name for printing.
 * The action name identifies the shortcut in configuration files and the
 * @ref KKeyChooser GUI.
 *
 * A KAccel object handles key events sent to its parent widget and to all
 * children of this parent widget.  The most recently created KAccel object
 * has precedence over any KAccel objects created before it.
 * When a shortcut pressed, KAccel calls the slot to which it has been
 * connected.
 *
 * Reconfiguration of a given shortcut can be prevented by specifying
 * that an accelerator item is not configurable when it is inserted. A special
 * group of non-configurable key bindings are known as the
 * standard accelerators.
 *
 * The standard accelerators appear repeatedly in applications for
 * standard document actions such as printing and saving. A convenience method is
 * available to insert and connect these accelerators which are configurable on
 * a desktop-wide basis.
 *
 * It is possible for a user to choose to have no key associated with
 * an action.
 *
 * The translated first argument for @ref insertItem() is used only
 * in the configuration dialog.
 *<pre>
 * KAccel* pAccel = new KAccel( this );
 *
 * // Insert an action "Scroll Up" which is associated with the "Up" key:
 * pAccel->insert( "Scroll Up", i18n("Scroll up"),
 *                       i18n("Scroll up the current document by one line."),
 *                       Qt::Key_Up, this, SLOT(slotScrollUp()) );
 * // Insert an standard acclerator action.
 * pAccel->insert( KStdAccel::Print, this, SLOT(slotPrint()) );
 *
 * // Update the shortcuts by read any user-defined settings from the
 * //  application's config file.
 * pAccel->readSettings();
 *</pre>
 *
 * @short Configurable shortcut support.
 * @version $Id$
 */

class KAccel : public QAccel
{
	Q_OBJECT
 public:
	KAccel( QWidget* pParent, const char* psName = 0 );
	KAccel( QWidget* watch, QObject* parent, const char* psName = 0 );
	virtual ~KAccel();

	KAccelActions& actions();
	const KAccelActions& actions() const;

	bool isEnabled();
	void setEnabled( bool bEnabled );

	bool getAutoUpdate();
	// return value of AutoUpdate flag before this call.
	bool setAutoUpdate( bool bAuto );

	/**
	 * Create an accelerator action.
	 *
	 * Usage:
	 *<pre>
	 * insert( "Do Something", i18n("Do Something"),
	 *   i18n("This action allows you to do something really great with this program to "
	 *        "the currently open document."),
	 *   ALT+Key_D, this, SLOT(slotDoSomething()) );
	 *</pre>
	 *
	 * @param sAction The internal name of the action.
	 * @param sLabel An i18n'ized short description of the action displayed when
	 *  using @ref KKeyChooser to reconfigure the shortcuts.
	 * @param sWhatsThis An extended description of the action.
	 * @param cutDef The default shortcut.
	 * @param pObjSlot Pointer to the slot object.
	 * @param psMethodSlot Pointer to the slot method.
	 * @param bConfigurable Allow the user to change this shortcut if set to 'true'.
	 * @param bEnabled The action will be activated by the shortcut if set to 'true'.
	 */
	KAccelAction* insert( const QString& sAction, const QString& sLabel, const QString& sWhatsThis,
	                 const KShortcut& cutDef,
	                 const QObject* pObjSlot, const char* psMethodSlot,
	                 bool bConfigurable = true, bool bEnabled = true );
	/**
	 * Same as first insert(), but with separate shortcuts defined for
	 * 3- and 4- modifier defaults.
	 */
	KAccelAction* insert( const QString& sAction, const QString& sLabel, const QString& sWhatsThis,
	                 const KShortcut& cutDef3, const KShortcut& cutDef4,
	                 const QObject* pObjSlot, const char* psMethodSlot,
	                 bool bConfigurable = true, bool bEnabled = true );
	/**
	 * This is an overloaded function provided for convenience.
	 * The advantage of this is when you want to use the same text for the name
	 * of the action as for the user-visible label.
	 *
	 * Usage: insert( I18N_NOOP("Do Something"), ALT+Key_D, this, SLOT(slotDoSomething()) );
	 *
	 * @param psAction The name AND label of the action.
	 * @param cutDef The default shortcut for this action.
	 */
	KAccelAction* insert( const char* psAction, const KShortcut& cutDef,
	                 const QObject* pObjSlot, const char* psMethodSlot,
	                 bool bConfigurable = true, bool bEnabled = true );
	/**
	 * Similar to the first insert() method, but with the action
	 * name, short description, help text, and default shortcuts all
	 * set according to one of the standard accelerators.
	 * @see KStdAccel.
	 */
	KAccelAction* insert( KStdAccel::StdAccel id,
	                 const QObject* pObjSlot, const char* psMethodSlot,
	                 bool bConfigurable = true, bool bEnabled = true );
	/**
	 * Use this to insert a label into the action list.  This will be
	 * displayed when the user configures shortcuts.
	 */
	KAccelAction* insert( const QString& sName, const QString& sLabel );

	bool remove( const QString& sAction );
	bool updateConnections();

	/**
	 * Return the shortcut associated with the action named by @p sAction.
	 */
	const KShortcut& shortcut( const QString& sAction ) const;

	/**
	 * Set the shortcut to be associated with the action named by @p sAction.
	 */
	bool setShortcut( const QString& sAction, const KShortcut& );
	/**
	 * Set the slot to be called when the shortcut of the action named
	 * by @p sAction is pressed.
	 */
	bool setSlot( const QString& sAction, const QObject* pObjSlot, const char* psMethodSlot );
	/**
	 * Enable or disable the action named by @p sAction.
	 */
	bool setEnabled( const QString& sAction, bool bEnabled );

	const QString& configGroup() const;
	void setConfigGroup( const QString& );

	/**
	 * Read all shortcuts from @p pConfig, or (if @p pConfig
	 * is zero) from the application's configuration file
	 * @ref KGlobal::config().
	 *
	 * The group in which the configuration is stored can be
	 * set with @ref setConfigGroup().
	 */
	bool readSettings( KConfigBase* pConfig = 0 );
	/**
	 * Write the current shortcuts to @p pConfig,
	 * or (if @p pConfig is zero) to the application's
	 * configuration file.
	 */
	bool writeSettings( KConfigBase* pConfig = 0 ) const;

	void emitKeycodeChanged();

 signals:
	void keycodeChanged();

#ifndef KDE_NO_COMPAT
 public:
	// Source compatibility to KDE 2.x
	bool insertItem( const QString& sLabel, const QString& sAction,
	                 const char* psKey,
	                 int nIDMenu = 0, QPopupMenu* pMenu = 0, bool bConfigurable = true );
	bool insertItem( const QString& sLabel, const QString& sAction,
	                 int key,
	                 int nIDMenu = 0, QPopupMenu* pMenu = 0, bool bConfigurable = true );
	bool insertStdItem( KStdAccel::StdAccel id, const QString& descr = QString::null );
	bool connectItem( const QString& sAction, const QObject* pObjSlot, const char* psMethodSlot, bool bActivate = true );
	bool connectItem( KStdAccel::StdAccel accel, const QObject* pObjSlot, const char* psMethodSlot )
		{ return insert( accel, pObjSlot, psMethodSlot ); }
	bool removeItem( const QString& sAction );
	bool setItemEnabled( const QString& sAction, bool bEnable );
	void changeMenuAccel( QPopupMenu *menu, int id, const QString& action );
	void changeMenuAccel( QPopupMenu *menu, int id, KStdAccel::StdAccel accel );
	static int stringToKey( const QString& );

	/**
	 * @deprecated.  Use shortcut().
	 * Retrieve the key code of the accelerator item with the action name
	 * @p action, or zero if either the action name cannot be
	 * found or the current key is set to no key.
	 */
	int currentKey( const QString& action ) const;

	/**
	 * @deprecated.  Use actions().actionPtr().
	 * Return the name of the accelerator item with the keycode @p key,
	 * or QString::null if the item cannot be found.
	 */
	QString findKey( int key ) const;
#endif // !KDE_NO_COMPAT

 protected:
        /// @internal
	virtual void virtual_hook( int id, void* data );
 private:
	class KAccelPrivate* d;
	friend class KAccelPrivate;
};

#endif // _KACCEL_H
