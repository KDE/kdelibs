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

#include "unit.h"

#include <klocalizedstring.h>

#include "unitcategory.h"

namespace KUnitConversion
{

Complex::Complex()
{
}

Complex::~Complex()
{
}

class Unit::Private
{
public:
    Private(UnitCategory* category, const Complex* complex = 0)
    : multiplier(1.0)
    , complex(complex)
    , category(category)
    {
    };

    ~Private()
    {
    };

    QString symbol;
    QString description;
    double multiplier;
    KLocalizedString real;
    KLocalizedString integer;
    const Complex* complex;
    UnitCategory* category;
    int id;
};

Unit::Unit(UnitCategory* category, int id, double multiplier, const QString& symbol,
           const QString& description, const QString& match,
           const KLocalizedString& real, const KLocalizedString& integer)
: d(new Unit::Private(category))
{
    if (category) {
        category->addUnitMapValues(UnitPtr(this), match);
        category->addIdMapValue(UnitPtr(this), id);
    }
    d->multiplier = multiplier;
    d->real = real;
    d->integer = integer;
    d->symbol = symbol;
    d->description = description;
    d->id = id;
}

Unit::Unit(UnitCategory* category, int id, const Complex* complex, const QString& symbol,
           const QString& description, const QString& match,
           const KLocalizedString& real, const KLocalizedString& integer)
: d(new Unit::Private(category, complex))
{
    if (category) {
        category->addUnitMapValues(UnitPtr(this), match);
        category->addIdMapValue(UnitPtr(this), id);
    }
    d->real = real;
    d->integer = integer;
    d->symbol = symbol;
    d->description = description;
    d->id = id;
}

Unit::~Unit()
{
    delete d;
}

UnitCategory* Unit::category() const
{
    return d->category;
}

QString Unit::description() const
{
    return d->description;
}

QString Unit::symbol() const
{
    return d->symbol;
}

double Unit::multiplier() const
{
    return d->multiplier;
}

void Unit::setMultiplier(double multiplier)
{
    d->multiplier = multiplier;
}

double Unit::toDefault(double value) const
{
    if (d->complex) {
        return d->complex->toDefault(value);
    } else {
        return value * d->multiplier;
    }
}

double Unit::fromDefault(double value) const
{
    if (d->complex) {
        return d->complex->fromDefault(value);
    } else {
        return value / d->multiplier;
    }
}

QString Unit::toString(double value, int fieldWidth, char format, int precision,
                       const QChar& fillChar) const
{
    if ((int)value == value && precision < 1) {
        return d->integer.subs((int)value).toString();
    }
    return d->real.subs(value, fieldWidth, format, precision, fillChar).toString();
}

QString Unit::toSymbolString(double value, int fieldWidth, char format, int precision,
                             const QChar& fillChar) const
{
    return category()->symbolStringFormat().subs(value, fieldWidth, format, precision, fillChar)
            .subs(d->symbol).toString();
}

bool Unit::isValid() const
{
    return !d->symbol.isEmpty();
}

int Unit::id() const
{
    return d->id;
}

}

