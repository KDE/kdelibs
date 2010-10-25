/*
   Copyright (c) 2010 Oszkar Ambrus <aoszkar@gmail.com>
   Copyright (C) 2010 Sebastian Trueg <trueg@kde.org>

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

#include "searchwidget.h"
#include "searchwidget_p.h"

#include "searchlineedit.h"
#include "facetwidget.h"
#include "simpleresourcemodel.h"

#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QListView>
#include <QtCore/QList>

#include "kdebug.h"
#include "kpushbutton.h"
#include "kdialog.h"

#include "query.h"
#include "queryparser.h"
#include "result.h"
#include "andterm.h"
#include "orterm.h"
#include "literalterm.h"
#include "comparisonterm.h"
#include "resourceterm.h"
#include "queryserviceclient.h"
#include "resource.h"

#include <Soprano/LiteralValue>
#include <Soprano/Vocabulary/NAO>

Q_DECLARE_METATYPE(Nepomuk::Resource)


void Nepomuk::Utils::SearchWidget::SearchWidgetPrivate::_k_queryComponentChanged()
{
    if ( !m_inQueryComponentChanged ) {
        m_inQueryComponentChanged = true;
        const Query::Query query = currentQuery();
        m_resourceModel->clear();
        // TODO: show busy indicator
        m_queryClient.query(query);
        m_facetWidget->setClientQuery(query);
        m_inQueryComponentChanged = false;
    }
}


void Nepomuk::Utils::SearchWidget::SearchWidgetPrivate::_k_listingFinished()
{
    // TODO: disable busy indicator
}


Nepomuk::Query::Query Nepomuk::Utils::SearchWidget::SearchWidgetPrivate::currentQuery( bool withBaseQuery ) const
{
    Query::Query query;
    if( withBaseQuery )
        query = m_baseQuery;

    Query::Term facetTerm = m_facetWidget->queryTerm();
    Query::Term userQueryTerm = m_queryEdit->query().term();

    kDebug() << "************ userQuery:  " << userQueryTerm;
    kDebug() << "************ facetQuery: " << facetTerm;

    return query && facetTerm && userQueryTerm;
}


Nepomuk::Utils::SearchWidget::SearchWidget(QWidget *parent)
    : QWidget(parent),
      d_ptr(new SearchWidgetPrivate())
{
    Q_D(SearchWidget);

    //query editor widget
    d->m_queryEdit = new SearchLineEdit(this);
    d->m_queryButton = new KPushButton(i18n("Search"), this);
    connect(d->m_queryEdit, SIGNAL(queryChanged(QString)), this, SLOT(_k_queryComponentChanged()));
    connect(d->m_queryButton, SIGNAL(clicked()), this, SLOT(_k_queryComponentChanged()));

    //item widget
    d->m_itemWidget = new QListView(this);
    connect(d->m_itemWidget->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SIGNAL(selectionChanged()));

    d->m_resourceModel = new Utils::SimpleResourceModel(this);
    d->m_itemWidget->setModel(d->m_resourceModel);

    //facets widget
    d->m_facetWidget = new Nepomuk::Utils::FacetWidget(this);
    connect(d->m_facetWidget, SIGNAL(facetsChanged()), this, SLOT(_k_queryComponentChanged()));

    //layout and config
    QGridLayout* layout = new QGridLayout( this );
    layout->addWidget( d->m_queryEdit, 0, 0 );
    layout->addWidget( d->m_queryButton, 0, 1 );
    layout->addWidget( d->m_itemWidget, 1, 0, 1, 2 );
    layout->addWidget( d->m_facetWidget, 0, 2, 2, 1 );

    // query client setup
    connect( &d->m_queryClient, SIGNAL(newEntries(QList<Nepomuk::Query::Result>)),
            d->m_resourceModel, SLOT(addResults(QList<Nepomuk::Query::Result>)) );
    connect( &d->m_queryClient, SIGNAL(finishedListing()),
            this, SLOT(_k_listingFinished()));

    setSelectionMode(QListView::ExtendedSelection);
    setConfigFlags( DefaultConfigFlags );
}

Nepomuk::Utils::SearchWidget::~SearchWidget()
{
    delete d_ptr;
}

void Nepomuk::Utils::SearchWidget::setConfigFlags( ConfigFlags flags )
{
    Q_D(SearchWidget);
    d->m_configFlags = flags;
    d->m_facetWidget->setShown( flags&ShowFacets );
    d->m_queryButton->setShown( !( flags&SearchWhileYouType ) );
    d->m_queryEdit->setSearchWhileTypingEnabled( flags&SearchWhileYouType );
}

Nepomuk::Utils::SearchWidget::ConfigFlags Nepomuk::Utils::SearchWidget::configFlags() const
{
    Q_D(const SearchWidget);
    return d->m_configFlags;
}

void Nepomuk::Utils::SearchWidget::setSelectionMode ( QAbstractItemView::SelectionMode mode )
{
    Q_D(SearchWidget);

    d->m_itemWidget->setSelectionMode(mode);
}

QAbstractItemView::SelectionMode Nepomuk::Utils::SearchWidget::selectionMode () const
{
    Q_D(const SearchWidget);

    return d->m_itemWidget->selectionMode();
}

void Nepomuk::Utils::SearchWidget::setQuery(const Nepomuk::Query::Query &query)
{
    Query::Query restQuery( query );
    restQuery.setTerm( setQueryTerm( query.term() ) );
    setBaseQuery( restQuery );
}

Nepomuk::Query::Term Nepomuk::Utils::SearchWidget::setQueryTerm( const Nepomuk::Query::Term& term )
{
    Q_D(SearchWidget);

    Query::Term restTerm( term);

    // try to extract as much as possible from the query as facets
    restTerm = d->m_facetWidget->extractFacetsFromTerm( restTerm );

    // try to get the rest into the line edit
    restTerm = d->m_queryEdit->extractUsableTerms( restTerm );

    return restTerm;
}

void Nepomuk::Utils::SearchWidget::setBaseQuery( const Query::Query& query )
{
    Q_D(SearchWidget);
    d->m_baseQuery = query;
    d->_k_queryComponentChanged();
}

Nepomuk::Query::Query Nepomuk::Utils::SearchWidget::baseQuery() const
{
    Q_D(const SearchWidget);
    return d->m_baseQuery;
}

Nepomuk::Query::Query Nepomuk::Utils::SearchWidget::query() const
{
    Q_D(const SearchWidget);
    return d->currentQuery();
}

Nepomuk::Resource Nepomuk::Utils::SearchWidget::currentResource() const
{
    Q_D(const SearchWidget);
    return d->m_itemWidget->currentIndex().data(Utils::ResourceModel::ResourceRole).value<Resource>();
}

QList<Nepomuk::Resource> Nepomuk::Utils::SearchWidget::selectedResources() const
{
    Q_D(const SearchWidget);
    QList<Nepomuk::Resource> resourceList;
    foreach(const QModelIndex& index, d->m_itemWidget->selectionModel()->selectedIndexes()) {
        resourceList << index.data(Utils::ResourceModel::ResourceRole).value<Resource>();
    }
    return resourceList;
}

// static
Nepomuk::Resource Nepomuk::Utils::SearchWidget::searchResource( QWidget* parent,
                                                               const Nepomuk::Query::Query& baseQuery,
                                                               SearchWidget::ConfigFlags flags )
{
    KDialog dlg( parent );
    dlg.setButtons(KDialog::Ok | KDialog::Cancel);
    SearchWidget* searchWidget = new SearchWidget(&dlg);
    dlg.setMainWidget(searchWidget);

    searchWidget->setBaseQuery( baseQuery );
    searchWidget->setConfigFlags( flags );
    searchWidget->setSelectionMode( QAbstractItemView::SingleSelection );

    if( dlg.exec() == QDialog::Accepted ) {
        return searchWidget->currentResource();
    }
    else {
        return Nepomuk::Resource();
    }
}


// static
QList<Nepomuk::Resource> Nepomuk::Utils::SearchWidget::searchResources( QWidget* parent,
                                                                       const Nepomuk::Query::Query& baseQuery,
                                                                       SearchWidget::ConfigFlags flags )
{
    KDialog dlg( parent );
    dlg.setButtons(KDialog::Ok | KDialog::Cancel);
    SearchWidget* searchWidget = new SearchWidget(&dlg);
    dlg.setMainWidget(searchWidget);

    searchWidget->setBaseQuery( baseQuery );
    searchWidget->setConfigFlags( flags );
    searchWidget->setSelectionMode( QAbstractItemView::ExtendedSelection );

    if( dlg.exec() == QDialog::Accepted ) {
        return searchWidget->selectedResources();
    }
    else {
        return QList<Nepomuk::Resource>();
    }
}

#include "searchwidget.moc"
