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

#include "kpluginselector.h"
#include "kpreferencesmodule.h"

#include <qvbox.h>
#include <qlabel.h>
#include <qstrlist.h>
#include <qfile.h>
#include <qstring.h>
#include <qlayout.h>
#include <qptrlist.h>
#include <qwidgetstack.h>

#include <kdebug.h>
#include <klocale.h>
#include <klistview.h>
#include <ksimpleconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kinstance.h>
#include <kstandarddirs.h>
#include <ktabctl.h>

static const int DEBUGAREA = 1000;

class KPluginInfo::KPluginInfoPrivate
{
	public:
		KPluginInfoPrivate() : hidden( false ) {}
		QString specfile; // the filename of the file containing all the info
/*		QString name;
		QString comment;
		QString author;
		QString email;
		QString type;
		QString pluginname; // the name attribute in the .rc file
		QString version;
		QString website; // URL to the website of the plugin/author
		QString license;*/
		QStringList requirements;
		bool hidden;
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
		if( file.readBoolEntry( "Hidden", false ) || file.readEntry( "Type" ) != "Plugin" )
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
	{ // provided for noatun compatibility
		m_propertymap = file.entryMap( QString::null );
		//m_propertymap[ "Name" ] = file.readEntry( "Name" );
		//m_propertymap[ "Comment" ] = file.readEntry( "Comment" );

		m_propertymap[ "PluginName" ] = file.readEntry( "Filename" );
		//m_propertymap[ "Author" ] = file.readEntry( "Author" );
		m_propertymap[ "Website" ] = file.readEntry( "Site" );
		//m_propertymap[ "Email" ] = file.readEntry( "Email" );
		//m_propertymap[ "Type" ] = file.readEntry( "Type" );
		//m_propertymap[ "License" ] = file.readEntry( "License" );
		d->requirements = file.readListEntry( "Require" );
	}
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
}

const KPluginInfo & KPluginInfo::operator=( const KPluginInfo & tocp )
{
	d = new KPluginInfoPrivate;
	m_propertymap = tocp.m_propertymap;
	m_loaded = tocp.m_loaded;
	d->specfile = tocp.d->specfile;
	d->requirements = tocp.d->requirements;
	d->hidden = tocp.d->hidden;
	return *this;
}

KPluginInfo::~KPluginInfo()
{
	delete d;
}

bool KPluginInfo::isHidden() const
{
	return d->hidden;
}

void KPluginInfo::setPluginLoaded( bool loaded )
{
	m_loaded = loaded;
}

bool KPluginInfo::pluginLoaded() const
{
	return m_loaded;
}

const QString & KPluginInfo::specfile() const
{
	return d->specfile;
}

const QStringList & KPluginInfo::requirements() const
{
	return d->requirements;
}

/************************************************************/
/********************* KPluginSelector **********************/
/************************************************************/

struct KPluginSelector::KPluginSelectorPrivate
{
	KPluginSelectorPrivate( KInstance * _instance )
		: instance( _instance )
		, widgetstack( 0 )
		, currentchecked( false )
	{}

	QMap<QCheckListItem*, KPluginInfo> pluginInfoMap;

	KInstance * instance; // == 0 for non-KParts plugins
	QWidgetStack * widgetstack;

	QDict<KPreferencesModule> pluginconfigmodules;
	QMap<QString, int> widgetIDs;

	QString currentpluginname;
	bool currentchecked;
};

KPluginSelector::KPluginSelector( KInstance * instance, QWidget * parent, const char * name )
	: QWidget( parent, name )
	, d( new KPluginSelectorPrivate( instance ) )
{
	init( kpartsPluginInfos(), 0 );
}

KPluginSelector::KPluginSelector( const QStrList & types, KInstance * instance, QWidget * parent, const char * name )
	: QWidget( parent, name )
	, d( new KPluginSelectorPrivate( instance ) )
{
	init( kpartsPluginInfos(), &types );
}

KPluginSelector::KPluginSelector( const QValueList<KPluginInfo> & plugininfos, QWidget * parent, const char * name )
    : QWidget( parent, name )
	, d( new KPluginSelectorPrivate( 0 ) )
{
	init( plugininfos, 0 );
}

