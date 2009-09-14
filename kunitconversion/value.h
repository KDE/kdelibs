/*
 *   Copyright (C) 2007-2009 Petri Damstén <damu@iki.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KUNITCONVERSION_VALUE_H
#define KUNITCONVERSION_VALUE_H

#include <QtCore/QString>
#include "unit.h"
#include "kunitconversion_export.h"

class QVariant;

namespace KUnitConversion
{

class KUNITCONVERSION_EXPORT Value
{
public:
    Value();
    Value(double n, UnitPtr u);
    Value(double n, const QString& u);
    Value(double n, int u);
    Value(const QVariant& n, const QString& u);
    ~Value();

    /**
     * Check if value is valid.
     *
     * @return True if value is valid
     **/
    bool isValid() const;

    /**
     * Convert value to a string
     * @param fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * @param format type of floating point formating, like in QString::arg
     * @param precision number of digits after the decimal separator
     * @param fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     * @return value as a string
     **/
    QString toString(int fieldWidth = 0, char format = 'g', int precision = -1,
                     const QChar& fillChar = QLatin1Char(' ')) const;

    /**
     * Convert value to a string with symbol
     * @param fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * @param format type of floating point formating, like in QString::arg
     * @param precision number of digits after the decimal separator
     * @param fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     * @return value as a string
     **/
    QString toSymbolString(int fieldWidth = 0, char format = 'g', int precision = -1,
                           const QChar& fillChar = QLatin1Char(' ')) const;

    /**
     * Number part of the value
     **/
    double number() const;

    /**
     * rounds value to decimal count
     * @param decimals decimal count.
     **/
    Value& round(uint decimals);

    /**
     * Unit part of the value
     **/
    UnitPtr unit() const;

    /**
    * convert to another unit
    **/
    Value convertTo(UnitPtr unit) const;

    /**
    * convert to another unit
    **/
    Value convertTo(int unit) const;

    /**
    * convert to another unit
    **/
    Value convertTo(const QString& unit) const;

    Value& operator=(const Value&);

private:
    class Private;
    Private* const d;
};

} // KUnitConversion namespace

#endif
