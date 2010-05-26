/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2008 Sebastian Trueg <trueg@kde.org>
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

#ifndef _NEPOMUK_TOOLS_H_
#define _NEPOMUK_TOOLS_H_

#include <QtCore/QString>
#include <QtCore/QList>

#include "variant.h"
#include "nepomuk_export.h"

#include <soprano/node.h>

#include <kdemacros.h>

namespace Nepomuk {
    /**
     * Used internally by Resource.
     * Converts a Variant into a literal value to be used in the RDF store.
     */
    NEPOMUK_EXPORT QList<Soprano::Node> valuesToRDFNodes( const Variant& );

    /**
     * Used internally by Resource.
     * Converts a non-list Variant into a Soprano::Node.
     */
    NEPOMUK_EXPORT Soprano::Node valueToRDFNode( const Variant& );

    /**
     * Used internally by Resource.
     * Convert a list of resources to a list of Ts where T needs to be a subclass
     * of Resource.
     *
     * \return A list containing all resources in \p l represented as a T.
     */
    template<typename T> QList<T> convertResourceList( const QList<Resource>& l ) {
        QList<T> rl;
        for( QList<Resource>::const_iterator it = l.constBegin();
             it != l.constEnd(); ++it )
            rl.append( T( *it ) );
        return rl;
    }

    /**
     * Used internally by Resource.
     * Convert a list of Ts to a list of Resources where T needs to be a subclass
     * of Resource.
     *
     * \return A list containing all resources in \p l.
     */
    template<typename T> QList<Resource> convertResourceList( const QList<T>& l ) {
        QList<Resource> rl;
        Q_FOREACH( const T& r, l )
            rl.append( Resource( r ) );
        return rl;
    }

    /**
     * \deprecated Use Variant::fromNode() instead.
     */
    KDE_DEPRECATED NEPOMUK_EXPORT Variant RDFLiteralToValue( const Soprano::Node& node );

    /**
     * \deprecated Has no effect anymore.
     */
    KDE_DEPRECATED NEPOMUK_EXPORT void setDefaultRepository( const QString& s );

    /**
     * \return The name of the default Nepomuk repository ("main").
     *
     * \deprecated Use Nepomuk::ResourceManager::mainModel()
     */
    KDE_DEPRECATED NEPOMUK_EXPORT QString defaultGraph();

    /**
     * \deprecated Use Soprano::Vocabulary::RDF::type()
     */
    KDE_DEPRECATED NEPOMUK_EXPORT QString typePredicate();

    /**
     * \deprecated Use Soprano::Vocabulary::RDF::rdfNamepace()
     */
    KDE_DEPRECATED NEPOMUK_EXPORT QString rdfNamepace();

    /**
     * \deprecated Use Soprano::Vocabulary::RDFS::rdfsNamepace()
     */
    KDE_DEPRECATED NEPOMUK_EXPORT QString rdfsNamespace();

    /**
     * \deprecated Use Soprano::Vocabulary::NRL::nrlNamepace()
     */
    KDE_DEPRECATED NEPOMUK_EXPORT QString nrlNamespace();

    /**
     * \deprecated Use Soprano::Vocabulary::NAO::naoNamepace()
     */
    KDE_DEPRECATED NEPOMUK_EXPORT QString naoNamespace();
}

#endif
