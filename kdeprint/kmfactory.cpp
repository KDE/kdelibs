#include "kmfactory.h"
#include "kmmanager.h"
#include "kmjobmanager.h"
#include "kmuimanager.h"
#include "kmvirtualmanager.h"
#include "kprinterimpl.h"

#include <klibloader.h>
#include <kconfig.h>
#include <kstddirs.h>
#include <kiconloader.h>
#include <kdebug.h>

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
	m_manager = 0;
	m_jobmanager = 0;
	m_uimanager = 0;
	m_virtualmanager = 0;
	m_implementation = 0;
	m_factory = 0;

	KGlobal::iconLoader()->addAppDir("kdeprint");
}

KMFactory::~KMFactory()
{
	if (m_manager) delete m_manager;
	if (m_jobmanager) delete m_jobmanager;
	if (m_uimanager) delete m_uimanager;
	if (m_virtualmanager) delete m_virtualmanager;
	if (m_implementation) delete m_implementation;
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
	if (!m_virtualmanager)
		m_virtualmanager = new KMVirtualManager(0, "VirtualManager");
	CHECK_PTR(m_virtualmanager);
	return m_virtualmanager;
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

void KMFactory::loadFactory()
{
	if (!m_factory)
	{
		KConfig	conf("kdeprintrc");
		conf.setGroup("General");
		QString	sys = conf.readEntry("PrintSystem","cups");
		QString	libname = QString::fromLatin1("libkdeprint_%1").arg(sys);
		m_factory = KLibLoader::self()->factory(libname.latin1());
		if (!m_factory)
			kdWarning() << QString::fromLatin1("Unable to locate library '%1'.").arg(libname) << endl;
	}
}
