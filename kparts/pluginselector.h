/*  This file is part of the KDE project
    Copyright (C) 2002 Matthias Kretz <kretz@kde.org>

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

#ifndef PLUGINSELECTOR_H
#define PLUGINSELECTOR_H

#include <qwidget.h>

class KConfig;
class KInstance;
class QStrList;

namespace KParts
{

/**
 * This is a widget to configure what KPart Plugins should be loaded. Normally
 * you'd put this widget into your applications configuration dialog.
 *
 * Since the user needs a way to know what a specific plugin does every plugin
 * sould install a desktop file containing a name, comment and type field. The
 * type is usefull for applications that can use different kinds of plugins like
 * a playlist, skin or visualization.
 *
 * The location of these desktop files may be passed to the widget in the
 * constructor.
 *
 * Before you can reload the plugins you should call the @ref save method to
 * save the changes to a configuration object.
 *
 * @author Matthias Kretz <kretz@kde.org>
 */
class PluginSelector : public QWidget
{
	Q_OBJECT
	public:
		/**
		 * Create a new Plugin Selector widget. If you want to support different
		 * types of plugins use one of the following constructors. Using this
		 * constructor the Type field will be ignored.
		 * The information about the plugins will be loaded from the
		 * share/apps/<instancename>/kpartplugins directory.
		 */
		PluginSelector( KInstance * instance, QWidget * parent, const char * name = 0 );

		/**
		 * Create a new Plugin Selector widget. You need to pass a list of
		 * strings identifying the possible types of plugins that should be
		 * listed. Every type will be available in it's own tab.
		 */
		PluginSelector( const QStrList & types, KInstance * instance, QWidget * parent, const char * name = 0 );

		/**
		 * Create a new Plugin Selector widget. You need to pass a list of
		 * strings identifying the possible types of plugins that should be
		 * listed. Every type will be available in it's own tab.
		 * Load the information for the plugins from the desktop files. You need
		 * to pass a list of filenames.
		 */
		PluginSelector( const QStringList & desktopfilenames, const QStrList & types, KInstance * instance, QWidget * parent, const char * name = 0 );

		~PluginSelector();

		/**
		 * Save the Plugin configuration to the given KConfig object. If you
		 * don't pass a KConfig object the global configuration object will be
		 * used.
		 */
		void save( KConfig * config = 0 );

	private:
		void init( const QStringList & desktopfilenames, const QStrList * types );

		struct PluginSelectorPrivate;
		PluginSelectorPrivate * d;
};

};

// vim:sw=4:ts=4

#endif // PLUGINSELECTOR_H
