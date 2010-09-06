/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2010 Sebastian Trueg <trueg@kde.org>
   Copyright (C) 2010 Oszkar Ambrus <aoszkar@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "facetmodel.h"
#include "standardqueries.h"
#include "facet.h"
#include "facetcontainer_p.h"
#include "andterm.h"

#include <QtCore/QDate>

#include <Soprano/Vocabulary/NAO>

#include "andterm.h"
#include "orterm.h"
#include "comparisonterm.h"
#include "literalterm.h"
#include "resourceterm.h"
#include "resourcetypeterm.h"

#include "tag.h"

#include "tmo.h"
#include "nmo.h"
#include "nco.h"
#include "nfo.h"
#include "nie.h"

#include "kdebug.h"
#include "kglobal.h"
#include "klocale.h"
#include "kcalendarsystem.h"


class Nepomuk::Query::FacetModel::Private
{
public:
    QList<FacetContainer*> m_facets;

    bool m_frequenciesEnabled;
    Query m_baseQuery;

    bool m_interalFacetUpdate;

    void addFacet( const Facet& facet );
    QModelIndex parentIndexForFacet( FacetContainer* fc ) const;

    void updateFrequencies();

    void _k_facetSelectionChanged( Nepomuk::Query::FacetContainer* );
    void _k_facetFrequencyChanged( Nepomuk::Query::FacetContainer*, int, int );

    FacetModel* q;
};


void Nepomuk::Query::FacetModel::Private::addFacet( const Facet& facet )
{
    FacetContainer* fc = new FacetContainer(facet, q);
    q->connect(fc, SIGNAL(selectionChanged(Nepomuk::Query::FacetContainer*)),
               SLOT(_k_facetSelectionChanged(Nepomuk::Query::FacetContainer*)) );
    q->connect(fc, SIGNAL(frequencyChanged(Nepomuk::Query::FacetContainer*, int, int)),
               SLOT(_k_facetFrequencyChanged(Nepomuk::Query::FacetContainer*, int, int)) );
    m_facets.append(fc);
}


QModelIndex Nepomuk::Query::FacetModel::Private::parentIndexForFacet( FacetContainer* fc ) const
{
    const int i = m_facets.indexOf( fc );
    return q->index( i, 0, QModelIndex() );
}


void Nepomuk::Query::FacetModel::Private::updateFrequencies()
{
    kDebug() << m_baseQuery;
    Q_FOREACH( FacetContainer* fc, m_facets ) {
        fc->resetFrequencies();
        if( m_frequenciesEnabled ) {
            Query query(m_baseQuery);
            for( QList<FacetContainer*>::const_iterator it = m_facets.constBegin();
                 it != m_facets.constEnd(); ++it ) {
                if( fc != *it || !fc->facet().exclusive() ) {
                    query.setTerm( query.term() && (*it)->selectedTerm() );
                }
            }
            fc->updateFrequencies( query );
        }
    }
}


void Nepomuk::Query::FacetModel::Private::_k_facetSelectionChanged( Nepomuk::Query::FacetContainer* fc )
{
    if( !m_interalFacetUpdate ) {
        QModelIndex parent = parentIndexForFacet( fc );
        emit q->dataChanged( q->index( 0, 0, parent ), q->index( q->rowCount(parent)-1, 0, parent ) );
        emit q->facetsChanged();
        updateFrequencies();
    }
}


void Nepomuk::Query::FacetModel::Private::_k_facetFrequencyChanged( Nepomuk::Query::FacetContainer* fc, int index, int )
{
    const QModelIndex parentIndex = q->createIndex( m_facets.indexOf(fc), 0 );
    const QModelIndex itemIndex = q->index( index, 0, parentIndex );
    emit q->dataChanged( itemIndex, itemIndex );
}


Nepomuk::Query::FacetModel::FacetModel( QObject* parent )
    : QAbstractItemModel( parent ),
      d(new Private() )
{
    d->q = this;
    d->m_frequenciesEnabled = false;
    d->m_interalFacetUpdate = false;
}


Nepomuk::Query::FacetModel::~FacetModel()
{
    delete d;
}


