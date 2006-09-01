/* This file is part of the KDE libraries
    Copyright (C) 2001,2002 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#ifndef _KGLOBALACCEL_H_
#define _KGLOBALACCEL_H_

#include <QtCore/QObject>

#include <kdelibs_export.h>

class QWidget;
class KConfigBase;
class KAction;

/**
 * @short Configurable global shortcut support
 *
 * KGlobalAccel allows you to have global accelerators that are independent of
 * the focused window.  Unlike regular shortcuts, the application's window does not need focus
 * for them to be activated.
 *
 * @see KKeyChooser
 * @see KKeyDialog
 */
class KDEUI_EXPORT KGlobalAccel : public QObject
{
	friend class KGlobalAccelImpl;
	Q_OBJECT

public:
	/// Destructor
	virtual ~KGlobalAccel();

	/**
	 * Returns (and creates if necessary) the singleton instance
	 */
	static KGlobalAccel* self();

	/**
	 * Checks whether the accelerators are enabled.
	 * @return true if the KGlobalAccel is enabled
	 */
	bool isEnabled();

	/**
	 * Enables or disables the KGlobalAccel
	 * @param enabled true if the KGlobalAccel should be enabled, false if it
	 *  should be disabled.
	 */
	void setEnabled( bool enabled );

	/**
	 * Check to see if \a action has global accelerators or not, and grab or release them
	 * as appropriate.
	 *
	 * You should not need to call this function yourself, it is called as needed by
	 * KAction::setGlobalShortcut()
	 */
	void checkAction(KAction* action);

	/**
	 * Returns the configuration group that is used to save the accelerators.
	 * @return the configuration group
	 * @see KConfig
	 */
	const QString& configGroup() const;

	/**
	 * Sets the configuration group that is used to save the accelerators.
	 * @param cg the configuration group
	 * @see KConfig
	 */
	void setConfigGroup( const QString &cg );

	/**
	 * Read all shortcuts from @p config, or (if @p config
	 * is zero) from the application's configuration file
	 * KGlobal::config().
	 *
	 * @param config the configuration file to read from, or null for the application
	 *                 configuration file
	 * @return true if successful, otherwise false
	 */
	bool readSettings( KConfigBase* config = 0 );

	/**
	 * Write the current global shortcuts to @p config,
	 * or (if @p config is zero) to the application's
	 * configuration file.  Alternatively, if global is true, write
	 * to kdeglobals.
	 *
	 * @param config the configuration file to read from, or null for the application
	 *                 configuration file
    * \param writeDefaults set to true to write settings which are already at defaults.
	 * @param oneAction pass an action here if you only want its settings to be saved
	 *                  (eg. if you know this action is the only one which has changed).
	 *
	 * @return true if successful, otherwise false
	 */
	bool writeSettings( KConfigBase* config = 0L, bool writeDefaults = false, KAction* oneAction = 0L) const;
	
	/**
	 * Return a list of all KActions which currently have a valid global shortcut assigned.
	 */
	const QList<KAction*> actionsWithGlobalShortcut() const;

private Q_SLOTS:
	/**
	 * Listens to action change() events and respond when the action is enabled
	 * or disabled.
	 */
	void actionChanged();
	
private:
	/// The money slot - the global \a key was pressed
	/// Returns whether it was consumed
	bool keyPressed(int key);

	/// Something necessitates a repeat grabKey() for each key
	void regrabKeys();

	void enableImpl(bool enable);

	/// Creates a new KGlobalAccel object
	KGlobalAccel();

	void grabKey(int key, bool grab, KAction* action);

	static KGlobalAccel* s_instance;
	class KGlobalAccelData* const d;
	class KGlobalAccelImpl* const i;
};

#endif // _KGLOBALACCEL_H_
