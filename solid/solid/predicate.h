/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef KDEHW_PREDICATE_H
#define KDEHW_PREDICATE_H

#include <QVariant>

#include <kdelibs_export.h>

#include <kdehw/capability.h>

namespace KDEHW
{
    namespace Ifaces
    {
        class Device;
    }

    /**
     * This class implements predicates for devices.
     *
     * A predicate is a logical condition that a given device can match or not.
     * It's a constraint about the value a property must have in a given device
     * capability, or any combination (conjunction, disjunction) of such
     * constraints.
     *
     * FIXME: Add an example.
     */
    class KDE_EXPORT Predicate
    {
    public:
        /**
         * Constructs an invalid predicate.
         */
        Predicate();

        /**
         * Copy constructor.
         *
         * @param other the predicate to copy
         */
        Predicate( const Predicate &other );

        /**
         * Constructs a predicate matching the value of a property in
         * a given capability.
         *
         * @param capability the capability the device must have
         * @param property the property name of the capability
         * @param value the value the property must have to make the device match
         */
        Predicate( const Capability::Type &capability,
                   const QString &property, const QVariant &value );

        /**
         * Constructs a predicate matching the value of a property in
         * a given capability.
         *
         * @param capability the capability the device must have
         * @param property the property name of the capability
         * @param value the value the property must have to make the device match
         */
        Predicate( const QString &capability,
                   const QString &property, const QVariant &value );

        /**
         * Destroys a Predicate object.
         */
        ~Predicate();


        /**
         * Assignement operator.
         *
         * @param other the predicate to assign
         * @return this predicate after having assigned 'other' to it
         */
        Predicate &operator=( const Predicate &other );


        /**
         * 'And' operator.
         *
         * @param other the second operand
         * @return a new 'and' predicate having 'this' and 'other' as operands
         */
        Predicate operator&( const Predicate &other );

        /**
         * 'Or' operator.
         *
         * @param other the second operand
         * @return a new 'or' predicate having 'this' and 'other' as operands
         */
        Predicate operator|( const Predicate &other );


        /**
         * Indicates if the predicate is valid.
         *
         * Predicate() is the only invalid predicate.
         *
         * @return true if the predicate is valid, false otherwise
         */
        bool isValid() const;

        /**
         *
         * @param device the device to match against the predicate
         * @return true if the given device matches the predicate, false otherwise
         */
        bool matches( Ifaces::Device *device ) const;

        /**
         * Converts the predicate to its string form.
         *
         * @return a string representation of the predicate
         */
        QString toString() const;

        /**
         * Converts a string to a predicate.
         *
         * @param predicate the string to convert
         * @return a new valid predicate if the given string is syntactically
         * correct, Predicate() otherwise
         */
        static Predicate fromString( const QString &predicate );

    private:
        class Private;
        Private *d;
    };
}

#endif