KPluginSelector::KPluginSelector( const QStrList & types, const QValueList<KPluginInfo> & plugininfos,
		QWidget * parent, const char * name )
    : QWidget( parent, name )
	, d( new KPluginSelectorPrivate( 0 ) )
{
	init( plugininfos, &types );
}

QValueList<KPluginInfo> KPluginSelector::kpartsPluginInfos() const
{
	QValueList<KPluginInfo> infolist;
	if( d->instance )
	{
		QStringList desktopfilenames = d->instance->dirs()->findAllResources(
				"data", d->instance->instanceName() + "/kpartplugins/*.desktop", true, false );
		KConfigGroup cfgGroup( d->instance->config(), "KParts Plugins" );
		for( QStringList::ConstIterator it = desktopfilenames.begin(); it != desktopfilenames.end(); ++it )
		{
			KPluginInfo info( *it );
			info.setPluginLoaded( cfgGroup.readBoolEntry( info.pluginname() + "Enabled", false ) );
			infolist += info;
		}
	}
	return infolist;
}

void KPluginSelector::init( const QValueList<KPluginInfo> & plugininfos, const QStrList * types )
{
	// setup Widgets
	( new QVBoxLayout( this, 0, KDialog::spacingHint() ) )->setAutoAdd( true );
	if( types )
	{
		KTabCtl * tabControl = new KTabCtl( this );
		QStrList typelist( *types );
		for( char * typeit = typelist.first(); typeit; typeit = typelist.next() )
		{
			QFrame * f = new QFrame( tabControl );
			( new QVBoxLayout( f, KDialog::marginHint(), KDialog::spacingHint() ) )->setAutoAdd( true ); 
			KListView * listview = new KListView( f );
			connect( listview, SIGNAL( pressed( QListViewItem * ) ), this, SLOT( executed( QListViewItem * ) ) );
			connect( listview, SIGNAL( spacePressed( QListViewItem * ) ), this, SLOT( executed( QListViewItem * ) ) );
			connect( listview, SIGNAL( returnPressed( QListViewItem * ) ), this, SLOT( executed( QListViewItem * ) ) );
			//connect( listview, SIGNAL( selectionChanged( QListViewItem * ) ), this, SLOT( executed( QListViewItem * ) ) );
			listview->setAcceptDrops( false );
			listview->setSelectionModeExt( KListView::Single );
			listview->addColumn( i18n( "Name" ) );
			listview->addColumn( i18n( "Description" ) );
			listview->addColumn( i18n( "Author" ) );
			listview->addColumn( i18n( "Version" ) );
			listview->addColumn( i18n( "License" ) );
			for( QValueList<KPluginInfo>::ConstIterator it = plugininfos.begin(); it != plugininfos.end(); ++it )
			{
				if( ! ( *it ).isHidden() && ( *it ).type() == typeit )
				{
					QCheckListItem * item = new QCheckListItem( listview, ( *it ).name(), QCheckListItem::CheckBox );
					item->setText( 1, ( *it ).comment() );
					item->setText( 2, ( *it ).author()  );
					item->setText( 3, ( *it ).version() );
					item->setText( 3, ( *it ).license() );
					item->setOn( ( *it ).pluginLoaded() );
					d->pluginInfoMap.insert( item, *it );
				}
			}
			tabControl->addTab( f, i18n( typeit ) );
		}
	}
	else
	{
		QFrame * f = new QFrame( this );
		( new QVBoxLayout( f, 0, KDialog::spacingHint() ) )->setAutoAdd( true ); 
		KListView * listview = new KListView( f );
		connect( listview, SIGNAL( pressed( QListViewItem * ) ), this, SLOT( executed( QListViewItem * ) ) );
		connect( listview, SIGNAL( spacePressed( QListViewItem * ) ), this, SLOT( executed( QListViewItem * ) ) );
		connect( listview, SIGNAL( returnPressed( QListViewItem * ) ), this, SLOT( executed( QListViewItem * ) ) );
		//connect( listview, SIGNAL( selectionChanged( QListViewItem * ) ), this, SLOT( executed( QListViewItem * ) ) );
		listview->setAcceptDrops( false );
		listview->setSelectionModeExt( KListView::Single );
		listview->addColumn( i18n( "Name" ) );
		listview->addColumn( i18n( "Description" ) );
		listview->addColumn( i18n( "Author" ) );
		listview->addColumn( i18n( "Version" ) );
		listview->addColumn( i18n( "License" ) );
		for( QValueList<KPluginInfo>::ConstIterator it = plugininfos.begin(); it != plugininfos.end(); ++it )
		{
			if( ! ( *it ).isHidden() )
			{
				QCheckListItem * item = new QCheckListItem( listview, ( *it ).name(), QCheckListItem::CheckBox );
				item->setText( 1, ( *it ).comment() );
				item->setText( 2, ( *it ).author()  );
				item->setText( 3, ( *it ).version() );
				item->setText( 3, ( *it ).license() );
				item->setOn( ( *it ).pluginLoaded() );
				d->pluginInfoMap.insert( item, *it );
			}
		}
	}

	// widgetstack
	d->widgetstack = new QWidgetStack( this, "KPluginSelector Config Pages" );
	d->widgetstack->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	d->widgetstack->setMinimumSize( 100, 100 );

	QLabel * label = new QLabel( i18n( "Select a plugin to configure it (if it is configurable, that is)." ), d->widgetstack, "info message page 1001" );
	label->setAlignment( Qt::AlignCenter );
	d->widgetstack->addWidget( label, 1001 );

	label = new QLabel( i18n( "This plugin isn't configurable." ), d->widgetstack, "info message page 1" );
	label->setAlignment( Qt::AlignCenter );
	d->widgetstack->addWidget( label, 1 );

	label = new QLabel( i18n( "You need to press \"Apply\" to actually load the plugin." ), d->widgetstack, "info message page 2" );
	label->setAlignment( Qt::AlignCenter );
	d->widgetstack->addWidget( label, 2 );

	label = new QLabel( i18n( "You need to enable the plugin and press the \"Apply\" button to configure the plugin" ), d->widgetstack, "info message page 3" );
	label->setAlignment( Qt::AlignCenter );
	d->widgetstack->addWidget( label, 3 );
}