int Nepomuk::Query::FacetModel::columnCount( const QModelIndex& ) const
{
    return 1;
}


QVariant Nepomuk::Query::FacetModel::data( const QModelIndex& index, int role ) const
{
    if( index.isValid() ) {
        if( index.internalPointer() ) {
            FacetContainer* facet = static_cast<FacetContainer*>( index.internalPointer() );
            switch( role ) {
            case Qt::DisplayRole: {
                QString title = facet->facet().titleAt(index.row());
                const int freq = facet->frequency(index.row());
                if(freq >= 0)
                    title += QString::fromLatin1(" (%1)").arg(freq);
                return title;
            }
            case Qt::CheckStateRole:
                return( facet->isSelected(index.row()) ? Qt::Checked : Qt::Unchecked );
            }
        }
        else {
            switch( role ) {
            case Qt::DisplayRole:
                return d->m_facets[index.row()]->facet().title();
            }
        }
    }

    return QVariant();
}


bool Nepomuk::Query::FacetModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
    if( role == Qt::CheckStateRole && index.internalPointer() ) {
        FacetContainer* facet = static_cast<FacetContainer*>( index.internalPointer() );
        facet->setSelected( index.row(), value.toBool() );
        return true;
    }
    else {
        return false;
    }
}


bool Nepomuk::Query::FacetModel::hasChildren( const QModelIndex& parent ) const
{
    // facet terms don't have children
    if( parent.internalPointer() ) {
        return false;
    }
    else {
        return rowCount(parent) > 0;
    }
}


QModelIndex Nepomuk::Query::FacetModel::parent( const QModelIndex& index ) const
{
    if( index.internalPointer() ) {
        FacetContainer* facet = static_cast<FacetContainer*>( index.internalPointer() );
        return createIndex( d->m_facets.indexOf(facet), 0 );
    }
    else {
        return QModelIndex();
    }
}


int Nepomuk::Query::FacetModel::rowCount( const QModelIndex& parent ) const
{
    if( !parent.isValid() ) {
        return d->m_facets.count();
    }
    else if( parent.row() < d->m_facets.count() ) {
        return d->m_facets[parent.row()]->facet().count();
    }
    else {
        return 0;
    }
}


QModelIndex Nepomuk::Query::FacetModel::index( int row, int column, const QModelIndex& parent ) const
{
    if(row < 0 || row >= rowCount(parent) || column < 0 || column >= columnCount(parent)) {
        return QModelIndex();
    }
    if( parent.isValid() ) {
        return createIndex( row, column, d->m_facets[parent.row()] );
    }
    else {
        return createIndex( row, column );
    }
}


Qt::ItemFlags Nepomuk::Query::FacetModel::flags( const QModelIndex& index ) const
{
    if(!index.isValid()) {
        return Qt::NoItemFlags;
    }
    // we do not even allow selection since that does not make much sense for our use case
    Qt::ItemFlags flags = Qt::ItemIsEnabled;
    if( index.internalPointer() ) {
        flags |= Qt::ItemIsUserCheckable;
    }
    return flags;
}


void Nepomuk::Query::FacetModel::setFrequenciesEnabled( bool enabled )
{
    d->m_frequenciesEnabled = enabled;
    d->updateFrequencies();
}


bool Nepomuk::Query::FacetModel::frequenciesEnabled() const
{
    return d->m_frequenciesEnabled;
}


void Nepomuk::Query::FacetModel::setBaseQuery( const Query& query )
{
    d->m_baseQuery = query;
    d->updateFrequencies();
}


Nepomuk::Query::Query Nepomuk::Query::FacetModel::baseQuery() const
{
    return d->m_baseQuery;
}


void Nepomuk::Query::FacetModel::addFacet( const Facet& facet )
{
    d->addFacet( facet );
    reset();
    emit facetsChanged();
    // TODO: do not use reset but the fancy begin/end methods
}


void Nepomuk::Query::FacetModel::setFacets( const QList<Facet>& facets )
{
    clear();
    Q_FOREACH( const Facet& facet, facets ) {
        d->addFacet( facet );
    }
    reset();
    emit facetsChanged();
}


