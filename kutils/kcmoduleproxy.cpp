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
#include <q3frame.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpoint.h>
#include <q3scrollview.h>
#include <qtextstream.h>

#include <qwidget.h>

#ifdef Q_WS_X11
#include <QX11EmbedWidget>
#include <qx11info_x11.h>
#include <X11/Xlib.h>
#endif 

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

#include <fixx11h.h>
#include <kcmoduleloader.h>

#include "kcmoduleproxy.h"
#include "kcmoduleproxyIface.h"
#include "kcmoduleproxyIfaceImpl.h"

/***************************************************************/
class KCModuleProxy::KCModuleProxyPrivate
{
	public:
		KCModuleProxyPrivate( const KCModuleInfo & info )
			: args( 0 )
			, kcm( 0 )
			//, view( 0 )
			, embedWidget( 0 )
			, rootProcess ( 0 )
			, embedFrame ( 0 )
			, dcopObject( 0 )
			, dcopClient( 0 )
			, topLayout( 0 )
			, rootCommunicator( 0 )
			, rootInfo( 0 )
			, modInfo( info )
			, withFallback( false )
			, changed( false )
			, rootMode( false )
			, bogusOccupier( false )
			, isInitialized( false )
		{}

		~KCModuleProxyPrivate()
		{
			delete rootInfo; // Delete before embedWidget!
			delete embedWidget; // Delete before embedFrame!
			delete embedFrame;
			delete dcopClient;
			delete dcopObject;
			delete rootCommunicator;
			delete rootProcess;
			delete kcm;
		}

		QStringList							args;
		KCModule							*kcm;
		QX11EmbedWidget							*embedWidget;
		KProcess							*rootProcess;
		KVBox								*embedFrame;
		KCModuleProxyIfaceImpl  			*dcopObject;
		DCOPClient							*dcopClient;
		QVBoxLayout							*topLayout; /* Contains QScrollView view, and root stuff */
		KCModuleProxyRootCommunicatorImpl	*rootCommunicator;
		QLabel								*rootInfo;
		DCOPCString							dcopName;
		KCModuleInfo 						modInfo;
		bool 								withFallback;
		bool 								changed;
		bool 								rootMode;
		bool								bogusOccupier;
		bool								isInitialized;
};
/***************************************************************/



/*
 TODO:

 - How KCModuleProxy behaves wrt memory leaks and behavior, when exiting
 	from root mode is not tested, because no code make use of it. It needs
	work, if it should be used.

 - Should write a document which outlines test cases, to avoid
 	regressions. This class is a hazard.

 - Two Layout problems in runAsRoot:
 	* lblBusy doesn't show
 	* d->kcm/d->rootInfo doesn't get it right when the user
		presses cancel in the kdesu dialog

 - Resizing horizontally is contrained; minimum size is set somewhere.
 	It appears to be somehow derived from the module's size.

 - Prettify: set icon in KCMultiDialog.

 - Perhaps it's possible to link against kdesu such that
 	the dialog is in process?

 */
