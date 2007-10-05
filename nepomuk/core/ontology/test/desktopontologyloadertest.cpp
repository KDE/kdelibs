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

#include "desktopontologyloadertest.h"
#include "desktopontologyloader.h"

#include <QtTest/QTest>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QList>

#include <kstandarddirs.h>
#include <kconfiggroup.h>
#include <kdesktopfile.h>
#include <kurl.h>
#include <kio/global.h>
#include <kio/deletejob.h>
#include <qtest_kde.h>

#include <Soprano/Statement>


void DesktopOntologyLoaderTest::initTestCase()
{
    // create our test folder
    QDir tmpDir( "/tmp" );
    tmpDir.mkpath( "konto_desktopontologyloader_test/knepomuk/ontologies" );
    KGlobal::dirs()->addResourceDir( "data", "/tmp/konto_desktopontologyloader_test" );

    // create our little test ontology
    QFile ontoFile( "/tmp/konto_desktopontologyloader_test/knepomuk/ontologies/test.nrl" );
    ontoFile.open( QIODevice::WriteOnly );
    QTextStream ontoStream( &ontoFile );

    // the header
    ontoStream << "<?xml version='1.0' encoding='UTF-8'?>" << endl
               << "<!DOCTYPE rdf:RDF [" << endl
               << "<!ENTITY rdf 'http://www.w3.org/1999/02/22-rdf-syntax-ns#'>" << endl
               << "<!ENTITY nrl 'http://semanticdesktop.org/ontologies/2006/11/24/nrl#'>" << endl
               << "<!ENTITY nao 'http://semanticdesktop.org/ontologies/2007/03/31/nao#'>" << endl
               << "<!ENTITY xsd 'http://www.w3.org/2001/XMLSchema#'>" << endl
               << "<!ENTITY rdfs 'http://www.w3.org/2000/01/rdf-schema#'>" << endl
               << "<!ENTITY test 'http://test.org/test#'>" << endl
               << "]>" << endl
               << "<rdf:RDF xmlns:rdf=\"&rdf;\" xmlns:rdfs=\"&rdfs;\" xmlns:nrl=\"&nrl;\" xmlns:nao=\"&nao;\" xmlns:xsd=\"&xsd;\">" << endl;

    // one class
    ontoStream << "<rdf:Description rdf:about=\"&test;Test\">" << endl
               << "<rdfs:label>test</rdfs:label>" << endl
               << "<rdf:type rdf:resource=\"&rdfs;Class\"/>" << endl
               << "</rdf:Description>" << endl;

    // the footer
    ontoStream << "</rdf:RDF>" << endl;

    KDesktopFile desktopFile( "/tmp/konto_desktopontologyloader_test/knepomuk/ontologies/test.desktop" );
    KConfigGroup desktopGroup( desktopFile.desktopGroup() );
    desktopGroup.writeEntry( "Type", "Link" );
    desktopGroup.writeEntry( "Name", "Test Ontology" );
    desktopGroup.writeEntry( "MimeType", "text/rdf" );
    desktopGroup.writeEntry( "Path", "test.nrl" );
    desktopGroup.writeEntry( "URL", "http://test.org/test" );
}


void DesktopOntologyLoaderTest::testLoading()
{
    Nepomuk::DesktopOntologyLoader loader;
    QList<Soprano::Statement> sl = loader.loadOntology( QUrl( "http://test.org/test" ) );
    QVERIFY( !sl.isEmpty() );
    QCOMPARE( sl.count(), 2 );
}


void DesktopOntologyLoaderTest::cleanupTestCase()
{
    QFile::remove( "/tmp/konto_desktopontologyloader_test/knepomuk/ontologies/test.desktop" );
    QFile::remove( "/tmp/konto_desktopontologyloader_test/knepomuk/ontologies/test.nrl" );
    QDir( "/tmp/konto_desktopontologyloader_test/knepomuk/" ).rmdir( "ontologies" );
    QDir( "/tmp/konto_desktopontologyloader_test" ).rmdir( "knepomuk" );
    QDir( "/tmp/" ).rmdir( "konto_desktopontologyloader_test" );
}

QTEST_KDEMAIN( DesktopOntologyLoaderTest, NoGUI )

#include "desktopontologyloadertest.moc"
