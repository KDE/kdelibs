#include "kdxsview.h"
#include <kapplication.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kcmdlineargs.h>

int main(int argc, char **argv)
{
	KAboutData about("kdxspreview",
		I18N_NOOP("KDXS Preview"),
		"0.1",
		I18N_NOOP("KNewstuff2 DXS Preview"),
		KAboutData::License_GPL,
		"(C) 2005, 2006 Josef Spillner",
		0,
		0,
		"spillner@kde.org");

	KCmdLineArgs::init(argc, argv, &about);
	KApplication app;

	KDXSView view;
	app.setMainWidget(&view);
	return view.exec();
}
