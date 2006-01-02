/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kmfactory.h"
#include "kmmanager.h"
#include "kmjobmanager.h"
#include "kmuimanager.h"
#include "kprinterimpl.h"
#include "kprinter.h"
#include "kpreloadobject.h"
#include "kdeprintcheck.h"
#include "kxmlcommand.h"

#include <qdir.h>
#include <qfile.h>
#include <qsettings.h>

#include <klibloader.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kstaticdeleter.h>
#include <kapplication.h>
#include <dcopclient.h>
#include <dcopref.h>
#include <kio/authinfo.h>

#include <unistd.h>

#define	UNLOAD_OBJECT(x) if (x != 0) { delete x; x = 0; }

#ifdef Q_WS_X11
extern void qt_generate_epsf( bool b );
#endif

KMFactory* KMFactory::m_self = 0;
static KStaticDeleter<KMFactory> s_kmfactorysd;

KMFactory* KMFactory::self()
{
	if (!m_self)
		m_self = s_kmfactorysd.setObject(m_self, new KMFactory());
	return m_self;
}

bool KMFactory::exists()
{
    return m_self != 0L;
}

void KMFactory::release()
{
	if (m_self)
	{
		KMFactory* p = m_self;
		m_self = 0; // so that exists() says false
		delete p;
	}
}

KMFactory::KMFactory()
	: QObject(0)
{
    setObjectName( "Factory" );
	m_settings = new Settings;
	m_settings->application = KPrinter::Dialog;
	m_settings->pageSelection = KPrinter::SystemSide;
	m_settings->standardDialogPages = KPrinter::CopiesPage;
	m_settings->pageSize = -1;
	m_settings->orientation = -1;


	m_manager = 0;
	m_jobmanager = 0;
	m_uimanager = 0;
	m_implementation = 0;
	m_factory = 0;
	m_printconfig = 0;

	// By default, embed PS fonts
	bool ok = false;
	QSettings settings;
	settings.readBoolEntry( "/qt/embedFonts", true, &ok );
	if ( !ok )
		settings.writeEntry( "/qt/embedFonts", true );

	KGlobal::iconLoader()->addAppDir("kdeprint");
        KGlobal::locale()->insertCatalog("kdeprint");

	// create DCOP signal connection
	connectDCOPSignal(0, 0, "pluginChanged(pid_t)", "slot_pluginChanged(pid_t)", false);
	connectDCOPSignal(0, 0, "configChanged()", "slot_configChanged()", false);
}

KMFactory::~KMFactory()
{
	delete m_settings;
	// The only object to be destroyed is m_printconfig. All other objects have been
	// created with "this" as parent, so we don't need to care about their destruction
	UNLOAD_OBJECT(m_printconfig);
	m_self = 0;
}

KMManager* KMFactory::manager()
{
	if (!m_manager)
		createManager();
	Q_CHECK_PTR(m_manager);
	return m_manager;
}

KMJobManager* KMFactory::jobManager()
{
	if (!m_jobmanager)
		createJobManager();
	Q_CHECK_PTR(m_jobmanager);
	return m_jobmanager;
}

KMUiManager* KMFactory::uiManager()
{
	if (!m_uimanager)
		createUiManager();
	Q_CHECK_PTR(m_uimanager);
	return m_uimanager;
}

KPrinterImpl* KMFactory::printerImplementation()
{
	if (!m_implementation)
		createPrinterImpl();
	Q_CHECK_PTR(m_implementation);
	return m_implementation;
}

KMVirtualManager* KMFactory::virtualManager()
{
	return manager()->m_virtualmgr;
}

KMSpecialManager* KMFactory::specialManager()
{
	return manager()->m_specialmgr;
}

KXmlCommandManager* KMFactory::commandManager()
{
	return KXmlCommandManager::self();
}

void KMFactory::createManager()
{
	loadFactory();
	if (m_factory) m_manager = (KMManager*)m_factory->create(this,"Manager","KMManager");
	if (!m_manager) {
            m_manager = new KMManager(this );
            m_manager->setObjectName( "Manager" );
        }
}

void KMFactory::createJobManager()
{
	loadFactory();
	if (m_factory) m_jobmanager = (KMJobManager*)m_factory->create(this,"JobManager","KMJobManager");
	if (!m_jobmanager) {
            m_jobmanager = new KMJobManager( this );
            m_jobmanager->setObjectName( "JobManager" );
        }

}

