/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2008-2009 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "graphwrapper_p.h"

#include <Soprano/Model>
#include <Soprano/Node>
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/NRL>
#include <Soprano/Vocabulary/NAO>

#include <QtCore/QDateTime>
#include <QtCore/QUuid>
#include <QtCore/QRegExp>

namespace {
    QUrl createGraphUri() {
        return QUrl( "urn:nepomuk:local:" + QUuid::createUuid().toString().remove( QRegExp( "[\\{\\}]" ) ) );
    }

    QUrl createUniqueGraphUri( Soprano::Model* model ) {
        while( 1 ) {
            QUrl uri = createGraphUri();
            if ( !model->executeQuery( QString("ask where { { <%1> ?p1 ?o1 . } UNION { ?r2 <%1> ?o2 . } UNION { ?r3 ?p3 <%1> . } }")
                                       .arg( QString::fromAscii( uri.toEncoded() ) ), Soprano::Query::QueryLanguageSparql ).boolValue() ) {
                return uri;
            }
        }
    }
}


Nepomuk::GraphWrapper::GraphWrapper( QObject* parent )
    : QObject( parent ),
      m_currentGraphStored( true )
{
    connect( &m_timer, SIGNAL(timeout()),
             this, SLOT(slotTimeout()) );
    m_timer.setSingleShot( true );
}


Nepomuk::GraphWrapper::~GraphWrapper()
{
}


void Nepomuk::GraphWrapper::setModel( Soprano::Model* model )
{
    if( m_model != model ) {
        m_model = model;
        m_currentGraph = QUrl();
    }
}


QUrl Nepomuk::GraphWrapper::lookupCurrentGraph() const
{
    return m_currentGraph;
}


QUrl Nepomuk::GraphWrapper::currentGraph()
{
    if( !m_currentGraph.isValid() ) {
        createNewGraph();
    }

    if( !m_currentGraphStored ) {
        storeGraph( m_currentGraph );
        m_currentGraphStored = true;
    }
    return m_currentGraph;
}


void Nepomuk::GraphWrapper::slotTimeout()
{
    // generate a new graph every event loop if the last one was used
    if( m_currentGraphStored ) {
        createNewGraph();
    }
}


void Nepomuk::GraphWrapper::createNewGraph()
{
    m_currentGraph = createUniqueGraphUri( m_model );
    m_currentGraphStored = false;
}


void Nepomuk::GraphWrapper::storeGraph( const QUrl& graph )
{
    QUrl metadataGraph = createUniqueGraphUri( m_model );

    m_model->addStatement( graph,
                           Soprano::Vocabulary::RDF::type(),
                           Soprano::Vocabulary::NRL::InstanceBase(),
                           metadataGraph );
    m_model->addStatement( graph,
                           Soprano::Vocabulary::NAO::created(),
                           Soprano::LiteralValue( QDateTime::currentDateTime() ),
                           metadataGraph );
    m_model->addStatement( metadataGraph,
                           Soprano::Vocabulary::RDF::type(),
                           Soprano::Vocabulary::NRL::GraphMetadata(),
                           metadataGraph );
    m_model->addStatement( metadataGraph,
                           Soprano::Vocabulary::NRL::coreGraphMetadataFor(),
                           graph,
                           metadataGraph );

    //
    // We update the graph every 200 mseconds but only when entering
    // the event loop.
    // I think it is ok for the user to think that two things
    // created a quarter of a second apart are created at the same time
    //
    m_timer.start( 200 );
}

#include "graphwrapper_p.moc"
