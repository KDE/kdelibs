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

#include "temperature.h"
#include "converter.h"
#include <klocalizedstring.h>

using namespace KUnitConversion;

class CelsiusConv : public Complex
{
    double toDefault(double value) const { return value + 273.15; };
    double fromDefault(double value) const { return value - 273.15; };
};

class FahrenheitConv : public Complex
{
    double toDefault(double value) const { return (value + 459.67) * 5.0 / 9.0; };
    double fromDefault(double value) const { return (value * 9.0 / 5.0) - 459.67; };
};

class DelisleConv : public Complex
{
    double toDefault(double value) const { return 373.15 - (value * 2.0 / 3.0); };
    double fromDefault(double value) const { return (373.15 - value) * 3.0 / 2.0; };
};

class NewtonConv : public Complex
{
    double toDefault(double value) const { return (value * 100.0 / 33.0) + 273.15; };
    double fromDefault(double value) const { return (value - 273.15) * 33.0 / 100.0; };
};

class ReaumurConv : public Complex
{
    double toDefault(double value) const { return (value * 5.0 / 4.0) + 273.15; };
    double fromDefault(double value) const { return (value - 273.15) * 4.0 / 5.0; };
};

class RomerConv : public Complex
{
    double toDefault(double value) const { return (value - 7.5) * 40.0 / 21.0 + 273.15; };
    double fromDefault(double value) const { return (value - 273.15) * 21.0 / 40.0 + 7.5; };
};


Temperature::Temperature() : UnitCategory(TemperatureCategory)
{
    setName(i18n("Temperature"));
    setSymbolStringFormat(ki18nc("%1 value, %2 unit symbol (temperature)", "%1 %2"));

    setDefaultUnit(UP(Kelvin, 1,
      i18nc("temperature unit symbol", "K"),
      i18nc("unit description in lists", "kelvins"),
      i18nc("unit synonyms for matching user input", "kelvin;kelvins;K"),
      ki18nc("amount in units (real)", "%1 kelvins"),
      ki18ncp("amount in units (integer)", "%1 kelvin", "%1 kelvins")
    ));
    U(Celsius, new CelsiusConv(),
      i18nc("temperature unit symbol", "°C"),
      i18nc("unit description in lists", "Celsius"),
      i18nc("unit synonyms for matching user input", "Celsius;°C;C"),
      ki18nc("amount in units (real)", "%1 degrees Celsius"),
      ki18ncp("amount in units (integer)", "%1 degree Celsius", "%1 degrees Celsius")
    );
    U(Fahrenheit, new FahrenheitConv(),
      i18nc("temperature unit symbol", "°F"),
      i18nc("unit description in lists", "Fahrenheit"),
      i18nc("unit synonyms for matching user input", "Fahrenheit;°F;F"),
      ki18nc("amount in units (real)", "%1 degrees Fahrenheit"),
      ki18ncp("amount in units (integer)", "%1 degree Fahrenheit", "%1 degrees Fahrenheit")
    );
    U(Rankine, 0.555556,
      i18nc("temperature unit symbol", "R"),
      i18nc("unit description in lists", "Rankine"),
      i18nc("unit synonyms for matching user input", "Rankine;°R;R;Ra"),
      ki18nc("amount in units (real)", "%1 Rankine"),
      ki18ncp("amount in units (integer)", "%1 Rankine", "%1 Rankine")
    );
    U(Delisle, new DelisleConv(),
      i18nc("temperature unit symbol", "°De"),
      i18nc("unit description in lists", "Delisle"),
      i18nc("unit synonyms for matching user input", "Delisle;°De;De"),
      ki18nc("amount in units (real)", "%1 degrees Delisle"),
      ki18ncp("amount in units (integer)", "%1 degree Delisle", "%1 degrees Delisle")
    );
    U(TemperatureNewton, new NewtonConv(),
      i18nc("temperature unit symbol", "°N"),
      i18nc("unit description in lists", "Newton"),
      i18nc("unit synonyms for matching user input", "Newton;°N;N"),
      ki18nc("amount in units (real)", "%1 degrees Newton"),
      ki18ncp("amount in units (integer)", "%1 degree Newton", "%1 degrees Newton")
    );
    U(Reaumur, new ReaumurConv(),
      i18nc("temperature unit symbol", "°Ré"),
      i18nc("unit description in lists", "Réaumur"),
      i18nc("unit synonyms for matching user input", "Réaumur;°Ré;Ré;Reaumur;°Re;Re"),
      ki18nc("amount in units (real)", "%1 degrees Réaumur"),
      ki18ncp("amount in units (integer)", "%1 degree Réaumur", "%1 degrees Réaumur")
    );
    U(Romer, new RomerConv(),
      i18nc("temperature unit symbol", "°Rø"),
      i18nc("unit description in lists", "Rømer"),
      i18nc("unit synonyms for matching user input", "Rømer;°Rø;Rø;Romer;°Ro;Ro"),
      ki18nc("amount in units (real)", "%1 degrees Rømer"),
      ki18ncp("amount in units (integer)", "%1 degree Rømer", "%1 degrees Rømer")
    );

    setMostCommonUnits(QList<int>() << Kelvin << Celsius << Fahrenheit);
}
