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

namespace Nepomuk {
  namespace KMetaData {
    /**
     * Used internally by Resource.
     * \return The URI of the RDF graph meta data is stored in.
     */
    KMETADATA_EXPORT QString defaultGraph();

    /**
     * Used internally by Resource.
     * \return The URI of the predicate used to state the type of a resource.
     */
    KMETADATA_NO_EXPORT QString typePredicate();

    /**
     * Used internally by Resource.
     * Converts a Variant into a literal value to be used in the RDF store.
     */
    KMETADATA_EXPORT QString valueToRDFLiteral( const Variant& );

    /**
     * Used internally by Resource.
     * Converts a list Variant into a literal value to be used in the RDF store.
     */
    KMETADATA_EXPORT QStringList valuesToRDFLiterals( const Variant& );

    /**
     * Used internally by Resource.
     * Converts a literal value from the RDF store into a Variant.
     */
    KMETADATA_EXPORT Variant RDFLiteralToValue( const QString& );

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
	rl.append( T( (*it).uri() ) );
      return rl;
    }

  }
}

#endif
