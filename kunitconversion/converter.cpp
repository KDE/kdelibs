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
    InvalidCategory(QObject* parent = 0) : UnitCategory(parent)
    {
        const QString s;
        const KLocalizedString ls;
        setObjectName("invalid");
        setName(i18n("Invalid"));
        setDefaultUnit(U(InvalidUnit, 1.0, s, s, s, ls, ls));
    };
};

class Converter::Private
{
public:
    Private()
    {
    };

    ~Private()
    {
    };
};

class ConverterSingleton
{
    public:
        Converter self;
};

K_GLOBAL_STATIC(ConverterSingleton, s_instance)

Converter::Converter(QObject* parent)
: QObject(parent)
, d(/*new Converter::Private*/0)
{
    KGlobal::locale()->insertCatalog("libconversion");
    new InvalidCategory(this);
    new Length(this);
    new Area(this);
    new Volume(this);
    new Temperature(this);
    new Velocity(this);
    new Mass(this);
    new Pressure(this);
    new Energy(this);
    new Currency(this);
    new Power(this);
    new Time(this);
    new FuelEfficiency(this);
    new Density(this);
}

Converter::~Converter()
{
    //delete d;
}

Converter* Converter::self()
{
    return &s_instance->self;
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

Unit* Converter::unit(const QString& unit) const
{
    foreach (UnitCategory* u, categories()) {
        Unit* unitClass = u->unit(unit);
        if (unitClass) {
            return unitClass;
        }
    }
    return 0;
}

Unit* Converter::unit(int unitId) const
{
    foreach (UnitCategory* u, categories()) {
        Unit* unitClass = u->unit(unitId);
        if (unitClass) {
            return unitClass;
        }
    }
    return 0;
}

UnitCategory* Converter::category(const QString& category) const
{
    QList<UnitCategory*> categories = findChildren<UnitCategory*>(category);
    if (!categories.isEmpty()) {
        return categories[0];
    }
    return 0;
}

QList<UnitCategory*> Converter::categories() const
{
    QList<UnitCategory*> categories = findChildren<UnitCategory*>();
    categories.removeAll(category("invalid"));
    return categories;
}

}

#include "converter.moc"
