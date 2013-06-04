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

#include "kstatusnotifieritem.h"
#include <QDateTime>
#include <QLabel>
#include <QMenu>
#include <QMovie>
#include <QApplication>

#include <qcommandlineparser.h>

#include <kiconloader.h>
#include <qdebug.h>

KStatusNotifierItemTest::KStatusNotifierItemTest(QObject *parent, KStatusNotifierItem *tray)
  : QObject(parent)
{
    QMenu *menu = tray->contextMenu();
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
    qDebug()<<"Asking for attention";
    m_tray->showMessage("message test", "Test of the new systemtray notifications wrapper", "konqueror", 3000);
    m_tray->setStatus(KStatusNotifierItem::NeedsAttention);
}

void KStatusNotifierItemTest::setActive()
{
    qDebug()<<"Systray icon in active state";
    m_tray->setStatus(KStatusNotifierItem::Active);
}

void KStatusNotifierItemTest::setPassive()
{
    qDebug()<<"Systray icon in passive state";
    m_tray->setStatus(KStatusNotifierItem::Passive);
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QCommandLineParser* parser = new QCommandLineParser;
    parser->addHelpOption(QCoreApplication::translate("main", "KStatusNotifierItemtest"));
    parser->addVersionOption("5.0");
    parser->addOption(QCommandLineOption(QStringList() << "active-icon", QCoreApplication::translate("main", "Name of active icon"), "name", false, QStringList() << "konqueror"));
    parser->addOption(QCommandLineOption(QStringList() << "ksni-count", QCoreApplication::translate("main", "How many instances of KStatusNotifierItem to create"), "count", false, QStringList() << "1"));

    if (parser->remainingArguments().count() != 0) {
        parser->showHelp();
        return ( 1 );
    }

    QLabel *l = new QLabel("System Tray Main Window", 0L);

    int ksniCount = parser->argument("ksni-count").toInt();
    for (int x=0; x < ksniCount; ++x) {
        KStatusNotifierItem *tray = new KStatusNotifierItem(l);

        new KStatusNotifierItemTest(0, tray);

        tray->setTitle("DBus System tray test");
        tray->setIconByName(parser->argument("active-icon"));
        //tray->setImage(QIcon::fromTheme("konqueror"));
        //tray->setAttentionIconByName("kmail");
        tray->setOverlayIconByName("emblem-important");
        //tray->setAttentionMovieByName(KIconLoader::global()->moviePath( QLatin1String( "newmessage" ), KIconLoader::Panel ));

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
