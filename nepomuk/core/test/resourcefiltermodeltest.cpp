/*
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING.LIB" for the exact licensing terms.
 */

#include "resourcefiltermodeltest.h"

#include <kdebug.h>
#include <qtest_kde.h>
#include "../resourcefiltermodel.h"


void ResourceFilterModelTest::testEnsureResource()
{
    // ensure a resource
    // check that we have a new graph that is an InstanceBase
    // ensure the same resource
    // check that nothing changed

}


void ResourceFilterModelTest::testUpdateProperty1()
{
    // add a resource
    // add a property
    // check that we have a new graph which is an InstanceBase and contains the new property
    // check that we have a new graph that is a GraphMetadata and contains info about the InstanceBase

}


void ResourceFilterModelTest::testUpdateProperty2()
{
    // do the same thing as above, except add multiple properties

}

void ResourceFilterModelTest::testRemoveStatement()
{
    // add data manually including the graph (2 statements)
    // remove one statement
    // make sure the graph still exists
    // remove the second statement
    // make sure the graph is gone

    QUrl dataGraph( "http://www.nepomuk.org/test/dataGraph" );
    QUrl metadataGraph( "http://www.nepomuk.org/test/metadataGraph" );

    QUrl res1 = "http://www.nepomuk.org/test/res1";
    QUrl res2 = "http://www.nepomuk.org/test/res2";

    QUrl prop1 = Soprano::Vocabulary::RDFS::label();
    QUrl prop2 = Soprano::Vocabulary::RDFS::label();

    Soprano::LiteralValue val1( "test1" );
    Soprano::LiteralValue val2( "test2" );

    Soprano::Model* model = Soprano::createModel();
    Q_VERIFY( model );
    Nepomuk::ResourceFilterModel fm( model );

    model->addStatement( res1, prop1, val1, dataGraph );
    model->addStatement( res1, prop2, val2, dataGraph );

    // metadata
    model->addStatement( dataGraph, Soprano::Vocabulary::RDF::type(), Soprano::Vocabulary::NRL::InstanceBase(), metadataGraph );
    model->addStatement( metadataGraph, Soprano::Vocabulary::RDF::type(), Soprano::Vocabulary::NRL::GraphMetadata(), metadataGraph );

}


void ResourceFilterModelTest::testRemoveAllStatements()
{

}

QTEST_KDEMAIN(ResourceFilterModelTest, NoGUI)

#include "resourcefiltermodeltest.moc"
