/*  This file is part of the KDE project
    Copyright (C) 2002-2003 Matthias Kretz <kretz@kde.org>

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
// $Id$

#ifndef KPREFERENCESMODULE_H
#define KPREFERENCESMODULE_H

#include <qobject.h>

class QFrame;

/**
 * Baseclass for preferences dialog pages
 *
 * @author Matthias Kretz <kretz@kde.org>
 */
class KPreferencesModule : public QObject
{
	friend class KPreferencesDialog;
	friend class KPluginSelector;

	Q_OBJECT

	public:
		/**
		 * Constructs a page for the preferences dialog.
		 *
		 * @param itemName  String used in the list or as tab item name.
		 * @param header    Header text use in the list modes. Ignored in @p Tabbed
		 *                  mode. If empty, the item text is used instead.
		 * @param pixmap    The name of the icon, without extension
		 * @param parent    The parent of this object - the module will
		 *                  automatically be deleted by Qt object model.
		 *                  The parent object is used to find the
		 *                  @ref KPreferencesDialog to be used for this module.
		 *                  Furhermore all modules belonging to the same
		 *                  parent will be in the same group in the preferences
		 *                  dialog.
		 */
		KPreferencesModule( const QString & itemName, const QString & header,
		                    const QString & pixmap, QObject * parent, const char * name = 0 );

		/**
		 * Constructor for plugins
		 *
		 * @param pluginname The internal name of the plugin (the same as used
		 *                   the desktop file).
		 * @param parent    The parent of this object - the module will
		 *                  automatically be deleted by Qt object model.
		 *                  The parent object is used to find the
		 *                  @ref KPluginSelectorModule this plugin config page
		 *                  belongs to.
		 *                  Furhermore all modules belonging to the same
		 *                  parent will be in the same group in the preferences
		 *                  dialog.
		 */
		KPreferencesModule( const QString & pluginname, QObject * parent, const char * name = 0 );

		virtual ~KPreferencesModule();

		/**
		 * All modules with the same parent will be shown in one group. Use this
		 * method to set the name for the group. If there are other modules in
		 * this group this method will overwrite the group name. (so the last
		 * module calling this method set's the name)
		 */
		void setGroupName( const QString & name );

	protected:
		/**
		 * Apply the changes that were done in this module.
		 */
		virtual void applyChanges() = 0;

		/**
		 * Resets the configuration changes of this module.
		 */
		virtual void reset() = 0;

		/**
		 * Creates the modules page. The default values are already set. So @ref reset() won't
		 * be called additionally.
		 *
		 * @param page      The parent for the dialog page. Put your widgets
		 *                  in there.
		 */
		virtual void createPage( QFrame * page ) = 0;

		const QString & itemName() const;
		const QString & header() const;
		const QString & pixmap() const;

		// called for plugin config pages by the dtor of KPluginSelectorModule
		void parentModuleDestroyed();

	private slots:
		void registerModule();

	private:
		struct KPreferencesModulePrivate;
		KPreferencesModulePrivate * d;
};

// vim: sw=4 ts=4
#endif // KPREFERENCESMODULE_H