KPluginSelector::~KPluginSelector()
{
	for( QDictIterator<KPreferencesModule> it( d->pluginconfigmodules ); it.current(); ++it )
		it.current()->parentModuleDestroyed(); // give the module a chance to not have a dangling pointer
	delete d;
}

bool KPluginSelector::pluginIsLoaded( const QString & pluginname ) const
{
	for( QMap<QCheckListItem*, KPluginInfo>::ConstIterator it = d->pluginInfoMap.begin();
			it != d->pluginInfoMap.end(); ++it )
		if( it.data().pluginname() == pluginname )
			return it.data().pluginLoaded();
	return false;
}

void KPluginSelector::registerPlugin( const QString & pluginname, KPreferencesModule * module )
{
	kdDebug( DEBUGAREA ) << k_funcinfo << pluginname << endl;
	d->pluginconfigmodules.insert( pluginname, module );
	if( pluginname == d->currentpluginname )
		updateConfigPage( d->currentpluginname, d->currentchecked );
}

void KPluginSelector::unregisterPlugin( const QString & pluginname )
{
	kdDebug( DEBUGAREA ) << k_funcinfo << pluginname << endl;
	d->pluginconfigmodules.remove( pluginname );
	if( pluginname == d->currentpluginname )
		updateConfigPage( d->currentpluginname, d->currentchecked );
}

