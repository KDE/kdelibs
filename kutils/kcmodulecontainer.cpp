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

#include <qlayout.h>
#include <qpixmap.h>
#include <qstringlist.h>
#include <qtabwidget.h>
#include <qtooltip.h>
#include <q3valuelist.h>

#include <kcmodule.h>
#include <kcmoduleinfo.h>
#include <kcmoduleloader.h>
#include <kcmoduleproxy.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <kservice.h>
#include <kstdguiitem.h>

#include "kcmodulecontainer.h"
#include "kcmodulecontainer.moc"

/***********************************************************************/
class KCModuleContainer::KCModuleContainerPrivate
{
	public:
		KCModuleContainerPrivate( const QStringList& mods )
			: modules( mods )
			, tabWidget( 0 )
			, buttons( 0 )
			, hasRootKCM( false )
			, btnRootMode( 0 )
			, btnLayout( 0 )
			, topLayout( 0 )
			{};

		QStringList modules;
		QTabWidget *tabWidget;
		int buttons;
		bool hasRootKCM: 1;
		KPushButton *btnRootMode;
		QHBoxLayout *btnLayout;
		QVBoxLayout *topLayout;


};
/***********************************************************************/




#warning the old code created an instance (through the constructor) - should this stay?

/***********************************************************************/
KCModuleContainer::KCModuleContainer( QWidget* parent, const QString& mods )
	: KCModule( KGlobal::instance(), parent )
{
	d = new KCModuleContainerPrivate( QStringList::split( ",", QString(mods).remove( " " )) );
	init();
}

KCModuleContainer::KCModuleContainer( QWidget* parent, const QStringList& mods )
	: KCModule( KGlobal::instance(), parent ), d( new KCModuleContainerPrivate( mods ) )
{
	init();
}

void KCModuleContainer::init()
{
	d->topLayout = new QVBoxLayout( this, 0, KDialog::spacingHint(), "topLayout" );
	d->tabWidget = new QTabWidget(this, "tabWidget");
	d->tabWidget->setMargin(KDialog::marginHint());
	connect( d->tabWidget, SIGNAL( currentChanged( QWidget* ) ), SLOT( tabSwitched( QWidget* ) ));
	d->topLayout->addWidget( d->tabWidget );

	if ( !d->modules.isEmpty() )
	{
		/* Add our modules */
		for ( QStringList::Iterator it = d->modules.begin(); it != d->modules.end(); ++it )
			addModule( (*it) );

		finalize();
	}

}

void KCModuleContainer::finalize()
{
	setButtons( d->buttons );
	if ( d->hasRootKCM ) /* Add a root mode button */
	{
		if(!d->btnLayout) /* It could already be added */
		{
			d->btnLayout = new QHBoxLayout(this, 0, 0, "btnLayout");
			d->btnRootMode = new KPushButton(KStdGuiItem::adminMode(), this);
                        d->btnRootMode->setObjectName("btnRootMode");
					
			d->btnLayout->addWidget( d->btnRootMode );
			d->btnLayout->addStretch();
			d->topLayout->addLayout( d->btnLayout );
		}
	}
}

void KCModuleContainer::addModule( const QString& module )
{
	/* In case it doesn't exist we just silently drop it.
	 * This allows people to easily extend containers.
	 * For example, KCM monitor gamma can be in kdegraphics.
	 */
	if ( !KService::serviceByDesktopName( module ) )
	{
		kdDebug(713) << "KCModuleContainer: module '" << 
			module << "' was not found and thus not loaded" << endl;
		return;
	}

	if( !KCModuleLoader::testModule( module ))
		return;

	KCModuleProxy* proxy = new KCModuleProxy( module, false, d->tabWidget, module.toLatin1());
	allModules.append( proxy );

	d->tabWidget->addTab( proxy, QIcon(KGlobal::iconLoader()->loadIcon(
					proxy->moduleInfo().icon(), KIcon::Desktop)),
			/* QT eats ampersands for dinner. But not this time. */
			proxy->moduleInfo().moduleName().replace( "&", "&&" ));

	d->tabWidget->setTabToolTip( proxy, proxy->moduleInfo().comment() );

	connect( proxy, SIGNAL(changed(KCModuleProxy *)), SLOT(moduleChanged(KCModuleProxy *)));

	/* Collect our buttons - we go for the common deliminator */
	d->buttons = d->buttons | proxy->realModule()->buttons();

	/* If we should add an Administrator Mode button */
	if ( proxy->moduleInfo().needsRootPrivileges() )
		d->hasRootKCM=true;


}

void KCModuleContainer::tabSwitched( QWidget * module )
{
	if ( !d->hasRootKCM )
		return;

	/* Not like this. Not like this. */
	disconnect( d->btnRootMode, 0, 0, 0 );
	/* Welcome to the real world huh baby? */
	
	KCModuleProxy* mod = (KCModuleProxy *) module;

	if ( mod->moduleInfo().needsRootPrivileges() && !mod->rootMode() )
	{
		d->btnRootMode->setEnabled( true );
		connect( d->btnRootMode, SIGNAL( clicked() ), 
				SLOT( runAsRoot() ));
		connect( mod, SIGNAL( childClosed() ), 
				SLOT ( rootExited() ));
	}
	else
		d->btnRootMode->setEnabled( false );

	setQuickHelp( mod->quickHelp() );
	setAboutData( const_cast<KAboutData*>(mod->aboutData()) );

}

void KCModuleContainer::runAsRoot()
{
	if ( d->tabWidget->currentPage() )
		( (KCModuleProxy *) d->tabWidget->currentPage() )->runAsRoot();
	d->btnRootMode->setEnabled( false );
}

void KCModuleContainer::rootExited()
{
	connect( d->btnRootMode, SIGNAL( clicked() ), SLOT( runAsRoot() ));
	d->btnRootMode->setEnabled( true );
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




