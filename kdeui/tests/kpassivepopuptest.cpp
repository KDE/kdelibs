#include "kpassivepopuptest.h"
#include "kpassivepopuptest.moc"
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kpassivepopup.h>
#include <QtGui/QPushButton>
#include <ksystemtrayicon.h>

QPushButton *pb;
QPushButton *pb2;
QPushButton *pb3;
QPushButton *pb4;
QPushButton *pb5;
KSystemTrayIcon *icon;

void Test::showIt()
{
  KPassivePopup::message( "Hello World", pb );
}

void Test::showIt2()
{
  KPassivePopup::message( "The caption is...", "Hello World", pb2 );
}

void Test::showIt3()
{
  KPassivePopup *pop = new KPassivePopup( pb3->winId() );
  pop->setView( "Caption", "test" );
  pop->show();
}

void Test::showIt4()
{
  KPassivePopup::message( KPassivePopup::Boxed, "The caption is...", "Hello World", pb4 );
}

void Test::showIt5()
{
  KPassivePopup::message( KPassivePopup::Balloon, "The caption is...", "Hello World", pb5 );
}

void Test::showIt6(QSystemTrayIcon::ActivationReason reason)
{
  if (reason == QSystemTrayIcon::Trigger)
    KPassivePopup::message( "QSystemTrayIcon test", "Hello World", icon);
}

int main( int argc, char **argv )
{
    KCmdLineArgs::init( argc, argv, "test", 0, ki18n("Test"), "1.0", ki18n("test app"));
    KApplication app;

    Test *t = new Test();

    pb = new QPushButton();
    pb->setText( "By taskbar entry" );
    pb->connect( pb, SIGNAL(clicked()), t, SLOT( showIt() ) );
    pb->show();

    pb2 = new QPushButton();
    pb2->setText( "By taskbar entry (with caption)" );
    pb2->connect( pb2, SIGNAL(clicked()), t, SLOT( showIt2() ) );
    pb2->show();

    pb3 = new QPushButton();
    pb3->setText( "By WinId" );
    pb3->connect( pb3, SIGNAL(clicked()), t, SLOT( showIt3() ) );
    pb3->show();

    pb4 = new QPushButton();
    pb4->setText( "Boxed taskbar entry" );
    pb4->connect( pb4, SIGNAL(clicked()), t, SLOT( showIt4() ) );
    pb4->show();

    pb5 = new QPushButton();
    pb5->setText( "Balloon taskbar entry" );
    pb5->connect( pb5, SIGNAL(clicked()), t, SLOT( showIt5() ) );
    pb5->show();

    icon = new KSystemTrayIcon();
    icon->setIcon(icon->loadIcon("xorg"));
    icon->connect( icon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), t, SLOT( showIt6(QSystemTrayIcon::ActivationReason) ) );
    icon->show();

    return app.exec();

}
