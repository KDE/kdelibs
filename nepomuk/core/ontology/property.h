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

#ifndef _NEPOMUK_PROPERTY_H_
#define _NEPOMUK_PROPERTY_H_

#include <QtCore/QList>
#include <QtCore/QUrl>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QSharedData>

#include "entity.h"
#include "nepomuk_export.h"

namespace Nepomuk {
    namespace Types {

        class Class;
        class Literal;

        /**
         * \class Property property.h Nepomuk/Types/Property
         *
         * \brief A property is a resource of type rdf:Property which relates a domain
         * with a range. The latter one can be a Literal or a Class.
         *
         * Property instances are explicitly shared. Two instances created
         * with the same uri reference the same data.
         *
         * While Resource objects can be changed Property instances are considered
         * to be static and never changed during their lifetime.
         *
         * \author Sebastian Trueg <trueg@kde.org>
         */ 
        class NEPOMUK_EXPORT Property : public Entity
        {
        public:
            /**
             * Default constructor. Creates an empty Property.
             */
            Property();

            /**
             * Create the property referred to by \p uri.
             * The result is either a valid property which could be loaded from the
             * Nepomuk store or a simple property which only contains the uri.
             *
             * Be aware that the data is only loaded once read.
             *
             * Subsequent calls result in a simple hash lookup of cached data.
             */
            Property( const QUrl& uri );

            /**
             * Default copy constructor
             */
            Property( const Property& );

            /**
             * Destructor
             */
            ~Property();

            /**
             * Copy operator.
             */
            Property& operator=( const Property& );

            /**
             * Each property can have multiple parent properties.
             * \return A list of all parent properties of this property.
             * If the list is emppty it means that the property has no direct
             * parents, i.e. it is derived from rdf:Resource.
             */
            QList<Property> parentProperties();

            /**
             * \return A list of all properties that have this property as a parent.
             * Be aware that this list can never be final since other ontologies
             * that have not been loaded yet may contain properties that are derived
             * from this property.
             */
            QList<Property> subProperties();

            /**
             * The inverse property (nrl:inverseProperty).
             * \return A Property instance representing the inverse property of this one
             * or 0 if none was specified in the ontology.
             */
            Property inverseProperty();

            /**
             * The range of the property.
             * \return The range of the property or an invalid Class in case
             * the range of this poperty is a literal.
             * \sa literalRange
             */
            Class range();

            /**
             * If the rage of this property is a literal (i.e. range returns an invalid Class)
             * this method provides the literal type.
             *
             * \returns the literal type of this property or an empty, invalid Literal
             * if the range is a Class.
             *
             * \sa range
             */
            Literal literalRangeType();

            /**
             * The domain of the property.
             */
            Class domain();

            /**
             * The cardinality of this property as specified by nrl:cardinality.
             *
             * \return the cardinality of the property or -1 if none was set.
             */
            int cardinality();

            /**
             * The minimum cardinality of this property as specified by nrl:minCardinality.
             *
             * \return the minimum cardinality of the property or -1 if none was set.
             */
            int minCardinality();

            /**
             * The maximum cardinality of this property as specified by nrl:maxCardinality.
             *
             * \return the maximum cardinality of the property or -1 if none was set.
             */
            int maxCardinality();

            /**
             * Check if a property inherits this property. This is a recursive method which
             * does not only check direct child propertyes.
             *
             * \return true if other is derived from this property, false otherwise.
             */
            bool isParentOf( const Property& other );

            /**
             * Check if this property is derived from another property. This is a recursive method which
             * does not only check direct child propertyes.
             *
             * \return true if this property is derived from other, false otherwise.
             */
            bool isSubPropertyOf( const Property& other );
        };
    }
}




namespace Nepomuk {

    class Ontology;
    class Class;
    class Literal;

    /**
     * A property is a resource of type rdf:Property which relates a domain
     * with a range. The latter one can be a literal in addition to a Class.
     *
     * \deprecated in favor of Nepomuk::Types::Property
     */ 
    class KDE_DEPRECATED NEPOMUK_EXPORT Property : public Entity
    {
    public:
	/**
	 * Default constructor. Creates an empty Property.
	 */
	Property();

	/**
	 * Default copy constructor
	 */
	Property( const Property& );

	/**
	 * Destructor
	 */
	~Property();

	Property& operator=( const Property& );

	/**
	 * Each property can have multiple parent properties.
	 * \return A list of all parent properties of this property.
	 * If the list is emppty it means that the property has no direct
	 * parents, i.e. it is derived from rdf:Resource.
	 */
	QList<const Property*> parentProperties() const;

	/**
	 * \return A list of all properties that have this property as a parent.
	 * Be aware that this list can never be final since other ontologies
	 * that have not been loaded yet may contain properties that are derived
	 * from this property.
	 */
	QList<const Property*> parentOf() const;

	/**
	 * The inverse property (nrl:inverseProperty).
	 * \return A Property instance representing the inverse property of this one
	 * or 0 if none was specified in the ontology.
	 */
	const Property* inverseProperty() const;

	/**
	 * The range of the property.
	 * \return The range of the property or an invalid Class in case
	 * the range of this poperty is a literal.
	 * \sa literalRange
	 */
	const Class* range() const;

	/**
	 * If the rage of this property is a literal (i.e. range returns 0)
	 * this method provides the literal type.
	 *
	 * \returns the literal type of this property or an empty, invalid Literal
	 * if the range is a Class.
	 *
	 * \sa range
	 */
	Literal literalRangeType() const;

	/**
	 * The domain of the property.
	 */
	const Class* domain() const;

	/**
	 * The cardinality of this property as specified by nrl:cardinality.
	 *
	 * \return the cardinality of the property or -1 if none was set.
	 */
	int cardinality() const;

	/**
	 * The minimum cardinality of this property as specified by nrl:minCardinality.
	 *
	 * \return the minimum cardinality of the property or -1 if none was set.
	 */
	int minCardinality() const;

	/**
	 * The maximum cardinality of this property as specified by nrl:maxCardinality.
	 *
	 * \return the maximum cardinality of the property or -1 if none was set.
	 */
	int maxCardinality() const;

	// QString unit() const;

	/**
	 * Loads a property actually including the whole ontology
	 * it is declared in.
	 *
	 * Dependancies are resolved automatically if possible.
	 *
	 * \return A property object representing the property identified
	 * by uri or an invalid Property object if the resource identified
	 * by uri is either not a property or does not exist.
	 */
	static const Property* load( const QUrl& uri );

    private:
	class Private;
	QSharedDataPointer<Private> d;

	friend class OntologyManager;
    };
}

#endif
