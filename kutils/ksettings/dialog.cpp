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

#include "kconfiguredialog.h"

#include <qptrlist.h>

#include "kcddispatcher.h"
#include "kcmultidialog.h"
#include <kglobal.h>
#include <klocale.h>
#include <kinstance.h>
#include <kservicegroup.h>
#include <kjanuswidget.h>
#include <kdebug.h>
#include <ktrader.h>
#include "kselectentriesdialog.h"
#include "kplugininfo.h"

class KCMISortedList : public QPtrList<KCModuleInfo>
{
	public:
		KCMISortedList() : QPtrList<KCModuleInfo>() {}
		KCMISortedList( const KCMISortedList &list ) : QPtrList<KCModuleInfo>( list ) {}
		~KCMISortedList() { clear(); }
		KCMISortedList &operator=( const KCMISortedList &list )
		{ return ( KCMISortedList& )QPtrList<KCModuleInfo>::operator=( list ); }

	protected:
		int compareItems( QPtrCollection::Item item1, QPtrCollection::Item item2 )
		{
			KCModuleInfo * info1 = static_cast<KCModuleInfo *>( item1 );
			KCModuleInfo * info2 = static_cast<KCModuleInfo *>( item2 );

			int size1 = info1->groups().size();
			int size2 = info2->groups().size();

			if( size1 == size2 )
				return 0;
			else if( size1 < size2 )
				return -1;
			else
				return 1;
		}
};

class KConfigureDialog::KConfigureDialogPrivate
{
	public:
		KConfigureDialogPrivate()
			: dlg( 0 )
			{
				moduleinfos.setAutoDelete( true );
			}

		bool staticlistview;
		KCMISortedList moduleinfos;
		KCMultiDialog * dlg;
		QValueList<KService::Ptr> services;
		QMap<QString, KPluginInfo*> plugininfomap;
};

	KConfigureDialog::KConfigureDialog( ContentInListView content, QObject * parent, const char * name )
	: QObject( parent, name )
, d( new KConfigureDialogPrivate )
{
	d->staticlistview = ( content == Static );
	d->services = instanceServices();
}

	KConfigureDialog::KConfigureDialog( const QStringList & kcdparents, ContentInListView content, QObject * parent, const char * name )
	: QObject( parent, name )
, d( new KConfigureDialogPrivate )
{
	d->staticlistview = ( content == Static );
	d->services = instanceServices() + parentComponentsServices( kcdparents );
}

KConfigureDialog::~KConfigureDialog()
{
	delete d;
}

void KConfigureDialog::addPluginInfos( const QValueList<KPluginInfo*> & plugininfos )
{
	for( QValueList<KPluginInfo*>::ConstIterator it = plugininfos.begin();
			it != plugininfos.end(); ++it )
		d->plugininfomap[ ( *it )->pluginname() ] = *it;
}

void KConfigureDialog::show()
{
	if( 0 == d->dlg )
		createDialogFromServices();
	KCDDispatcher::self()->syncConfiguration();
	return d->dlg->show();
}

KCMultiDialog * KConfigureDialog::dialog()
{
	if( 0 == d->dlg )
		createDialogFromServices();
	return d->dlg;
}

QValueList<KService::Ptr> KConfigureDialog::instanceServices() const
{
	kdDebug( 700 ) << k_funcinfo << endl;
	QString instanceName = KGlobal::instance()->instanceName();
	kdDebug( 700 ) << "calling KServiceGroup::childGroup( " << instanceName << " )" << endl;
	KServiceGroup::Ptr service = KServiceGroup::childGroup( instanceName );

	QValueList<KService::Ptr> ret;

	if( service && service->isValid() )
	{
		kdDebug( 700 ) << "call was successfull" << endl;
		KServiceGroup::List list = service->entries();
		for( KServiceGroup::List::ConstIterator it = list.begin(); it != list.end(); ++it )
		{
			KSycocaEntry * p = *it;
			if( p->isType( KST_KService ) )
			{
				kdDebug( 700 ) << "found service" << endl;
				ret << static_cast<KService *>( p );
			}
			else
				kdWarning( 700 ) << "KServiceGroup::childGroup returned something else than a KService (kinda)" << endl;
		}
	}

	return ret;
}

QValueList<KService::Ptr> KConfigureDialog::parentComponentsServices( const QStringList & kcdparents ) const
{
	QString constraint = kcdparents.join( "' in [X-KDE-ParentComponents]) or ('" );
	constraint = "('" + constraint + "' in [X-KDE-ParentComponents])";

	kdDebug( 700 ) << "constraint = " << constraint << endl;
	return KTrader::self()->query( "KCModule", constraint );
}

