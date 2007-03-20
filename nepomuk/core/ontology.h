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

#ifndef _NEPOMUK_ONTOLOGY_H_
#define _NEPOMUK_ONTOLOGY_H_

#include <QtCore>

#include <kmetadata/variant.h>
#include <kmetadata/kmetadata_export.h>

namespace Nepomuk {
    namespace KMetaData {
	/**
	 * \deprecated This class will be replaced with a generic Ontology class
	 * that can and will be used to represent arbitrary ontologies that are
	 * either parsed from an NRL file or read from the RDF store.
	 *
	 * \brief This class represents the \ref page_ontology.
	 *
	 * Ontology represents the %Nepomuk desktop ontology and as such can be used
	 * to retrieve information about the types and properties defined in it.
	 *
	 * The most important methods, and thus, most often used ones are
	 *
	 * - Ontology::typeName
	 * - Ontology::propertyName
	 *
	 * which return user-readable names for classes and properties in the ontology.
	 * This allows to create a GUI listing all properties defined for an arbitrary
	 * resource (See \ref examples).
	 */
	class KMETADATA_EXPORT Ontology
	    {
	    public:
		/**
		 * Constructs a new Ontology object.
		 *
		 * Normally there is no need to create an instance of Ontology.
		 * Use ResourceManager::ontology instead.
		 */
		Ontology();
		~Ontology();

		/**
		 * The default namespace used in the ontology.
		 */
		QString defaultNamespace() const;

		/**
		 * The default type, i.e. the base type of all classes
		 * in the ontology.
		 * \return URI of the default type
		 */
		QString defaultType() const;

		/**
		 * All types defined in the ontology.
		 * \return A list or type URIs
		 */
		QStringList types() const;

		/**
		 * Each type except for the defaultType has a parent type whose
		 * properties it inherits.
		 *
		 * \return The parent type of \a uri.
		 */
		QString parentType( const QString& uri );

		/**
		 * All properties of \a type defined in the ontology.
		 * \param type If not empty only the properties defined for the specified type
		 *             will be returned.
		 * \return A list of property URIs.
		 */
		QStringList properties( const QString& type = QString() ) const;

		/**
		 * \param type The URI of a type defined in \ref page_ontology.
		 * \return A user-readable name for \a type.
		 *
		 * \sa types
		 */
		QString typeName( const QString& type ) const;

		/**
		 * \param property The URI of a property defined in \ref page_ontology.
		 * \return A user-readable name for \a property.
		 *
		 * \sa properties
		 */
		QString propertyName( const QString& property ) const;

		static QString rdfNamepace();
		static QString rdfsNamespace();
		static QString nrlNamespace();
		static QString naoNamespace();

	    private:
		class Private;
		Private* d;
	    };
    }
}

#endif
