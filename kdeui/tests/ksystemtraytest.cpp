#include <ksystemtray.h>
#include <kapp.h>

int main(int argc, char **argv)
{
	KApplication app( argc, argv, "ksystemtraytest" );
	QLabel *l = new QLabel("System Tray Main Window", 0L);
	KSystemTray *tray = new KSystemTray( l );
    tray->setText("Test");
	l->show();
	tray->show();

	return app.exec();
}
