#include "knewstuff2_test.h"

#include <knewstuff2/ktranslatable.h>
#include <knewstuff2/providerhandler.h>
#include <knewstuff2/entryhandler.h>
#include <knewstuff2/engine.h>
#include <knewstuff2/entryloader.h> // tmp (go to engine)

#include <kstandarddirs.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>

#include <qfile.h>

#include <unistd.h> // for exit()
#include <stdio.h> // for stdout

KNewStuff2Test::KNewStuff2Test()
: QObject()
{
	m_engine = NULL;
}

void KNewStuff2Test::entryTest()
{
	kDebug() << "-- test kns2 entry class" << endl;

	QDomDocument doc;
	QFile f(QString("%1/testdata/entry.xml").arg(KNSSRCDIR));
	if(!f.open(QIODevice::ReadOnly))
	{
		kDebug() << "Error loading entry file." << endl;
		quitTest();
	}
	if(!doc.setContent(&f))
	{
		kDebug() << "Error parsing entry file." << endl;
		f.close();
		quitTest();
	}
	f.close();

	KNS::EntryHandler eh(doc.documentElement());
	KNS::Entry e = eh.entry();

	kDebug() << "-- xml->entry test result: " << eh.isValid() << endl;

	KNS::EntryHandler eh2(e);
	QDomElement exml = eh2.entryXML();

	kDebug() << "-- entry->xml test result: " << eh.isValid() << endl;

	if(!eh.isValid())
	{
		quitTest();
	}
	else
	{
		QTextStream out(stdout);
		out << exml;
	}
}

void KNewStuff2Test::providerTest()
{
	kDebug() << "-- test kns2 provider class" << endl;

	QDomDocument doc;
	QFile f(QString("%1/testdata/provider.xml").arg(KNSSRCDIR));
	if(!f.open(QIODevice::ReadOnly))
	{
		kDebug() << "Error loading provider file." << endl;
		quitTest();
	}
	if(!doc.setContent(&f))
	{
		kDebug() << "Error parsing provider file." << endl;
		f.close();
		quitTest();
	}
	f.close();

	KNS::ProviderHandler ph(doc.documentElement());
	KNS::Provider p = ph.provider();

	kDebug() << "-- xml->provider test result: " << ph.isValid() << endl;

	KNS::ProviderHandler ph2(p);
	QDomElement pxml = ph2.providerXML();

	kDebug() << "-- provider->xml test result: " << ph.isValid() << endl;

	if(!ph.isValid())
	{
		quitTest();
	}
	else
	{
		QTextStream out(stdout);
		out << pxml;
	}
}

void KNewStuff2Test::engineTest()
{
	kDebug() << "-- test kns2 engine" << endl;

	m_engine = new KNS::Engine();
	bool ret = m_engine->init("knewstuff2_test.knsrc");

	kDebug() << "-- engine test result: " << ret << endl;

	if(ret)
	{
		connect(m_engine,
			SIGNAL(signalProvidersLoaded(KNS::Provider::List*)),
			SLOT(slotProvidersLoaded(KNS::Provider::List*)));
		connect(m_engine,
			SIGNAL(signalProvidersFailed()),
			SLOT(slotProvidersFailed()));
	}
	else
	{
		kWarning() << "ACHTUNG: you probably need to 'make install' the knsrc file first." << endl;
		kWarning() << "Although this is not required anymore, so something went really wrong." << endl;
		quitTest();
	}
}

void KNewStuff2Test::slotProvidersLoaded(KNS::Provider::List *list)
{
	kDebug() << "SLOT: slotProvidersLoaded" << endl;
	kDebug() << "-- providers: " << list->count() << endl;

	connect(m_engine,
		SIGNAL(signalEntriesLoaded(KNS::Entry::List*)),
		SLOT(slotEntriesLoaded(KNS::Entry::List*)));
	connect(m_engine,
		SIGNAL(signalEntriesFailed()),
		SLOT(slotEntriesFailed()));

	for(KNS::Provider::List::Iterator it = list->begin(); it != list->end(); it++)
	{
		KNS::Provider *provider = (*it);
		kDebug() << "-- provider: " << provider->name().representation() << endl;

		m_engine->loadEntries(provider);
	}
}

void KNewStuff2Test::slotEntriesLoaded(KNS::Entry::List *list)
{
	kDebug() << "SLOT: slotEntriesLoaded" << endl;
	kDebug() << "-- entries: " << list->count() << endl;

	for(KNS::Entry::List::Iterator it = list->begin(); it != list->end(); it++)
	{
		KNS::Entry *entry = (*it);
		kDebug() << "-- entry: " << entry->name().representation() << endl;
	}

	kDebug() << "-- now, download the first entry" << endl;

	connect(m_engine, SIGNAL(signalPayloadLoaded(KUrl)), SLOT(slotPayloadLoaded(KUrl)));
	connect(m_engine, SIGNAL(signalPayloadFailed()), SLOT(slotPayloadFailed()));

	m_engine->downloadPreview((*list->begin()));
	m_engine->downloadPayload((*list->begin()));
}

void KNewStuff2Test::slotPayloadLoaded(KUrl payload)
{
	kDebug() << "-- entry downloaded successfully" << endl;
	kDebug() << "-- downloaded to " << payload.prettyUrl() << endl;
}

void KNewStuff2Test::slotPayloadFailed()
{
	kDebug() << "SLOT: slotPayloadFailed" << endl;
	quitTest();
}

void KNewStuff2Test::slotProvidersFailed()
{
	kDebug() << "SLOT: slotProvidersFailed" << endl;
	quitTest();
}

void KNewStuff2Test::slotEntriesFailed()
{
	kDebug() << "SLOT: slotEntriesFailed" << endl;
	quitTest();
}

void KNewStuff2Test::quitTest()
{
	kDebug() << "-- quitting now..." << endl;
	if(1 == 0)
	{
		// this would be the soft way out...
		delete m_engine;
		deleteLater();
		kapp->quit();
	}
	else
	{
		exit(1);
	}
}

int main(int argc, char **argv)
{
	KCmdLineArgs::init(argc, argv, "knewstuff2_test", NULL, NULL, NULL);
	KApplication app(false);

	// Take source directory into account
	kDebug() << "-- adding source directory " << KNSSRCDIR << endl;
	KGlobal::dirs()->addResourceDir("config", KNSSRCDIR);

	KNewStuff2Test *test = new KNewStuff2Test();
	test->entryTest();
	test->providerTest();
	test->engineTest();

	return app.exec();
}

#include "knewstuff2_test.moc"
