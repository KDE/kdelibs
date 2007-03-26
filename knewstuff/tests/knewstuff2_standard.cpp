#include "knewstuff2_standard.h"

#include <knewstuff2/engine.h>

#include <kstandarddirs.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>

KNewStuff2Standard::KNewStuff2Standard()
: QObject()
{
    m_engine = NULL;
}

void KNewStuff2Standard::run(bool upload, QString file)
{
    kDebug() << "-- test kns2 engine" << endl;

    m_engine = new KNS::Engine();
    bool success = m_engine->init("knewstuff2_test.knsrc");

    kDebug() << "-- engine test result: " << success << endl;

    if(success)
    {
        //m_engine->start();
	if(upload)
	{
		kDebug() << "-- start upload" << endl;
		m_engine->uploadDialogModal(file);
		kDebug() << "-- upload finished" << endl;
	}
	else
	{
		m_engine->downloadDialog();
	}
    }
    else
    {
    }
}

static KCmdLineOptions options[] =
{
	{"upload <file>", "Tests upload dialog", 0},
	{"download", "Tests download dialog", 0},
	KCmdLineLastOption
};

int main(int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "knewstuff2_standard", NULL, NULL, NULL);
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app;

    // Take source directory into account
    kDebug() << "-- adding source directory " << KNSSRCDIR << endl;
    KGlobal::dirs()->addResourceDir("config", KNSSRCDIR);

    KNewStuff2Standard *standard = new KNewStuff2Standard();
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if(args->isSet("upload"))
    {
        standard->run(true, args->getOption("upload"));
    }
    else if(args->isSet("download"))
    {
        standard->run(false, QString());
    }
    else
    {
        return -1;
    }

    return app.exec();
}

#include "knewstuff2_standard.moc"