/***************************************************************/
KCModule * KCModuleProxy::realModule() const
{

	/*
	 * Note, don't call any function that calls realModule() since
	 * that leads to an infinite loop.
	 */

	kdDebug(711) << k_funcinfo << endl;

	/* Already loaded */
	if( d->kcm )
		return d->kcm;

	/* /We/ have no kcm, but kcmshell running with root prevs does.. */
	if( d->rootMode )
		return 0;

	QApplication::setOverrideCursor( Qt::WaitCursor );

	KCModuleProxy * that = const_cast<KCModuleProxy*>( this );

	if( !d->isInitialized )
	{
  		d->dcopName = moduleInfo().handle().prepend("KCModuleProxy-").toUtf8();
		d->topLayout = new QVBoxLayout( that, 0, 0, "topLayout" );

		d->isInitialized = true;
	}

	if( !d->dcopClient )
		d->dcopClient = new DCOPClient();

	if( !d->dcopClient->isRegistered() )
  		d->dcopClient->registerAs( d->dcopName, false );

	d->dcopClient->setAcceptCalls( true );

	if( d->dcopClient->appId() == d->dcopName || d->bogusOccupier )
	{ /* We got the name we requested, because no one was before us,
	   * or, it was an random application which had picked that name */
		kdDebug(711) << "Module not already loaded, loading module" << endl;

		d->dcopObject = new KCModuleProxyIfaceImpl( d->dcopName, that );

		d->kcm = KCModuleLoader::loadModule( moduleInfo(), KCModuleLoader::Inline, d->withFallback,
			that, name(), d->args );

		connect( d->kcm, SIGNAL( changed( bool ) ),
				SLOT(moduleChanged(bool)) );
		connect( d->kcm, SIGNAL( destroyed() ),
				SLOT( moduleDestroyed() ) );
		connect( d->kcm, SIGNAL(quickHelpChanged()),
				SIGNAL(quickHelpChanged()));
		that->setWhatsThis(d->kcm->quickHelp() );

		d->topLayout->addWidget( d->kcm );

		if ( !d->rootInfo && /* If it's already done */
				moduleInfo().needsRootPrivileges() /* root, anyone? */ &&
				!KUser().isSuperUser() ) /* Not necessary if we're root */
		{

			d->rootInfo = new QLabel( that, "rootInfo" );
			d->topLayout->insertWidget( 0, d->rootInfo );

			d->rootInfo->setFrameShape(Q3Frame::Box);
			d->rootInfo->setFrameShadow(Q3Frame::Raised);

			const QString msg = d->kcm->rootOnlyMsg();
			if( msg.isEmpty() )
				d->rootInfo->setText(i18n(
   					  "<b>Changes in this section requires root access.</b><br />"
					  "Click the \"Administrator Mode\" button to "
					  "allow modifications."));
			else
				d->rootInfo->setText(msg);

			d->rootInfo->setWhatsThis(i18n(
				  "This section requires special permissions, probably "
				  "for system-wide changes; therefore, it is "
				  "required that you provide the root password to be "
				  "able to change the module's properties. If "
				  "you do not provide the password, the module will be "
				  "disabled."));
		}
	}
	else
	{
		kdDebug(711) << "Module already loaded, loading KCMError" << endl;

		d->dcopClient->detach();
 		/* Re-register as anonymous */
		d->dcopClient->attach();

		d->dcopClient->setNotifications( true );
		connect( d->dcopClient, SIGNAL( applicationRemoved( const QByteArray& )),
			SLOT( applicationRemoved( const QByteArray& )));

		/* Figure out the name of where the module is already loaded */
		QByteArray replyData, data;
		DCOPCString replyType;
		QString result;
		QDataStream arg, stream( replyData );

		if( d->dcopClient->call( d->dcopName, d->dcopName, DCOPCString("applicationName()"),
					data, replyType, replyData ))
		{
			stream >> result;

			d->kcm = KCModuleLoader::reportError( KCModuleLoader::Inline,
					i18n( "Argument is application name", "This configuration section is "
						"already opened in %1" ).arg( result ), " ", that );

			d->topLayout->addWidget( d->kcm );
		}
		else
		{
			kdDebug(711) << "Calling KCModuleProxy's DCOP interface for fetching the name failed." << endl;
			d->bogusOccupier = true;
			QApplication::restoreOverrideCursor();
			return realModule();
		}
	}

	QApplication::restoreOverrideCursor();

	return d->kcm;
}

void KCModuleProxy::applicationRemoved( const QByteArray & app )
{
	if( app == d->dcopName )
	{
		/* Violence: Get rid of KCMError & CO, so that
		 * realModule() attempts to reload the module */
		delete d->kcm;
		d->kcm = 0;
		d->dcopClient->setNotifications( false );
		realModule();
		d->kcm->show();
	}
}

void KCModuleProxy::showEvent( QShowEvent * ev )
{

	kdDebug(711) << k_funcinfo << endl;
	( void )realModule();

	/* We have no kcm, if we're in root mode */
	if( d->kcm )
		d->kcm->show();

	QWidget::showEvent( ev );

}

