/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  $Id$
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "kmfactory.h"
#include "kmmanager.h"
#include "kmjobmanager.h"
#include "kmuimanager.h"
#include "kmfiltermanager.h"
#include "kprinterimpl.h"
#include "kprinter.h"
#include "kpreloadobject.h"
#include "kdeprintcheck.h"

#include <qdir.h>
#include <klibloader.h>
#include <kconfig.h>
#include <kstddirs.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kstddirs.h>
#include <ksimpleconfig.h>

#define	UNLOAD_OBJECT(x) if (x != 0) { delete x; x = 0; }

void qt_generate_epsf( bool b );

KMFactory* KMFactory::m_self = 0;

KMFactory* KMFactory::self()
{
	if (!m_self)
		m_self = new KMFactory();
	return m_self;
}

void KMFactory::release()
{
	if (m_self)
	{
		delete m_self;
		m_self = 0;
	}
}

KMFactory::KMFactory()
{
	m_settings = new Settings;
	m_settings->application = KPrinter::Dialog;
	m_settings->pageSelection = KPrinter::SystemSide;
	m_settings->standardDialogPages = KPrinter::CopiesPage;
	m_settings->pageSize = -1;
	m_settings->orientation = -1;

	m_objects.setAutoDelete(false);

	m_manager = 0;
	m_jobmanager = 0;
	m_uimanager = 0;
	m_filtermanager = 0;
	m_implementation = 0;
	m_factory = 0;
	m_printconfig = 0;
#if QT_VERSION >= 230
	// Qt's default behaviour, to generate EPS in some cases and not in others, sucks.
	// This is fixed in Qt 3.0, but for Qt 2.x we need to disable it explicitely.
	// If this is a problem for anyone, we can add a public method to set this flag.
	// (David Faure, doing as advised by Lars Knoll)
	qt_generate_epsf( false );
#endif

	KGlobal::iconLoader()->addAppDir("kdeprint");
}

KMFactory::~KMFactory()
{
	unload();
	delete m_settings;
}

KMManager* KMFactory::manager()
{
	if (!m_manager)
		createManager();
	CHECK_PTR(m_manager);
	return m_manager;
}

KMJobManager* KMFactory::jobManager()
{
	if (!m_jobmanager)
		createJobManager();
	CHECK_PTR(m_jobmanager);
	return m_jobmanager;
}

KMUiManager* KMFactory::uiManager()
{
	if (!m_uimanager)
		createUiManager();
	CHECK_PTR(m_uimanager);
	return m_uimanager;
}

KPrinterImpl* KMFactory::printerImplementation()
{
	if (!m_implementation)
		createPrinterImpl();
	CHECK_PTR(m_implementation);
	return m_implementation;
}

KMVirtualManager* KMFactory::virtualManager()
{
	return manager()->m_virtualmgr;
}

KMFilterManager* KMFactory::filterManager()
{
	if (!m_filtermanager)
		m_filtermanager = new KMFilterManager(0, "FilterManager");
	CHECK_PTR(m_filtermanager);
	return m_filtermanager;
}

void KMFactory::createManager()
{
	loadFactory();
	if (m_factory) m_manager = (KMManager*)m_factory->create(NULL,"Manager","KMManager");
	if (!m_manager) m_manager = new KMManager(NULL,"Manager");
}

void KMFactory::createJobManager()
{
	loadFactory();
	if (m_factory) m_jobmanager = (KMJobManager*)m_factory->create(NULL,"JobManager","KMJobManager");
	if (!m_jobmanager) m_jobmanager = new KMJobManager(NULL,"JobManager");
}

void KMFactory::createUiManager()
{
	loadFactory();
	if (m_factory) m_uimanager = (KMUiManager*)m_factory->create(NULL,"UiManager","KMUiManager");
	if (!m_uimanager) m_uimanager = new KMUiManager(NULL,"UiManager");
}

