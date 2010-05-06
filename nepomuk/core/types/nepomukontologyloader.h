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

#ifndef _NEPOMUK_NEPOMUK_ONTOLOGY_LOADER_H_
#define _NEPOMUK_NEPOMUK_ONTOLOGY_LOADER_H_

#include "ontologyloader.h"
#include "nepomuk_export.h"

namespace Nepomuk {
    /**
     * \deprecated Ontologies should now be handled using the classes
     * in the Nepomuk::Types namespace.
     */
    class KDE_DEPRECATED NEPOMUK_EXPORT NepomukOntologyLoader : public OntologyLoader
    {
    public:
	NepomukOntologyLoader();
	~NepomukOntologyLoader();

	QList<Soprano::Statement> loadOntology( const QUrl& url );

    private:
	class Private;
	Private* const d;
    };
}

#endif