void KCModuleProxy::runAsRoot()
{
	if ( !moduleInfo().needsRootPrivileges() )
		return;

	QApplication::setOverrideCursor( Qt::WaitCursor );

	delete d->rootProcess;
	delete d->embedWidget;
	delete d->embedFrame;

	d->embedFrame = new KVBox( this/*, "embedFrame"*/ );
	d->embedFrame->setFrameStyle( Q3Frame::Box | Q3Frame::Raised );

	QPalette pal( Qt::red );
	pal.setColor( QColorGroup::Background,
		colorGroup().background() );
	d->embedFrame->setPalette( pal );
	d->embedFrame->setLineWidth( 2 );
	d->embedFrame->setMidLineWidth( 2 );
	d->topLayout->addWidget(d->embedFrame,1);

	d->embedWidget = new QX11EmbedWidget( d->embedFrame );

	d->embedFrame->show();

	QLabel *lblBusy = new QLabel(i18n("<big>Loading...</big>"), d->embedWidget, "lblBusy" );
	lblBusy->setTextFormat(Qt::RichText);
	lblBusy->setAlignment(Qt::AlignCenter);
	lblBusy->setGeometry(0,0, d->kcm->width(), d->kcm->height());
	lblBusy->show();

	deleteClient();
	/* The DCOP registration is now gone, and it will occur again when kcmshell soon
	 * registers. Here's a race condition in other words, but how likely is that?
	 *
	 * - It's a user initiated action, which means the user have to do weird stuff, very
	 *   quick.
	 * - If the user _do_ manage to fsck up, the code will recover gracefully, see realModule().
	 *
	 * So no worry. At the end of this function, communication with
	 * the DCOP object is established.
	 */

	/* Prepare the process to run the kcmshell */
	QString cmd = moduleInfo().service()->exec().trimmed();
	if (cmd.left(5) == "kdesu")
	{
		cmd = cmd.remove(0,5).trimmed();

		/* Remove all kdesu switches */
		while( cmd.length() > 1 && cmd[ 0 ] == '-' )
			cmd = cmd.remove( 0, cmd.find( ' ' ) ).trimmed();
	}

	if (cmd.left(8) == "kcmshell")
		cmd = cmd.remove(0,8).trimmed();

	/* Run the process */
	QString kdesu = KStandardDirs::findExe("kdesu");
	if (!kdesu.isEmpty())
	{

		d->rootProcess = new KProcess;

		*d->rootProcess << kdesu;
		*d->rootProcess << "--nonewdcop" << "-n" << "-d" << QString( "-i%1" ).arg(moduleInfo().icon());

		*d->rootProcess << QString("kcmshell %1 --embed-proxy %2 --lang %3").arg(cmd).arg
			(d->embedWidget->winId()).arg(KGlobal::locale()->language());

		connect(d->rootProcess, SIGNAL(processExited(KProcess*)), SLOT(rootExited()));

		if ( !d->rootProcess->start( KProcess::NotifyOnExit ))
		{
			d->rootMode = false;
			rootExited();
		}
		else
		{
			d->rootMode = true;
			KApplication::dcopClient();
			d->rootCommunicator = new KCModuleProxyRootCommunicatorImpl( d->dcopName + "-RootCommunicator", this );
		}

		delete lblBusy;
		QApplication::restoreOverrideCursor();
		return;
	}

	/* Clean up in case of failure */
	delete d->embedWidget;
	d->embedWidget = 0;
	delete d->embedFrame;
	d->embedFrame = 0;

	QApplication::restoreOverrideCursor();
}

void KCModuleProxy::rootExited()
{
	kdDebug(711) << k_funcinfo << endl;

	if ( d->embedWidget->containerWinId() )
		XDestroyWindow(QX11Info::display(), d->embedWidget->containerWinId());

	delete d->embedWidget;
	d->embedWidget = 0;

	delete d->rootProcess;
	d->rootProcess = 0;

	delete d->embedFrame;
	d->embedFrame=0;

	delete d->rootCommunicator;
	d->rootCommunicator = 0;

	/* Such that the "ordinary" module loads again */
	d->rootMode = false;

	d->topLayout->invalidate();

	QShowEvent ev;
	showEvent( &ev );

	moduleChanged( false );
	emit childClosed();
}

KCModuleProxy::~KCModuleProxy()
{
	deleteClient();
	KCModuleLoader::unloadModule(moduleInfo());

	delete d;
}