void KMFactory::createUiManager()
{
	loadFactory();
	if (m_factory) m_uimanager = (KMUiManager*)m_factory->create(this,"UiManager","KMUiManager");
	if (!m_uimanager) {
            m_uimanager = new KMUiManager(this );
            m_uimanager->setObjectName( "UiManager" );
}
}

void KMFactory::createPrinterImpl()
{
	loadFactory();
	if (m_factory) m_implementation = (KPrinterImpl*)m_factory->create(this,"PrinterImpl","KPrinterImpl");
	if (!m_implementation) {
            m_implementation = new KPrinterImpl( this );
            m_implementation->setObjectName( "PrinterImpl" );
        }
}

void KMFactory::loadFactory(const QString& syst)
{
	if (!m_factory)
	{
		QString	sys(syst);
		if (sys.isEmpty())
			// load default configured print plugin
			sys = printSystem();
		QString	libname = QString::fromLatin1("kdeprint_%1").arg(sys);
		m_factory = KLibLoader::self()->factory(QFile::encodeName(libname));
                if (!m_factory)
                {
                        KMessageBox::error(0,
                            i18n("<qt>There was an error loading %1. The diagnostic is:<p>%2</p></qt>")
                            .arg(libname).arg(KLibLoader::self()->lastErrorMessage()));
                }
	}
}

KConfig* KMFactory::printConfig(const QString& group)
{
	if (!m_printconfig)
	{
		m_printconfig = new KConfig("kdeprintrc");
		Q_CHECK_PTR(m_printconfig);
	}
	if (!group.isEmpty())
		m_printconfig->setGroup(group);
	return m_printconfig;
}

QString KMFactory::printSystem()
{
	KConfig	*conf = printConfig();
	conf->setGroup("General");
	QString	sys = conf->readEntry("PrintSystem");
	if (sys.isEmpty())
	{
		// perform auto-detection (will at least return "lpdunix")
		sys = autoDetect();
		// save the result
		conf->writeEntry("PrintSystem", sys);
		conf->sync();
	}
	else if ( sys.length()==1 && sys[0].isDigit() ) // discard old-style settings
        	sys = "lpdunix";
	return sys;
}

void KMFactory::unload()
{
	UNLOAD_OBJECT(m_manager);
	UNLOAD_OBJECT(m_jobmanager);
	UNLOAD_OBJECT(m_uimanager);
	UNLOAD_OBJECT(m_implementation);
	// factory will be automatically unloaded by KLibLoader as all object have been deleted.
	// But to have loadFactory() to work, we need to set m_factory to NULL.
	m_factory = 0;
}

void KMFactory::reload(const QString& syst, bool saveSyst)
{
	// notify all registered objects about the coming reload
	QListIterator<KPReloadObject*>	it(m_objects);
	while(it.hasNext())
		it.next()->aboutToReload();

	// unload all objects from the plugin
	unload();
	if (saveSyst)
	{
		KConfig	*conf = printConfig();
		conf->setGroup("General");
		conf->writeEntry("PrintSystem", syst);
		conf->sync();

		// notify all other apps using DCOP signal
		emit pluginChanged(getpid());
	}

	// reload the factory
	loadFactory(syst);

	// notify all registered objects
	it.toBack();
	while(it.hasNext())
		it.next()->reload();
}

QList<KMFactory::PluginInfo> KMFactory::pluginList()
{
	QDir	d(locate("data", "kdeprint/plugins/"), "*.print", QDir::Name, QDir::Files);
	QList<PluginInfo>	list;
	for (uint i=0; i<d.count(); i++)
	{
		PluginInfo	info(pluginInfo(d.absoluteFilePath(d[i])));
		if (info.name.isEmpty())
			continue;
		list.append(info);
	}
	return list;
}

KMFactory::PluginInfo KMFactory::pluginInfo(const QString& name)
{
	QString	path(name);
	if (path[0] != '/')
		path = locate("data", QString::fromLatin1("kdeprint/plugins/%1.print").arg(name));
	KSimpleConfig	conf(path);
	PluginInfo	info;

	conf.setGroup("KDE Print Entry");
	info.name = conf.readEntry("PrintSystem");
	info.comment = conf.readEntry("Comment");
	if (info.comment.isEmpty())
		info.comment = info.name;
	info.detectUris = conf.readListEntry("DetectUris");
	info.detectPrecedence = conf.readNumEntry("DetectPrecedence", 0);
	info.mimeTypes = conf.readListEntry("MimeTypes");
	if (info.mimeTypes.isEmpty())
		info.mimeTypes << "application/postscript";
	info.primaryMimeType = conf.readEntry("PrimaryMimeType", info.mimeTypes[0]);

	return info;
}

