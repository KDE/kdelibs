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

#ifndef _NEPOMUK_FILE_ONTOLOGY_LOADER_H_
#define _NEPOMUK_FILE_ONTOLOGY_LOADER_H_

#include "ontologyloader.h"
#include "nepomuk_export.h"

#include <soprano/parser.h>
#include <soprano/sopranotypes.h>


namespace Nepomuk {
    /**
     * \deprecated Ontologies should now be handled using the classes
     * in the Nepomuk::Types namespace.
     */
    class KDE_DEPRECATED NEPOMUK_EXPORT FileOntologyLoader : public OntologyLoader
    {
    public:
	/**
	 * Default constructor.
	 * Creates a FileOntologyLoader that will try to load the 
	 * ontology directly from its URL, i.e. the internet.
	 */
	FileOntologyLoader();

	/**
	 * Creates a FileOntologyLoader that will load the 
	 * ontology from the specified filename.
	 */
	explicit FileOntologyLoader( const QString& filename,
                                     Soprano::RdfSerialization serialization = Soprano::SerializationUnknown );

	/**
	 * Destructor
	 */
	~FileOntologyLoader();

	/**
	 * Set the filename to be used for reading the ontologies.
	 * If no filename has been set FileOntologyLoader will try
	 * load the ontology file from the internet, i.e. directly
	 * use the ontology URL - NOT IMPLEMENTED YET.
	 */
	void setFileName( const QString& filename );

	/**
	 * The filename that has been set.
	 * \sa setFileName
	 * \return The filename set (can be empty)
	 */
	QString fileName() const;
	
	/**
	 * Set the serialization that is used in the file.
	 * Defaults to Soprano::SerializationUnknown
	 */
	void setSerialization( Soprano::RdfSerialization );

	/**
	 * reimplemented from OntologyLoader
	 */
	QList<Soprano::Statement> loadOntology( const QUrl& url );

    private:
	class Private;
	Private* const d;
    };
}

#endif
