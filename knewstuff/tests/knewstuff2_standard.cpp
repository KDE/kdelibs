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

void KNewStuff2Standard::run(bool upload, bool modal, QString file)
{
    kDebug() << "-- test kns2 engine" << endl;

    m_engine = new KNS::Engine();
    bool success = m_engine->init("knewstuff2_test.knsrc");

    kDebug() << "-- engine test result: " << success << endl;

	if(!success)
		return;

	//m_engine->start();
	if(upload)
	{
		if(modal)
		{
			kDebug() << "-- start upload (modal)" << endl;
			m_engine->uploadDialogModal(file);
			kDebug() << "-- upload (modal) finished" << endl;
		}
		else
		{
			kDebug() << "-- start upload (non-modal); will not block" << endl;
			m_engine->uploadDialog(file);
		}
	}
	else
	{
		if(modal)
		{
			kDebug() << "-- start download (modal)" << endl;
			m_engine->downloadDialogModal();
			kDebug() << "-- download (modal) finished" << endl;
		}
		else
		{
			kDebug() << "-- start download (non-modal); will not block" << endl;
			m_engine->downloadDialog();
		}
	}
}

static KCmdLineOptions options[] =
{
	{"upload <file>", "Tests upload dialog", 0},
	{"download", "Tests download dialog", 0},
	{"modal", "Show modal dialogs", 0},
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
    bool modal = false;
    if(args->isSet("modal"))
    {
        modal = true;
    }
    if(args->isSet("upload"))
    {
        standard->run(true, modal, args->getOption("upload"));
    }
    else if(args->isSet("download"))
    {
        standard->run(false, modal, QString());
    }
    else
    {
        return -1;
    }

    return app.exec();
}

#include "knewstuff2_standard.moc"
