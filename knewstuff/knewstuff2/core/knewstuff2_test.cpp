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

	KNS::Engine *engine = new KNS::Engine();
	bool ret = engine->init("knewstuff2_test.knsrc");

	kDebug() << "-- engine test result: " << ret << endl;

	if(ret)
	{
		connect(engine,
			SIGNAL(signalProvidersLoaded(KNS::Provider::List*)),
			SLOT(slotProvidersLoaded(KNS::Provider::List*)));
		connect(engine,
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

	for(KNS::Provider::List::Iterator it = list->begin(); it != list->end(); it++)
	{
		KNS::Provider *provider = (*it);
		kDebug() << "-- provider: " << provider->name().representation() << endl;

		KNS::EntryLoader *el = new KNS::EntryLoader();
		el->load(provider->downloadUrl().url());
		// FIXME: loaders should probably accept KUrl directly

		// FIXME: the engine should do this... for all feeds!
		connect(el,
			SIGNAL(signalEntriesLoaded(KNS::Entry::List*)),
			SLOT(slotEntriesLoaded(KNS::Entry::List*)));
		connect(el,
			SIGNAL(signalEntriesFailed()),
			SLOT(slotEntriesFailed()));
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
	deleteLater();
	kapp->quit();
	exit(1);
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
