#include "kmmanager.h"
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <klocale.h>

static KCmdLineOptions options[] =
{
	{ "kdeconfig", I18N_NOOP("Configure KDE Print"), 0 },
	{ "serverconfig", I18N_NOOP("Configure print server"), 0 },
	{0, 0, 0}
};

int main(int argc, char *argv[])
{
	KCmdLineArgs::init(argc, argv, "kaddprinterwizard",
			I18N_NOOP("Start the add printer wizard"),
			"0.1");
	KCmdLineArgs::addCmdLineOptions(options);

	KApplication app;
	KCmdLineArgs	*args = KCmdLineArgs::parsedArgs();
	bool	doConfig = args->isSet("kdeconfig");
	bool	doSrvConfig = args->isSet("serverconfig");

	if (doConfig)
		KMManager::self()->invokeOptionsDialog();
	else if (doSrvConfig)
		KMManager::self()->configureServer();
	else if (KMManager::self()->addPrinterWizard() == -1)
	{
		KMessageBox::error(0, KMManager::self()->errorMsg().prepend("<qt>").append("</qt>"));
	}
	
	return 0;
}
