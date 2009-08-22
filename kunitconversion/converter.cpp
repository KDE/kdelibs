/*
 *   Copyright (C) 2008-2009 Petri Damstén <damu@iki.fi>
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

#include "converter.h"

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include "unitcategory.h"
#include "area.h"
#include "length.h"
#include "currency.h"
#include "density.h"
#include "energy.h"
#include "fuel_efficiency.h"
#include "mass.h"
#include "power.h"
#include "pressure.h"
#include "temperature.h"
#include "timeunit.h"
#include "unit.h"
#include "velocity.h"
#include "volume.h"

namespace KUnitConversion
{

class InvalidCategory : public UnitCategory
{
public:
    InvalidCategory()
    {
        const QString s;
        const KLocalizedString ls;
        setName(i18n("Invalid"));
        setDefaultUnit(UP(InvalidUnit, 1.0, s, s, s, ls, ls));
    };
};

class ConverterPrivate
{
public:
    QList<UnitCategory *> categories;
    ConverterPrivate()
    {
        KGlobal::locale()->insertCatalog("libconversion");

        // the invalid category MUST be the first item
        categories.append(new InvalidCategory);

        categories.append(new Length);
        categories.append(new Area);
        categories.append(new Volume);
        categories.append(new Temperature);
        categories.append(new Velocity);
        categories.append(new Mass);
        categories.append(new Pressure);
        categories.append(new Energy);
        categories.append(new Currency);
        categories.append(new Power);
        categories.append(new Time);
        categories.append(new FuelEfficiency);
        categories.append(new Density);
    };

    ~ConverterPrivate()
    {
        qDeleteAll(categories);
    };
};

K_GLOBAL_STATIC(ConverterPrivate, static_d)

Converter::Converter(QObject* parent)
: QObject(parent), d(static_cast<ConverterPrivate *>(static_d))
{
}

Converter::~Converter()
{
}

Value Converter::convert(const Value& value, const QString& toUnit) const
{
    UnitCategory* category = value.unit()->category();
    if (!category) {
        return Value();
    }
    return category->convert(value, toUnit);
}

Value Converter::convert(const Value& value, int toUnit) const
{
    UnitCategory* category = value.unit()->category();
    if (!category) {
        return Value();
    }
    return category->convert(value, toUnit);
}

UnitCategory* Converter::categoryForUnit(const QString& unit) const
{
    foreach (UnitCategory* u, categories()) {
        if (u->hasUnit(unit)) {
            return u;
        }
    }
    return 0;
}

UnitPtr Converter::unit(const QString& unit) const
{
    foreach (UnitCategory* u, categories()) {
        UnitPtr unitClass = u->unit(unit);
        if (unitClass) {
            return unitClass;
        }
    }
    return UnitPtr();
}

UnitPtr Converter::unit(int unitId) const
{
    foreach (UnitCategory* u, categories()) {
        UnitPtr unitClass = u->unit(unitId);
        if (unitClass) {
            return unitClass;
        }
    }
    return UnitPtr();
}

UnitCategory* Converter::category(const QString& category) const
{
    foreach (UnitCategory *u, categories()) {
        if (u->name() == category)
            return u;
    }

    // not found
    return 0;
}

QList<UnitCategory*> Converter::categories() const
{
    QList<UnitCategory*> categories = d->categories;
    categories.removeAt(0);
    return categories;
}

}

#include "converter.moc"
