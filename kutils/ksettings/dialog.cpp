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

#include "ksettings/dialog.h"


#include <kcmultidialog.h>
#include <klocale.h>
#include <kservicegroup.h>
#include <kdebug.h>
#include <ktrader.h>
#include <kplugininfo.h>
#include "ksettings/dispatcher.h"
#include "ksettings/componentsdialog.h"
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kiconloader.h>

namespace KSettings
{

class KCMISortedList : public QPtrList<KCModuleInfo>
{
	public:
		KCMISortedList() : QPtrList<KCModuleInfo>() {}
		KCMISortedList( const KCMISortedList &list )
			: QPtrList<KCModuleInfo>( list ) {}
		~KCMISortedList() { clear(); }
		KCMISortedList &operator=( const KCMISortedList &list )
		{ return ( KCMISortedList& )QPtrList<KCModuleInfo>::operator=( list ); }

	protected:
		int compareItems( QPtrCollection::Item item1,
				QPtrCollection::Item item2 )
		{
			KCModuleInfo * info1 = static_cast<KCModuleInfo *>( item1 );
			KCModuleInfo * info2 = static_cast<KCModuleInfo *>( item2 );

			if( info1->weight() == info2->weight() )
				return 0;
			else if( info1->weight() < info2->weight() )
				return -1;
			else
				return 1;
		}
};

class Dialog::DialogPrivate
{
	public:
		DialogPrivate()
			: dlg( 0 )
			{
				moduleinfos.setAutoDelete( true );
			}

		bool staticlistview;
		KCMISortedList moduleinfos;
		KCMultiDialog * dlg;
		QValueList<KService::Ptr> services;
		QMap<QString, KPluginInfo*> plugininfomap;
		QWidget * parentwidget;

		struct GroupInfo
		{
			QString name;
			QString comment;
			QString icon;
			int weight;
			QString parent;
		};

