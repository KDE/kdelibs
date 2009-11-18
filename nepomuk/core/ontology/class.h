/* This file is part of the Nepomuk-KDE libraries
   Copyright (c) 2007-2009 Sebastian Trueg <trueg@kde.org>

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

#ifndef _NEPOMUK_CLASS_H_
#define _NEPOMUK_CLASS_H_

#include <QtCore/QList>
#include <QtCore/QUrl>
#include <QtCore/QString>
#include <QtCore/QSharedData>

#include "entity.h"
#include "nepomuk_export.h"


namespace Nepomuk {
    namespace Types {
        class Property;

        /**
         * \class Class class.h Nepomuk/Types/Class
         *
         * \brief A Class is a resource of type rdf:Class.
         *
         * Class instances are explicitly shared. Two instances created
         * with the same uri reference the same data.
         *
         * While Resource objects can be changed Class instances are considered
         * to be static and never changed during their lifetime.
         *
         * \author Sebastian Trueg <trueg@kde.org>
         */
        class NEPOMUK_EXPORT Class : public Entity
        {
            friend class ClassPrivate;

        public:
            /**
             * Default constructor. Creates an empty Class.
             */
            Class();

            /**
             * Create the class referred to by \p uri.
             * The result is either a valid class which could be loaded from the
             * Nepomuk store or a simple class which only contains the uri.
             *
             * Be aware that the data is only loaded once read.
             *
             * Subsequent calls result in a simple hash lookup of cached data.
             */
            Class( const QUrl& uri );

            /**
             * Default copy constructor
             */
            Class( const Class& );

            /**
             * Destructor
             */
            ~Class();

            /**
             * Copy operator.
             */
            Class& operator=( const Class& );

            /**
             * A Property has a certain range which is a Class or a Literal.
             * \return A list of all properties that have this Class as a range.
             * \sa Property::range()
             */
            QList<Property> rangeOf();

            /**
             * A Property has a certain range which is a Class or a Literal.
             * \return A list of all properties that have this Class as a range.
             * \sa Property::range()
             *
             * Const version.
             *
             * \since 4.4
             */
            QList<Property> rangeOf() const;

            /**
             * A Property has a certain domain which is a Class.
             * \return A list of all properties that have this Class as a domain.
             * \sa Property::domain()
             */
            QList<Property> domainOf();

            /**
             * A Property has a certain domain which is a Class.
             * \return A list of all properties that have this Class as a domain.
             * \sa Property::domain()
             *
             * Const version.
             *
             * \since 4.4
             */
            QList<Property> domainOf() const;

            /**
             * Search for a property in the class by its name.
             * \param name The name of the property.
             * \return the Property object identified by name or an invalid property if it could not be found.
             */
            Property findPropertyByName( const QString& name );

            /**
             * Search for a property in the class by its name.
             * \param name The name of the property.
             * \return the Property object identified by name or an invalid property if it could not be found.
             *
             * Const version.
             *
             * \since 4.4
             */
            Property findPropertyByName( const QString& name ) const;

            /**
             * Search for a property in the class by its label.
             * \param label The label of the property (i.e. rdfs:label)
             * \param language The language in which the label was specified. If empty the default rdfs:label
             * is returned.
             * \return the Property object identified by label or an invalid property if it could not be found.
             */
            Property findPropertyByLabel( const QString& label, const QString& language = QString() );

            /**
             * Search for a property in the class by its label.
             * \param label The label of the property (i.e. rdfs:label)
             * \param language The language in which the label was specified. If empty the default rdfs:label
             * is returned.
             * \return the Property object identified by label or an invalid property if it could not be found.
             *
             * Const version.
             *
             * \since 4.4
             */
            Property findPropertyByLabel( const QString& label, const QString& language = QString() ) const;

            /**
             * Each class can have multiple parent classes. This method
             * provides a list of all direct parents.
             * \return A list of all parent classes of this class.
             * If the list is emppty it means that the class has no direct
             * parents, i.e. it is derived from rdf:Resource.
             * \sa allParentClasses()
             */
            QList<Class> parentClasses();

            /**
             * Each class can have multiple parent classes. This method
             * provides a list of all direct parents.
             * \return A list of all parent classes of this class.
             * If the list is emppty it means that the class has no direct
             * parents, i.e. it is derived from rdf:Resource.
             * \sa allParentClasses()
             *
             * Const version.
             *
             * \since 4.4
             */
            QList<Class> parentClasses() const;

            /**
             * \return A list of all classes that have this class as a parent.
             * Be aware that this list can never be final since other ontologies
             * that have not been loaded yet may contain classes that are derived
             * from this class.
             */
            QList<Class> subClasses();

            /**
             * \return A list of all classes that have this class as a parent.
             * Be aware that this list can never be final since other ontologies
             * that have not been loaded yet may contain classes that are derived
             * from this class.
             *
             * Const version.
             *
             * \since 4.4
             */
            QList<Class> subClasses() const;

            /**
             * Recursively determines all parent classes of this class, not
             * only the direct ones.
             * \return A list of parent classes of this class.
             * \sa parentClasses()
             */
            QList<Class> allParentClasses();

            /**
             * Recursively determines all parent classes of this class, not
             * only the direct ones.
             * \return A list of parent classes of this class.
             * \sa parentClasses()
             *
             * Const version.
             *
             * \since 4.4
             */
            QList<Class> allParentClasses() const;

            /**
             * Recursively determines all sub classes of this class, not
             * only the direct ones.
             * \return A list of sub classes of this class.
             * \sa subClasses()
             */
            QList<Class> allSubClasses();

            /**
             * Recursively determines all sub classes of this class, not
             * only the direct ones.
             * \return A list of sub classes of this class.
             * \sa subClasses()
             *
             * Const version.
             *
             * \since 4.4
             */
            QList<Class> allSubClasses() const;

            /**
             * Check if a class inherits this class. This is a recursive method which
             * does not only check direct child classes.
             *
             * \return true if other is derived from this class, false otherwise.
             */
            bool isParentOf( const Class& other );

            /**
             * Check if a class inherits this class. This is a recursive method which
             * does not only check direct child classes.
             *
             * \return true if other is derived from this class, false otherwise.
             *
             * Const version.
             *
             * \since 4.4
             */
            bool isParentOf( const Class& other ) const;

            /**
             * Check if this class is derived from another class. This is a recursive method which
             * does not only check direct child classes.
             *
             * \return true if this class is derived from other, false otherwise.
             */
            bool isSubClassOf( const Class& other );

            /**
             * Check if this class is derived from another class. This is a recursive method which
             * does not only check direct child classes.
             *
             * \return true if this class is derived from other, false otherwise.
             *
             * Const version.
             *
             * \since 4.4
             */
            bool isSubClassOf( const Class& other ) const;
        };
    }
}


