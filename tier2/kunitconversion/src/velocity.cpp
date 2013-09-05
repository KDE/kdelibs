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

#include "velocity.h"
#include "converter.h"
#include <math.h>
#include <klocalizedstring.h>

using namespace KUnitConversion;

class BeaufortConv : public Complex
{
    double toDefault(double value) const { return 0.836 * pow(value, 3.0 / 2.0); };
    double fromDefault(double value) const { return pow(value / 0.836, 2.0 / 3.0); };
};

Velocity::Velocity() : UnitCategory(VelocityCategory)
{
    setName(i18n("Speed"));
    setSymbolStringFormat(ki18nc("%1 value, %2 unit symbol (velocity)", "%1 %2"));

    setDefaultUnit(UP(MeterPerSecond, 1,
      i18nc("velocity unit symbol", "m/s"),
      i18nc("unit description in lists", "meters per second"),
      i18nc("unit synonyms for matching user input", "meter per second;meters per second;m/s;ms"),
      ki18nc("amount in units (real)", "%1 meters per second"),
      ki18ncp("amount in units (integer)", "%1 meter per second", "%1 meters per second")
    ));
    U(KilometerPerHour, 0.277778,
      i18nc("velocity unit symbol", "km/h"),
      i18nc("unit description in lists", "kilometers per hour"),
      i18nc("unit synonyms for matching user input",
            "kilometer per hour;kilometers per hour;km/h;kmh"),
      ki18nc("amount in units (real)", "%1 kilometers per hour"),
      ki18ncp("amount in units (integer)", "%1 kilometer per hour", "%1 kilometers per hour")
    );
    U(MilePerHour, 0.44704,
      i18nc("velocity unit symbol", "mph"),
      i18nc("unit description in lists", "miles per hour"),
      i18nc("unit synonyms for matching user input", "mile per hour;miles per hour;mph"),
      ki18nc("amount in units (real)", "%1 miles per hour"),
      ki18ncp("amount in units (integer)", "%1 mile per hour", "%1 miles per hour")
    );
    U(FootPerSecond, 0.3048,
      i18nc("velocity unit symbol", "ft/s"),
      i18nc("unit description in lists", "feet per second"),
      i18nc("unit synonyms for matching user input",
            "foot per second;feet per second;ft/s;ft/sec;fps"),
      ki18nc("amount in units (real)", "%1 feet per second"),
      ki18ncp("amount in units (integer)", "%1 foot per second", "%1 feet per second")
    );
    U(InchPerSecond, 0.0254,
      i18nc("velocity unit symbol", "in/s"),
      i18nc("unit description in lists", "inches per second"),
      i18nc("unit synonyms for matching user input",
            "inch per second;inches per second;in/s;in/sec;ips"),
      ki18nc("amount in units (real)", "%1 inches per second"),
      ki18ncp("amount in units (integer)", "%1 inch per second", "%1 inches per second")
    );
    U(Knot, 0.514444,
      i18nc("velocity unit symbol", "kt"),
      i18nc("unit description in lists", "knots"),
      i18nc("unit synonyms for matching user input", "knot;knots;kt;nautical miles per hour"),
      ki18nc("amount in units (real)", "%1 knots"),
      ki18ncp("amount in units (integer)", "%1 knot", "%1 knots")
    );
    // http://en.wikipedia.org/wiki/Speed_of_sound
    U(Mach, 343,
      i18nc("velocity unit symbol", "Ma"),
      i18nc("unit description in lists", "Mach"),
      i18nc("unit synonyms for matching user input", "mach;machs;Ma;speed of sound"),
      ki18nc("amount in units (real)", "Mach %1"),
      ki18ncp("amount in units (integer)", "Mach %1", "Mach %1")
    );
    U(SpeedOfLight, 2.99792458e+08,
      i18nc("velocity unit symbol", "c"),
      i18nc("unit description in lists", "speed of light"),
      i18nc("unit synonyms for matching user input", "speed of light;c"),
      ki18nc("amount in units (real)", "%1 speed of light"),
      ki18ncp("amount in units (integer)", "%1 speed of light", "%1 speed of light")
    );
    // http://en.wikipedia.org/wiki/Beaufort_scale
    U(Beaufort, new BeaufortConv(),
      i18nc("velocity unit symbol", "bft"),
      i18nc("unit description in lists", "Beaufort"),
      i18nc("unit synonyms for matching user input", "Beaufort;Bft"),
      ki18nc("amount in units (real)", "%1 on the Beaufort scale"),
      ki18ncp("amount in units (integer)", "%1 on the Beaufort scale", "%1 on the Beaufort scale")
    );

    setMostCommonUnits(QList<int>() <<
            MeterPerSecond << KilometerPerHour << MilePerHour << Knot << Mach);
}
