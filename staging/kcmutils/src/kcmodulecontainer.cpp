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

#include <QLayout>
#include <QPixmap>
#include <QtCore/QStringList>
#include <QTabWidget>

#include <kcmodule.h>
#include <kcmoduleinfo.h>
#include <kcmoduleproxy.h>
#include <QDebug>
#include <kservice.h>

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
		QTabWidget *tabWidget;
		KCModule::Buttons buttons;
		QVBoxLayout *topLayout;


};
/***********************************************************************/



// The KCModuleContainer is only a wrapper around real KCModules.
/***********************************************************************/
KCModuleContainer::KCModuleContainer( QWidget* parent, const QString& mods )
    : KCModule( parent ),
      d(new KCModuleContainerPrivate( QString(mods).remove( QLatin1Char(' ') ).split( QLatin1Char(','), QString::SkipEmptyParts ) ))
{
	init();
}

KCModuleContainer::KCModuleContainer( QWidget* parent, const QStringList& mods )
    : KCModule( parent ),
      d( new KCModuleContainerPrivate( mods ) )
{
	init();
}

void KCModuleContainer::init()
{
	d->topLayout = new QVBoxLayout( this );
	d->topLayout->setMargin( 0 );
	d->topLayout->setObjectName( QStringLiteral("topLayout"));
	d->tabWidget = new QTabWidget(this);
	d->tabWidget->setObjectName( QStringLiteral("tabWidget"));
	connect( d->tabWidget, SIGNAL(currentChanged(int)), SLOT(tabSwitched(int)));
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
		// qDebug() << "KCModuleContainer: module '" <<
			// module << "' was not found and thus not loaded" << endl;
		return;
	}

	if ( service->noDisplay() )
		return;

	KCModuleProxy* proxy = new KCModuleProxy( service, d->tabWidget );
	allModules.append( proxy );

	proxy->setObjectName(module);

	d->tabWidget->addTab( proxy, QIcon::fromTheme( proxy->moduleInfo().icon() ),
			/* Qt eats ampersands for dinner. But not this time. */
			proxy->moduleInfo().moduleName().replace( QLatin1Char('&'), QStringLiteral("&&") ));

	d->tabWidget->setTabToolTip( d->tabWidget->indexOf( proxy ), proxy->moduleInfo().comment() );

	connect( proxy, SIGNAL(changed(KCModuleProxy*)), SLOT(moduleChanged(KCModuleProxy*)));

	/* Collect our buttons - we go for the common deliminator */
	setButtons( buttons() | proxy->realModule()->buttons() );
}

void KCModuleContainer::tabSwitched(int index)
{
	KCModuleProxy* mod = static_cast<KCModuleProxy *>(d->tabWidget->widget(index));
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




