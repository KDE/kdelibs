/* This file is part of the KDE libraries

   Copyright (c) 2007 Jos van den Oever <jos@vandenoever.info>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation; either
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
#ifndef PREDICATEPROPERTIES_H
#define PREDICATEPROPERTIES_H

#include <kio/kio_export.h>
#include <QtCore/QSharedData>
#include <QtCore/QVariant>
class QValidator;

/**
 * A predicate is part of the RDF trinity: subject, predicate, object.
 * It is identified by URI and it defines the type of the relationship.
 * For file metadata, a predicate can be seen as a fieldname.
 * It has a data type, a description, a short id, a cardinality
 **/
class KIO_EXPORT PredicateProperties {
friend class PredicatePropertyProvider;
public:
    PredicateProperties(const QString& predicate = QString());
    PredicateProperties(const PredicateProperties& p);
    ~PredicateProperties();
    const PredicateProperties& operator=(const PredicateProperties& p);
    /**
     * This enum is used to specify some attributes that an item can have,
     * which fit neither in the Hint nor in the Unit enum.
     */
    enum Attributes
    {
        Addable     =  1, ///< The item or group can be added by a user
        Removable   =  2, ///< It can be removed
        Modifiable  =  4, ///< The value can be edited (no meaning for a group)
        Cumulative =  8,  /**< If an application wants to display information
                               for more than one file, it may add up the values
                               for this item (e.g. play time of an mp3 file) */
        Averaged    = 16, /**< Similar to Cumulative, but the average should
                               be calculated instead of the sum */
        MultiLine   = 32, /**< This attribute says that a string item is likely
                               to be more than one line long, so for editing, a
                               widget capable for multline text should be used
                               */
        SqueezeText = 64  /**< If the text for this item is very long, it
                               should be squeezed to the size of the widget
                               where it's displayed
                               */
    };
    /**
     *  Get the attributes of this group (see Attributes)
     *
     *  @return the attributes
     */
    uint attributes() const;
    /**
     * Key associated with this value.
     **/
    const QString& key() const;
    /**
     * The type for this field.
     **/
    QVariant::Type type() const;
    /**
     * Localized name of the predicate.
     **/
    const QString& name() const;
    /**
     * Localized description of the predicate.
     **/
    const QString& description() const;
    QValidator* createValidator() const;
    const QStringList& suggestedValues() const;
    uint minCardinality() const;
    uint maxCardinality() const;
    const PredicateProperties& parent() const;
    /**
     * Return a url that identifies the unit in which this property
     * is expressed.
     **/
    const QString& unit() const;
    bool isValid() const;
private:
    class Private;
    QSharedDataPointer<Private> p;
};

#endif