		QMap<QString, GroupInfo> groupmap;
		QMap<KCModuleInfo *, QStringList> parentmodulenames;
};

Dialog::Dialog( QWidget * parent, const char * name )
	: QObject( parent, name )
, d( new DialogPrivate )
{
	d->parentwidget = parent;
	d->staticlistview = true;
	d->services = instanceServices();
}

Dialog::Dialog( ContentInListView content,
		QWidget * parent, const char * name )
	: QObject( parent, name )
, d( new DialogPrivate )
{
	d->parentwidget = parent;
	d->staticlistview = ( content == Static );
	d->services = instanceServices();
}

Dialog::Dialog( const QStringList & components,
		QWidget * parent, const char * name )
	: QObject( parent, name )
, d( new DialogPrivate )
{
	d->parentwidget = parent;
	d->staticlistview = true;
	d->services = instanceServices() + parentComponentsServices( components );
}

Dialog::Dialog( const QStringList & components,
		ContentInListView content, QWidget * parent, const char * name )
	: QObject( parent, name )
, d( new DialogPrivate )
{
	d->parentwidget = parent;
	d->staticlistview = ( content == Static );
	d->services = instanceServices() + parentComponentsServices( components );
}

Dialog::~Dialog()
{
	delete d;
}

void Dialog::addPluginInfos( const QValueList<KPluginInfo*> & plugininfos )
{
	for( QValueList<KPluginInfo*>::ConstIterator it = plugininfos.begin();
			it != plugininfos.end(); ++it )
		d->plugininfomap[ ( *it )->pluginName() ] = *it;
}

void Dialog::show()
{
	if( 0 == d->dlg )
		createDialogFromServices();
	Dispatcher::self()->syncConfiguration();
	return d->dlg->show();
}

KCMultiDialog * Dialog::dialog()
{
	if( 0 == d->dlg )
		createDialogFromServices();
	return d->dlg;
}

QValueList<KService::Ptr> Dialog::instanceServices() const
{
	kdDebug( 700 ) << k_funcinfo << endl;
	QString instanceName = KGlobal::instance()->instanceName();
	kdDebug( 700 ) << "calling KServiceGroup::childGroup( " << instanceName
		<< " )" << endl;
	KServiceGroup::Ptr service = KServiceGroup::childGroup( instanceName );

	QValueList<KService::Ptr> ret;

	if( service && service->isValid() )
	{
		kdDebug( 700 ) << "call was successfull" << endl;
		KServiceGroup::List list = service->entries();
		for( KServiceGroup::List::ConstIterator it = list.begin();
				it != list.end(); ++it )
		{
			KSycocaEntry * p = *it;
			if( p->isType( KST_KService ) )
			{
				kdDebug( 700 ) << "found service" << endl;
				ret << static_cast<KService *>( p );
			}
			else
				kdWarning( 700 ) << "KServiceGroup::childGroup returned"
					" something else than a KService (kinda)" << endl;
		}
	}

	return ret;
}

QValueList<KService::Ptr> Dialog::parentComponentsServices(
		const QStringList & kcdparents ) const
{
	QString constraint = kcdparents.join(
			"' in [X-KDE-ParentComponents]) or ('" );
	constraint = "('" + constraint + "' in [X-KDE-ParentComponents])";

	kdDebug( 700 ) << "constraint = " << constraint << endl;
	return KTrader::self()->query( "KCModule", constraint );
}

void Dialog::setGroupIcons()
{
	// iterate over the groups and set the icons
	for( QMap<QString, DialogPrivate::GroupInfo>::ConstIterator it
			= d->groupmap.begin(); it != d->groupmap.end(); ++it )
	{
		if( ( *it ).icon.isNull() )
			continue;

		const DialogPrivate::GroupInfo * info = &( *it );
		QStringList path = info->name;
		while( ! info->parent.isNull() &&
				d->groupmap.contains( info->parent ) )
		{
			info = &d->groupmap[ info->parent ];
			path.prepend( info->name );
		}
		kdDebug( 700 ) << "set icon for group: " << path << ( *it ).icon
			<< endl;
		d->dlg->setFolderIcon( path,
				SmallIcon( ( *it ).icon, IconSize( KIcon::Small ) ) );
	}
}

bool Dialog::isPluginForKCMEnabled( KCModuleInfo * moduleinfo ) const
{
	// and if the user of this class requested to hide disabled modules
	// we check whether it should be enabled or not
	bool enabled = false;
	kdDebug( 700 ) << "check whether the " << moduleinfo->moduleName()
		<< " KCM should be shown" << endl;
	// for all parent components
	QStringList parentComponents = moduleinfo->service()->property(
			"X-KDE-ParentComponents" ).toStringList();
	for( QStringList::ConstIterator pcit = parentComponents.begin();
			pcit != parentComponents.end(); ++pcit )
	{
		// we check if the parent component is a plugin
		if( ! d->plugininfomap.contains( *pcit ) )
		{
			// if not the KCModule must be enabled
			enabled = true;
			// we're done for this KCModuleInfo
			break;
		}
		// if it is a plugin we check whether the plugin is enabled
		KPluginInfo * pinfo = d->plugininfomap[ *pcit ];
		pinfo->load();
		enabled = pinfo->isPluginEnabled();
		kdDebug( 700 ) << "parent " << *pcit << " is "
			<< ( enabled ? "enabled" : "disabled" ) << endl;
		// if it is enabled we're done for this KCModuleInfo
		if( enabled )
			break;
	}
	return enabled;
}

void Dialog::parseGroupFile( const QString & filename )
{
	KSimpleConfig file( filename );
	QStringList groups = file.groupList();
	for( QStringList::ConstIterator it = groups.begin(); it != groups.end();
			++it )
	{
		DialogPrivate::GroupInfo group;
		QString id = *it;
		file.setGroup( id.utf8() );
		group.name = file.readEntry( "Name" );
		group.comment = file.readEntry( "Comment" );
		group.weight = file.readNumEntry( "Weight", 100 );
		group.parent = file.readEntry( "Parent" );
		group.icon = file.readEntry( "Icon" );
		d->groupmap[ id ] = group;
	}
}

void Dialog::createDialogFromServices()
{
	QString setdlgpath = locate( "appdata",
			KGlobal::instance()->instanceName() + ".setdlg" );
	QStringList setdlgaddon = KGlobal::dirs()->findAllResources( "appdata",
			"ksettingsdialog/*.setdlg" );
	if( ! setdlgpath.isNull() )
		parseGroupFile( setdlgpath );
	if( setdlgaddon.size() > 0 )
		for( QStringList::ConstIterator it = setdlgaddon.begin();
				it != setdlgaddon.end(); ++it )
			parseGroupFile( *it );

	// for all services
	for( QValueList<KService::Ptr>::ConstIterator it = d->services.begin();
			it != d->services.end(); ++it )
	{
		// we create the KCModuleInfo
		KCModuleInfo * moduleinfo = new KCModuleInfo( *it );
		if( ! d->staticlistview && ! isPluginForKCMEnabled( moduleinfo ) )
		{
			// it's not enabled so the KCModuleInfo is not needed anymore
			delete moduleinfo;
			continue;
		}
		d->moduleinfos.append( moduleinfo );
	}

	kdDebug( 700 ) << "creating KCMultiDialog" << endl;
	if( d->groupmap.size() > 1 )
	{
		// we need a treelist dialog
		d->dlg = new KCMultiDialog( KJanusWidget::TreeList,
				i18n( "Preferences" ) );
		d->dlg->setShowIconsInTreeList( true );

		setGroupIcons();

		for( KCModuleInfo * info = d->moduleinfos.first(); info;
				info = d->moduleinfos.next() )
		{
			QVariant tmp = info->service()->property( "X-KDE-CfgDlgHierarchy",
					QVariant::String );
			if( ! tmp.isValid() )
				continue;
			QString id = tmp.toString();
			if( ! d->groupmap.contains( id ) )
				continue;
			DialogPrivate::GroupInfo gi = d->groupmap[ id ];
			QStringList parentnames;
			parentnames.prepend( gi.name );
			while( ! gi.parent.isNull() && d->groupmap.contains( gi.parent ) )
			{
				gi = d->groupmap[ gi.parent ];
				parentnames.prepend( gi.name );
			}
			d->parentmodulenames[ info ] = parentnames;
		}
	}
	else
		d->dlg = new KCMultiDialog( KJanusWidget::IconList,
			i18n( "Preferences" ), d->parentwidget );

	// TODO: Don't show the reset button until the issue with the
	// KPluginSelector::load() method is solved.
	// Problem:
	// KCMultiDialog::show() call KCModule::load() to reset all KCMs
	// (KPluginSelector::load() resets all plugin selections and all plugin
	// KCMs).
	// The reset button calls KCModule::load(), too but in this case we want the
	// KPluginSelector to only reset the current visible plugin KCM and not
	// touch the plugin selections.
	// I have no idea how to check that in KPluginSelector::load()...
	//d->dlg->showButton( KDialogBase::User1, true );
	if( ! d->staticlistview )
		d->dlg->addButtonBelowList( i18n( "Configure..." ), this,
			SLOT( configureTree() ) );

	connect( d->dlg, SIGNAL( okClicked() ), Dispatcher::self(),
		SLOT( syncConfiguration() ) );
	connect( d->dlg, SIGNAL( applyClicked() ), Dispatcher::self(),
		SLOT( syncConfiguration() ) );
	connect( d->dlg, SIGNAL( configCommitted( const QCString & ) ),
		Dispatcher::self(), SLOT( reparseConfiguration( const QCString & ) ) );
	d->moduleinfos.sort();
	for( KCModuleInfo * info = d->moduleinfos.first(); info;
		info = d->moduleinfos.next() )
	{
		kdDebug( 700 ) << "add module: " << info->fileName() << endl;
		d->dlg->addModule( *info, d->parentmodulenames[ info ] );
	}

	if( d->groupmap.size() > 1 )
		d->dlg->unfoldTreeList( true );
}

void Dialog::configureTree()
{
	kdDebug( 700 ) << k_funcinfo << endl;
	ComponentsDialog * subdlg = new ComponentsDialog( d->dlg );
	subdlg->setPluginInfos( d->plugininfomap );
	subdlg->show();
	connect( subdlg, SIGNAL( okClicked() ), this, SLOT( updateTreeList() ) );
	connect( subdlg, SIGNAL( applyClicked() ), this, SLOT( updateTreeList() ) );
}

void Dialog::updateTreeList()
{
	// FIXME: very inefficient code
	kdDebug( 700 ) << k_funcinfo << endl;
	// for all services
	for( QValueList<KService::Ptr>::ConstIterator it = d->services.begin();
			it != d->services.end(); ++it )
	{
		// we create the KCModuleInfo
		KCModuleInfo * moduleinfo = new KCModuleInfo( *it );
		if( ! isPluginForKCMEnabled( moduleinfo ) )
		{
			// it's not enabled so the KCModuleInfo is not needed anymore
			for( KCModuleInfo * torm = d->moduleinfos.first(); torm;
					torm = d->moduleinfos.next() )
				if( *torm == *moduleinfo )
				{
					kdDebug( 700 ) << "remove module " << torm->moduleName()
						<< endl;
					d->dlg->removeModule( *torm );
					d->moduleinfos.remove( torm ); //autodeleted
					break;
				}
			delete moduleinfo;
		}
		else
		{
			// it's enabled, check if we need to add it (if it's not there yet)
			bool needtoadd = true;
			for( KCModuleInfo * inthere = d->moduleinfos.first(); inthere;
					inthere = d->moduleinfos.next() )
				if( *inthere == *moduleinfo )
				{
					needtoadd = false;
					break;
				}
			if( needtoadd )
			{
				// it's not there yet - add it
				d->dlg->addModule( *moduleinfo );
				d->moduleinfos.append( moduleinfo );
			}
		}
	}
}

} //namespace

#include "dialog.moc"

// vim: sw=4 ts=4 noet
