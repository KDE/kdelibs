/*  This file is part of the KDE project
    Copyright (C) 2003 Matthias Kretz <kretz@kde.org>

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

#ifndef KPLUGININFO_H
#define KPLUGININFO_H

#include <qstring.h>
#include <qmap.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <kservice.h>

class KConfigGroup;

/**
 * Information about a plugin.
 *
 * This holds all the information about a plugin there is. It's used for the
 * user to decide whether he want's to use this plugin or not.
 *
 * @author Matthias Kretz <kretz@kde.org>
 * @since 3.2
 */
class KPluginInfo
{
	public:
		/**
		 * Read plugin info from @p filename.
		 *
		 * The file should be of the following form:
		 * \verbatim
		   [Desktop Entry]
		   Name=User Visible Name
		   Comment=Description of what the plugin does
		   Type=Plugin
		  
		   [X-KDE Plugin Info]
		   Author=Author's Name
		   Email=author@foo.bar
		   PluginName=internalname
		   Version=1.1
		   Website=http://www.plugin.org/
		   Category=playlist
		   Require=plugin1,plugin3
		   License=GPL
		   EnabledByDefault=true
		   \endverbatim
		 * The first three entries in the "Desktop Entry" group always need to be
		 * present. The Type is always "Plugin".
		 *
		 * In the "X-KDE Plugin Info" section you may add further entries which
		 * will be available using @ref property(). The Website,Category,Require
		 * keys are optional.
		 * For EnabledByDefault look at setPluginEnabledByDefault and/or
		 * pluginEnabledByDefault.
		 *
		 * @param filename  The filename of the .desktop file.
		 */
		KPluginInfo( const QString & filename );

		/**
		 * Create an empty hidden plugin.
		 * @internal
		 */
		KPluginInfo();

		/**
		 * Standard copy ctor. (deep copy)
		 */
		KPluginInfo( const KPluginInfo & );

		/**
		 * Make a deep copy.
		 */
		const KPluginInfo & operator=( const KPluginInfo & );

		virtual ~KPluginInfo();

		/**
		 * @return A list of KPluginInfo objects constructed from a list of
		 * KService objects. If you get a trader offer of the plugins you want
		 * to use you can just pass them to this function.
		 */
		static QValueList<KPluginInfo> fromServices( const KService::List & services );

		/**
		 * @return A list of KPluginInfo objects constructed from a list of
		 * filenames. If you make a lookup using, for example,
		 * KStandardDirs::findAllResources() you pass the list of files to this
		 * function.
		 */
		static QValueList<KPluginInfo> fromFiles( const QStringList & files );

		/**
		 * @return A list of KPluginInfo objects for the KParts plugins of an
		 * instance. You only need the name of the instance not a pointer to the
		 * KInstance object.
		 */
		static QValueList<KPluginInfo> fromKPartsInstanceName( const QString & );

		/**
		 * @return Whether the plugin should be hidden.
		 */
		bool isHidden() const;

		/**
		 * Set whether the plugin is currently loaded.
		 *
		 * You might need to reimplement this method for special needs.
		 *
		 * @see pluginEnabled()
		 */
		virtual void setPluginEnabled( bool enabled );

		/**
		 * @return Whether the plugin is currently loaded.
		 *
		 * You might need to reimplement this method for special needs.
		 *
		 * @see setPluginEnabled()
		 */
		virtual bool pluginEnabled() const;

		/**
		 * @return The default value whether the plugin is enabled or not.
		 * Defaults to the value set in the desktop file, or if that isn't set
		 * to false.
		 *
		 * @see setPluginEnabledByDefault( bool )
		 */
		bool pluginEnabledByDefault() const;

		/**
		 * @return The string associated with the @p key.
		 *
		 * @see operator[]
		 */
		const QString & property( const QString & key ) const
			{ return m_propertymap[ key ]; }

		/**
		 * This is the same as @ref property(). It is provided for convenience.
		 *
		 * @return The string associated with the @p key.
		 *
		 * @see property()
		 */
		const QString & operator[]( const QString & key ) const
			{ return property( key ); }

		/**
		 * @return The user visible name of the plugin.
		 */
		const QString & name() const
			{ return m_propertymap[ QString::fromLatin1("Name") ]; }

		/**
		 * @return A comment describing the plugin.
		 */
		const QString & comment() const 
			{ return m_propertymap[ QString::fromLatin1("Comment") ]; }

		/**
		 * @return The file containing the information about the plugin.
		 */
		const QString & specfile() const;

		/**
		 * @return The author of this plugin.
		 */
		const QString & author() const 
			{ return m_propertymap[ QString::fromLatin1("Author") ]; }

		/**
		 * @return The email address of the author.
		 */
		const QString & email() const 
			{ return m_propertymap[ QString::fromLatin1("Email") ]; }
		
		/**
		 * @return The category of this plugin (e.g. playlist/skin).
		 */
		const QString & category() const 
			{ return m_propertymap[ QString::fromLatin1("Category") ]; }

		/**
		 * @return The internal name of the plugin (for KParts Plugins this is
		 * the same name as set in the .rc file).
		 */
		const QString & pluginname() const 
			{ return m_propertymap[ QString::fromLatin1("PluginName") ]; }

		/**
		 * @return The version of the plugin.
		 */
		const QString & version() const 
			{ return m_propertymap[ QString::fromLatin1("Version") ]; }

		/**
		 * @return The website of the plugin/author.
		 */
		const QString & website() const 
			{ return m_propertymap[ QString::fromLatin1("Website") ]; }


		/**
		 * @return The license of this plugin.
		 */
		const QString & license() const 
			{ return m_propertymap[ QString::fromLatin1("License") ]; }

		/**
		 * @return A list of plugins required for this plugin to be enabled. Use
		 *         the pluginname in this list.
		 */
		const QStringList & requirements() const;

		/**
		 * @return A list of Service pointers if the plugin installs one or more
		 *         @ref KCModule
		 */
		const QValueList<KService::Ptr> & services() const;

		/**
		 * Save state of the plugin - enabled or not. This function is provided
		 * for reimplementation if you need to save somewhere else.
		 * @param config    The KConfigGroup holding the information whether
		 *                  plugin is enabled.
		 */
		virtual void save( KConfigGroup * config );

		/**
		 * Load the state of the plugin - enabled or not. This function is provided
		 * for reimplementation if you need to save somewhere else.
		 * @param config    The KConfigGroup holding the information whether
		 *                  plugin is enabled.
		 */
		virtual void load( KConfigGroup * config );

		/**
		 * Restore defaults (enabled or not).
		 */
		virtual void defaults();

	private:
		QMap<QString,QString> m_propertymap;
		bool m_loaded;

		class KPluginInfoPrivate;
		KPluginInfoPrivate * d;
};

// vim: sw=4 ts=4
#endif // KPLUGININFO_H
