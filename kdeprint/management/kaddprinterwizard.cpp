#include "kmmanager.h"
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <klocale.h>

static KCmdLineOptions options[] =
{
	{0, 0, 0}
};

int main(int argc, char *argv[])
{
	KCmdLineArgs::init(argc, argv, "kaddprinterwizard",
			I18N_NOOP("Start the add printer wizard"),
			"0.1");
	KCmdLineArgs::addCmdLineOptions(options);

	KApplication app;
	if (KMManager::self()->addPrinterWizard() == -1)
	{
		KMessageBox::error(0, KMManager::self()->errorMsg());
	}
	
	return 0;
}
