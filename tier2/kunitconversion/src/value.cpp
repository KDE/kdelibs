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

#include "value.h"
#include "converter.h"
#include <qmath.h>

namespace KUnitConversion
{

class Value::Private
{
public:
    Private(double n = 0.0, int u = InvalidUnit)
    : number(n)
    {
        unit = converter.unit(u);
    }

    Private(double n, UnitPtr u)
    : number(n)
    , unit(u)
    {
    }

    Private(double n, const QString& u)
    : number(n)
    {
        unit = converter.unit(u);
    }

    ~Private()
    {
    }

    double number;
    UnitPtr unit;
    Converter converter;
};

Value::Value()
: d(new Value::Private())
{
}

Value::Value(double n, UnitPtr u)
: d(new Value::Private(n, u))
{
}

Value::Value(double n, const QString& u)
: d(new Value::Private(n, u))
{
}

Value::Value(double n, int u)
: d(new Value::Private(n, u))
{
}

Value::Value(const QVariant& n, const QString& u)
: d(new Value::Private(n.toDouble(), u))
{
}

Value::~Value()
{
    delete d;
}

bool Value::isValid() const
{
    return (d->unit && d->unit->isValid());
}

QString Value::toString(int fieldWidth, char format, int precision, const QChar& fillChar) const
{
    if (isValid()) {
        return d->unit->toString(d->number, fieldWidth, format, precision, fillChar);
    }
    return QString();
}

QString Value::toSymbolString(int fieldWidth, char format, int precision,
                              const QChar& fillChar) const
{
    if (isValid()) {
        return d->unit->toSymbolString(d->number, fieldWidth, format, precision, fillChar);
    }
    return QString();
}

Value& Value::round(uint decimals)
{
    uint div = qPow(10, decimals);
    double add = 0.5 / (double)div;

    d->number = (int)((d->number + add) * div) / (double)div;
    return *this;
}

double Value::number() const
{
    return d->number;
}

UnitPtr Value::unit() const
{
    if (!d->unit) {
       d->unit = d->converter.unit(InvalidUnit);
    }
    return d->unit;
}

Value& Value::operator=(const Value& value)
{
    d->number = value.d->number;
    d->unit = value.d->unit;
    return *this;
}

Value Value::convertTo(UnitPtr unit) const
{
    return d->converter.convert(*this, unit);
}

Value Value::convertTo(int unit) const
{
    return d->converter.convert(*this, unit);
}

Value Value::convertTo(const QString& unit) const
{
    return d->converter.convert(*this, unit);
}

}
