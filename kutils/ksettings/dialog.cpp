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

class KConfigureDialog::KConfigureDialogPrivate
{
	public:
		KConfigureDialogPrivate()
			: dlg( 0 )
		{
			moduleinfos.setAutoDelete( true );
		}

		QPtrList<KCModuleInfo> moduleinfos;
		KCMultiDialog * dlg;
};

KConfigureDialog::KConfigureDialog( QObject * parent, const char * name )
	: QObject( parent, name )
	, d( new KConfigureDialogPrivate )
{
	QValueList<KService::Ptr> services = instanceServices();
	createDialogFromServices( services );
}

KConfigureDialog::KConfigureDialog( const QStringList & kcdparents, QObject * parent, const char * name )
	: QObject( parent, name )
	, d( new KConfigureDialogPrivate )
{
	QValueList<KService::Ptr> services = instanceServices() + parentComponentsServices( kcdparents );
	createDialogFromServices( services );
}

KConfigureDialog::~KConfigureDialog()
{
	delete d;
}

#if 0
void KConfigureDialog::addKPartsPluginPage()
{
	kdDebug( 700 ) << k_funcinfo << endl;
	QValueList<KService::Ptr> services = KTrader::self()->query( "KCModule", "Library == 'kpartspluginpage'" );
	kdDebug( 700 ) << "got " << services.count() << " offers" << endl;
	for( QValueList<KService::Ptr>::ConstIterator it = services.begin(); it != services.end(); ++it )
	{
		kdDebug( 700 ) << k_funcinfo << ( *it )->type() << ( *it )->name() << ( *it )->library() << endl;
	}
	KService::Ptr service = services.first();
	if( service.data() )
	{
		KCModuleInfo * moduleinfo = new KCModuleInfo( service );
		d->moduleinfos.append( moduleinfo );
		d->dlg->addModule( *moduleinfo );
	}
}
#endif

void KConfigureDialog::show()
{
	KCDDispatcher::self()->syncConfiguration();
	return d->dlg->show();
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

void KConfigureDialog::createDialogFromServices( const QValueList<KService::Ptr> & services )
{
	int dialogface = KJanusWidget::IconList;
	QString basegroup;
	for( QValueList<KService::Ptr>::const_iterator it = services.begin(); it != services.end(); ++it )
	{
		KCModuleInfo * moduleinfo = new KCModuleInfo( *it );
		d->moduleinfos.append( moduleinfo );
		if( basegroup.isNull() )
			basegroup = moduleinfo->groups().front();
		else if( basegroup != moduleinfo->groups().front() )
			// we need a treelist dialog
			dialogface = KJanusWidget::TreeList;
	}

	kdDebug( 700 ) << "creating KCMultiDialog" << endl;
	d->dlg = new KCMultiDialog( dialogface, i18n( "Preferences" ) );
	if( dialogface == KJanusWidget::TreeList )
	{
		d->dlg->setShowIconsInTreeList( true );
		d->dlg->unfoldTreeList( true );
	}
	connect( d->dlg, SIGNAL( okClicked() ), KCDDispatcher::self(), SLOT( syncConfiguration() ) );
	connect( d->dlg, SIGNAL( applyClicked() ), KCDDispatcher::self(), SLOT( syncConfiguration() ) );
	connect( d->dlg, SIGNAL( configCommitted( const QCString & ) ), KCDDispatcher::self(), SLOT( reparseConfiguration( const QCString & ) ) );
	for( KCModuleInfo * info = d->moduleinfos.first(); info; info = d->moduleinfos.next() )
	{
		kdDebug( 700 ) << "add module: " << info->fileName() << " with ParentComponents=" << info->parentComponents() << endl;
		d->dlg->addModule( *info );
	}
}

#include "kconfiguredialog.moc"

// vim: sw=4 ts=4
