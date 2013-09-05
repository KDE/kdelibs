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

#include <klocalizedstring.h>

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
#include "acceleration.h"
#include "force.h"
#include "angle.h"
#include "frequency.h"

namespace KUnitConversion
{

class Invalid : public UnitCategory
{
public:
    Invalid() : UnitCategory(InvalidCategory)
    {
        const QString s;
        const KLocalizedString ls;
        setName(i18n("Invalid"));
        setDefaultUnit(UP(InvalidUnit, 1.0, s, s, s, ls, ls));
        setSymbolStringFormat(ki18nc("%1 value, %2 unit symbol (default)", "%1 %2"));
    };
};

class ConverterPrivate
{
public:
    QMap<int, UnitCategory *> categories;
    ConverterPrivate()
    {
        categories[InvalidCategory] = new Invalid;
        categories[LengthCategory] = new Length;
        categories[AreaCategory] = new Area();
        categories[VolumeCategory] = new Volume;
        categories[TemperatureCategory] = new Temperature;
        categories[VelocityCategory] = new Velocity;
        categories[MassCategory] = new Mass;
        categories[PressureCategory] = new Pressure;
        categories[EnergyCategory] = new Energy;
        categories[CurrencyCategory] = new Currency;
        categories[PowerCategory] = new Power;
        categories[TimeCategory] = new Time;
        categories[FuelEfficiencyCategory] = new FuelEfficiency;
        categories[DensityCategory] = new Density;
        categories[AccelerationCategory] = new Acceleration;
        categories[ForceCategory] = new Force;
        categories[AngleCategory] = new Angle;
        categories[FrequencyCategory] = new Frequency;
    };

    ~ConverterPrivate()
    {
        qDeleteAll(categories);
    };
};

Q_GLOBAL_STATIC(ConverterPrivate, static_d)

Converter::Converter(QObject* parent)
: QObject(parent), d(static_d())
{
}

Converter::~Converter()
{
}

Value Converter::convert(const Value& value, const QString& toUnit) const
{
    if (value.unit()) {
        UnitCategory* category = value.unit()->category();
        if (category) {
            return category->convert(value, toUnit);
        }
    }
    return Value();
}

Value Converter::convert(const Value& value, int toUnit) const
{
    if (value.unit()) {
        UnitCategory* category = value.unit()->category();
        if (category) {
            return category->convert(value, toUnit);
        }
    }
    return Value();
}

Value Converter::convert(const Value& value, UnitPtr toUnit) const
{
    if (toUnit && value.unit() && value.unit()->isValid()) {
        UnitCategory* category = value.unit()->category();
        if (category) {
            return category->convert(value, toUnit);
        }
    }
    return Value();
}

UnitCategory* Converter::categoryForUnit(const QString& unit) const
{
    foreach (UnitCategory* u, categories()) {
        if (u->hasUnit(unit)) {
            return u;
        }
    }
    return d->categories[InvalidCategory];
}

UnitPtr Converter::unit(const QString& unitString) const
{
    foreach (UnitCategory* u, d->categories) {
        UnitPtr unitClass = u->unit(unitString);
        if (unitClass) {
            return unitClass;
        }
    }
    return unit(InvalidUnit);
}

UnitPtr Converter::unit(int unitId) const
{
    foreach (UnitCategory* u, d->categories) {
        UnitPtr unitClass = u->unit(unitId);
        if (unitClass) {
            return unitClass;
        }
    }
    return d->categories[InvalidCategory]->defaultUnit();
}

UnitCategory* Converter::category(const QString& category) const
{
    foreach (UnitCategory *u, d->categories) {
        if (u->name() == category)
            return u;
    }
    // not found
    return d->categories[InvalidCategory];
}

UnitCategory* Converter::category(int categoryId) const
{
    if (d->categories.contains(categoryId)) {
        return d->categories[categoryId];
    }
    // not found
    return d->categories[InvalidCategory];
}

QList<UnitCategory*> Converter::categories() const
{
    QList<UnitCategory*> categories = d->categories.values();
    categories.removeAt(0);
    return categories;
}

}

