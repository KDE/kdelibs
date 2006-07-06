/*  This file is part of the KDE project
    Copyright (C) 2004 Frans Englich <frans.englich@telia.com>
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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <qapplication.h>
#include <qcursor.h>
#include <qdatastream.h>
#include <qevent.h>
#include <qfileinfo.h>
#include <qframe.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpoint.h>
#include <qtextstream.h>

#include <qwidget.h>

#include <QtDBus/QtDBus>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmodule.h>
#include <kcmoduleinfo.h>

#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <kprocess.h>
#include <kservice.h>
#include <kstandarddirs.h>
#include <kuser.h>

#include <kvbox.h>

#include <kcmoduleloader.h>

#include "kcmoduleproxy.h"
#include "ksettingswidgetadaptor.h"

/***************************************************************/
class KCModuleProxy::Private
{
	public:
		Private( const KCModuleInfo & info, KCModuleProxy* _parent )
			: kcm( 0 )
			//, view( 0 )
			, topLayout( 0 )
			, rootInfo( 0 )
			, modInfo( info )
			, changed( false )
			, bogusOccupier( false )
			, parent( _parent )
		{}

		~Private()
		{
			delete rootInfo; // Delete before embedWidget!
			delete kcm;
		}

		void loadModule();

		QStringList args;
		KCModule *kcm;
		QVBoxLayout *topLayout; /* Contains QScrollView view, and root stuff */
		QLabel *rootInfo;
		QString dbusService;
		QString dbusPath;
		KCModuleInfo modInfo;
		bool changed;
		bool bogusOccupier;
		KCModuleProxy* parent;
};
/***************************************************************/



/*
 TODO:

 - Two Layout problems in runAsRoot:
 	* lblBusy doesn't show
 	* d->kcm/d->rootInfo doesn't get it right when the user
		presses cancel in the kdesu dialog

 - Resizing horizontally is contrained; minimum size is set somewhere.
 	It appears to be somehow derived from the module's size.

 - Prettify: set icon in KCMultiDialog.

 */
/***************************************************************/
KCModule* KCModuleProxy::realModule() const
{
	/*
	 * Note, don't call any function that calls realModule() since
	 * that leads to an infinite loop.
	 */

	/* Already loaded */
	if( !d->kcm )
	{
		QApplication::setOverrideCursor( Qt::WaitCursor );
		d->loadModule();
		QApplication::restoreOverrideCursor();
	}
	return d->kcm;
}

void KCModuleProxy::Private::loadModule()
{
	if( !topLayout )
	{
		topLayout = new QVBoxLayout( parent );
		topLayout->setMargin( 0 );
		topLayout->setSpacing( 0 );

		QString name = modInfo.handle();
		dbusPath = QLatin1String("/internal/KSettingsWidget/") + name;
		dbusService = QLatin1String("org.kde.internal.KSettingsWidget-") + name;
	}

	if( QDBus::sessionBus().registerService( dbusService ) || bogusOccupier )
	{ /* We got the name we requested, because no one was before us,
	   * or, it was an random application which had picked that name */
		kDebug(711) << "Module not already loaded, loading module " << modInfo.moduleName() << " from library " << modInfo.library() << " using symbol " << modInfo.handle() << endl;

		kcm = KCModuleLoader::loadModule( modInfo, KCModuleLoader::Inline, parent, args );

		QObject::connect( kcm, SIGNAL(changed(bool)), parent, SLOT(moduleChanged(bool)) );
		QObject::connect( kcm, SIGNAL(destroyed()), parent, SLOT(moduleDestroyed()) );
		QObject::connect( kcm, SIGNAL(quickHelpChanged()), parent, SIGNAL(quickHelpChanged()) );
		parent->setWhatsThis( kcm->quickHelp() );

		topLayout->addWidget( kcm );
		//QDBus::sessionBus().registerObject( dbusPath, parent, QDBusConnection::ExportSlots );
		QDBus::sessionBus().registerObject( dbusPath, new KSettingsWidgetAdaptor( parent ) );

		if ( !rootInfo && /* If it's not already done */
				kcm->useRootOnlyMessage() && /* kcm wants root message */
				!KUser().isSuperUser() ) /* Not necessary if we're root */
		{
			rootInfo = new QLabel( parent );
			topLayout->insertWidget( 0, rootInfo );

			rootInfo->setFrameShape( QFrame::Box );
			rootInfo->setFrameShadow( QFrame::Raised );

			const QString message = kcm->rootOnlyMessage();
			if( message.isEmpty() )
				rootInfo->setText( i18n(
   					  "<b>Changes in this section requires root access.</b><br />"
					  "On applying your changes you will have to supply your root "
					  "password." ) );
			else
				rootInfo->setText(message);

			rootInfo->setWhatsThis( i18n(
				  "This section requires special permissions, probably "
				  "for system-wide changes; therefore, it is "
				  "required that you provide the root password to be "
				  "able to change the module's properties. If "
				  "you cannot provide the password, the changes of the "
				  "module cannot be saved " ) );
		}
	}
	else
	{
		kDebug(711) << "Module already loaded, loading KCMError" << endl;

		/* Figure out the name of where the module is already loaded */
		QDBusInterface proxy( dbusService, dbusPath, "org.kde.internal.KSettingsWidget" );
		QDBusReply<QString> reply = proxy.call("applicationName");

		if( reply.isValid() )
		{
			QObject::connect( QDBus::sessionBus().interface(), SIGNAL(serviceOwnerChanged(QString,QString,QString)),
					parent, SLOT(ownerChanged(QString,QString,QString)) );
			kcm = KCModuleLoader::reportError( KCModuleLoader::Inline,
					i18nc( "Argument is application name", "This configuration section is "
						"already opened in %1" ,  reply.value() ), " ", parent );
			topLayout->addWidget( kcm );
		}
		else
		{
			kDebug(711) << "Calling KCModuleProxy's DBus interface for fetching the name failed." << endl;
			bogusOccupier = true;
			loadModule();
		}
	}
}

