/*
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING.LIB" for the exact licensing terms.
 */

#ifndef _NEPOMUK_ABSTRACTCODE_H_
#define _NEPOMUK_ABSTRACTCODE_H_

#include <QtCore/QString>

class Property;
class ResourceClass;

/**
 * @short A class that encapsulates the definition generation methods.
 *
 * This class encapsulates the code generation for the definitions of
 * the resource and property getter and setter methods.
 */
class AbstractCode
{
    public:
        virtual ~AbstractCode() {}

        /**
         * Returns the declaration of the property setter method.
         *
         * @param property The property to work on.
         * @param resourceClass The resource the property belongs to.
         * @param withNamespace Whether the namespace shall be included or not.
         */
        virtual QString propertySetterDeclaration( const Property *property,
                                                   const ResourceClass* resourceClass,
                                                   const QString &nameSpace = QString() ) const = 0;

        /**
         * Returns the declaration of the property getter method.
         *
         * @param property The property to work on.
         * @param resourceClass The resource the property belongs to.
         * @param withNamespace Whether the namespace shall be included or not.
         */
        virtual QString propertyGetterDeclaration( const Property *property,
                                                   const ResourceClass* resourceClass,
                                                   const QString &nameSpace = QString() ) const = 0;

        /**
         * Returns the declaration of the property adder method.
         *
         * @param property The property to work on.
         * @param resourceClass The resource the property belongs to.
         * @param withNamespace Whether the namespace shall be included or not.
         */
        virtual QString propertyAdderDeclaration( const Property *property,
                                                  const ResourceClass* resourceClass,
                                                  const QString &nameSpace = QString() ) const = 0;

        /**
         * Returns the declaration of the property for the reverse property getter method.
         *
         * @param property The property to work on.
         * @param resourceClass The resource the property belongs to.
         * @param withNamespace Whether the namespace shall be included or not.
         */
        virtual QString propertyReversePropertyGetterDeclaration( const Property *property,
                                                                  const ResourceClass* resourceClass,
                                                                  const QString &nameSpace = QString() ) const = 0;

        /**
         * Returns the declaration of the resource method that returns all resources.
         *
         * @param resourceClass The resource to work on.
         * @param withNamespace Whether the namespace shall be included or not.
         */
        virtual QString resourceAllResourcesDeclaration( const ResourceClass* resourceClass,
                                                         const QString &nameSpace = QString() ) const = 0;

        /**
         * Returns the declaration of the resource method that provides pseudo inheritance.
         *
         * @param resourceBaseClass The resource to work on.
         * @param resourceClass The resource to cast to.
         * @param withNamespace Whether the namespace shall be included or not.
         */
        virtual QString resourcePseudoInheritanceDeclaration( const ResourceClass* resourceBaseClass,
                                                              const ResourceClass* resourceClass,
                                                              const QString &nameSpace = QString() ) const = 0;

        /**
         * Returns the definition of the property setter method.
         *
         * @param property The property to work on.
         * @param resourceClass The resource the property belongs to.
         */
        virtual QString propertySetterDefinition( const Property *property,
                                                  const ResourceClass* resourceClass ) const = 0;

        /**
         * Returns the definition of the property getter method.
         *
         * @param property The property to work on.
         * @param resourceClass The resource the property belongs to.
         */
        virtual QString propertyGetterDefinition( const Property *property,
                                                  const ResourceClass* resourceClass ) const = 0;

        /**
         * Returns the definition of the property adder method.
         *
         * @param property The property to work on.
         * @param resourceClass The resource the property belongs to.
         */
        virtual QString propertyAdderDefinition( const Property *property,
                                                 const ResourceClass* resourceClass ) const = 0;

        /**
         * Returns the definition of the property for the reverse property getter method.
         *
         * @param property The property to work on.
         * @param resourceClass The resource the property belongs to.
         */
        virtual QString propertyReversePropertyGetterDefinition( const Property *property,
                                                                 const ResourceClass* resourceClass ) const = 0;

        /**
         * Returns the definition of the resource method that returns all resources.
         *
         * @param resourceClass The resource to work on.
         */
        virtual QString resourceAllResourcesDefinition( const ResourceClass* resourceClass ) const = 0;

        /**
         * Returns the definition of the resource method that provides pseudo inheritance.
         *
         * @param resourceBaseClass The resource to work on.
         * @param resourceClass The resource to cast to.
         */
        virtual QString resourcePseudoInheritanceDefinition( const ResourceClass* resourceBaseClass,
                                                             const ResourceClass* resourceClass ) const = 0;
};

#endif
