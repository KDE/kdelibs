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

#ifndef KPLUGINSELECTOR_H
#define KPLUGINSELECTOR_H

#include <qwidget.h>
#include <qmap.h>
#include <qstring.h>

class KConfig;
class KInstance;
class QStrList;
class QListViewItem;
class KPreferencesModule;

/**
 * Information about a plugin.
 *
 * This holds all the information about a plugin there is. It's used for the
 * user to decide whether he want's to use this plugin or not.
 *
 * @author Matthias Kretz <kretz@kde.org>
 * @version $Id$
 * @since 3.2
 */
class KPluginInfo
{
	public:
		/**
		 * Read plugin info from @p filename.
		 *
		 * The file should be of the following form:
		 * <pre>
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
		   Type=playlist
		   Require=plugin1,plugin3
		   License=GPL
		 * </pre>
		 * The first three entries in the "Desktop Entry" group always need to be
		 * present. The Type is always "Plugin". Be aware, that there's a Type
		 * key in the "X-KDE Plugin Info" group, too. This is the type of the
		 * plugin @see type().
		 *
		 * In the "X-KDE Plugin Info" section you may add further entries which
		 * will be available using @ref property(). The Website,Type,Require
		 * keys are optional.
		 */
		KPluginInfo( const QString & filename );

		/**
		 * Create an empty hidden plugin.
		 * @internal
		 */
		KPluginInfo();

		//copy ctor
		KPluginInfo( const KPluginInfo & );
		//copy operator
		const KPluginInfo & operator=( const KPluginInfo & );

		virtual ~KPluginInfo();

		/**
		 * @return Whether the plugin should be hidden.
		 */
		bool isHidden() const;

		/**
		 * Set whether the plugin is currently loaded.
		 */
		void setPluginLoaded( bool loaded );

		/**
		 * @return Whether the plugin is currently loaded.
		 */
		bool pluginLoaded() const;

		/**
		 * @return The string associated with the @p key.
		 *
		 * @see operator[]
		 */
		const QString & property( const QString & key ) const { return m_propertymap[ key ]; }

		/**
		 * This is the same as @ref property(). It is provided for convenience.
		 *
		 * @return The string associated with the @p key.
		 *
		 * @see property()
		 */
		const QString & operator[]( const QString & key ) const { return property( key ); }

		/**
		 * @return The user visible name of the plugin.
		 */
		const QString & name() const { return m_propertymap[ "Name" ]; }

		/**
		 * @return A comment describing the plugin.
		 */
		const QString & comment() const { return m_propertymap[ "Comment" ]; }

		/**
		 * @return The file containing the information about the plugin.
		 */
		const QString & specfile() const;

		/**
		 * @return The author of this plugin.
		 */
		const QString & author() const { return m_propertymap[ "Author" ]; }

		/**
		 * @return The email address of the author.
		 */
		const QString & email() const { return m_propertymap[ "Email" ]; }
		
		/**
		 * @return The type of this plugin (e.g. playlist/skin).
		 */
		const QString & type() const { return m_propertymap[ "Type" ]; }

		/**
		 * @return The internal name of the plugin (for KParts Plugins this is
		 * the same name as set in the .rc file).
		 */
		const QString & pluginname() const { return m_propertymap[ "PluginName" ]; }

		/**
		 * @return The version of the plugin.
		 */
		const QString & version() const { return m_propertymap[ "Version" ]; }

		/**
		 * @return The website of the plugin/author.
		 */
		const QString & website() const { return m_propertymap[ "Website" ]; }
		
		/**
		 * @return The license of this plugin.
		 */
		const QString & license() const { return m_propertymap[ "License" ]; }

		/**
		 * @return A list of plugins required for this plugin to be enabled. Use
		 *         the pluginname in this list.
		 */
		const QStringList & requirements() const;

	private:
		QMap<QString,QString> m_propertymap;
		bool m_loaded;

		class KPluginInfoPrivate;
		KPluginInfoPrivate * d;
};

/**
 * This is a widget to configure what Plugins should be loaded. Normally
 * you'd put this widget into your applications configuration dialog.
 *
 * Since the user needs a way to know what a specific plugin does every plugin
 * sould install a desktop file containing a name, comment and type field. The
 * type is usefull for applications that can use different kinds of plugins like
 * a playlist, skin or visualization.
 *
 * The location of these desktop files is the
 * share/apps/<instancename>/<plugindir> directory. But if you need you may use
 * a different directory.
 *
 * @author Matthias Kretz <kretz@kde.org>
 * @version $Id$
 * @since 3.2
 */