void Nepomuk::Query::FacetModel::clear()
{
    qDeleteAll(d->m_facets);
    d->m_facets.clear();
    reset();
}


QList<Nepomuk::Query::Facet> Nepomuk::Query::FacetModel::facets() const
{
    QList<Nepomuk::Query::Facet> facets;
    Q_FOREACH( FacetContainer* fc, d->m_facets )
        facets << fc->facet();
    return facets;
}

QList<Nepomuk::Query::Term> Nepomuk::Query::FacetModel::selectedTerms() const
{
    QList<Nepomuk::Query::Term> terms;
    Q_FOREACH( FacetContainer* fc, d->m_facets ) {
        terms.append(fc->selectedTerm());
    }
    return terms;
}


Nepomuk::Query::Query Nepomuk::Query::FacetModel::constructQuery() const
{
    Query query( d->m_baseQuery );
    Q_FOREACH( const Term& term, selectedTerms() ) {
        query.setTerm( query.term() && term );
    }
    return query.optimized();
}


// FIXME: we need greedy FacetContainers that extract as much as possible from a term
//        ideally we would then give the terms to the container extracting the most
//        This would handle the situation where one facet term contains another facet term
//        but adds more restrictions.
//        Then FacetContainer::selectFromTerm would also be able to handle AndTerms
Nepomuk::Query::Query Nepomuk::Query::FacetModel::setQuery( const Query& query )
{
    d->m_interalFacetUpdate = true;

    // reset all facets in the model
    // ================================
    Q_FOREACH( FacetContainer* fc, d->m_facets ) {
        fc->reset();
    }

    // we extract all facets we can find and leave the rest in the query
    // ================================
    d->m_baseQuery = query.optimized();

    // first we check if the main term is already a facet term
    // (this way we can also handle facets that use AndTerms)
    // ================================
    Term term = d->m_baseQuery.term();
    // if any of the facets contains the term, set it to selected
    Q_FOREACH( Nepomuk::Query::FacetContainer *fc, d->m_facets ) {
        if( fc->selectFromTerm( term ) ) {
            d->m_baseQuery.setTerm( Term() );
            break;
        }
    }

    // now go into an AndTerm and check each sub term for facet
    // ================================
    if( d->m_baseQuery.term().isAndTerm() ) {
        AndTerm restAndTerm;
        foreach( const Term& term, d->m_baseQuery.term().toAndTerm().subTerms() ) {
            bool termFound = false;

            // if any of the facets contains the term, set it to selected
            Q_FOREACH( Nepomuk::Query::FacetContainer *fc, d->m_facets ) {
                if( fc->selectFromTerm( term ) ) {
                    termFound = true;
                    break;
                }
            }

            // we did not find a matching facet
            if( !termFound )
                restAndTerm.addSubTerm( term );
        }

        d->m_baseQuery.setTerm( restAndTerm );
    }

    d->updateFrequencies();

    d->m_interalFacetUpdate = false;

    d->m_baseQuery = d->m_baseQuery.optimized();

    // tell clients that the selection changed
    for( int i = 0; i < d->m_facets.count(); ++i ) {
        QModelIndex parent = index( i, 0, QModelIndex() );
        emit dataChanged( index( 0, 0, parent ), index( rowCount(parent)-1, 0, parent ) );
    }

    return d->m_baseQuery;
}


