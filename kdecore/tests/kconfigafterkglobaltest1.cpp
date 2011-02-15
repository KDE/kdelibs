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
#include <kcomponentdata.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>

class Tester
{
    public:
        void initConfig();
        ~Tester();

    private:
        KConfig *m_config;
};

void Tester::initConfig()
{
    m_config = new KConfig("kconfigafterkglobaltest");
}

K_GLOBAL_STATIC(Tester, globalTestObject)

int main(int argc, char **argv)
{
    Tester *t = globalTestObject;
    Q_UNUSED(t);

    QCoreApplication app(argc, argv);
    KComponentData componentData("kconfigafterkglobaltest");

    t->initConfig();
}

Tester::~Tester()
{
    // the KGlobal K_GLOBAL_STATIC should already be deleted
    // If this fails, check that a new global static in kdecore didn't create KGlobal indirectly;
    // e.g. this can happen if someone writes 'static int s_area = KDebug::registerArea("foo");' in
    // a .cpp file, rather than using a function.
    Q_ASSERT(!KGlobal::hasMainComponent());
    KConfigGroup group = m_config->group("test");
    group.writeEntry("test", 1);
    delete m_config; // this calls KConfig::sync() which needs KGlobal::locale()
}
