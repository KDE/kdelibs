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

#include "kplugininfo.h"
#include <ksimpleconfig.h>
#include <ktrader.h>
#include <kdebug.h>
#include <kconfigbase.h>
#include <kglobal.h>
#include <kstandarddirs.h>

class KPluginInfo::KPluginInfoPrivate
{
	public:
		KPluginInfoPrivate()
			: hidden( false )
			, enabledbydefault( false )
		{}

		QString specfile; // the filename of the file containing all the info
/*		QString name;
		QString comment;
		QString author;
		QString email;
		QString category;
		QString pluginname; // the name attribute in the .rc file
		QString version;
		QString website; // URL to the website of the plugin/author
		QString license;*/
		QStringList requirements;
		QValueList<KService::Ptr> services;
		bool hidden;
		bool enabledbydefault;
		/** WARNING: add every entry to the cctor and operator= */
};

KPluginInfo::KPluginInfo( const QString & filename )
	: m_loaded( false )
	, d( new KPluginInfoPrivate )
{
	KSimpleConfig file( filename );

	d->specfile = filename;

	if( filename.endsWith( QString::fromAscii( ".desktop" ) ) )
	{
		file.setDesktopGroup();
		if( file.readBoolEntry( "Hidden", false ) ||
				file.readEntry( "Type" ) != "Plugin" )
		{
			d->hidden = true;
			return;
		}

		m_propertymap = file.entryMap( "X-KDE Plugin Info" );
		m_propertymap[ "Name" ] = file.readEntry( "Name" );
		m_propertymap[ "Comment" ] = file.readEntry( "Comment" );

		file.setGroup( "X-KDE Plugin Info" );
		d->requirements = file.readListEntry( "Require" );
	}
	else if( filename.endsWith( QString::fromAscii( ".plugin" ) ) )
	{ // provided for noatun style .plugin files compatibility
		m_propertymap = file.entryMap( QString::null );
		//m_propertymap[ "Name" ] = file.readEntry( "Name" );
		//m_propertymap[ "Comment" ] = file.readEntry( "Comment" );

		m_propertymap[ "PluginName" ] = file.readPathEntry( "Filename" );
		//m_propertymap[ "Author" ] = file.readEntry( "Author" );
		m_propertymap[ "Website" ] = file.readEntry( "Site" );
		//m_propertymap[ "Email" ] = file.readEntry( "Email" );
		m_propertymap[ "Category" ] = file.readEntry( "Type" );
		//m_propertymap[ "License" ] = file.readEntry( "License" );
		d->requirements = file.readListEntry( "Require" );
	}
	d->services = KTrader::self()->query( "KCModule", "'" + pluginname() + "' in [X-KDE-ParentComponents]" );
	kdDebug( 702 ) << "found " << d->services.count() << " offers for " << pluginname() << endl;
	d->enabledbydefault = file.readBoolEntry( "EnabledByDefault", d->enabledbydefault );
}

KPluginInfo::KPluginInfo()
	: d( new KPluginInfoPrivate )
{
	d->hidden = true;
}

KPluginInfo::KPluginInfo( const KPluginInfo & tocp )
{
	d = new KPluginInfoPrivate;
	m_propertymap = tocp.m_propertymap;
	m_loaded = tocp.m_loaded;
	d->specfile = tocp.d->specfile;
	d->requirements = tocp.d->requirements;
	d->hidden = tocp.d->hidden;
	d->services = tocp.d->services;
	d->enabledbydefault = tocp.d->enabledbydefault;
}

const KPluginInfo & KPluginInfo::operator=( const KPluginInfo & tocp )
{
	delete d;
	d = new KPluginInfoPrivate;
	m_propertymap = tocp.m_propertymap;
	m_loaded = tocp.m_loaded;
	d->specfile = tocp.d->specfile;
	d->requirements = tocp.d->requirements;
	d->hidden = tocp.d->hidden;
	d->services = tocp.d->services;
	d->enabledbydefault = tocp.d->enabledbydefault;
	return *this;
}

KPluginInfo::~KPluginInfo()
{
	delete d;
}

QValueList<KPluginInfo> KPluginInfo::fromServices( const KService::List & services )
{
	QValueList<KPluginInfo> infolist;
	for( KService::List::ConstIterator it = services.begin();
			it != services.end(); ++it )
	{
		infolist += KPluginInfo( ( *it )->desktopEntryPath() );
	}
	return infolist;
}

QValueList<KPluginInfo> KPluginInfo::fromFiles( const QStringList & files )
{
	QValueList<KPluginInfo> infolist;
	for( QStringList::ConstIterator it = files.begin(); it != files.end(); ++it )
		infolist += KPluginInfo( *it );
	return infolist;
}

QValueList<KPluginInfo> KPluginInfo::fromKPartsInstanceName( const QString & name )
{
	QStringList files = KGlobal::dirs()->findAllResources( "data", name +
			"/kpartplugins/*.desktop", true, false );
	return fromFiles( files );
}

bool KPluginInfo::isHidden() const
{
	return d->hidden;
}

void KPluginInfo::setPluginEnabled( bool loaded )
{
	kdDebug( 702 ) << k_funcinfo << endl;
	m_loaded = loaded;
}

bool KPluginInfo::pluginEnabled() const
{
	kdDebug( 702 ) << k_funcinfo << endl;
	return m_loaded;
}

bool KPluginInfo::pluginEnabledByDefault() const
{
	kdDebug( 702 ) << k_funcinfo << endl;
	return d->enabledbydefault;
}

const QString & KPluginInfo::specfile() const
{
	return d->specfile;
}

const QStringList & KPluginInfo::requirements() const
{
	return d->requirements;
}

const QValueList<KService::Ptr> & KPluginInfo::services() const
{
	return d->services;
}

void KPluginInfo::save( KConfigGroup * config )
{
	kdDebug( 702 ) << k_funcinfo << endl;
	config->writeEntry( pluginname() + "Enabled", pluginEnabled() );
}

void KPluginInfo::load( KConfigGroup * config )
{
	kdDebug( 702 ) << k_funcinfo << endl;
	setPluginEnabled( config->readBoolEntry( pluginname() + "Enabled", pluginEnabledByDefault() ) );
}

void KPluginInfo::defaults()
{
	kdDebug( 702 ) << k_funcinfo << endl;
	setPluginEnabled( pluginEnabledByDefault() );
}

// vim: sw=4 ts=4 noet
