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

#include "kcmoduleproxy.h"
#include "kcmoduleproxy_p.h"

#include <QApplication>
#include <QCursor>
#include <QtCore/QDataStream>
#include <QKeyEvent>
#include <QtCore/QFileInfo>
#include <QFrame>
#include <QLabel>
#include <QLayout>
#include <QtCore/QPoint>

#include <QImage>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusReply>

#include <kaboutdata.h>
#include <kcmodule.h>
#include <kcmoduleinfo.h>

#include <QDebug>
#include <klocalizedstring.h>
#include <kservice.h>

#include <kuser.h>

#include <kcmoduleloader.h>

#include "kcolorscheme.h"

#include "ksettingswidgetadaptor.h"

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
    Q_D(const KCModuleProxy);
	/*
	 * Note, don't call any function that calls realModule() since
	 * that leads to an infinite loop.
	 */

	/* Already loaded */
	if( !d->kcm )
	{
		QApplication::setOverrideCursor( Qt::WaitCursor );
        const_cast<KCModuleProxyPrivate *>(d)->loadModule();
		QApplication::restoreOverrideCursor();
	}
	return d->kcm;
}

void KCModuleProxyPrivate::loadModule()
{
	if( !topLayout )
	{
		topLayout = new QVBoxLayout( parent );
		topLayout->setMargin( 0 );

		QString name = modInfo.handle();
		name.replace(QLatin1Char('-'),QLatin1Char('_')); //hyphen is not allowed in dbus, only [A-Z][a-z][0-9]_
		dbusPath = QLatin1String("/internal/KSettingsWidget/") + name;
		dbusService = QLatin1String("org.kde.internal.KSettingsWidget_") + name;
	}

	if( QDBusConnection::sessionBus().registerService( dbusService ) || bogusOccupier )
	{ /* We got the name we requested, because no one was before us,
	   * or, it was an random application which had picked that name */
		// qDebug() << "Module not already loaded, loading module " << modInfo.moduleName() << " from library " << modInfo.library() << " using symbol " << modInfo.handle();

		kcm = KCModuleLoader::loadModule( modInfo, KCModuleLoader::Inline, parent, args );

        QObject::connect(kcm, SIGNAL(changed(bool)), parent, SLOT(_k_moduleChanged(bool)));
        QObject::connect(kcm, SIGNAL(destroyed()), parent, SLOT(_k_moduleDestroyed()));
		QObject::connect( kcm, SIGNAL(quickHelpChanged()), parent, SIGNAL(quickHelpChanged()) );
		parent->setWhatsThis( kcm->quickHelp() );

		if ( kcm->layout() ) {
			kcm->layout()->setMargin( 0 );
		}
		topLayout->addWidget( kcm );
		if( !modInfo.handle().isEmpty() )
			QDBusConnection::sessionBus().registerObject(dbusPath, new KSettingsWidgetAdaptor(parent), QDBusConnection::ExportAllSlots);

		if ( !rootInfo && /* If it's not already done */
				kcm->useRootOnlyMessage() && /* kcm wants root message */
				!KUser().isSuperUser() ) /* Not necessary if we're root */
		{
			/*rootInfo = new QLabel( parent );
			topLayout->insertWidget( 0, rootInfo );

			QPalette palette = rootInfo->palette();
			KStatefulBrush stbrush(KColorScheme::Window, KColorScheme::NeutralBackground);
			qDebug() << stbrush.brush(rootInfo);
			palette.setBrush(QPalette::Window, stbrush.brush(rootInfo));
			rootInfo->setPalette(palette);
			rootInfo->setAutoFillBackground(true);

			const QString message = kcm->rootOnlyMessage();
			if( message.isEmpty() )
				rootInfo->setText( i18n(
   					  "<b>Changes in this section require root access.</b><br />"
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
				  "module cannot be saved " ) );*/
		}
	}
	else
	{
		// qDebug() << "Module already loaded, loading KCMError";

		/* Figure out the name of where the module is already loaded */
		QDBusInterface proxy( dbusService, dbusPath, QStringLiteral("org.kde.internal.KSettingsWidget"));
        QDBusReply<QString> reply = proxy.call(QStringLiteral("applicationName"));

		if( reply.isValid() )
		{
			QObject::connect( QDBusConnection::sessionBus().interface(), SIGNAL(serviceOwnerChanged(QString,QString,QString)),
                    parent, SLOT(_k_ownerChanged(QString,QString,QString)));
			kcm = KCModuleLoader::reportError( KCModuleLoader::Inline,
					i18nc( "Argument is application name", "This configuration section is "
						"already opened in %1" ,  reply.value() ), QStringLiteral(" "), parent );
			topLayout->addWidget( kcm );
		}
		else
		{
			// qDebug() << "Calling KCModuleProxy's DBus interface for fetching the name failed.";
			bogusOccupier = true;
			loadModule();
		}
	}
}

