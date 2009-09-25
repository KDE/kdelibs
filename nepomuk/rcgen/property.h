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

#ifndef _NEPOMUK_PROPERTY_H_
#define _NEPOMUK_PROPERTY_H_

#include <QtCore/QList>
#include <QtCore/QUrl>

class ResourceClass;

/**
 * @short Represents the property of a resource.
 *
 * This class keeps all the information of a resource property
 * that has been collected by the ontology parser.
 */
class Property
{
public:
    typedef QList<const Property*> ConstPtrList;

    /**
     * Creates a new property.
     */
    Property();

    /**
     * Creates a new property of a given type and with a given uri.
     *
     * @param uri The uri that defines the property.
     * @param type The type the property is of.
     */
    //Property( const QUrl& uri, const QString& type );

    /**
     * Sets the uri of the property.
     */
    void setUri( const QUrl &uri );

    /**
     * Returns the uri of the property.
     */
    QUrl uri() const;

    /**
     * Sets the type of the property.
     */
    void setRange( ResourceClass* type );

    /**
     * Set the literal range of the property.
     * Cannot be used with setRange at the same time.
     *
     * \param Name of the Qt type to use
     */
    void setLiteralRange( const QString& range );

    /**
     * Returns the scope of the property.
     */
    ResourceClass* range() const;

    /**
     * Returns the literal range of the property
     * (the name of the Qt type to be used.)
     */
    QString literalRange() const;

    /**
     * Sets the comment of the property.
     */
    void setComment( const QString &comment );

    /**
     * Returns the comment of the property.
     */
    QString comment() const;

    /**
     * Sets whether the property is a list of values.
     */
    void setIsList( bool isList );

    /**
     * Returns whether the property is a list of values.
     */
    bool isList() const;

    /**
     * Sets the domain the property belongs to.
     */
    void setDomain( ResourceClass *domain );

    /**
     * Returns the domain resource the property belongs to.
     */
    ResourceClass* domain( bool onlyReturnGeneratedClass = false ) const;

    /**
     * Sets the inverse property of this property.
     */
    void setInverseProperty( Property *property );

    /**
     * Returns the inverse property of this property.
     */
    Property* inverseProperty() const;

    /**
     * Returns the name of the property.
     */
    QString name() const;

    /**
     * Returns the conversion method of the property.
     */
    QString literalTypeConversionMethod() const;

    /**
     * Retrieve a string representation of the range. Handles both literal ranges and class
     * ranges. Also ResourceClass::generateClass is taken into account.
     */
    QString typeString( bool simple = false, const QString &nameSpace = QString() ) const;

    /**
     * Returns whether the property is of simple type.
     */
    bool hasSimpleType() const;

private:
    QUrl m_uri;
    ResourceClass* m_range;
    QString m_literalRange;
    QString m_comment;
    bool m_isList;
    ResourceClass* m_domain;
    Property* m_inverseProperty;
};

#endif
