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

#ifndef _NEPOMUK_RESOURCE_CLASS_H_
#define _NEPOMUK_RESOURCE_CLASS_H_

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QUrl>

#include "property.h"

/**
 * @short Represents a resource.
 *
 * This class keeps all the information of a resource
 * that has been collected by the ontology parser.
 */
class ResourceClass
{
    public:
        ResourceClass();
        ResourceClass( const QUrl& uri );
        ~ResourceClass();

        /**
         * Sets the uri of the resource.
         */
        void setUri( const QUrl &uri );

        /**
         * Returns the uri of the resource.
         */
        QUrl uri() const;

        /**
         * Sets the comment of the resource.
         */
        void setComment( const QString &comment );

        /**
         * Returns the comment of the resource.
         */
        QString comment() const;

        /**
         * Sets the parent resource of the resource.
         */
        void setParentResource( ResourceClass* parent );

        /**
         * Returns the parent resource of the resource.
         */
        ResourceClass* parentClass( bool considerGenerateClass = true ) const;

        /**
         * Adds a parent resource to the resource.
         */
        void addParentResource( ResourceClass* parent );

        /**
         * Returns all parent resource of the resource.
         */
        QList<ResourceClass*> allParentResources() const;

        /**
         * Adds a new property to the resource.
         */
        void addProperty( Property* property );

        /**
         * Returns the list of all properties of the resource.
         */
        Property::ConstPtrList allProperties() const;

        /**
         * Adds a reverse property to the resource.
         */
        void addReverseProperty( Property* property );

        /**
         * Returns the list of all reverse properties of the resource.
         */
        Property::ConstPtrList allReverseProperties() const;

        /**
         * Sets whether code for this class shall be generated.
         */
        void setGenerateClass( bool generate );

        /**
         * Returns @c true if this class should be generated.
         * Normally this always returns true except for the base class
         * Resource.
         */
        bool generateClass() const;

        /**
         * Returns the name of the resource.
         *
         * @param nameSpace The namespace that shall be included.
         */
        QString name( const QString &nameSpace = QString() ) const;

        /**
         * Returns the name of the header file for this resource.
         */
        QString headerName() const;

        /**
         * Returns the name of the source file for this resource.
         */
        QString sourceName() const;

    private:
        QUrl m_uri;
        QString m_comment;
        ResourceClass* m_parentClass;
        QList<ResourceClass*> m_allParentResources;
        Property::ConstPtrList m_properties;
        Property::ConstPtrList m_reverseProperties;
        bool m_generateClass;
};

#endif
