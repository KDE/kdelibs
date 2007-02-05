#include "knewstuff2_test.h"

KNewStuff2Test::KNewStuff2Test()
: QObject()
{
}

void KNewStuff2Test::providerTest()
{
	kDebug() << "-- test kns2 provider class" << endl;

	KNS::Provider p;
	p.setName("GHNS Example Provider");
	p.setDownloadUrl(KUrl("http://localhost:8080"));

	KNS::ProviderHandler ph(p);
	QDomElement pxml = ph.providerXML();

	kDebug() << "-- provider test result: " << pxml.text() << endl;

	if(pxml.isNull())
	{
		quitTest();
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
		quitTest();
	}
}

void KNewStuff2Test::slotProvidersLoaded(KNS::Provider::List *list)
{
	kDebug() << "SLOT: slotProvidersLoaded" << endl;
	kDebug() << "-- providers: " << list->count() << endl;
}

void KNewStuff2Test::slotProvidersFailed()
{
	kDebug() << "SLOT: slotProvidersFailed" << endl;
	quitTest();
}

void KNewStuff2Test::quitTest()
{
	kDebug() << "-- quitting now..." << endl;
	deleteLater();
	kapp->quit();
}

int main(int argc, char **argv)
{
	KCmdLineArgs::init(argc, argv, "knewstuff2_test", NULL, NULL, NULL);
	KApplication app(false);

	KNewStuff2Test *test = new KNewStuff2Test();
	test->providerTest();
	test->engineTest();

	return app.exec();
}

#include "knewstuff2_test.moc"
