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

#include "kpreferencesmodule.h"
#include "kpreferencesdialog.h"
#include "kpluginselector.h"

#include <qtimer.h>

#include <kdebug.h>

struct KPreferencesModule::KPreferencesModulePrivate
{
	KPreferencesModulePrivate()
		: dialog( 0 )
		, pluginmodule( 0 )
	{}

	QString itemName;
	QString header;
	QString pixmap;
	QString pluginname;
	KPreferencesDialog * dialog;
	KPluginSelector * pluginmodule;
};

KPreferencesModule::KPreferencesModule( const QString & itemName, const QString & header,
                                        const QString & pixmap, QObject * parent, const char * name )
	: QObject( parent, name )
	, d( new KPreferencesModulePrivate )
{
	d->itemName = itemName;
	d->header = header;
	d->pixmap = pixmap;
	
#ifndef NDEBUG
	if( ! parent )
		kdFatal( 297 ) << "You need to pass a parent QObject to KPreferencesModule" << endl;
#endif

	QObject * p = parent;
	QObject * child = p->child( 0, "KPreferencesDialog", false ); // not recursive
	while( child == 0 && p->parent() )
	{
		p = p->parent();
		child = p->child( 0, "KPreferencesDialog", false ); // not recursive
	}
	if( ! child )
	{
		// create one in our parent
		kdWarning( 297 ) << "create new KPreferencesDialog - none found" << endl;
		d->dialog = new KPreferencesDialog( parent, "automatically created KPreferencesDialog" );
		d->dialog->createMenuEntry();
	}
	else
	{
		d->dialog = static_cast<KPreferencesDialog*>( child );
	}

	QTimer::singleShot( 0, this, SLOT( registerModule() ) );
	// XXX: handle the case that the dialog gets deleted before the module
	// i.e.: take care that the dangling d->dialog pointer won't do any harm
}

KPreferencesModule::KPreferencesModule( const QString & pluginname, QObject * parent, const char * name )
	: QObject( parent, name )
	, d( new KPreferencesModulePrivate )
{
	d->pluginname = pluginname;

#ifndef NDEBUG
	if( ! parent )
		kdFatal( 297 ) << "You need to pass a parent QObject to KPreferencesModule" << endl;
#endif

	// search for the KPluginSelector:
	// first we look for a list of KPreferencesModules then, we... AAAHHH @#&%@%*!#
	// XXX/FIXME/TODO
	// The KPluginSelector is a child of a page in the KDialogBase of
	// KPreferencesDialog - there's no way I can find it there.
	if( parent->parent() )
	{
		QObject * p = parent->parent();
		QObject * child = p->child( 0, "KPluginSelector", false ); // not recursive
		while( child == 0 && p->parent() )
		{
			p = p->parent();
			child = p->child( 0, "KPluginSelector", false ); // not recursive
		}
		if( ! child )
			kdWarning( 297 ) << "no KPluginSelector found" << endl;
		else
		{
			d->pluginmodule = static_cast<KPluginSelector *>( child );
			registerModule();
		}
	}
}

KPreferencesModule::~KPreferencesModule()
{
	if( d->pluginmodule )
		d->pluginmodule->unregisterPlugin( d->pluginname );
	delete d;
}

void KPreferencesModule::setGroupName( const QString & name )
{
	if( d->dialog )
		d->dialog->setGroupName( this, name );
}

const QString & KPreferencesModule::itemName() const
{
	return d->itemName;
}

const QString & KPreferencesModule::header() const
{
	return d->header;
}

const QString & KPreferencesModule::pixmap() const
{
	return d->pixmap;
}

void KPreferencesModule::parentModuleDestroyed()
{
	d->pluginmodule = 0;
}

void KPreferencesModule::registerModule()
{
	if( d->dialog )
		d->dialog->registerModule( this );
	else if( d->pluginmodule )
		d->pluginmodule->registerPlugin( d->pluginname, this );
}

#include "kpreferencesmodule.moc"

// vim: sw=4 ts=4