void KPluginSelector::updateConfigPage( const QString & pluginname, bool checked )
{
	kdDebug( DEBUGAREA ) << k_funcinfo << endl;
	d->currentpluginname = pluginname;
	d->currentchecked = checked;
	KPreferencesModule * module = d->pluginconfigmodules[ pluginname ];
	if( module )
	{
		kdDebug( DEBUGAREA ) << "got the module" << endl;
		// The plugin registered a config page, so we're going to show it to the
		// user.
		if( d->widgetIDs.contains( pluginname ) )
		{
			int id = d->widgetIDs[ pluginname ];
			d->widgetstack->raiseWidget( id );
			d->widgetstack->widget( id )->setEnabled( checked ); // disable config page if the plugin is disabled
		}
		else
		{
			QFrame * page = new QFrame( d->widgetstack, "PluginConfigPage" );
			module->createPage( page );
			page->setEnabled( checked ); // disable config page if the plugin is disabled
			d->widgetIDs[ pluginname ] = d->widgetstack->addWidget( page );
			d->widgetstack->raiseWidget( page );
		}
	}
	else
	{
		kdDebug( DEBUGAREA ) << "module not loaded" << endl;
		// no config page, this could mean two things: the plugin doesn't have
		// one or the plugin isn't loaded yet (or it has a config page is
		// already loaded but didn't register the config page - shouldn't
		// happen, hopefully).
		if( checked )
			if( pluginIsLoaded( pluginname ) )
				infoPage( LoadedAndEnabled );
			else
				infoPage( NotLoadedAndEnabled );
		else
			if( pluginIsLoaded( pluginname ) )
				infoPage( LoadedAndDisabled );
			else
				infoPage( NotLoadedAndDisabled );
	}
}

void KPluginSelector::executed( QListViewItem * item )
{
	kdDebug( DEBUGAREA ) << k_funcinfo << endl;
	if( item->rtti() != 1 ) //check for a QCheckListItem
		return;

	QCheckListItem * citem = static_cast<QCheckListItem *>( item );
	bool checked = citem->isOn();
	kdDebug( DEBUGAREA ) << "it's a " << ( checked ? "checked" : "unchecked" ) << " QCheckListItem" << endl;

	const KPluginInfo * info = &d->pluginInfoMap[ citem ];
	if( info->isHidden() )
		kdFatal( DEBUGAREA ) << "bummer" << endl;
	checkDependencies( *info );

	updateConfigPage( info->pluginname(), checked );
}

void KPluginSelector::save( KConfig * config )
{
	kdDebug( DEBUGAREA ) << k_funcinfo << endl;
	if( ! config )
	{
		if( ! d->instance )
		{
			kdError( DEBUGAREA ) << "Cannot save configuration without KConfig object" << endl;
			return;
		}
		config = d->instance->config();
	}

	KConfigGroup cfgGroup( config, ( d->instance ? "KParts Plugins" : "Plugins" ) );
		//assume KParts plugins when d->instance != 0
	for( QMap<QCheckListItem*, KPluginInfo>::Iterator it = d->pluginInfoMap.begin();
			it != d->pluginInfoMap.end(); ++it )
	{
		bool checked = it.key()->isOn();
		cfgGroup.writeEntry( ( *it ).pluginname() + "Enabled", checked );
		it.data().setPluginLoaded( checked );
	}
	updateConfigPage( d->currentpluginname, d->currentchecked );
}

inline void KPluginSelector::infoPage( InfoPageName id )
{
	switch( id )
	{
		case LoadedAndDisabled:
		case LoadedAndEnabled:
			d->widgetstack->raiseWidget( 1 );
			break;
		case NotLoadedAndEnabled:
			d->widgetstack->raiseWidget( 2 );
			break;
		case NotLoadedAndDisabled:
			d->widgetstack->raiseWidget( 3 );
			break;
	}
}

void KPluginSelector::checkDependencies( const KPluginInfo & info )
{
	if( info.requirements().isEmpty() )
		return;

	for( QStringList::ConstIterator it = info.requirements().begin(); it != info.requirements().end(); ++it )
		for( QMap<QCheckListItem*, KPluginInfo>::Iterator infoIt = d->pluginInfoMap.begin();
				infoIt != d->pluginInfoMap.end(); ++it )
			if( infoIt.data().pluginname() == *it )
			{
				if( ! infoIt.key()->isOn() )
				{
					infoIt.key()->setOn( true );
					checkDependencies( infoIt.data() );
				}
				continue;
			}
}

// vim: sw=4 ts=4

#include "kpluginselector.moc"
