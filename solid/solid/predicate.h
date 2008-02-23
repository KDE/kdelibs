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

#ifndef SOLID_PREDICATE_H
#define SOLID_PREDICATE_H

#include <QtCore/QVariant>
#include <QtCore/QSet>

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

namespace Solid
{
    class Device;

    /**
     * This class implements predicates for devices.
     *
     * A predicate is a logical condition that a given device can match or not.
     * It's a constraint about the value a property must have in a given device
     * interface, or any combination (conjunction, disjunction) of such
     * constraints.
     *
     * FIXME: Add an example.
     */
    class SOLID_EXPORT Predicate
    {
    public:
        /**
         * The comparison operator which can be used for matching within the predicate.
         *
         * - Equals, the property and the value will match for strict equality
         * - Mask, the property and the value will match if the bitmasking is not null
         */
        enum ComparisonOperator { Equals, Mask };

        /**
         * Constructs an invalid predicate.
         */
        Predicate();

        /**
         * Copy constructor.
         *
         * @param other the predicate to copy
         */
        Predicate(const Predicate &other);

        /**
         * Constructs a predicate matching the value of a property in
         * a given device interface.
         *
         * @param ifaceType the device interface type the device must have
         * @param property the property name of the device interface
         * @param value the value the property must have to make the device match
         * @param compOperator the operator to apply between the property and the value when matching
         */
        Predicate(const DeviceInterface::Type &ifaceType,
                   const QString &property, const QVariant &value,
                   ComparisonOperator compOperator = Equals);

        /**
         * Constructs a predicate matching the value of a property in
         * a given device interface.
         *
         * @param ifaceName the name of the device interface the device must have
         * @param property the property name of the device interface
         * @param value the value the property must have to make the device match
         * @param compOperator the operator to apply between the property and the value when matching
         */
        Predicate(const QString &ifaceName,
                   const QString &property, const QVariant &value,
                   ComparisonOperator compOperator = Equals);

        /**
         * Constructs a predicate matching devices being of a particular device interface
         *
         * @param ifaceType the device interface the device must have
         */
        explicit Predicate(const DeviceInterface::Type &ifaceType);

        /**
         * Constructs a predicate matching devices being of a particular device interface
         *
         * @param ifaceName the name of the device interface the device must have
         */
        explicit Predicate(const QString &ifaceName);

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
        Predicate &operator=(const Predicate &other);


        /**
         * 'And' operator.
         *
         * @param other the second operand
         * @return a new 'and' predicate having 'this' and 'other' as operands
         */
        Predicate operator &(const Predicate &other);

        /**
         * 'AndEquals' operator.
         *
         * @param other the second operand
         * @return assigns to 'this' a new 'and' predicate having 'this' and 'other' as operands
         */
        Predicate &operator &=(const Predicate &other);

        /**
         * 'Or' operator.
         *
         * @param other the second operand
         * @return a new 'or' predicate having 'this' and 'other' as operands
         */
        Predicate operator|(const Predicate &other);

        /**
         * 'OrEquals' operator.
         *
         * @param other the second operand
         * @return assigns to 'this' a new 'or' predicate having 'this' and 'other' as operands
         */
        Predicate &operator|=(const Predicate &other);

        /**
         * Indicates if the predicate is valid.
         *
         * Predicate() is the only invalid predicate.
         *
         * @return true if the predicate is valid, false otherwise
         */
        bool isValid() const;

        /**
         * Checks if a device matches the predicate.
         *
         * @param device the device to match against the predicate
         * @return true if the given device matches the predicate, false otherwise
         */
        bool matches(const Device &device) const;

        /**
         * Retrieves the device interface types used in this predicate.
         *
         * @return all the device interface types used in this predicate
         */
        QSet<DeviceInterface::Type> usedTypes() const;

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
        static Predicate fromString(const QString &predicate);

    private:
        class Private;
        Private * const d;
    };
}

#endif
