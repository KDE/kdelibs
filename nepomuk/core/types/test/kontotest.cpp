/* This file is part of the Nepomuk-KDE libraries
    Copyright (c) 2007 Sebastian Trueg <trueg@kde.org>

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

#include "kontotest.h"
#include "../ontologyloader.h"
#include "../ontologymanager.h"
#include "../ontology.h"
#include "../class.h"
#include "../global.h"
#include "../property.h"

#include <Soprano/Statement>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/RDFS>
#include <Soprano/Vocabulary/NRL>

#include <QtTest/QTest>
#include <QtCore/QCoreApplication>


using namespace Nepomuk;
using namespace Soprano;

class DummyOntologyLoader : public OntologyLoader
{
public:
    QList<Statement> loadOntology( const QUrl& uri )
    {
        // create some dummy onto stuff
        QList<Statement> sl;

        QString ns = uri.toString();
        if ( !ns.endsWith( "#" ) ) {
            ns += '#';
        }

        // one dummy class
        sl.append( Statement( Node( QUrl( ns + "DummyClass" ) ),
                              Node( Soprano::Vocabulary::RDF::type() ),
                              Node( Soprano::Vocabulary::RDFS::Class() ) ) );
        sl.append( Statement( Node( QUrl( ns + "DummyClass" ) ),
                              Node( Soprano::Vocabulary::RDFS::label() ),
                              Node( LiteralValue( "A dummy class" ) ) ) );

        sl.append( Statement( Node( QUrl( ns + "DummySubClass" ) ),
                              Node( Soprano::Vocabulary::RDF::type() ),
                              Node( Soprano::Vocabulary::RDFS::Class() ) ) );
        sl.append( Statement( Node( QUrl( ns + "DummySubClass" ) ),
                              Node( Soprano::Vocabulary::RDFS::subClassOf() ),
                              Node( QUrl( ns + "DummyClass" ) ) ) );

        sl.append( Statement( Node( QUrl( ns + "DummySubSubClass" ) ),
                              Node( Soprano::Vocabulary::RDF::type() ),
                              Node( Soprano::Vocabulary::RDFS::Class() ) ) );
        sl.append( Statement( Node( QUrl( ns + "DummySubSubClass" ) ),
                              Node( Soprano::Vocabulary::RDFS::subClassOf() ),
                              Node( QUrl( ns + "DummySubClass" ) ) ) );

        // one dummy property
        sl.append( Statement( Node( QUrl( ns + "hasBrother" ) ),
                              Node( Soprano::Vocabulary::RDF::type() ),
                              Node( Soprano::Vocabulary::RDF::Property() ) ) );
        sl.append( Statement( Node( QUrl( ns + "hasBrother" ) ),
                              Node( Soprano::Vocabulary::RDFS::domain() ),
                              Node( QUrl( ns + "DummyClass" ) ) ) );
        sl.append( Statement( Node( QUrl( ns + "hasBrother" ) ),
                              Node( Soprano::Vocabulary::RDFS::range() ),
                              Node( QUrl( ns + "DummyClass" ) ) ) );
        sl.append( Statement( Node( QUrl( ns + "hasBrother" ) ),
                              Node( Soprano::Vocabulary::NRL::inverseProperty() ),
                              Node( QUrl( ns + "isBrotherOf" ) ) ) );

        // and its reverse property
        sl.append( Statement( Node( QUrl( ns + "isBrotherOf" ) ),
                              Node( Soprano::Vocabulary::RDF::type() ),
                              Node( Soprano::Vocabulary::RDF::Property() ) ));
        sl.append( Statement( Node( QUrl( ns + "isBrotherOf" ) ),
                              Node( Soprano::Vocabulary::RDFS::domain() ),
                              Node( QUrl( ns + "DummyClass" ) ) ) );
        sl.append( Statement( Node( QUrl( ns + "isBrotherOf" ) ),
                              Node( Soprano::Vocabulary::RDFS::range() ),
                              Node( QUrl( ns + "DummyClass" ) ) ) );
        sl.append( Statement( Node( QUrl( ns + "isBrotherOf" ) ),
                              Node( Soprano::Vocabulary::NRL::inverseProperty() ),
                              Node( QUrl( ns + "hasBrother" ) ) ) );

        return sl;
    }
};


void KontoTest::initTestCase()
{
    OntologyManager::instance()->setOntologyLoader( new DummyOntologyLoader() );
}


void KontoTest::testOntology()
{
    const Ontology* onto = Ontology::load( QUrl( "test" ) );
    QVERIFY( onto != 0 );
    QCOMPARE( onto->uri(), QUrl( "test" ) );
}


void KontoTest::testClass()
{
    const Ontology* onto = Ontology::load( QUrl( "test" ) );

    QCOMPARE( onto->allClasses().count(), 3 );
    QVERIFY( onto->findClassByName( "DummyClass" ) != 0 );
    QVERIFY( onto->findClassByUri( QUrl( "test#DummyClass" ) ) != 0 );

    const Class* dummyClass = Class::load( QUrl( "test#DummyClass" ) );
    QVERIFY( dummyClass != 0 );

    QCOMPARE( dummyClass->name(), QString( "DummyClass" ) );
    QCOMPARE( dummyClass->label(), QString( "A dummy class" ) );

    const Class* dummySubClass = onto->findClassByName( "DummySubClass" );
    const Class* dummySubSubClass = onto->findClassByName( "DummySubSubClass" );

    QVERIFY( dummySubClass != 0 );
    QVERIFY( dummySubSubClass != 0 );
    QVERIFY( dummyClass->subClasses().contains( dummySubClass ) );
    QVERIFY( dummySubClass->subClasses().contains( dummySubSubClass ) );
    QVERIFY( dummySubClass->parentClasses().contains( dummyClass ) );
    QVERIFY( dummySubSubClass->parentClasses().contains( dummySubClass ) );

    QVERIFY( dummyClass->isParentOf( dummySubClass ) );
    QVERIFY( dummyClass->isParentOf( dummySubSubClass ) );
    QVERIFY( dummySubClass->isParentOf( dummySubSubClass ) );
    QVERIFY( !dummySubClass->isParentOf( dummyClass ) );

    QVERIFY( dummySubSubClass->isSubClassOf( dummyClass ) );
    QVERIFY( dummySubSubClass->isSubClassOf( dummySubClass ) );
    QVERIFY( dummySubClass->isSubClassOf( dummyClass ) );
    QVERIFY( !dummySubClass->isSubClassOf( dummySubSubClass ) );
}


void KontoTest::testProperty()
{
    const Property* hasBrother = Property::load( QUrl( "test#hasBrother" ) );
    const Property* isBrotherOf = Property::load( QUrl( "test#isBrotherOf" ) );

    QVERIFY( hasBrother != 0 );
    QVERIFY( isBrotherOf != 0 );

    QCOMPARE( hasBrother->inverseProperty(), isBrotherOf );
    QCOMPARE( isBrotherOf->inverseProperty(), hasBrother );

    const Class* dummyClass = Class::load( QUrl( "test#DummyClass" ) );
    QCOMPARE( hasBrother->range(), dummyClass );
    QCOMPARE( hasBrother->domain(), dummyClass );

    QCOMPARE( isBrotherOf->range(), dummyClass );
    QCOMPARE( isBrotherOf->domain(), dummyClass );
}


QTEST_MAIN( KontoTest )

#include "kontotest.moc"
