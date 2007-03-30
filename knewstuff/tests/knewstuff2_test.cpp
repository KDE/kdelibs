#include "knewstuff2_test.h"

#include <knewstuff2/core/ktranslatable.h>
#include <knewstuff2/core/providerhandler.h>
#include <knewstuff2/core/entryhandler.h>
#include <knewstuff2/core/coreengine.h>

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
	m_testall = false;
}

void KNewStuff2Test::setTestAll(bool testall)
{
	m_testall = testall;
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

	m_engine = new KNS::CoreEngine();
	bool ret = m_engine->init("knewstuff2_test.knsrc");

	kDebug() << "-- engine test result: " << ret << endl;

	if(ret)
	{
		connect(m_engine,
			SIGNAL(signalProviderLoaded(KNS::Provider*)),
			SLOT(slotProviderLoaded(KNS::Provider*)));
		connect(m_engine,
			SIGNAL(signalProvidersFailed()),
			SLOT(slotProvidersFailed()));
		connect(m_engine,
			SIGNAL(signalEntryLoaded(KNS::Entry*)),
			SLOT(slotEntryLoaded(KNS::Entry*)));
		connect(m_engine,
			SIGNAL(signalEntriesFinished()),
			SLOT(slotEntriesFinished()));
		connect(m_engine,
			SIGNAL(signalEntriesFailed()),
			SLOT(slotEntriesFailed()));
		connect(m_engine,
			SIGNAL(signalPayloadLoaded(KUrl)),
			SLOT(slotPayloadLoaded(KUrl)));
		connect(m_engine,
			SIGNAL(signalPayloadFailed()),
			SLOT(slotPayloadFailed()));

		m_engine->start(false);
	}
	else
	{
		kWarning() << "ACHTUNG: you probably need to 'make install' the knsrc file first." << endl;
		kWarning() << "Although this is not required anymore, so something went really wrong." << endl;
		quitTest();
	}
}

void KNewStuff2Test::slotProviderLoaded(KNS::Provider *provider)
{
	kDebug() << "SLOT: slotProviderLoaded" << endl;
	kDebug() << "-- provider: " << provider->name().representation() << endl;

	m_engine->loadEntries(provider);
}

void KNewStuff2Test::slotEntryLoaded(KNS::Entry *entry)
{
	kDebug() << "SLOT: slotEntryLoaded" << endl;
	kDebug() << "-- entry: " << entry->name().representation() << endl;

	if(m_testall)
	{
		kDebug() << "-- now, download the entry's preview and payload file" << endl;

		if(!entry->preview().isEmpty())
			m_engine->downloadPreview(entry);
		if(!entry->payload().isEmpty())
			m_engine->downloadPayload(entry);
	}
}

void KNewStuff2Test::slotEntriesFinished()
{
	quitTest();
}

void KNewStuff2Test::slotPayloadLoaded(KUrl payload)
{
	kDebug() << "-- entry downloaded successfully" << endl;
	kDebug() << "-- downloaded to " << payload.prettyUrl() << endl;

	kDebug() << "-- OK, finish test" << endl;
	quitTest();
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

static KCmdLineOptions options[] =
{
	{"testall", "Downloads all previews and payloads", 0},
	KCmdLineLastOption
};

int main(int argc, char **argv)
{
	KCmdLineArgs::init(argc, argv, "knewstuff2_test", NULL, NULL, NULL);
	KCmdLineArgs::addCmdLineOptions(options);
	KApplication app(false);

	// Take source directory into account
	kDebug() << "-- adding source directory " << KNSSRCDIR << endl;
	KGlobal::dirs()->addResourceDir("config", KNSSRCDIR);

	KNewStuff2Test *test = new KNewStuff2Test();
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	if(args->isSet("testall"))
	{
		test->setTestAll(true);
		test->entryTest();
		test->providerTest();
	}
	test->engineTest();

	return app.exec();
}

#include "knewstuff2_test.moc"
