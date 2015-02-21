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

#include <QtCore/QCoreApplication>
#include <QtCore/QtGlobal>
#include <QTimer>
#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <stdio.h>

class Tester
{
public:
    void initConfig();
    ~Tester();
};

void Tester::initConfig()
{
    fprintf(stderr, "app Tester\n");
    KConfigGroup group(KSharedConfig::openConfig(), "test");
    group.writeEntry("test", 0);
}

Tester::~Tester()
{
    fprintf(stderr, "app ~Tester\n");
    KConfigGroup group(KSharedConfig::openConfig(), "test");
    group.writeEntry("test", 1);
}

Q_GLOBAL_STATIC(Tester, globalTestObject)

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    Tester *t = globalTestObject();
    t->initConfig();

    QTimer::singleShot(0, qApp, SLOT(quit()));
    return app.exec();
}