// static
QList<Nepomuk::Query::Facet> Nepomuk::Query::FacetModel::defaultFacets( DefaultFacetFlags flags )
{
    /*
     * Some of this code is based on code form Sembrowser:
     * Copyright (c) 2009-2010 Alessandro Sivieri <alessandro.sivieri@gmail.com>
     */

    QList<Facet> facets;

    // Types facet
    // ==================================
    Facet facetTypes;
    facetTypes.setTitle( i18n("Type") );
    facetTypes.setNoSelectionTitle( i18n("All") );
    facetTypes.setExclusive( true );

    if( flags & CreateFileFacets ) {
        facetTypes.addTerm( ResourceTypeTerm(Nepomuk::Vocabulary::NFO::Document()),
                            i18n("Documents") );

        // need to check the mimetype as well since strigi is still not perfect
        facetTypes.addTerm( ResourceTypeTerm(Nepomuk::Vocabulary::NFO::Audio()) ||
                            ResourceTypeTerm(Nepomuk::Vocabulary::NFO::Video()) ||
                            ComparisonTerm(Nepomuk::Vocabulary::NIE::mimeType(), LiteralTerm(QLatin1String("video"))) ||
                            ComparisonTerm(Nepomuk::Vocabulary::NIE::mimeType(), LiteralTerm(QLatin1String("audio"))),
                            i18n("Media") );

        facetTypes.addTerm( ResourceTypeTerm(Nepomuk::Vocabulary::NFO::Image()),
                            i18n("Images") );
    }
    else {
        facetTypes.addTerm(ResourceTypeTerm(Nepomuk::Vocabulary::NFO::FileDataObject()),
                           i18n("Files"));
        facetTypes.addTerm(ResourceTypeTerm(Nepomuk::Vocabulary::NCO::Contact()),
                           i18n("Contacts"));
        facetTypes.addTerm(ResourceTypeTerm(Nepomuk::Vocabulary::NMO::Email()),
                           i18n("Emails"));
        facetTypes.addTerm(ResourceTypeTerm(Nepomuk::Vocabulary::TMO::Task()),
                           i18n("Tasks"));
        facetTypes.addTerm(ResourceTypeTerm(Soprano::Vocabulary::NAO::Tag()),
                           i18n("Tags"));
    }

    facets.append(facetTypes);


    // Tags facet
    // ==================================
    Facet facetTags;
    facetTags.setTitle(i18n("Tags"));
    facetTags.setExclusive(false);
    foreach(Nepomuk::Tag tag, Nepomuk::Tag::allTags()) {
        facetTags.addTerm(Soprano::Vocabulary::NAO::hasTag() == ResourceTerm(tag), i18n("Tagged '%1'", tag.genericLabel()));
    }
    facets.append(facetTags);


    // Priority facet
    // ==================================
    Facet facetPriority;
    facetPriority.setTitle(i18n("Prioritize"));
    facetPriority.setExclusive(true);
    facetPriority.setNoSelectionTitle(i18n("No priority"));
    facetPriority.addTerm( standardQuery( LastModifiedFilesQuery ).term(), i18n("Last modified") );
    facetPriority.addTerm( standardQuery( MostImportantResourcesQuery ).term(), i18n("Most important") );
    facetPriority.addTerm( standardQuery( NeverOpenedFilesQuery ).term(), i18n("Never opened") );
    facets.append(facetPriority);


    // Date facet
    // ==================================
    Facet facetDate;
    facetDate.setTitle( i18n("Date") );
    facetDate.setExclusive(true);
    const QDate today = QDate::currentDate();

    facetDate.setNoSelectionTitle( i18n("Anytime") );

    facetDate.addTerm( dateRangeQuery( today, today ).term(),
                       i18n("Today") );
    facetDate.addTerm( dateRangeQuery( today.addDays(-1), today.addDays(-1) ).term(),
                       i18n("Yesterday") );
    facetDate.addTerm( dateRangeQuery( today.addDays(-today.dayOfWeek()), today.addDays(7-today.dayOfWeek() ) ).term(),
                       i18n("This week") );
    facetDate.addTerm( dateRangeQuery( QDate( QDate::currentDate().year(), QDate::currentDate().month(), 1 ),
                                       QDate( QDate::currentDate().year(), QDate::currentDate().month(),
                                              KGlobal::locale()->calendar()->daysInMonth( QDate::currentDate() ) ) ).term(),
                       i18n("This month") );
    facetDate.addTerm( dateRangeQuery( QDate( QDate::currentDate().year(), 1, 1 ),
                                       QDate( QDate::currentDate().year(), 12, 31) ).term(),
                       i18n("This year") );
    facets.append(facetDate);

    return facets;
}

#include "facetmodel.moc"
