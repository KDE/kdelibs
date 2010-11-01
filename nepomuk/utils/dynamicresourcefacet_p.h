#ifndef _NEPOMUK_QUERY_DYNAMIC_RESOURCE_FACET_P_H_
#define _NEPOMUK_QUERY_DYNAMIC_RESOURCE_FACET_P_H_

#include "dynamicresourcefacet.h"

#include "query.h"
#include "queryserviceclient.h"
#include "property.h"
#include "class.h"
#include "resource.h"
#include "result.h"

#include <QtCore/QSet>


class Nepomuk::Utils::DynamicResourceFacet::Private
{
public:
    Private()
        : m_selectionMode(Facet::MatchAny),
          m_maxRows( 5 ),
          m_haveMore( false ) {
    }
    virtual ~Private() {
    }

    Types::Class resourceType() const {
        if ( m_resourceType.isValid() )
            return m_resourceType;
        else
            return m_property.range();
    }

    /// rebuild the choices from the query, optionally clearing the selection
    void rebuild( bool clearSelection = true );

    /// actually run the query in rebuild - this is virtual only for the unit test
    virtual void startQuery( const Query::Query& query );

    /// add a resource to the list of choices if it is not already in
    void addResource( const Nepomuk::Resource& res );

    void _k_newEntries( const QList<Nepomuk::Query::Result>& );
    void _k_populateFinished();

    Query::QueryServiceClient m_queryClient;

    Facet::SelectionMode m_selectionMode;
    Types::Property m_property;
    Types::Class m_resourceType;
    int m_maxRows;
    bool m_haveMore;

    /// The query which the client is currently using and which we use
    /// to select a subset of resources that make sense
    Query::Query m_currentQuery;

    // list of resources built by rebuild()
    QList<Resource> m_resources;

    QSet<Resource> m_selectedResources;

    DynamicResourceFacet* q;
};

#endif
