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

#ifndef KPARTS_PLUGINSELECTORMODULE_H
#define KPARTS_PLUGINSELECTORMODULE_H

#include "kpreferencesmodule.h"

class KInstance;

namespace KParts
{

/**
 * @ref KPreferencesModule for KParts plugins
 *
 * @author Matthias Kretz <kretz@kde.org>
 * @version $Id$
 * @since 3.2
 */
class PluginSelectorModule : public KPreferencesModule
{
	Q_OBJECT
	public:
		PluginSelectorModule( KInstance *, QObject * parent = 0, const char * name = 0 );
		~PluginSelectorModule();

	signals:
		void configChanged();

	protected:
		void applyChanges();
		void reset();
		void createPage( QFrame * page );

	//private slots:
		//void pageDestroyed();

	private:
		struct PluginSelectorModulePrivate;
		PluginSelectorModulePrivate * d;
};
} //namespace

#endif // KPARTS_PLUGINSELECTORMODULE_H

// vim: sw=4 ts=4