class KPluginSelector : public QWidget
{
	friend class KPreferencesModule;

	Q_OBJECT
	public:
		/**
		 * Create a new Plugin Selector widget for KParts plugins.
		 *
		 * If you want to support different
		 * types of plugins use the following constructor.
		 * Using this constructor the Type field will be ignored.
		 *
		 * The information about the plugins will be loaded from the
		 * share/apps/<instancename>/kpartplugins directory.
		 *
		 * @param instance     The KInstance object of the plugin's parent.
		 * @param parent       The parent of the widget.
		 */
		KPluginSelector( KInstance * instance, QWidget * parent, const char * name = 0 );

		/**
		 * Create a new Plugin Selector widget for KParts plugins.
		 *
		 * The information about the plugins will be loaded from the
		 * share/apps/<instancename>/kpartplugins directory.
		 *
		 * @param types        A list of strings identifying the possible types
		 *                     of plugins that should be listed. These strings
		 *                     need to marked for translation with I18N_NOOP().
		 *                     Every type will be available in it's own tab.
		 * @param instance     The KInstance object of the plugin's parent.
		 * @param parent       The parent of the widget.
		 */
		KPluginSelector( const QStrList & types, KInstance * instance, QWidget * parent,
				const char * name = 0 );

		/**
		 * Create a new Plugin Selector widget for non-KParts plugins.
		 *
		 * If you want to support different
		 * types of plugins use the following constructor.
		 * Using this constructor the Type field will be ignored.
		 *
		 * @param plugininfos  A list of @ref KPluginInfo objects containing the
		 *                     necessary information for the plugins you want to
		 *                     add the selector's list.
		 */
		KPluginSelector( const QValueList<KPluginInfo> & plugininfos, QWidget * parent,
				const char * name = 0 );

		/**
		 * Create a new Plugin Selector widget for non-KParts plugins.
		 *
		 * @param plugininfos  A list of @ref KPluginInfo objects containing the
		 *                     necessary information for the plugins you want to
		 *                     add the selector's list.
		 * @param types        A list of strings identifying the possible types
		 *                     of plugins that should be listed. These strings
		 *                     need to marked for translation with I18N_NOOP().
		 *                     Every type will be available in it's own tab.
		 */
		KPluginSelector( const QStrList & types, const QValueList<KPluginInfo> & plugininfos,
				QWidget * parent, const char * name = 0 );

		virtual ~KPluginSelector();

		/**
		 * Save the current state (which one's are enabled/disabled) to a
		 * KConfig object.
		 *
		 * @param config  The KConfig object to save to. If you used one of the
		 *                KParts constructors you may ignore this. The
		 *                configuration will be saved in the "KParts Plugins"
		 *                group.
		 *
		 *                If you used a different constructor you have to pass a
		 *                KConfig object. The configuration will be saved in the
		 *                "Plugins" group.
		 */
		virtual void save( KConfig * config = 0 );

		/**
		 * @return whether the plugin is enabled in the ListView or not.
		 */
		bool pluginChecked( const QString & pluginname ) const;

	protected:
		/**
		 * Reimplement in your subclass if you have special needs: The standard
		 * implementation looks at the @ref KPluginInfo objects to find the
		 * needed information. But if, for some reason, your program doesn't
		 * work with that here's your chance to get it working.
		 *
		 * @return Whether the plugin is loaded.
		 */
		virtual bool pluginIsLoaded( const QString & pluginname ) const;

	private:
		/**
		 * @internal
		 * for the plugin's KPM to embedd itself in this widget
		 */
		void registerPlugin( const QString &, KPreferencesModule * );
		/**
		 * @internal
		 * for the plugin's KPM to remove itself from this widget
		 */
		void unregisterPlugin( const QString & );

	private slots:
		void executed( QListViewItem * );
		void updateConfigPage( const QString & pluginname, bool checked );

	private:
		QValueList<KPluginInfo> kpartsPluginInfos() const;
		void init( const QValueList<KPluginInfo> & plugininfos, const QStrList * types );
		enum InfoPageName { LoadedAndEnabled, LoadedAndDisabled, NotLoadedAndEnabled, NotLoadedAndDisabled };
		void infoPage( InfoPageName id );
		void checkDependencies( const KPluginInfo & );

		struct KPluginSelectorPrivate;
		KPluginSelectorPrivate * d;
};

// vim: sw=4 ts=4

#endif // KPLUGINSELECTOR_H
