/* This file is part of the KDE libraries
    Copyright (c) 2005 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QtTest/qttest_kde.h>
#include "kcomponentfactorytest.h"
#include "kcomponentfactorytest.moc"

QTTEST_KDEMAIN( KComponentFactoryTest, false )

#include <kcomponentfactory.h>

void KComponentFactoryTest::testNonWorking()
{
    int error = 0;
    QObject* obj = KComponentFactory::createInstanceFromLibrary<QObject>( "idontexist", this, 0, QStringList(), &error );
    COMPARE( obj, (QObject*)0 );
    COMPARE( error, (int)KComponentFactory::ErrNoLibrary );
}

void KComponentFactoryTest::testWorking()
{
    SKIP( "TODO find a module to dlopen", SkipSingle );
    QObject* obj = KComponentFactory::createInstanceFromLibrary<QObject>( "kunittest_kconfig", this );
    VERIFY( obj != 0 );
}

void KComponentFactoryTest::testWrongClass()
{
    int error = 0;
    SKIP( "TODO find a module to dlopen", SkipSingle );

    KComponentFactoryTest* obj = KComponentFactory::createInstanceFromLibrary<KComponentFactoryTest>( "kunittest_kconfig", this, 0, QStringList(), &error );
    COMPARE( obj, (KComponentFactoryTest*)0 );
    COMPARE( error, (int)KComponentFactory::ErrNoComponent );
}

// ## TODO complete tests, after deciding on the final API of kcomponentfactory
