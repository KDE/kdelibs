#include <ksystemtrayicon.h>
#include <QApplication>
#include <QLabel>

int main(int argc, char **argv)
{
	QApplication::setApplicationName("ksystemtraytest");
    QApplication app(argc, argv);
	QLabel *l = new QLabel("System Tray Main Window", 0L);
	KSystemTrayIcon *tray = new KSystemTrayIcon( "test", l );
	l->show();
	tray->show();

	return app.exec();
}