#ifndef DISABLE_NEPOMUK_LEGACY

namespace Nepomuk {

    class Ontology;
    class Property;

    /**
     * A Class is a resource of type rdf:Class.
     *
     * \deprecated in favor of Nepomuk::Types::Class
     */
    class KDE_DEPRECATED NEPOMUK_EXPORT Class : public Entity
    {
    public:
	/**
	 * Default constructor. Creates an empty Class.
	 */
	Class();

	/**
	 * Default copy constructor
	 */
	Class( const Class& );

	/**
	 * Destructor
	 */
	~Class();

	Class& operator=( const Class& );

	// an alternative would be: QList<Property> rangeOf() and QList<Property> domainOf()
	QList<const Property*> allProperties() const;

	/**
	 * Search for a property in the class by its name.
	 * \param name The name of the property.
	 * \return the Property object identified by name or 0 if the property could not be found.
	 */
	const Property* findPropertyByName( const QString& name ) const;

	/**
	 * Search for a property in the class by its label.
	 * \param label The label of the property (i.e. rdfs:label)
	 * \param language The language in which the label was specified. If empty the default rdfs:label
	 * is returned.
	 * \return the Property object identified by label or 0 if the property could not be found.
	 */
	const Property* findPropertyByLabel( const QString& label, const QString& language = QString() ) const;

	/**
	 * Search for a property in the class by its name.
	 * \param uri the URI of the property
	 * \return the Property object identified by uri or 0 if the property could not be found.
	 */
	const Property* findPropertyByUri( const QUrl& uri ) const;

	/**
	 * Each class can have multiple parent classes.
	 * \return A list of all parent classes of this class.
	 * If the list is emppty it means that the class has no direct
	 * parents, i.e. it is derived from rdf:Resource.
	 */
	QList<const Class*> parentClasses() const;

	/**
	 * \return A list of all classes that have this class as a parent.
	 * Be aware that this list can never be final since other ontologies
	 * that have not been loaded yet may contain classes that are derived
	 * from this class.
	 */
	QList<const Class*> subClasses() const;

	/**
	 * Check if a class inherits this class. This is a recursive method which
	 * does not only check direct child classes.
	 *
	 * \return true if other is derived from this class, false otherwise.
	 */
	bool isParentOf( const Class* other ) const;

	/**
	 * Check if this class is derived from another class. This is a recursive method which
	 * does not only check direct child classes.
	 *
	 * \return true if this class is derived from other, false otherwise.
	 */
	bool isSubClassOf( const Class* other ) const;

	/**
	 * Loads a class actually including the whole ontology
	 * it is declared in.
	 *
	 * Dependancies are resolved automatically if possible.
	 *
	 * \return A Class object representing the class identified
	 * by uri or an invalid Class object if the resource identified
	 * by uri is either not a class or does not exist.
	 */
	static const Class* load( const QUrl& uri );

    private:
	class Private;
	QSharedDataPointer<Private> d;

	friend class OntologyManager;
    };
}

#endif
#endif
