/* 
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006 Sebastian Trueg <trueg@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef _NEPOMUK_KMETADATA_TOOLS_H_
#define _NEPOMUK_KMETADATA_TOOLS_H_

#include <QtCore>
#include <kmetadata/variant.h>
#include <kmetadata/kmetadata_export.h>

#include <knep/rdf/node.h>

namespace Nepomuk {
  namespace KMetaData {
    /**
     * Set a custom default repository to use. It is not recommended to use this
     * method in production code. It should only be used for testing purposes.
     */
    KMETADATA_EXPORT void setDefaultRepository( const QString& s );

    /**
     * Used internally by Resource.
     * \return The URI of the RDF graph meta data is stored in.
     */
    // FIXME: this should be called defaultRepository and once we use the OntologyService
    // it can be removed anyway
    KMETADATA_EXPORT QString defaultGraph();

    /**
     * Used internally by Resource.
     * \return The URI of the predicate used to state the type of a resource.
     */
    KMETADATA_NO_EXPORT QString typePredicate();

    /**
     * Used internally by Resource.
     * Converts a Variant into a literal value to be used in the RDF store.
     * Uses the language set in the current KDE session.
     */
    KMETADATA_EXPORT QList<RDF::Node> valuesToRDFNodes( const Variant& );
    KMETADATA_EXPORT RDF::Node valueToRDFNode( const Variant& );

    /**
     * Used internally by Resource.
     * Converts a literal value from the RDF store into a Variant.
     */
    KMETADATA_EXPORT Variant RDFLiteralToValue( const RDF::Node& node );

    /**
     * If \a uri does not have a namespace, append the default one.
     *
     * \sa Ontology::defaultNamespace
     */
    KMETADATA_NO_EXPORT QString ensureNamespace( const QString& uri );

    template<typename T> KMETADATA_EXPORT QList<T> convertResourceList( const QList<Resource>& l ) {
      QList<T> rl;
      for( QList<Resource>::const_iterator it = l.constBegin();
	   it != l.constEnd(); ++it )
	rl.append( T( *it ) );
      return rl;
    }

  }
}

#endif
