#include <ksystemtray.h>
#include <kapplication.h>
#include <QLabel>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

int main(int argc, char **argv)
{
	KAboutData aboutData( "ksystemtraytest", "ksystemtraytest", "1.0" );
	KCmdLineArgs::init(argc, argv, &aboutData);
        KApplication app;
	QLabel *l = new QLabel("System Tray Main Window", 0L);
	KSystemTray *tray = new KSystemTray( l );
	tray->setText("Test");
	l->show();
	tray->show();

	return app.exec();
}
