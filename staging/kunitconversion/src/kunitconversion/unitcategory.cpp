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

#include "unitcategory.h"
#include <klocalizedstring.h>
#include <QMap>

namespace KUnitConversion
{

class UnitCategory::Private
{
public:
    Private() :
        defaultUnit(0)
    {
    };

    ~Private()
    {
    };
    QString name;
    UnitPtr defaultUnit;
    QMap<QString, UnitPtr> unitMap;
    QMap<int, UnitPtr> idMap;
    QList<UnitPtr> units;
    QList<UnitPtr> mostCommonUnits;
    QString description;
    QUrl url;
    KLocalizedString symbolStringFormat;
    int id;
};

UnitCategory::UnitCategory(int id)
  : d(new UnitCategory::Private)
{
    d->id = id;
}

UnitCategory::~UnitCategory()
{
    delete d;
}

void UnitCategory::setSymbolStringFormat(const KLocalizedString& symbolStringFormat)
{
    d->symbolStringFormat = symbolStringFormat;
}

KLocalizedString UnitCategory::symbolStringFormat() const
{
    return d->symbolStringFormat;
}

QList<UnitPtr> UnitCategory::units() const
{
    return d->units;
}

QList<UnitPtr> UnitCategory::mostCommonUnits() const
{
    return d->mostCommonUnits;
}

void UnitCategory::setMostCommonUnits(const QList<int>& units)
{
    d->mostCommonUnits.clear();
    foreach (int u, units) {
        d->mostCommonUnits.append(unit(u));
    }
}

QStringList UnitCategory::allUnits() const
{
    return d->unitMap.keys();
}

bool UnitCategory::hasUnit(const QString &unit) const
{
    return d->unitMap.contains(unit);
}

Value UnitCategory::convert(const Value& value, const QString& toUnit)
{
    if ((toUnit.isEmpty() || d->unitMap.contains(toUnit)) && value.unit()->isValid()) {
        UnitPtr to = toUnit.isEmpty() ? defaultUnit() : d->unitMap[toUnit];
        return convert(value, to);
    }
    return Value();
}

Value UnitCategory::convert(const Value& value, int toUnit)
{
    if (d->idMap.contains(toUnit) && value.unit()->isValid()) {
        return convert(value, d->idMap[toUnit]);
    }
    return Value();
}

Value UnitCategory::convert(const Value& value, UnitPtr toUnit)
{
    if (toUnit) {
        double v = toUnit->fromDefault(value.unit()->toDefault(value.number()));
        return Value(v, toUnit);
    }
    return Value();
}

void UnitCategory::addUnitMapValues(UnitPtr unit, const QString& names)
{
    const QStringList list = names.split(';');
    foreach (const QString& name, list) {
        d->unitMap[name] = unit;
    }
}

void UnitCategory::addIdMapValue(UnitPtr unit, int id)
{
    d->idMap[id] = unit;
    d->units.append(unit);
}

UnitPtr UnitCategory::unit(const QString& s) const
{
    return d->unitMap.value(s);
}

UnitPtr UnitCategory::unit(int unitId) const
{
    if (d->idMap.contains(unitId)) {
        return d->idMap[unitId];
    }
    return UnitPtr();
}

QString UnitCategory::name() const
{
    return d->name;
}

void UnitCategory::setName(const QString& name)
{
    d->name = name;
}

void UnitCategory::setDefaultUnit(UnitPtr defaultUnit)
{
    d->defaultUnit = defaultUnit;
}

UnitPtr UnitCategory::defaultUnit() const
{
    return d->defaultUnit;
}

QString UnitCategory::description() const
{
    return d->description;
}

void UnitCategory::setDescription(const QString& description)
{
    d->description = description;
}

QUrl UnitCategory::url() const
{
    return d->url;
}

void UnitCategory::setUrl(const QUrl & url)
{
    d->url = url;
}

int UnitCategory::id() const
{
    return d->id;
}

}