void KCModuleProxyPrivate::_k_ownerChanged(const QString &service, const QString &oldOwner, const QString &)
{
    if (service == dbusService && !oldOwner.isEmpty()) {
        // Violence: Get rid of KCMError & CO, so that
        // realModule() attempts to reload the module
        delete kcm;
        kcm = 0;
        Q_Q(KCModuleProxy);
        q->realModule();

        Q_ASSERT(kcm);
        kcm->show();
    }
}

void KCModuleProxy::showEvent( QShowEvent * ev )
{
    Q_D(KCModuleProxy);

	( void )realModule();

	/* We have no kcm, if we're in root mode */
	if( d->kcm ) {
		d->kcm->showEvent(ev);
    }

	QWidget::showEvent( ev );

}

KCModuleProxy::~KCModuleProxy()
{
	deleteClient();
	KCModuleLoader::unloadModule(moduleInfo());

	delete d_ptr;
}

void KCModuleProxy::deleteClient()
{
    Q_D(KCModuleProxy);
	delete d->kcm;
	d->kcm = 0;
}

void KCModuleProxyPrivate::_k_moduleChanged(bool c)
{
    if(changed == c) {
        return;
    }

    Q_Q(KCModuleProxy);
    changed = c;
    emit q->changed(c);
    emit q->changed(q);
}

void KCModuleProxyPrivate::_k_moduleDestroyed()
{
    kcm = 0;
}

KCModuleProxy::KCModuleProxy( const KService::Ptr& service, QWidget * parent,
		const QStringList& args )
    : QWidget(parent), d_ptr(new KCModuleProxyPrivate(this, KCModuleInfo(service), args))
{
    d_ptr->q_ptr = this;
}

KCModuleProxy::KCModuleProxy( const KCModuleInfo& info, QWidget * parent,
		const QStringList& args )
    : QWidget(parent), d_ptr(new KCModuleProxyPrivate(this, info, args))
{
    d_ptr->q_ptr = this;
}

KCModuleProxy::KCModuleProxy( const QString& serviceName, QWidget * parent,
		const QStringList& args )
    : QWidget(parent), d_ptr(new KCModuleProxyPrivate(this, KCModuleInfo(serviceName), args))
{
    d_ptr->q_ptr = this;
}


void KCModuleProxy::load()
{
    Q_D(KCModuleProxy);
	if( realModule() )
	{
		d->kcm->load();
        d->_k_moduleChanged(false);
	}
}

void KCModuleProxy::save()
{
    Q_D(KCModuleProxy);
	if( d->changed && realModule() )
	{
		d->kcm->save();
        d->_k_moduleChanged(false);
	}
}

void KCModuleProxy::defaults()
{
    Q_D(KCModuleProxy);
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

bool KCModuleProxy::changed() const
{
    Q_D(const KCModuleProxy);
	return d->changed;
}

KCModuleInfo KCModuleProxy::moduleInfo() const
{
    Q_D(const KCModuleProxy);
	return d->modInfo;
}

QString KCModuleProxy::dbusService() const
{
    Q_D(const KCModuleProxy);
	return d->dbusService;
}

QString KCModuleProxy::dbusPath() const
{
    Q_D(const KCModuleProxy);
	return d->dbusPath;
}

QSize KCModuleProxy::minimumSizeHint() const
{
	return QWidget::minimumSizeHint();
}

//X void KCModuleProxy::emitQuickHelpChanged()
//X {
//X     emit quickHelpChanged();
//X }

/***************************************************************/
#include "moc_kcmoduleproxy.cpp"

// vim: ts=4
