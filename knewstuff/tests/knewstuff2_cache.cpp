#include "knewstuff2_cache.h"

#include <knewstuff2/core/entryhandler.h>
#include <knewstuff2/core/coreengine.h>

#include <kstandarddirs.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>

#include <qfile.h>

#include <unistd.h> // for exit()

KNewStuff2Cache::KNewStuff2Cache()
: QObject()
{
	m_engine = NULL;
}

void KNewStuff2Cache::run()
{
	kDebug() << "-- start the engine" << endl;

	m_engine = new KNS::CoreEngine();
	bool ret = m_engine->init("knewstuff2_test.knsrc");

	kDebug() << "-- engine initialisation result: " << ret << endl;

	if(ret)
	{
		connect(m_engine,
			SIGNAL(signalEntryLoaded(KNS::Entry*)),
			SLOT(slotEntryLoaded(KNS::Entry*)));
		connect(m_engine,
			SIGNAL(signalEntriesFailed()),
			SLOT(slotEntriesFailed()));
		connect(m_engine,
			SIGNAL(signalEntriesFinished()),
			SLOT(slotEntriesFinished()));

		m_engine->start(true);
	}
	else
	{
		kWarning() << "ACHTUNG: you probably need to 'make install' the knsrc file first." << endl;
		kWarning() << "Although this is not required anymore, so something went really wrong." << endl;
		quitTest();
	}
}

void KNewStuff2Cache::slotEntryLoaded(KNS::Entry *entry)
{
	kDebug() << "SLOT: slotEntryLoaded" << endl;
	kDebug() << "-- entry: " << entry->name().representation() << endl;
}

void KNewStuff2Cache::slotEntriesFailed()
{
	kDebug() << "SLOT: slotEntriesFailed" << endl;
	quitTest();
}

void KNewStuff2Cache::slotEntriesFinished()
{
	kDebug() << "SLOT: slotEntriesFinished" << endl;
	quitTest();
}

void KNewStuff2Cache::quitTest()
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
	KCmdLineArgs::init(argc, argv, "knewstuff2_cache", NULL, NULL, NULL);
	KApplication app(false);

	// Take source directory into account
	kDebug() << "-- adding source directory " << KNSSRCDIR << endl;
	KGlobal::dirs()->addResourceDir("config", KNSSRCDIR);

	KNewStuff2Cache *test = new KNewStuff2Cache();
	test->run();

	return app.exec();
}

#include "knewstuff2_cache.moc"
