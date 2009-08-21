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

#include <QtGui/QCheckBox>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtGui/QTextEdit>
#include <QtGui/QLineEdit>

#include <kicon.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <klocale.h>
#include <kstandarddirs.h>

Part1::Part1( QObject *parent, QWidget * parentWidget )
    : KParts::ReadOnlyPart(parent),
      m_componentData("kpartstestpart")
{
    setComponentData(m_componentData, false);
    m_edit = new QTextEdit( parentWidget );
    setWidget( m_edit );

    // KXMLGUIClient looks in the "data" resource for the .rc files
    // This line is for test programs only!
    m_componentData.dirs()->addResourceDir( "data", KDESRCDIR );
    setXMLFile( "kpartstest_part1.rc" );

    // An action and an action menu (test code for #70459)

    KAction* testAction = actionCollection()->addAction("p1_blah");
    testAction->setText("Part1's action");
    testAction->setShortcut(Qt::CTRL + Qt::Key_B);
    connect(testAction, SIGNAL(triggered()), this, SLOT(slotBlah()));

    KActionMenu * menu = new KActionMenu(KIcon("mail_forward"), "Foo", this);
    actionCollection()->addAction("p1_foo", menu);

    KAction* mailForward = new KAction(KIcon("mail_forward"), "Bar", this);
    mailForward->setShortcut(Qt::CTRL + Qt::Key_F);
    connect(mailForward, SIGNAL(triggered()), this, SLOT(slotFooBar()));
    actionCollection()->addAction("p1_foo_bar", mailForward);
    menu->addAction(mailForward);

    loadPlugins();
}

Part1::~Part1()
{
}

void Part1::slotBlah()
{
  m_edit->setText( "Blah" );
}

void Part1::slotFooBar()
{
  m_edit->setText( "FooBar" );
}

bool Part1::openFile()
{
    kDebug() << "Part1: opening " << QFile::encodeName(localFilePath());
    // Hehe this is from a tutorial I did some time ago :)
    QFile f(localFilePath());
    QString s;
    if ( f.open(QIODevice::ReadOnly) ) {
        QTextStream t( &f );
        while ( !t.atEnd() ) {
            s += t.readLine() + "\n";
        }
        f.close();
    } else
        return false;
    m_edit->setPlainText(s);

    emit setStatusBarText( url().prettyUrl() );

    return true;
}

Part2::Part2( QObject *parent, QWidget * parentWidget )
    : KParts::Part(parent),
    m_componentData("part2")
{
    setComponentData(m_componentData, false);
    QWidget * w = new QWidget( parentWidget );
    w->setObjectName( "Part2Widget" );
    setWidget( w );

    m_componentData.dirs()->addResourceDir( "data", KDESRCDIR );
    setXMLFile( "kpartstest_part2.rc" );

    /*QCheckBox * cb =*/ new QCheckBox( "something", w );

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
        emit setWindowCaption("[part2 activated]");
}

#include "parts.moc"
