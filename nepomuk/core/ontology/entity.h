/* This file is part of the Nepomuk-KDE libraries
    Copyright (c) 2007 Sebastian Trueg <trueg@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef _NEPOMUK_ENTITY_H_
#define _NEPOMUK_ENTITY_H_

#include <QtCore/QUrl>
#include <QtCore/QString>
#include <QtCore/QSharedData>

#include "nepomuk_export.h"

namespace Nepomuk {

    class Ontology;

    class NEPOMUK_EXPORT Entity
	{
	public:
	    Entity( const Entity& );
	    ~Entity();

	    Entity& operator=( const Entity& );

	    /**
	     * The ontology in which the resource is defined.
	     */
	    const Ontology* definingOntology() const;

	    /**
	     * The name of the resource. The name equals the fragment of the
	     * URI.
	     */
	    QString name() const;

	    /**
	     * The URI of the resource
	     */
	    QUrl uri() const;
	    
	    QString label( const QString& language = QString() ) const;
	    QString comment( const QString& language = QString() ) const;

	protected:
	    Entity();

	private:
	    class Private;
	    QSharedDataPointer<Private> d;

	    friend class OntologyManager;
	};
}

#endif
