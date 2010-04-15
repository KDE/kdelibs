/* This file is part of the KDE libraries
   Copyright 2009 by Marco Martin <notmart@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later
   version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kstatusnotifieritemtest.h"

#include "notifications/kstatusnotifieritem.h"
#include <QDateTime>
#include <QtGui/QLabel>
#include <QMovie>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kaboutdata.h>
#include <kmenu.h>
#include <kicon.h>

KStatusNotifierItemTest::KStatusNotifierItemTest(QObject *parent, KStatusNotifierItem *tray)
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

void KStatusNotifierItemTest::setNeedsAttention()
{
    kDebug()<<"Asking for attention";
    m_tray->showMessage("message test", "Test of the new systemtray notifications wrapper", "konqueror", 3000);
    m_tray->setStatus(KStatusNotifierItem::NeedsAttention);
}

void KStatusNotifierItemTest::setActive()
{
    kDebug()<<"Systray icon in active state";
    m_tray->setStatus(KStatusNotifierItem::Active);
}

void KStatusNotifierItemTest::setPassive()
{
    kDebug()<<"Systray icon in passive state";
    m_tray->setStatus(KStatusNotifierItem::Passive);
}

int main(int argc, char **argv)
{
    KAboutData aboutData( "kstatusnotifieritemtest", 0 , ki18n("KStatusNotifierItemtest"), "1.0" );
    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineOptions options;
    options.add("active-icon <name>", ki18n("Name of active icon"), "konqueror");
    options.add("ksni-count <count>", ki18n("How many instances of KStatusNotifierItem to create"), "1");
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    QLabel *l = new QLabel("System Tray Main Window", 0L);

    int ksniCount = args->getOption("ksni-count").toInt();
    for (int x=0; x < ksniCount; ++x) {
        KStatusNotifierItem *tray = new KStatusNotifierItem(l);

        new KStatusNotifierItemTest(0, tray);

        tray->setTitle("DBus System tray test");
        tray->setIconByName(args->getOption("active-icon"));
        //tray->setImage(KIcon("konqueror"));
        //tray->setAttentionIconByName("kmail");
        tray->setOverlayIconByName("emblem-important");
        tray->setAttentionMovieByName(KIconLoader::global()->moviePath( QLatin1String( "newmessage" ), KIconLoader::Panel ));

        tray->setToolTipIconByName("konqueror");
        tray->setToolTipTitle("DBus System tray test");
        tray->setToolTipSubTitle("This is a test of the new systemtray specification");

        tray->setToolTip("konqueror", QString("DBus System tray test #%1").arg(x + 1), "This is a test of the new systemtray specification");

        tray->showMessage("message test", "Test of the new systemtray notifications wrapper", "konqueror", 3000);
        //tray->setStandardActionsEnabled(false);
    }

    return app.exec();
}

#include <kstatusnotifieritemtest.moc>