void KMFactory::createPrinterImpl()
{
	loadFactory();
	if (m_factory) m_implementation = (KPrinterImpl*)m_factory->create(NULL,"PrinterImpl","KPrinterImpl");
	if (!m_implementation) m_implementation = new KPrinterImpl(NULL,"PrinterImpl");
}

void KMFactory::loadFactory(const QString& syst)
{
	if (!m_factory)
	{
		QString	sys(syst);
		if (sys.isEmpty())
			// load default configured print plugin
			sys = printSystem();
		QString	libname = QString::fromLatin1("libkdeprint_%1").arg(sys);
		m_factory = KLibLoader::self()->factory(libname.latin1());
                if (!m_factory)
                {
                        KMessageBox::error(0,
                            i18n("There was an error loading %1.\nThe diagnostic is:\n%2")
                            .arg(libname).arg(KLibLoader::self()->lastErrorMessage()));
                }
	}
}

KConfig* KMFactory::printConfig()
{
	if (!m_printconfig)
	{
		m_printconfig = new KConfig("kdeprintrc");
		CHECK_PTR(m_printconfig);
	}
	return m_printconfig;
}

QString KMFactory::printSystem()
{
	KConfig	*conf = printConfig();
	conf->setGroup("General");
	QString	sys = conf->readEntry("PrintSystem",QString::null);
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

void KMFactory::unload(bool pluginOnly)
{
	UNLOAD_OBJECT(m_manager);
	UNLOAD_OBJECT(m_jobmanager);
	UNLOAD_OBJECT(m_uimanager);
	UNLOAD_OBJECT(m_implementation);
	if (!pluginOnly)
	{
		UNLOAD_OBJECT(m_printconfig);
		UNLOAD_OBJECT(m_filtermanager);
	}
	// factory will be automatically unloaded by KLibLoader as all object have been deleted.
	// But to have loadFactory() to work, we need to set m_factory to NULL.
	m_factory = 0;
}

void KMFactory::reload(const QString& syst, bool saveSyst)
{
	// unload all objects from the plugin
	unload(true);
	if (saveSyst)
	{
		KConfig	*conf = printConfig();
		conf->setGroup("General");
		conf->writeEntry("PrintSystem", syst);
		conf->sync();
	}
	// reload the factory
	loadFactory(syst);
	// notify all registered objects
	QListIterator<KPReloadObject>	it(m_objects);
	for (;it.current();++it)
		it.current()->reload();
}

QValueList<PluginInfo> KMFactory::pluginList()
{
	QDir	d(locate("data", "kdeprint/plugins/"), "*.print", QDir::Name, QDir::Files);
	QValueList<PluginInfo>	list;
	for (int i=0; i<d.count(); i++)
	{
		PluginInfo	info;
		KSimpleConfig	conf(d.absFilePath(d[i]));

		conf.setGroup("KDE Print Entry");
		info.name = conf.readEntry("PrintSystem");
		info.comment = conf.readEntry("Comment");
		if (info.name.isEmpty())
			continue;
		if (info.comment.isEmpty())
			info.comment = info.name;
		info.detectUris = conf.readListEntry("DetectUris");
		info.detectPrecedence = conf.readNumEntry("DetectPrecedence", 0);
		list.append(info);
	}
	return list;
}

void KMFactory::registerObject(KPReloadObject *obj)
{
	// check if object already registered, then add it
	if (m_objects.findRef(obj) == -1)
	{
		m_objects.append(obj);
		kdDebug() << "kdeprint: registering " << (void*)obj << ", number of objects = " << m_objects.count() << endl;
	}
}

void KMFactory::unregisterObject(KPReloadObject *obj)
{
	// remove object from list (not deleted as autoDelete is false)
	m_objects.removeRef(obj);
	kdDebug() << "kdeprint: unregistering " << (void*)obj << ", number of objects = " << m_objects.count() << endl;
}

QString KMFactory::autoDetect()
{
	QValueList<PluginInfo>	plugins = pluginList();
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
	return (pluginIndex == -1 ? QString::fromLatin1("lpdunix") : plugins[pluginIndex].name);
}