void KMFactory::registerObject(KPReloadObject *obj, bool priority)
{
	// check if object already registered, then add it
	if (m_objects.indexOf(obj) == -1)
	{
		if (priority)
			m_objects.prepend(obj);
		else
			m_objects.append(obj);
		kdDebug(500) << "kdeprint: registering " << (void*)obj << ", number of objects = " << m_objects.count() << endl;
	}
}

void KMFactory::unregisterObject(KPReloadObject *obj)
{
	// remove object from list (not deleted as autoDelete is false)
	m_objects.removeAll(obj);
	kdDebug(500) << "kdeprint: unregistering " << (void*)obj << ", number of objects = " << m_objects.count() << endl;
}

QString KMFactory::autoDetect()
{
	QList<PluginInfo>	plugins = pluginList();
	int	pluginIndex(-1), currentPrecedence(0);
	for (int i=0;i<plugins.count();i++)
	{
		if (plugins[i].detectUris.count() > 0 && KdeprintChecker::check(plugins[i].detectUris)
		    && (pluginIndex == -1 || plugins[i].detectPrecedence >= currentPrecedence))
		{
			pluginIndex = i;
			currentPrecedence = plugins[i].detectPrecedence;
		}
	}
	return (pluginIndex == -1 ? QLatin1String("lpdunix") : plugins[pluginIndex].name);
}

void KMFactory::slot_pluginChanged(pid_t pid)
{
	// only do something if the notification comes from another process
	if (pid != getpid())
	{
		// Unload config object (avoid saving it)
		printConfig()->rollback();
		UNLOAD_OBJECT(m_printconfig);
		// Then reload everything and notified registered objects.
		// Do NOT re-save the new print system.
		QString	syst = printSystem();
		reload(syst, false);
	}
}

void KMFactory::slot_configChanged()
{
	kdDebug(500) << "KMFactory (" << getpid() << ") receiving DCOP signal configChanged()" << endl;
	// unload/reload config object (make it non dirty to
	// avoid saving it and overwriting the newly saved options
	// in the other application)
	printConfig()->rollback();
	UNLOAD_OBJECT(m_printconfig);
	printConfig();

	// notify all registered objects about the coming reload
	QListIterator<KPReloadObject*>	it(m_objects);
	/*for (;it.current();++it)
		it.current()->aboutToReload();*/

	// notify all object about the change
	while(it.hasNext())
		it.next()->configChanged();
}

void KMFactory::saveConfig()
{
	KConfig	*conf = printConfig();
	conf->sync();
	kdDebug(500) << "KMFactory (" << getpid() << ") emitting DCOP signal configChanged()" << endl;
	emit configChanged();
	// normally, the self application should also receive the signal,
	// anyway the config object has been updated "locally", so ne real
	// need to reload the config file.
}

QPair<QString,QString> KMFactory::requestPassword( int& seqNbr, const QString& user, const QString& host, int port )
{
	DCOPRef kdeprintd( "kded", "kdeprintd" );
	/**
	 * We do not use an internal event loop for 2 potential problems:
	 *  - the MessageWindow modality (appearing afterwards, it pops up on top
	 *    of the password dialog)
	 *  - KMTimer should be stopped, but it's unavailable from this object
	 */
	DCOPReply reply = kdeprintd.call( "requestPassword", user, host, port, seqNbr );
	if ( reply.isValid() )
	{
		QString replyString = reply;
		if ( replyString != "::" )
		{
			QStringList l = QStringList::split( ':', replyString, true );
			if ( l.count() == 3 )
			{
				seqNbr = l[ 2 ].toInt();
				return QPair<QString,QString>( l[ 0 ], l[ 1 ] );
			}
		}
	}
	return QPair<QString,QString>( QString(), QString() );
}

void KMFactory::initPassword( const QString& user, const QString& password, const QString& host, int port )
{
	DCOPRef kdeprintd( "kded", "kdeprintd" );
	/**
	 * We do not use an internal event loop for 2 potential problems:
	 *  - the MessageWindow modality (appearing afterwards, it pops up on top
	 *    of the password dialog)
	 *  - KMTimer should be stopped, but it's unavailable from this object
	 */
	kdeprintd.call( "initPassword", user, password, host, port );
}

#include "kmfactory.moc"
