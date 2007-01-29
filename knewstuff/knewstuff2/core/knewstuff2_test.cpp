#include <knewstuff2/provider.h>
#include <knewstuff2/providerhandler.h>
#include <knewstuff2/engine.h>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>

void providerTest()
{
	kDebug() << "-- test kns2 provider class" << endl;

	KNS::Provider p;
	p.setName("GHNS Example Provider");
	p.setDownloadUrl(KUrl("http://localhost:8080"));

	KNS::ProviderHandler ph(p);
	QDomElement e = ph.providerXML();

	kDebug() << "-- test result: " << e.text() << endl;
}

void engineTest()
{
	kDebug() << "-- test kns2 engine" << endl;

	KNS::Engine e;
	bool ret = e.init("knewstuff2.knsrc");

	kDebug() << "-- test result: " << ret << endl;
}

int main(int argc, char **argv)
{
	KCmdLineArgs::init(argc, argv, "knewstuff2_test", NULL, NULL, NULL);
	KApplication app(false);

	providerTest();
	engineTest();

	return app.exec();
}

