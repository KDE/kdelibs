#include "kplugininfo.h"
#include <ksimpleconfig.h>
#include <ktrader.h>
#include <kdebug.h>

class KPluginInfo::KPluginInfoPrivate
{
	public:
		KPluginInfoPrivate() : hidden( false ) {}
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
	d->services = KTrader::self()->query( "KCModule", "'" + pluginname() + "' in [X-KDE-KCDParents]" );
	//d->services = KTrader::self()->query( "KCModule", "'" + pluginname() + "' == [X-KDE-KCDParents]" );
	kdDebug( 702 ) << "found " << d->services.count() << " offers for " << pluginname() << endl;
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
}

const KPluginInfo & KPluginInfo::operator=( const KPluginInfo & tocp )
{
	d = new KPluginInfoPrivate;
	m_propertymap = tocp.m_propertymap;
	m_loaded = tocp.m_loaded;
	d->specfile = tocp.d->specfile;
	d->requirements = tocp.d->requirements;
	d->hidden = tocp.d->hidden;
	d->services = tocp.d->services;
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

const QValueList<KService::Ptr> & KPluginInfo::services() const
{
	return d->services;
}

// vim: sw=4 ts=4
