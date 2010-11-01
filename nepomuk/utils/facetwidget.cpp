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

#include "facetwidget.h"
#include "facetmodel.h"
#include "facet.h"
#include "facetdelegate.h"
#include "facetfiltermodel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTreeView>
#include <QUrl>
#include <QtGui/QListView>

#include "query.h"
#include "queryparser.h"
#include "result.h"
#include "comparisonterm.h"
#include "resourceterm.h"
#include "resourcetypeterm.h"
#include "resource.h"

#include <Soprano/Vocabulary/NAO>

#include "kdebug.h"
#include "kdescendantsproxymodel.h"


class Nepomuk::Utils::FacetWidget::Private
{
public:
    Nepomuk::Utils::FacetModel *m_facetModel;
    QListView *m_facetView;
};


Nepomuk::Utils::FacetWidget::FacetWidget( QWidget *parent )
    : QWidget( parent ),
      d( new Private() )
{
    d->m_facetModel = new Nepomuk::Utils::FacetModel( this );
    connect( d->m_facetModel, SIGNAL( queryTermChanged(Nepomuk::Query::Term) ),
             this, SIGNAL( queryTermChanged(Nepomuk::Query::Term) ) );

    d->m_facetView = new QListView( this );
    d->m_facetView->setItemDelegate( new FacetDelegate( d->m_facetView ) );
    d->m_facetView->setBackgroundRole(QPalette::NoRole);
    d->m_facetView->viewport()->setBackgroundRole(QPalette::NoRole);
    d->m_facetView->setFrameStyle(QFrame::NoFrame);

    FacetFilterModel* facetFilterModel = new FacetFilterModel( this );
    facetFilterModel->setSourceModel( d->m_facetModel );
    connect( d->m_facetModel, SIGNAL(queryTermChanged()), facetFilterModel, SLOT(invalidate()));
    connect( d->m_facetModel, SIGNAL(modelReset()), facetFilterModel, SLOT(invalidate()));

    KDescendantsProxyModel* proxy = new KDescendantsProxyModel( this );
    proxy->setSourceModel( facetFilterModel );
    d->m_facetView->setModel( proxy );

    new QVBoxLayout( this );
    layout()->addWidget( d->m_facetView );
}

Nepomuk::Utils::FacetWidget::~FacetWidget()
{
    delete d;
}


QList<Nepomuk::Utils::Facet*> Nepomuk::Utils::FacetWidget::facets() const
{
    return d->m_facetModel->facets();
}


Nepomuk::Query::Term Nepomuk::Utils::FacetWidget::extractFacetsFromTerm( const Nepomuk::Query::Term& term )
{
    return d->m_facetModel->extractFacetsFromTerm( term );
}


Nepomuk::Query::Term Nepomuk::Utils::FacetWidget::queryTerm() const
{
    return d->m_facetModel->queryTerm();
}


void Nepomuk::Utils::FacetWidget::setClientQuery(const Nepomuk::Query::Query& query)
{
    d->m_facetModel->setClientQuery(query);
}


void Nepomuk::Utils::FacetWidget::addFacet( Nepomuk::Utils::Facet* facet )
{
    d->m_facetModel->addFacet(facet);
}


void Nepomuk::Utils::FacetWidget::setFacets( const QList<Nepomuk::Utils::Facet*>& facets )
{
    d->m_facetModel->setFacets(facets);
}


void Nepomuk::Utils::FacetWidget::clear()
{
    d->m_facetModel->clear();
}

#include "facetwidget.moc"
