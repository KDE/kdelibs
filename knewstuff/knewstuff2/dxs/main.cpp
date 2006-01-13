#include "kdxs.h"
#include <kapplication.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kcmdlineargs.h>

int main(int argc, char **argv)
{
	KAboutData about("kdxs",
		I18N_NOOP("KDXS"),
		"0.1",
		I18N_NOOP("KNewstuff DXS Preview"),
		KAboutData::License_GPL,
		"(C) 2005 Josef Spillner",
		0,
		0,
		"spillner@kde.org");

	KCmdLineArgs::init(argc, argv, &about);
	KApplication app;

	KDXS *widget = new KDXS();
	app.setMainWidget(widget);
	widget->show();
	return app.exec();
}
