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

#include "pluginselector.h"

#include <qvbox.h>
#include <qlabel.h>
#include <qstrlist.h>
#include <qfile.h>
#include <qstring.h>
#include <qlayout.h>
#include <qptrlist.h>

#include <kdebug.h>
#include <klocale.h>
#include <klistview.h>
#include <ksimpleconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kinstance.h>
#include <kstandarddirs.h>
#include <ktabctl.h>

using namespace KParts;

struct PluginSelector::PluginSelectorPrivate
{
	struct PluginInfo
	{
		QString name;
		QString comment;
		QString author;
		QString email;
		QString type;
		QString pluginname; // the name attribute in the .rc file
		QCheckListItem * item;
	};

	QValueList<PluginInfo> pluginInfos;
	KInstance * instance;
};

PluginSelector::PluginSelector( KInstance * instance, QWidget * parent, const char * name )
	: QWidget( parent, name )
	, d( new PluginSelectorPrivate )
{
	d->instance = instance;
	init( instance->dirs()->findAllResources( "data", instance->instanceName() + "/kpartplugins/*.desktop", true, false ), 0 );
}

PluginSelector::PluginSelector( const QStrList & types, KInstance * instance, QWidget * parent, const char * name )
	: QWidget( parent, name )
	, d( new PluginSelectorPrivate )
{
	d->instance = instance;
	init( instance->dirs()->findAllResources( "data", instance->instanceName() + "/kpartplugins/*.desktop", true, false ), &types );
}

PluginSelector::PluginSelector( const QStringList & desktopfilenames, const QStrList & types, KInstance * instance, QWidget * parent, const char * name )
    : QWidget( parent, name )
	, d( new PluginSelectorPrivate )
{
	d->instance = instance;
	init( desktopfilenames, &types );
}

void PluginSelector::init( const QStringList & desktopfilenames, const QStrList * types )
{
	for( QStringList::ConstIterator it = desktopfilenames.begin(); it != desktopfilenames.end(); ++it )
	{
		QString filename = *it;
		kdDebug( 1000 ) << "Plugin filename: " << filename << endl;
		KSimpleConfig file( filename );
		PluginSelectorPrivate::PluginInfo info;
		info.name = file.readEntry( "Name" );
		info.comment = file.readEntry( "Comment" );
		info.author = file.readEntry( "Author" );
		info.email = file.readEntry( "Email" );
		info.type = file.readEntry( "Type" );
		info.pluginname = file.readEntry( "PluginName" );
		info.item = 0;
		d->pluginInfos += info;
	}

	KConfigGroup cfgGroup( d->instance->config(), "KParts Plugins" );

	( new QVBoxLayout( this ) )->setAutoAdd( true );
	if( types )
	{
		KTabCtl * tabControl = new KTabCtl( this );
		QStrList typelist( *types );
		char * typeit;
		for( typeit = typelist.first(); typeit; typeit = typelist.next() )
		{
			QFrame * f = new QFrame( tabControl );
			( new QVBoxLayout( f, KDialog::marginHint(), KDialog::spacingHint() ) )->setAutoAdd( true ); 
			KListView * listview = new KListView( f );
			listview->setAcceptDrops( false );
			listview->setSelectionModeExt( KListView::NoSelection );
			listview->addColumn( i18n( "Name" ) );
			listview->addColumn( i18n( "Description" ) );
			listview->addColumn( i18n( "Author" ) );
			for( QValueList<PluginSelectorPrivate::PluginInfo>::Iterator it = d->pluginInfos.begin(); it != d->pluginInfos.end(); ++it )
			{
				if( ( *it ).type == typeit )
				{
					( *it ).item = new QCheckListItem( listview, ( *it ).name, QCheckListItem::CheckBox );
					( *it ).item->setText( 1, ( *it ).comment );
					( *it ).item->setText( 2, ( *it ).author );
					( *it ).item->setOn( cfgGroup.readBoolEntry( ( *it ).pluginname + "Enabled", false ) );
				}
			}
			tabControl->addTab( f, i18n( typeit ) );
		}
	}
	else
	{
		QFrame * f = new QFrame( this );
		( new QVBoxLayout( f, KDialog::marginHint(), KDialog::spacingHint() ) )->setAutoAdd( true ); 
		//( void )new QLabel( i18n( "<b>Choose one or more interfaces to use:</b>" ), f );
		KListView * listview = new KListView( f );
		listview->setAcceptDrops( false );
		listview->setSelectionModeExt( KListView::NoSelection );
		listview->addColumn( i18n( "Name" ) );
		listview->addColumn( i18n( "Description" ) );
		listview->addColumn( i18n( "Author" ) );
		for( QValueList<PluginSelectorPrivate::PluginInfo>::Iterator it = d->pluginInfos.begin(); it != d->pluginInfos.end(); ++it )
		{
			( *it ).item = new QCheckListItem( listview, ( *it ).name, QCheckListItem::CheckBox );
			( *it ).item->setText( 1, ( *it ).comment );
			( *it ).item->setText( 2, ( *it ).author );
			bool b = cfgGroup.readBoolEntry( ( *it ).pluginname + "Enabled", false );
			( *it ).item->setOn( b );
		}
	}
}

PluginSelector::~PluginSelector()
{
	delete d;
}

void PluginSelector::save( KConfig * config )
{
	kdDebug( 1000 ) << k_funcinfo << endl;
	if( ! config )
		config = d->instance->config();

	KConfigGroup cfgGroup( config, "KParts Plugins" );

	for( QValueList<PluginSelectorPrivate::PluginInfo>::Iterator it = d->pluginInfos.begin(); it != d->pluginInfos.end(); ++it )
		cfgGroup.writeEntry( ( *it ).pluginname + "Enabled", ( *it ).item->isOn() );
}

// vim:sw=4:ts=4

#include "pluginselector.moc"
