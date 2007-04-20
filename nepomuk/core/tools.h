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

#include <QtCore/QString>
#include <QtCore/QList>

#include <kmetadata/variant.h>
#include <kmetadata/kmetadata_export.h>

#include <knepomuk/rdf/node.h>

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
	KMETADATA_EXPORT QString typePredicate();

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

	template<typename T> QList<T> convertResourceList( const QList<Resource>& l ) {
	    QList<T> rl;
	    for( QList<Resource>::const_iterator it = l.constBegin();
		 it != l.constEnd(); ++it )
		rl.append( T( *it ) );
	    return rl;
	}

	template<typename T> QList<Resource> convertResourceList( const QList<T>& l ) {
	    QList<Resource> rl;
	    foreach( T r, l )
/*       for( QList<T>::const_iterator it = l.constBegin(); */
/* 	   it != l.constEnd(); ++it ) */
		rl.append( Resource( r/**it*/ ) );
	    return rl;
	}

	KMETADATA_EXPORT QString rdfNamepace();
	KMETADATA_EXPORT QString rdfsNamespace();
	KMETADATA_EXPORT QString nrlNamespace();
	KMETADATA_EXPORT QString naoNamespace();
    }
}

#endif