void KConfigureDialog::createDialogFromServices()
{
	QStringList groupnames;
	// for all services
	for( QValueList<KService::Ptr>::ConstIterator it = d->services.begin(); it != d->services.end(); ++it )
	{
		// we create the KCModuleInfo
		KCModuleInfo * moduleinfo = new KCModuleInfo( *it );
		if( ! d->staticlistview )
		{
			// and if the user of this class requested to hide disabled modules
			// we check whether it should be enabled or not
			bool enabled = false;
			kdDebug( 700 ) << "check whether the " << moduleinfo->moduleName() << " KCM should be shown" << endl;
			// for all parent components
			for( QStringList::ConstIterator pcit = moduleinfo->parentComponents().begin();
					pcit != moduleinfo->parentComponents().end(); ++pcit )
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
				enabled = pinfo->pluginEnabled();
				kdDebug( 700 ) << "parent " << *pcit << " is " << ( enabled ? "enabled" : "disabled" ) << endl;
				// if it is enabled we're done for this KCModuleInfo
				if( enabled )
					break;
			}
			if( ! enabled )
			{
				// it's not enabled so the KCModuleInfo is not needed anymore
				delete moduleinfo;
				continue;
			}
		}
		d->moduleinfos.append( moduleinfo );
		for( QStringList::ConstIterator it2 = moduleinfo->groups().begin();
				it2 != moduleinfo->groups().end(); ++it2 )
			if( ! groupnames.contains( *it2 ) )
				groupnames.append( *it2 );
	}
	kdDebug( 700 ) << "creating KCMultiDialog" << endl;
	if( groupnames.size() > 1 )
	{
		// we need a treelist dialog
		d->dlg = new KCMultiDialog( KJanusWidget::TreeList, i18n( "Preferences" ) );
		d->dlg->setShowIconsInTreeList( true );
		d->dlg->unfoldTreeList( true );
		// We need to find the .desktop files for all group names (those where
		// Name=<group name>)

		// sort the services after their depth: toplevel first
		//X 		d->moduleinfos.sort();
		//X 		KCModuleInfo * info;
		//X 		for( info = d->moduleinfos.first(); info; info = d->moduleinfos.next() )
		//X 		{
		//X 			if( info->groups().size() == 0 );
		//X 		}
		/*for( QStringList::ConstIterator it = groupnames.begin();
		  it != groupnames.end(); ++it )
		  {
		  if( ( *it )->
		  }*/
	}
	else
		d->dlg = new KCMultiDialog( KJanusWidget::IconList, i18n( "Preferences" ) );

	if( ! d->staticlistview )
		d->dlg->addButtonBelowList( i18n( "Configure ..." ), this, SLOT( configureTree() ) );

	connect( d->dlg, SIGNAL( okClicked() ), KCDDispatcher::self(), SLOT( syncConfiguration() ) );
	connect( d->dlg, SIGNAL( applyClicked() ), KCDDispatcher::self(), SLOT( syncConfiguration() ) );
	connect( d->dlg, SIGNAL( configCommitted( const QCString & ) ), KCDDispatcher::self(), SLOT( reparseConfiguration( const QCString & ) ) );
	for( KCModuleInfo * info = d->moduleinfos.first(); info; info = d->moduleinfos.next() )
	{
		kdDebug( 700 ) << "add module: " << info->fileName() << " with ParentComponents=" << info->parentComponents() << endl;
		d->dlg->addModule( *info );
	}
}

void KConfigureDialog::configureTree()
{
	kdDebug( 700 ) << k_funcinfo << endl;
	KSelectEntriesDialog * subdlg = new KSelectEntriesDialog( d->dlg );
	subdlg->show();
	connect( subdlg, SIGNAL( okClicked() ), this, SLOT( updateTreeList() ) );
	connect( subdlg, SIGNAL( applyClicked() ), this, SLOT( updateTreeList() ) );
}

void KConfigureDialog::updateTreeList()
{
	// FIXME: very inefficient code
	kdDebug( 700 ) << k_funcinfo << endl;
	// for all services
	for( QValueList<KService::Ptr>::ConstIterator it = d->services.begin(); it != d->services.end(); ++it )
	{
		// we create the KCModuleInfo
		KCModuleInfo * moduleinfo = new KCModuleInfo( *it );
		bool enabled = false;
		kdDebug( 700 ) << "check whether the " << moduleinfo->moduleName() << " KCM should be shown" << endl;
		// for all parent components
		for( QStringList::ConstIterator pcit = moduleinfo->parentComponents().begin();
				pcit != moduleinfo->parentComponents().end(); ++pcit )
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
			if( pinfo->config() )
				pinfo->config()->reparseConfiguration();
			pinfo->load();
			enabled = pinfo->pluginEnabled();
			kdDebug( 700 ) << "parent " << *pcit << " is " << ( enabled ? "enabled" : "disabled" ) << endl;
			// if it is enabled we're done for this KCModuleInfo
			if( enabled )
				break;
		}
		if( ! enabled )
		{
			// it's not enabled so the KCModuleInfo is not needed anymore
			for( KCModuleInfo * torm = d->moduleinfos.first(); torm;
					torm = d->moduleinfos.next() )
				if( *torm == *moduleinfo )
				{
					kdDebug( 700 ) << "remove module " << torm->moduleName() << endl;
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

#include "kconfiguredialog.moc"

// vim: sw=4 ts=4 noet
