#include "knotificationitemtest.h"

#include "../knotificationitem.h"
#include <kapplication.h>
#include <QtGui/QLabel>
#include <QMovie>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kaboutdata.h>
#include <KMenu>
#include <KIcon>
#include <QDateTime>

using namespace Experimental;

KNotificationItemTest::KNotificationItemTest(QObject *parent, KNotificationItem *tray)
  : QObject(parent)
{
    KMenu *menu = tray->contextMenu();
    m_tray = tray;

    QAction *needsAttention = new QAction("Set needs attention", menu);
    QAction *active = new QAction("Set active", menu);
    QAction *passive = new QAction("Set passive", menu);

    menu->addAction(needsAttention);
    menu->addAction(active);
    menu->addAction(passive);

    connect(needsAttention, SIGNAL(triggered()), this, SLOT(setNeedsAttention()));
    connect(active, SIGNAL(triggered()), this, SLOT(setActive()));
    connect(passive, SIGNAL(triggered()), this, SLOT(setPassive()));
}

void KNotificationItemTest::setNeedsAttention()
{
    kDebug()<<"Asking for attention";
    m_tray->setStatus(KNotificationItem::NeedsAttention);
}

void KNotificationItemTest::setActive()
{
    kDebug()<<"Systray icon in active state";
    m_tray->setStatus(KNotificationItem::Active);
}

void KNotificationItemTest::setPassive()
{
    kDebug()<<"Systray icon in passive state";
    m_tray->setStatus(KNotificationItem::Passive);
}

int main(int argc, char **argv)
{
    KAboutData aboutData( "KNotificationItemtest", 0 , ki18n("KNotificationItemtest"), "1.0" );
    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;
    QLabel *l = new QLabel("System Tray Main Window", 0L);
    KNotificationItem *tray = new KNotificationItem(l);

    KNotificationItemTest *trayTest = new KNotificationItemTest(0, tray);


    tray->setTitle("DBus System tray test");
    //tray->setIcon("konqueror");
    tray->setImage(KIcon("konqueror"));
    tray->setAttentionIcon("kmail");
    //tray->setAttentionMovie(KIconLoader::global()->loadMovie( QLatin1String( "newmessage" ), KIconLoader::Panel ));

    tray->setToolTipIcon("konqueror");
    tray->setToolTipTitle("DBus System tray test");
    tray->setToolTipSubTitle("This is a test of the new systemtray specification");
    
    //tray->setToolTip("konqueror", "DBus System tray test", "This is a test of the new systemtray specification");

    tray->showMessage("message test", "Test of the new systemtray notifications wrapper", "konqueror", 3000);


    return app.exec();
}

#include <knotificationitemtest.moc>
