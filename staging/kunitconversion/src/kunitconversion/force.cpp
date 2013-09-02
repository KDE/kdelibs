/*
 *   Copyright (C) 2010 Petri Damstén <damu@iki.fi>
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

#include "force.h"
#include "converter.h"
#include <klocalizedstring.h>

using namespace KUnitConversion;

Force::Force() : UnitCategory(ForceCategory)
{
    setName(i18n("Force"));
    setSymbolStringFormat(ki18nc("%1 value, %2 unit symbol (force", "%1 %2"));

    U(Yottanewton, 1e+24,
      i18nc("force unit symbol", "YN"),
      i18nc("unit description in lists", "yottanewtons"),
      i18nc("unit synonyms for matching user input", "yottanewton;yottanewtons;YN"),
      ki18nc("amount in units (real)", "%1 yottanewtons"),
      ki18ncp("amount in units (integer)", "%1 yottanewton", "%1 yottanewtons")
    );
    U(Zettanewton, 1e+21,
      i18nc("force unit symbol", "ZN"),
      i18nc("unit description in lists", "zettanewtons"),
      i18nc("unit synonyms for matching user input", "zettanewton;zettanewtons;ZN"),
      ki18nc("amount in units (real)", "%1 zettanewtons"),
      ki18ncp("amount in units (integer)", "%1 zettanewton", "%1 zettanewtons")
    );
    U(Exanewton, 1e+18,
      i18nc("force unit symbol", "EN"),
      i18nc("unit description in lists", "exanewtons"),
      i18nc("unit synonyms for matching user input", "exanewton;exanewtons;EN"),
      ki18nc("amount in units (real)", "%1 exanewtons"),
      ki18ncp("amount in units (integer)", "%1 exanewton", "%1 exanewtons")
    );
    U(Petanewton, 1e+15,
      i18nc("force unit symbol", "PN"),
      i18nc("unit description in lists", "petanewtons"),
      i18nc("unit synonyms for matching user input", "petanewton;petanewtons;PN"),
      ki18nc("amount in units (real)", "%1 petanewtons"),
      ki18ncp("amount in units (integer)", "%1 petanewton", "%1 petanewtons")
    );
    U(Teranewton, 1e+12,
      i18nc("force unit symbol", "TN"),
      i18nc("unit description in lists", "teranewtons"),
      i18nc("unit synonyms for matching user input", "teranewton;teranewtons;TN"),
      ki18nc("amount in units (real)", "%1 teranewtons"),
      ki18ncp("amount in units (integer)", "%1 teranewton", "%1 teranewtons")
    );
    U(Giganewton, 1e+09,
      i18nc("force unit symbol", "GN"),
      i18nc("unit description in lists", "giganewtons"),
      i18nc("unit synonyms for matching user input", "giganewton;giganewtons;GN"),
      ki18nc("amount in units (real)", "%1 giganewtons"),
      ki18ncp("amount in units (integer)", "%1 giganewton", "%1 giganewtons")
    );
    U(Meganewton, 1e+06,
      i18nc("force unit symbol", "MN"),
      i18nc("unit description in lists", "meganewtons"),
      i18nc("unit synonyms for matching user input", "meganewton;meganewtons;MN"),
      ki18nc("amount in units (real)", "%1 meganewtons"),
      ki18ncp("amount in units (integer)", "%1 meganewton", "%1 meganewtons")
    );
    U(KilonewtonForce, 1000,
      i18nc("force unit symbol", "kN"),
      i18nc("unit description in lists", "kilonewtons"),
      i18nc("unit synonyms for matching user input", "kilonewton;kilonewtons;kN"),
      ki18nc("amount in units (real)", "%1 kilonewtons"),
      ki18ncp("amount in units (integer)", "%1 kilonewton", "%1 kilonewtons")
    );
    U(Hectonewton, 100,
      i18nc("force unit symbol", "hN"),
      i18nc("unit description in lists", "hectonewtons"),
      i18nc("unit synonyms for matching user input", "hectonewton;hectonewtons;hN"),
      ki18nc("amount in units (real)", "%1 hectonewtons"),
      ki18ncp("amount in units (integer)", "%1 hectonewton", "%1 hectonewtons")
    );
    U(Decanewton, 10,
      i18nc("force unit symbol", "daN"),
      i18nc("unit description in lists", "decanewtons"),
      i18nc("unit synonyms for matching user input", "decanewton;decanewtons;daN"),
      ki18nc("amount in units (real)", "%1 decanewtons"),
      ki18ncp("amount in units (integer)", "%1 decanewton", "%1 decanewtons")
    );
    setDefaultUnit(UP(Newton, 1,
      i18nc("force unit symbol", "N"),
      i18nc("unit description in lists", "newtons"),
      i18nc("unit synonyms for matching user input", "newton;newtons;N"),
      ki18nc("amount in units (real)", "%1 newtons"),
      ki18ncp("amount in units (integer)", "%1 newton", "%1 newtons")
    ));
    U(Decinewton, 0.1,
      i18nc("force unit symbol", "dN"),
      i18nc("unit description in lists", "decinewtons"),
      i18nc("unit synonyms for matching user input", "decinewton;decinewtons;dN"),
      ki18nc("amount in units (real)", "%1 decinewtons"),
      ki18ncp("amount in units (integer)", "%1 decinewton", "%1 decinewtons")
    );
    U(Centinewton, 0.01,
      i18nc("force unit symbol", "cN"),
      i18nc("unit description in lists", "centinewtons"),
      i18nc("unit synonyms for matching user input", "centinewton;centinewtons;cN"),
      ki18nc("amount in units (real)", "%1 centinewtons"),
      ki18ncp("amount in units (integer)", "%1 centinewton", "%1 centinewtons")
    );
    U(Millinewton, 0.001,
      i18nc("force unit symbol", "mN"),
      i18nc("unit description in lists", "millinewtons"),
      i18nc("unit synonyms for matching user input", "millinewton;millinewtons;mN"),
      ki18nc("amount in units (real)", "%1 millinewtons"),
      ki18ncp("amount in units (integer)", "%1 millinewton", "%1 millinewtons")
    );
    U(Micronewton, 1e-06,
      i18nc("force unit symbol", "µN"),
      i18nc("unit description in lists", "micronewtons"),
      i18nc("unit synonyms for matching user input", "micronewton;micronewtons;µm;uN"),
      ki18nc("amount in units (real)", "%1 micronewtons"),
      ki18ncp("amount in units (integer)", "%1 micronewton", "%1 micronewtons")
    );
    U(Nanonewton, 1e-09,
      i18nc("force unit symbol", "nN"),
      i18nc("unit description in lists", "nanonewtons"),
      i18nc("unit synonyms for matching user input", "nanonewton;nanonewtons;nN"),
      ki18nc("amount in units (real)", "%1 nanonewtons"),
      ki18ncp("amount in units (integer)", "%1 nanonewton", "%1 nanonewtons")
    );
    U(Piconewton, 1e-12,
      i18nc("force unit symbol", "pN"),
      i18nc("unit description in lists", "piconewtons"),
      i18nc("unit synonyms for matching user input", "piconewton;piconewtons;pN"),
      ki18nc("amount in units (real)", "%1 piconewtons"),
      ki18ncp("amount in units (integer)", "%1 piconewton", "%1 piconewtons")
    );
    U(Femtonewton, 1e-15,
      i18nc("force unit symbol", "fN"),
      i18nc("unit description in lists", "femtonewtons"),
      i18nc("unit synonyms for matching user input", "femtonewton;femtonewtons;fN"),
      ki18nc("amount in units (real)", "%1 femtonewtons"),
      ki18ncp("amount in units (integer)", "%1 femtonewton", "%1 femtonewtons")
    );
    U(Attonewton, 1e-18,
      i18nc("force unit symbol", "aN"),
      i18nc("unit description in lists", "attonewtons"),
      i18nc("unit synonyms for matching user input", "attonewton;attonewtons;aN"),
      ki18nc("amount in units (real)", "%1 attonewtons"),
      ki18ncp("amount in units (integer)", "%1 attonewton", "%1 attonewtons")
    );
    U(Zeptonewton, 1e-21,
      i18nc("force unit symbol", "zN"),
      i18nc("unit description in lists", "zeptonewtons"),
      i18nc("unit synonyms for matching user input", "zeptonewton;zeptonewtons;zN"),
      ki18nc("amount in units (real)", "%1 zeptonewtons"),
      ki18ncp("amount in units (integer)", "%1 zeptonewton", "%1 zeptonewtons")
    );
    U(Yoctonewton, 1e-24,
      i18nc("force unit symbol", "yN"),
      i18nc("unit description in lists", "yoctonewtons"),
      i18nc("unit synonyms for matching user input", "yoctonewton;yoctonewtons;yN"),
      ki18nc("amount in units (real)", "%1 yoctonewtons"),
      ki18ncp("amount in units (integer)", "%1 yoctonewton", "%1 yoctonewtons")
    );

    // http://en.wikipedia.org/wiki/Force#Units_of_measurement
    U(Dyne, 0.0001,
      i18nc("force unit symbol", "dyn"),
      i18nc("unit description in lists", "dynes"),
      i18nc("unit synonyms for matching user input", "dyne;dynes;dyn"),
      ki18nc("amount in units (real)", "%1 dynes"),
      ki18ncp("amount in units (integer)", "%1 dyne", "%1 dynes")
    );
    U(Kilopond, 9.80665,
      i18nc("force unit symbol", "kp"),
      i18nc("unit description in lists", "kiloponds"),
      i18nc("unit synonyms for matching user input", "kilogram-force;kilopond;kiloponds;kp"),
      ki18nc("amount in units (real)", "%1 kiloponds"),
      ki18ncp("amount in units (integer)", "%1 kilopond", "%1 kiloponds")
    );
    U(PoundForce, 4.448222,
      i18nc("force unit symbol", "lbf"),
      i18nc("unit description in lists", "pound-force"),
      i18nc("unit synonyms for matching user input", "pound-force;lbf"),
      ki18nc("amount in units (real)", "%1 pound-force"),
      ki18ncp("amount in units (integer)", "%1 pound-force", "%1 pound-force")
    );
    U(Poundal, 0.138255,
      i18nc("force unit symbol", "pdl"),
      i18nc("unit description in lists", "poundals"),
      i18nc("unit synonyms for matching user input", "poundal;poundals;pdl"),
      ki18nc("amount in units (real)", "%1 poundals"),
      ki18ncp("amount in units (integer)", "%1 poundal", "%1 poundals")
    );

    setMostCommonUnits(QList<int>() << KilonewtonForce << Newton);
}

