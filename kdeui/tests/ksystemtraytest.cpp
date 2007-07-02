#include <ksystemtrayicon.h>
#include <kapplication.h>
#include <QtGui/QLabel>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

int main(int argc, char **argv)
{
	KAboutData aboutData( "ksystemtraytest", 0 , ki18n("ksystemtraytest"), "1.0" );
	KCmdLineArgs::init(argc, argv, &aboutData);
        KApplication app;
	QLabel *l = new QLabel("System Tray Main Window", 0L);
	KSystemTrayIcon *tray = new KSystemTrayIcon( "test", l );
	l->show();
	tray->show();

	return app.exec();
}
