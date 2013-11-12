#include "kpassivepopuptest.h"
#include <QApplication>
#include <kpassivepopup.h>
#include <QPushButton>
#include <QLatin1String>
#include <qsystemtrayicon.h>

#include <config-knotifications.h>

#if HAVE_X11
#include <QX11Info>
#include <netwm.h>
#endif

QPushButton *pb;
QPushButton *pb2;
QPushButton *pb3;
QPushButton *pb4;
QPushButton *pb5;
#if HAVE_X11
QPushButton *pb6;
#endif
QSystemTrayIcon *icon;

void Test::showIt()
{
  KPassivePopup::message( QLatin1String("Hello World"), pb );
}

void Test::showIt2()
{
  KPassivePopup::message( QLatin1String("The caption is..."), QLatin1String("Hello World"), pb2 );
}

void Test::showIt3()
{
  KPassivePopup *pop = new KPassivePopup();
  pop->setView( QLatin1String("Caption"), QLatin1String("test") );
  pop->show();
}

void Test::showIt4()
{
  KPassivePopup::message( KPassivePopup::Boxed, QLatin1String("The caption is..."), QLatin1String("Hello World"), pb4 );
}

void Test::showIt5()
{
  KPassivePopup::message( KPassivePopup::Balloon, QLatin1String("The caption is..."), QLatin1String("Hello World"), pb5 );
}

#if HAVE_X11
void Test::showIt6()
{
  KPassivePopup::message( KPassivePopup::Boxed, QLatin1String("The caption is..."), QLatin1String("Hello World"), pb6 );
}
#endif

void Test::showItIcon(QSystemTrayIcon::ActivationReason reason)
{
  if (reason == QSystemTrayIcon::Trigger)
    KPassivePopup::message( QLatin1String("QSystemTrayIcon test"), QLatin1String("Hello World"), icon);
}

int main( int argc, char **argv )
{
    QApplication::setApplicationName(QLatin1String("test"));
    QApplication app(argc, argv);

    Test *t = new Test();

    pb = new QPushButton();
    pb->setText( QLatin1String("By taskbar entry (no caption, default style)") );
    pb->connect( pb, SIGNAL(clicked()), t, SLOT(showIt()) );
    pb->show();

    pb2 = new QPushButton();
    pb2->setText( QLatin1String("By taskbar entry (with caption, default style)") );
    pb2->connect( pb2, SIGNAL(clicked()), t, SLOT(showIt2()) );
    pb2->show();

    pb3 = new QPushButton();
    pb3->setText( QLatin1String("Without WinID") );
    pb3->connect( pb3, SIGNAL(clicked()), t, SLOT(showIt3()) );
    pb3->show();

    pb4 = new QPushButton();
    pb4->setText( QLatin1String("By taskbar entry (with caption, boxed)") );
    pb4->connect( pb4, SIGNAL(clicked()), t, SLOT(showIt4()) );
    pb4->show();

    pb5 = new QPushButton();
    pb5->setText( QLatin1String("By taskbar entry (with caption, balloon)") );
    pb5->connect( pb5, SIGNAL(clicked()), t, SLOT(showIt5()) );
    pb5->show();

#if HAVE_X11
    pb6 = new QPushButton();
    pb6->setText( QLatin1String("By window (with caption, balloon)") );
    pb6->connect( pb6, SIGNAL(clicked()), t, SLOT(showIt6()) );
    pb6->show();
    NETWinInfo ni(QX11Info::display(), pb6->effectiveWinId(),
                  QX11Info::appRootWindow(), NET::WMState);
    ni.setState(NET::SkipTaskbar, NET::SkipTaskbar);
#endif

    icon = new QSystemTrayIcon();
    // TODO icon->setIcon(icon->loadIcon("xorg"));
    icon->connect( icon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), t, SLOT(showItIcon(QSystemTrayIcon::ActivationReason)) );
    icon->show();

    return app.exec();

}
