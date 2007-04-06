#include <knotificationrestrictions.h>

#include <QtGui/QApplication>
#include <QtGui/QLabel>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QLabel* mainWidget = new QLabel();
    mainWidget->setText("You should see on console debug outpout for KNotificationRestrictions");

    KNotificationRestrictions knr(KNotificationRestrictions::ScreenSaver, 0);

    mainWidget->show();
    return app.exec();
}

