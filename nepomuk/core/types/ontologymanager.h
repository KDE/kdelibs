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

#ifndef _NEPOMUK_ONTOLOGY_MANAGER_H_
#define _NEPOMUK_ONTOLOGY_MANAGER_H_

#include <QtCore/QUrl>

#include "nepomuk_export.h"

namespace Soprano {
    class Statement;
}

namespace Nepomuk {

    class OntologyLoader;
    class Ontology;

    /**
     * The OntologyManager is the central ontology cache handler.
     * It is modeled as a singleton and its instance can be accessed
     * through OntologyManager::instance.
     *
     * \deprecated Ontologies should be handled via the classes in
     * Nepomuk::Types.
     */
    class KDE_DEPRECATED NEPOMUK_EXPORT OntologyManager
    {
    public:
	~OntologyManager();

	static OntologyManager* instance();

	/**
	 * Set an OntologyLoader to be used to retrieve 
	 * the RDF statements that make up an ontology.
	 *
	 * By default Nepomuk uses a combination of the
	 * NepomukOntologyLoader and DesktopOntologyLoader
	 * where the NepomukOntologyLoader is the preferred
	 * one, i.e. only if it is not able to load the
	 * ontology DesktopOntologyLoader is used as a fallback.
	 *
	 * If an OntologyLoader is set no fallback loader will
	 * be used. Thus, to force one of the default loaders
	 * (for example the DesktopOntologyLoader) just do:
	 *
	 * \code
	 * OntologyManager::instance()->setOntologyLoader( new DesktopOntologyLoader() );
	 * \endcode
	 *
	 * OntologyManager will take over ownership of loader.
	 *
	 * Overwriting the loader will delete any loader previously
	 * set via setOntologyLoader.
	 */
	void setOntologyLoader( OntologyLoader* loader );

	/**
	 * Attempt to load the ontology referenced by uri.
	 * If the manager already has the ontology in the cache
	 * it is simply returned. Otherwise the manager tries 
	 * to load the ontology from one of the configured
	 * OntologyLoader instances.
	 *
	 * \return The requested ontology or 0 in case it could
	 * not be found, i.e. none of the OntologyLoader instances
	 * could provide the RDF statements making up the ontology.
	 */
	const Ontology* getOntology( const QUrl& uri );

    private:
	OntologyManager();

	Ontology* constructOntology( const QUrl& uri, const QList<Soprano::Statement>& sl );

	class Private;
	Private* const d;
    };
}

#endif
