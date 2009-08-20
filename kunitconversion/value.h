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

#ifndef CONVERSION_VALUE_H
#define CONVERSION_VALUE_H

#include <QtCore/QString>
#include "unit.h"
#include "conversion_export.h"

class QVariant;

namespace KUnitConversion
{

class CONVERSION_EXPORT Value
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
     *
     * @return value as string
     **/
    QString toString() const;

    /**
     * Number part of the value
     **/
    double number() const;

    /**
     * Unit part of the value
     **/
    UnitPtr unit() const;

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

} // Conversion namespace

#endif
