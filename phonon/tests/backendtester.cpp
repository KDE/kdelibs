/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "backendtester.h"
#include <kapplication.h>
#include <kcmdlineargs.h>

MainWindow::MainWindow()
{
    setupUi(this);
}

MainWindow::~MainWindow()
{
}

int main(int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "backendtester", 0, ki18n("Backend Tester"), 0);
    KApplication app;
    MainWindow mw;
    mw.show();
    return app.exec();
}

#include "backendtester.moc"
// vim: sw=4 sts=4 et tw=100
