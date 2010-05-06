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

#ifndef _NEPOMUK_DESKTOP_ONTOLOGY_LOADER_H_
#define _NEPOMUK_DESKTOP_ONTOLOGY_LOADER_H_

#include "ontologyloader.h"
#include "nepomuk_export.h"

#include <QtCore/QUrl>
#include <QtCore/QList>

namespace Nepomuk {
    /**
     * The DesktopOntologyLoader reads ontologies from the installed
     * ontologies on the desktop using KStandardDirs.
     * All possible ontology installation folders are searched for
     * ontology desktop files.
     *
     * \deprecated Ontologies should now be handled using the classes
     * in the Nepomuk::Types namespace.
     */
    class KDE_DEPRECATED NEPOMUK_EXPORT DesktopOntologyLoader : public OntologyLoader
    {
    public:
	DesktopOntologyLoader();
	~DesktopOntologyLoader();

	/**
	 * Get a list of all ontology URIs that could be found
	 * on the system.
	 */
	QList<QUrl> allOntologies();

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
