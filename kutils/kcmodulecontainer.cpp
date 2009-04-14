/* This file is part of the KDE libraries
    Copyright (C) 2004 Frans Englich <frans.englich@telia.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kcmodulecontainer.h"
#include "kcmodulecontainer.moc"

#include <QtGui/QLayout>
#include <QtGui/QPixmap>
#include <QtCore/QStringList>

#include <kcmodule.h>
#include <kcmoduleinfo.h>
#include <kcmoduleloader.h>
#include <kcmoduleproxy.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kguiitem.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <kservice.h>
#include <kstandardguiitem.h>
#include <ktabwidget.h>

/***********************************************************************/
class KCModuleContainer::KCModuleContainerPrivate
{
	public:
		KCModuleContainerPrivate( const QStringList& mods )
			: modules( mods )
			, tabWidget( 0 )
			, topLayout( 0 )
			{}

		QStringList modules;
		KTabWidget *tabWidget;
		KCModule::Buttons buttons;
		QVBoxLayout *topLayout;


};
/***********************************************************************/



// The KCModuleContainer is only a wrapper around real KCModules. Therefore it doesn't need a
// special KComponentData and can just use the global instance. The contained KCModules create their own
// KComponentData objects when needed.
/***********************************************************************/
KCModuleContainer::KCModuleContainer( QWidget* parent, const QString& mods )
	: KCModule( KGlobal::mainComponent(), parent ),d(new KCModuleContainerPrivate( QString(mods).remove( ' ' ).split( ',' ) ))
{
	init();
}

KCModuleContainer::KCModuleContainer( QWidget* parent, const QStringList& mods )
	: KCModule( KGlobal::mainComponent(), parent ), d( new KCModuleContainerPrivate( mods ) )
{
	init();
}

void KCModuleContainer::init()
{
	d->topLayout = new QVBoxLayout( this );
	d->topLayout->setMargin( 0 );
	d->topLayout->setObjectName( "topLayout" );
	d->tabWidget = new KTabWidget(this);
	d->tabWidget->setObjectName( "tabWidget");
	connect( d->tabWidget, SIGNAL( currentChanged( QWidget* ) ), SLOT( tabSwitched( QWidget* ) ));
	d->topLayout->addWidget( d->tabWidget );

	if ( !d->modules.isEmpty() )
	{
		/* Add our modules */
		for ( QStringList::const_iterator it = d->modules.constBegin(); it != d->modules.constEnd(); ++it )
			addModule( (*it) );
	}
}

void KCModuleContainer::addModule( const QString& module )
{
	/* In case it doesn't exist we just silently drop it.
	 * This allows people to easily extend containers.
	 * For example, KCM monitor gamma can be in kdegraphics.
	 */
	KService::Ptr service = KService::serviceByDesktopName( module );
	if ( !service )
	{
		kDebug(713) << "KCModuleContainer: module '" << 
			module << "' was not found and thus not loaded" << endl;
		return;
	}

	if ( service->noDisplay() )
		return;

	KCModuleProxy* proxy = new KCModuleProxy( service, d->tabWidget );
	allModules.append( proxy );

	proxy->setObjectName( module.toLatin1() );

	d->tabWidget->addTab( proxy, KIcon( proxy->moduleInfo().icon() ),
			/* Qt eats ampersands for dinner. But not this time. */
			proxy->moduleInfo().moduleName().replace( '&', "&&" ));

	d->tabWidget->setTabToolTip( d->tabWidget->indexOf( proxy ), proxy->moduleInfo().comment() );

	connect( proxy, SIGNAL(changed(KCModuleProxy *)), SLOT(moduleChanged(KCModuleProxy *)));

	/* Collect our buttons - we go for the common deliminator */
	setButtons( buttons() | proxy->realModule()->buttons() );
}

void KCModuleContainer::tabSwitched( QWidget * module )
{
	KCModuleProxy* mod = (KCModuleProxy *) module;
	setQuickHelp( mod->quickHelp() );
	setAboutData( mod->aboutData() );
}

void KCModuleContainer::save()
{
	ModuleList list = changedModules;
	ModuleList::iterator it;
	for ( it = list.begin() ; it !=list.end() ; ++it )
	{
		(*it)->save();
	}

	emit changed( false );

}

void KCModuleContainer::load()
{
	ModuleList list = allModules;
	ModuleList::iterator it;
	for ( it = list.begin() ; it !=list.end() ; ++it )
	{
		(*it)->load();
	}

	emit changed( false );
}

void KCModuleContainer::defaults()
{
	ModuleList list = allModules;
	ModuleList::iterator it;
	for ( it = list.begin() ; it !=list.end() ; ++it )
	{
		(*it)->defaults();
	}

	emit changed( true );
}


void KCModuleContainer::moduleChanged(KCModuleProxy * proxy)
{
	changedModules.append( proxy );
	if( changedModules.isEmpty() )
		return;

	emit changed(true);
}

KCModuleContainer::~KCModuleContainer()
{
	delete d;
}

/***********************************************************************/