void KCModuleProxy::ownerChanged( const QString & service, const QString &oldOwner, const QString &)
{
	if( service == d->dbusService && !oldOwner.isEmpty() )
	{
		/* Violence: Get rid of KCMError & CO, so that
		 * realModule() attempts to reload the module */
		delete d->kcm;
		d->kcm = 0;
		realModule();
		d->kcm->show();
	}
}

void KCModuleProxy::showEvent( QShowEvent * ev )
{

	kDebug(711) << k_funcinfo << endl;
	( void )realModule();

	/* We have no kcm, if we're in root mode */
	if( d->kcm )
		d->kcm->show();

	QWidget::showEvent( ev );

}

KCModuleProxy::~KCModuleProxy()
{
	deleteClient();
	KCModuleLoader::unloadModule(moduleInfo());

	delete d;
}

void KCModuleProxy::deleteClient()
{
	delete d->kcm;
	d->kcm = 0;

	kapp->syncX();
}

void KCModuleProxy::moduleChanged( bool c )
{
	if(  d->changed == c )
		return;

	d->changed = c;
	emit changed( c );
	emit changed( this );
}

void KCModuleProxy::moduleDestroyed()
{
	d->kcm = 0;
}

KCModuleProxy::KCModuleProxy( const KService::Ptr& service, QWidget * parent,
		const QStringList& args )
	: QWidget( parent )
{
	init( KCModuleInfo( service ));
	d->args = args;
}

KCModuleProxy::KCModuleProxy( const KCModuleInfo& info, QWidget * parent,
		const QStringList& args )
	: QWidget( parent )
{
	init( info );
	d->args = args;
}

KCModuleProxy::KCModuleProxy( const QString& serviceName, QWidget * parent,
		const QStringList& args )
	: QWidget( parent )
{
	init( KCModuleInfo( serviceName ));
	d->args = args;
}

void KCModuleProxy::init( const KCModuleInfo& info )
{
	kDebug(711) << k_funcinfo << endl;

	d = new Private( info, this );

	/* This is all we do for now; all the heavy work is
	 * done in realModule(). It's called when the module
	 * _actually_ is needed, in for example showEvent().
	 * The module is loaded "on demand" -- lazy loading.
	 */

}

void KCModuleProxy::load()
{

	if( realModule() )
	{
		d->kcm->load();
		moduleChanged( false );
	}
}

void KCModuleProxy::save()
{
	if( d->changed && realModule() )
	{
		d->kcm->save();
		moduleChanged( false );
	}
}

void KCModuleProxy::defaults()
{
	if( realModule() )
		d->kcm->defaults();
}

QString KCModuleProxy::quickHelp() const
{
	return realModule() ? realModule()->quickHelp() : QString();
}

const KAboutData * KCModuleProxy::aboutData() const
{
	return realModule() ? realModule()->aboutData() : 0;
}

KCModule::Buttons KCModuleProxy::buttons() const
{
	if( realModule() )
		return realModule()->buttons();
	return KCModule::Buttons( KCModule::Help | KCModule::Default | KCModule::Apply );
}

QString KCModuleProxy::rootOnlyMessage() const
{
	return realModule() ? realModule()->rootOnlyMessage() : QString();
}

bool KCModuleProxy::useRootOnlyMessage() const
{
	return realModule() ? realModule()->useRootOnlyMessage() : true;
}

KInstance * KCModuleProxy::instance() const
{
	return realModule() ? realModule()->instance() : 0;
}

bool KCModuleProxy::changed() const
{
	return d->changed;
}

const KCModuleInfo& KCModuleProxy::moduleInfo() const
{
	return d->modInfo;
}

QString KCModuleProxy::dbusService() const
{
	return d->dbusService;
}

QString KCModuleProxy::dbusPath() const
{
	return d->dbusPath;
}

void KCModuleProxy::emitQuickHelpChanged()
{
	emit quickHelpChanged();
}

/***************************************************************/
#include "kcmoduleproxy.moc"

// vim: sw=4 ts=4 noet
