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
#include <variant.h>


namespace Nepomuk {
  namespace KMetaData {
    /**
     * Used internally by Resource.
     * \return The URI of the RDF graph meta data is stored in.
     */
    QString defaultGraph();

    /**
     * Used internally by Resource.
     * \return The URI of the predicate used to state the type of a resource.
     */
    QString typePredicate();

    /**
     * Used internally by Resource.
     * Converts a Variant into a literal value to be used in the RDF store.
     */
    QString valueToRDFLiteral( const Variant& );

    /**
     * Used internally by Resource.
     * Converts a list Variant into a literal value to be used in the RDF store.
     */
    QStringList valuesToRDFLiterals( const Variant& );

    /**
     * Used internally by Resource.
     * Converts a literal value from the RDF store into a Variant.
     */
    Variant RDFLiteralToValue( const QString& );

    /**
     * If \a uri does not have a namespace append the default one.
     *
     * \sa Ontology::defaultNamespace
     */
    QString ensureNamespace( const QString& uri );
  }
}

#endif
