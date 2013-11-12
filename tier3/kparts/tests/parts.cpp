/*
    Copyright (c) 1999, 2000 David Faure <faure@kde.org>
    Copyright (c) 1999, 2000 Simon Hausmann <hausmann@kde.org>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License or ( at
    your option ) version 3 or, at the discretion of KDE e.V. ( which shall
    act as a proxy as in section 14 of the GPLv3 ), any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kparts/event.h>

#include "parts.h"
#include <kactionmenu.h>
#include <kactioncollection.h>

#include <QAction>
#include <QCheckBox>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QTest>
#include <QTextEdit>
#include <QLineEdit>
#include <QTest>

#include <QDebug>
#include <klocalizedstring.h>

Part1::Part1( QObject *parent, QWidget * parentWidget )
    : KParts::ReadOnlyPart(parent),
    m_componentData(QStringLiteral("kpartstestpart"), QString(), i18n("KParts test part"), QStringLiteral("0.1"))
{
    setComponentData(m_componentData, false);
    m_edit = new QTextEdit( parentWidget );
    setWidget( m_edit );

    setXMLFile(QFINDTESTDATA("kpartstest_part1.rc"));

    // An action and an action menu (test code for #70459)

    QAction* testAction = actionCollection()->addAction(QStringLiteral("p1_blah"));
    testAction->setText(QStringLiteral("Part1's action"));
    testAction->setShortcut(Qt::CTRL + Qt::Key_B);
    connect(testAction, SIGNAL(triggered()), this, SLOT(slotBlah()));

    KActionMenu * menu = new KActionMenu(QIcon::fromTheme(QStringLiteral("mail_forward")), QStringLiteral("Foo"), this);
    actionCollection()->addAction(QStringLiteral("p1_foo"), menu);

    QAction* mailForward = new QAction(QIcon::fromTheme(QStringLiteral("mail_forward")), QStringLiteral("Bar"), this);
    mailForward->setShortcut(Qt::CTRL + Qt::Key_F);
    connect(mailForward, SIGNAL(triggered()), this, SLOT(slotFooBar()));
    actionCollection()->addAction(QStringLiteral("p1_foo_bar"), mailForward);
    menu->addAction(mailForward);

    loadPlugins();
}

Part1::~Part1()
{
}

void Part1::slotBlah()
{
  m_edit->setText( QStringLiteral("Blah") );
}

void Part1::slotFooBar()
{
  m_edit->setText( QStringLiteral("FooBar") );
}

bool Part1::openFile()
{
    // qDebug() << "Part1: opening " << QFile::encodeName(localFilePath());
    // Hehe this is from a tutorial I did some time ago :)
    QFile f(localFilePath());
    QString s;
    if ( f.open(QIODevice::ReadOnly) ) {
        QTextStream t( &f );
        while ( !t.atEnd() ) {
            s += t.readLine() + QStringLiteral("\n");
        }
        f.close();
    } else
        return false;
    m_edit->setPlainText(s);

    emit setStatusBarText(url().toString());

    return true;
}

Part2::Part2( QObject *parent, QWidget * parentWidget )
    : KParts::Part(parent),
    m_componentData(QStringLiteral("part2"), QString(), QStringLiteral("Part 2"), QStringLiteral("0.2"))
{
    setComponentData(m_componentData, false);
    QWidget * w = new QWidget( parentWidget );
    w->setObjectName( QStringLiteral("Part2Widget") );
    setWidget( w );

    setXMLFile(QFINDTESTDATA("kpartstest_part2.rc"));

    /*QCheckBox * cb =*/ new QCheckBox( QStringLiteral("something"), w );

    //QLineEdit * l = new QLineEdit( "something", widget() );
    //l->move(0,50);
    // Since the main widget is a dummy one, we HAVE to set
    // strong focus for it, otherwise we get the
    // the famous activating-file-menu-switches-part bug.
    w->setFocusPolicy( Qt::ClickFocus );

    // loadPlugins(); // in case we want to allow plugins for this part.
}

Part2::~Part2()
{
}

void Part2::guiActivateEvent( KParts::GUIActivateEvent * event )
{
    if (event->activated())
        emit setWindowCaption(QStringLiteral("[part2 activated]"));
}