void KCModuleProxy::deleteClient()
{
	if( d->embedWidget )
		XKillClient(QX11Info::display(), d->embedWidget->containerWinId());


	delete d->kcm;
	d->kcm = 0;

	delete d->dcopObject;
	d->dcopObject = 0;

	if( d->dcopClient && !d->dcopClient->detach() )
		kdDebug(711) << "Unregistering from DCOP failed." << endl;

	delete d->dcopClient;
	d->dcopClient = 0;

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

KCModuleProxy::KCModuleProxy( const KService::Ptr & service, bool withFallback,
		QWidget  * parent, const char * name, const QStringList & args)
	: QWidget( parent )
{
	init( KCModuleInfo( service ));
	d->args = args;
	d->withFallback = withFallback;
}

KCModuleProxy::KCModuleProxy( const KCModuleInfo & info, bool withFallback,
		QWidget * parent, const char * name, const QStringList & args )
	: QWidget( parent )
{
	init( info );
	d->args = args;
	d->withFallback = withFallback;
}

KCModuleProxy::KCModuleProxy( const QString& serviceName, bool withFallback,
		QWidget * parent, const char * name,
		const QStringList & args)
	: QWidget( parent )
{
	init( KCModuleInfo( serviceName ));
	d->args = args;
	d->withFallback = withFallback;
}

void KCModuleProxy::init( const KCModuleInfo& info )
{
	kdDebug(711) << k_funcinfo << endl;

	d = new KCModuleProxyPrivate( info );

	/* This is all we do for now; all the heavy work is
	 * done in realModule(). It's called when the module
	 * _actually_ is needed, in for example showEvent().
	 * The module is loaded "on demand" -- lazy loading.
	 */

}

void KCModuleProxy::load()
{

	if( d->rootMode )
		callRootModule( "load()" );
	else if( realModule() )
	{
		d->kcm->load();
		moduleChanged( false );
	}
}

void KCModuleProxy::save()
{
	if( d->rootMode )
		callRootModule( "save()" );
	else if( d->changed && realModule() )
	{
		d->kcm->save();
		moduleChanged( false );
	}
}

void KCModuleProxy::callRootModule( const QByteArray& function )
{
	QByteArray sendData, replyData;
	DCOPCString replyType;

	/* Note, we don't use d->dcopClient here, because it's used for
	 * the loaded module(and it's not "us" when this function is called) */
	if( !KApplication::dcopClient()->call( d->dcopName, d->dcopName, function, sendData,
			replyType, replyData, true, -1 ))
		kdDebug(711) << "Calling function '" << function << "' failed." << endl;

}

void KCModuleProxy::defaults()
{
	if( d->rootMode )
		callRootModule( "defaults()" );
	if( realModule() )
		d->kcm->defaults();
}

QString KCModuleProxy::quickHelp() const
{

	if( !d->rootMode )
		return realModule() ? realModule()->quickHelp() : QString::null;
	else
	{
		QByteArray data, replyData;
		DCOPCString replyType;

		if (KApplication::dcopClient()->call(d->dcopName, d->dcopName, "quickHelp()",
				  data, replyType, replyData))
			kdDebug(711) << "Calling DCOP function bool changed() failed." << endl;
		else
		{
			QDataStream reply(replyData);
			if (replyType == "QString")
			{
				QString result;
				reply >> result;
				return result;
			}
			else
				kdDebug(711) << "DCOP function changed() returned mumbo jumbo." << endl;
		}
		return QString::null;
	}
}

const KAboutData * KCModuleProxy::aboutData() const
{
	if( !d->rootMode )
		return realModule() ? realModule()->aboutData() : 0;
	else
	/* This needs fixing, perhaps cache a KAboutData copy
	 * while in root mode? */
		return 0;


}

int KCModuleProxy::buttons() const
{
	return realModule() ? realModule()->buttons() :
		KCModule::Help | KCModule::Default | KCModule::Apply ;
}

QString KCModuleProxy::rootOnlyMsg() const
{
	return realModule() ? realModule()->rootOnlyMsg() : QString::null;
}

bool KCModuleProxy::useRootOnlyMsg() const
{
	return realModule() ? realModule()->useRootOnlyMsg() : true;
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

bool KCModuleProxy::rootMode() const
{
	return d->rootMode;
}

QByteArray KCModuleProxy::dcopName() const
{
	return d->dcopName;
}

void KCModuleProxy::emitQuickHelpChanged()
{
	emit quickHelpChanged();
}

/***************************************************************/
#include "kcmoduleproxy.moc"

// vim: sw=4 ts=4 noet
