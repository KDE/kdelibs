/*
    This file is part of the KDE libraries
    Copyright (C) 2007 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kfiletreeviewtest.h"

#include <QtCore/QDir>
#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QPushButton>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kurl.h>

#include <kfiletreeview.h>

Window::Window()
  : KMainWindow(0)
{
    setObjectName("Test FileTreeView");

    QWidget *mainWidget = new QWidget(this);
    QGridLayout *layout = new QGridLayout(mainWidget);

    mTreeView = new KFileTreeView(mainWidget);
    layout->addWidget(mTreeView, 0, 1, 4, 1);

    QPushButton *button = new QPushButton("Root", mainWidget);
    layout->addWidget(button, 0, 0);
    connect(button, SIGNAL(clicked()), this, SLOT(showRoot()));

    button = new QPushButton("Home", mainWidget);
    layout->addWidget(button, 1, 0);
    connect(button, SIGNAL(clicked()), this, SLOT(showHome()));

    QCheckBox *dirOnlyMode = new QCheckBox("Show only Directories", mainWidget);
    layout->addWidget(dirOnlyMode, 2, 0);
    connect(dirOnlyMode, SIGNAL(toggled(bool)), mTreeView, SLOT(setDirOnlyMode(bool)));

    layout->setRowStretch(3, 1);
 
    setCentralWidget(mainWidget);
    resize(600, 400);
}

void Window::showHome()
{
    mTreeView->setCurrentUrl( KUrl( QDir::home().absolutePath() ) );
}

void Window::showRoot()
{
    mTreeView->setCurrentUrl( KUrl( QDir::root().absolutePath() ) );
}

int main(int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "kfiletreeviewtest", 0, ki18n("kfiletreeviewtest"), "0", ki18n("test app"));
    KApplication app;

    Window *window = new Window;
    window->show();

    return app.exec();
}

#include "kfiletreeviewtest.moc"
