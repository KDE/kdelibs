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

#include "pluginselectormodule.h"
#include <kpluginselector.h>

#include <qframe.h>
#include <qlayout.h>

#include <klocale.h>
#include <kinstance.h>
#include <kdialog.h>
#include <kdebug.h>

namespace KParts
{

struct PluginSelectorModule::PluginSelectorModulePrivate
{
	KInstance * instance;
	KPluginSelector * selector;
};

PluginSelectorModule::PluginSelectorModule( KInstance * instance, QObject * parent, const char * name )
	: KPreferencesModule( i18n( "Plugins" ), i18n( "Select and Configure Plugins" ),
			"input_devices_settings", parent, name )
	, d( new PluginSelectorModulePrivate )
{
	kdDebug( 1000 ) << k_funcinfo << endl;
	d->instance = instance;
	d->selector = 0;
}

PluginSelectorModule::~PluginSelectorModule()
{
	kdDebug( 1000 ) << k_funcinfo << endl;
	delete d;
}

void PluginSelectorModule::applyChanges()
{
	kdDebug( 1000 ) << k_funcinfo << endl;
	if( d->selector )
	{
		d->selector->save();
		emit configChanged();
	}
}

void PluginSelectorModule::reset()
{
	kdDebug( 1000 ) << k_funcinfo << endl;
	// TODO
}

void PluginSelectorModule::createPage( QFrame * page )
{
	kdDebug( 1000 ) << k_funcinfo << endl;
	QBoxLayout * layout = new QVBoxLayout( page, 0, KDialog::spacingHint() );
	layout->setAutoAdd( true );
	d->selector = new KPluginSelector( d->instance, page );
}

} //namespace

#include "pluginselectormodule.moc"

// vim: sw=4 ts=4
