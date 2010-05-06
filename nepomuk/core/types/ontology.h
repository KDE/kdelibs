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

#ifndef _NEPOMUK_ONTOLOGY_H_
#define _NEPOMUK_ONTOLOGY_H_

#include <QtCore/QList>
#include <QtCore/QUrl>
#include <QtCore/QString>
#include <QtCore/QSharedData>

#include "entity.h"
#include "nepomuk_export.h"


namespace Nepomuk {
    namespace Types {

        class Class;
        class Property;

        /**
         * \class Ontology ontology.h Nepomuk/Types/Ontology
         *
         * \brief Represents one ontology.
         *
         * \author Sebastian Trueg <trueg@kde.org>
         */
        class NEPOMUK_EXPORT Ontology : public Entity
        {
        public:
            /**
             * Default constructor. Creates an empty Ontology.
             */
            Ontology();

            /**
             * Create the ontology referred to by \p uri.
             * The result is either a valid ontology which could be loaded from the
             * Nepomuk store or a simple class which only contains the uri.
             *
             * Be aware that the data is only loaded once read.
             *
             * Subsequent calls result in a simple hash lookup of cached data.
             */
            Ontology( const QUrl& uri );

            /**
             * Default copy constructor
             */
            Ontology( const Ontology& );

            /**
             * Destructor
             */
            ~Ontology();

            Ontology& operator=( const Ontology& );

            /**
             * All classes defined in this ontology, i.e. its namespace.
             */
            QList<Class> allClasses();

            /**
             * Search for a class in the ontology by its name.
             * \param name The name of the class.
             * \return the Class object identified by name or an invalid one if the class could not be found.
             */
            Class findClassByName( const QString& name );

            /**
             * Search for a class in the ontology by its label.
             * \param label The label of the class (i.e. rdfs:label)
             * \param language The language in which the label was specified. If empty the default rdfs:label
             * is returned.
             * \return the Class object identified by label or an invalid one if the class could not be found.
             */
            Class findClassByLabel( const QString& label, const QString& language = QString() );

            /**
             * A list of all properties defined in this ontology. This does not include properties that use
             * classes of this ontology but are defined in a different one.
             */
            QList<Property> allProperties();

            /**
             * Search for a property in the ontology by its name.
             * \param name The name of the property.
             * \return the Property object identified by name or an invalid one if the property could not be found.
             */
            Property findPropertyByName( const QString& name );

            /**
             * Search for a property in the ontology by its label.
             * \param label The label of the property (i.e. rdfs:label)
             * \param language The language in which the label was specified. If empty the default rdfs:label
             * is returned.
             * \return the Property object identified by label or an invalid one if the property could not be found.
             */
            Property findPropertyByLabel( const QString& label, const QString& language = QString() );
        };
    }
}


#ifndef DISABLE_NEPOMUK_LEGACY

namespace Nepomuk {

    class Class;
    class Property;

    /**
     * Ontology represents one ontology specified using NRL.
     *
     * \deprecated in favor of Nepomuk::Types::Ontology
     */
    class KDE_DEPRECATED NEPOMUK_EXPORT Ontology
    {
    public:
	/**
	 * Default constructor. Creates an empty Ontology.
	 */
	Ontology();

	/**
	 * Default copy constructor
	 */
	Ontology( const Ontology& );

	/**
	 * Destructor
	 */
	~Ontology();

	Ontology& operator=( const Ontology& );

	/**
	 * The URI of the ontology, i.e. its namespace
	 */
	QUrl uri() const;

	/**
	 * All classes defined in this ontology, i.e. its namespace.
	 */
	QList<const Class*> allClasses() const;

	/**
	 * Search for a class in the ontology by its name.
	 * \param name The name of the class.
	 * \return the Class object identified by name or 0 if the class could not be found.
	 */
	const Class* findClassByName( const QString& name ) const;

	/**
	 * Search for a class in the ontology by its label.
	 * \param label The label of the class (i.e. rdfs:label)
	 * \param language The language in which the label was specified. If empty the default rdfs:label
	 * is returned.
	 * \return the Class object identified by label or 0 if the class could not be found.
	 */
	const Class* findClassByLabel( const QString& label, const QString& language = QString() ) const;

	/**
	 * Search for a class in the ontology by its name.
	 * \param uri the URI of the class
	 * \return the Class object identified by uri or 0 if the class could not be found.
	 */
	const Class* findClassByUri( const QUrl& uri ) const;

        /**
	 * A list of all properties defined in this ontology. This does not include properties that use
	 * classes of this ontology but are defined in a different one.
	 */
	QList<const Property*> allProperties() const;

	/**
	 * Search for a property in the ontology by its name.
	 * \param name The name of the property.
	 * \return the Property object identified by name or 0 if the property could not be found.
	 */
	const Property* findPropertyByName( const QString& name ) const;

	/**
	 * Search for a property in the ontology by its label.
	 * \param label The label of the property (i.e. rdfs:label)
	 * \param language The language in which the label was specified. If empty the default rdfs:label
	 * is returned.
	 * \return the Property object identified by label or 0 if the property could not be found.
	 */
	const Property* findPropertyByLabel( const QString& label, const QString& language = QString() ) const;

	/**
	 * Search for a property in the ontology by its name.
	 * \param uri the URI of the property
	 * \return the Property object identified by uri or 0 if the property could not be found.
	 */
	const Property* findPropertyByUri( const QUrl& uri ) const;

	/**
	 * Loads an ontology.
	 *
	 * Dependancies are resolved automatically if possible.
	 *
	 * \return An Ontology object representing the ontology identified
	 * by uri or an invalid Ontology object if the resource identified
	 * by uri is either not an ontology or does not exist.
	 */
	static const Ontology* load( const QUrl& uri );

    private:
	class Private;
	QSharedDataPointer<Private> d;

	friend class OntologyManager;
    };
}

#endif // DISABLE_NEPOMUK_LEGACY

#endif
