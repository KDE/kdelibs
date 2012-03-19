/*
 *   Copyright (C) 2009 Petri Damstén <damu@iki.fi>
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

#include "energy.h"
#include "converter.h"
#include <klocalizedstring.h>

using namespace KUnitConversion;

class PhotonWavelengthConv : public Complex
{
    double toDefault(double value) const { return (2.99792458e+08 * 6.62606896e-34) / (value * 1e-09); };
    double fromDefault(double value) const { return ((2.99792458e+08 * 6.62606896e-34) / value) / 1e-09; };
};

Energy::Energy() : UnitCategory(EnergyCategory)
{
    setName(i18n("Energy"));
    setSymbolStringFormat(ki18nc("%1 value, %2 unit symbol (energy)", "%1 %2"));

    U(Yottajoule, 1e+24,
      i18nc("energy unit symbol", "YJ"),
      i18nc("unit description in lists", "yottajoules"),
      i18nc("unit synonyms for matching user input", "yottajoule;yottajoules;YJ"),
      ki18nc("amount in units (real)", "%1 yottajoules"),
      ki18ncp("amount in units (integer)", "%1 yottajoule", "%1 yottajoules")
    );
    U(Zettajoule, 1e+21,
      i18nc("energy unit symbol", "ZJ"),
      i18nc("unit description in lists", "zettajoules"),
      i18nc("unit synonyms for matching user input", "zettajoule;zettajoules;ZJ"),
      ki18nc("amount in units (real)", "%1 zettajoules"),
      ki18ncp("amount in units (integer)", "%1 zettajoule", "%1 zettajoules")
    );
    U(Exajoule, 1e+18,
      i18nc("energy unit symbol", "EJ"),
      i18nc("unit description in lists", "exajoules"),
      i18nc("unit synonyms for matching user input", "exajoule;exajoules;EJ"),
      ki18nc("amount in units (real)", "%1 exajoules"),
      ki18ncp("amount in units (integer)", "%1 exajoule", "%1 exajoules")
    );
    U(Petajoule, 1e+15,
      i18nc("energy unit symbol", "PJ"),
      i18nc("unit description in lists", "petajoules"),
      i18nc("unit synonyms for matching user input", "petajoule;petajoules;PJ"),
      ki18nc("amount in units (real)", "%1 petajoules"),
      ki18ncp("amount in units (integer)", "%1 petajoule", "%1 petajoules")
    );
    U(Terajoule, 1e+12,
      i18nc("energy unit symbol", "TJ"),
      i18nc("unit description in lists", "terajoules"),
      i18nc("unit synonyms for matching user input", "terajoule;terajoules;TJ"),
      ki18nc("amount in units (real)", "%1 terajoules"),
      ki18ncp("amount in units (integer)", "%1 terajoule", "%1 terajoules")
    );
    U(Gigajoule, 1e+09,
      i18nc("energy unit symbol", "GJ"),
      i18nc("unit description in lists", "gigajoules"),
      i18nc("unit synonyms for matching user input", "gigajoule;gigajoules;GJ"),
      ki18nc("amount in units (real)", "%1 gigajoules"),
      ki18ncp("amount in units (integer)", "%1 gigajoule", "%1 gigajoules")
    );
    U(Megajoule, 1e+06,
      i18nc("energy unit symbol", "MJ"),
      i18nc("unit description in lists", "megajoules"),
      i18nc("unit synonyms for matching user input", "megajoule;megajoules;MJ"),
      ki18nc("amount in units (real)", "%1 megajoules"),
      ki18ncp("amount in units (integer)", "%1 megajoule", "%1 megajoules")
    );
    U(Kilojoule, 1000,
      i18nc("energy unit symbol", "kJ"),
      i18nc("unit description in lists", "kilojoules"),
      i18nc("unit synonyms for matching user input", "kilojoule;kilojoules;kJ"),
      ki18nc("amount in units (real)", "%1 kilojoules"),
      ki18ncp("amount in units (integer)", "%1 kilojoule", "%1 kilojoules")
    );
    U(Hectojoule, 100,
      i18nc("energy unit symbol", "hJ"),
      i18nc("unit description in lists", "hectojoules"),
      i18nc("unit synonyms for matching user input", "hectojoule;hectojoules;hJ"),
      ki18nc("amount in units (real)", "%1 hectojoules"),
      ki18ncp("amount in units (integer)", "%1 hectojoule", "%1 hectojoules")
    );
    U(Decajoule, 10,
      i18nc("energy unit symbol", "daJ"),
      i18nc("unit description in lists", "decajoules"),
      i18nc("unit synonyms for matching user input", "decajoule;decajoules;daJ"),
      ki18nc("amount in units (real)", "%1 decajoules"),
      ki18ncp("amount in units (integer)", "%1 decajoule", "%1 decajoules")
    );
    setDefaultUnit(UP(Joule, 1,
      i18nc("energy unit symbol", "J"),
      i18nc("unit description in lists", "joules"),
      i18nc("unit synonyms for matching user input", "joule;joules;J"),
      ki18nc("amount in units (real)", "%1 joules"),
      ki18ncp("amount in units (integer)", "%1 joule", "%1 joules")
    ));
    U(Decijoule, 0.1,
      i18nc("energy unit symbol", "dJ"),
      i18nc("unit description in lists", "decijoules"),
      i18nc("unit synonyms for matching user input", "decijoule;decijoules;dJ"),
      ki18nc("amount in units (real)", "%1 decijoules"),
      ki18ncp("amount in units (integer)", "%1 decijoule", "%1 decijoules")
    );
    U(Centijoule, 0.01,
      i18nc("energy unit symbol", "cJ"),
      i18nc("unit description in lists", "centijoules"),
      i18nc("unit synonyms for matching user input", "centijoule;centijoules;cJ"),
      ki18nc("amount in units (real)", "%1 centijoules"),
      ki18ncp("amount in units (integer)", "%1 centijoule", "%1 centijoules")
    );
    U(Millijoule, 0.001,
      i18nc("energy unit symbol", "mJ"),
      i18nc("unit description in lists", "millijoules"),
      i18nc("unit synonyms for matching user input", "millijoule;millijoules;mJ"),
      ki18nc("amount in units (real)", "%1 millijoules"),
      ki18ncp("amount in units (integer)", "%1 millijoule", "%1 millijoules")
    );
    U(Microjoule, 1e-06,
      i18nc("energy unit symbol", "µJ"),
      i18nc("unit description in lists", "microjoules"),
      i18nc("unit synonyms for matching user input", "microjoule;microjoules;µJ;uJ"),
      ki18nc("amount in units (real)", "%1 microjoules"),
      ki18ncp("amount in units (integer)", "%1 microjoule", "%1 microjoules")
    );
    U(Nanojoule, 1e-09,
      i18nc("energy unit symbol", "nJ"),
      i18nc("unit description in lists", "nanojoules"),
      i18nc("unit synonyms for matching user input", "nanojoule;nanojoules;nJ"),
      ki18nc("amount in units (real)", "%1 nanojoules"),
      ki18ncp("amount in units (integer)", "%1 nanojoule", "%1 nanojoules")
    );
    U(Picojoule, 1e-12,
      i18nc("energy unit symbol", "pJ"),
      i18nc("unit description in lists", "picojoules"),
      i18nc("unit synonyms for matching user input", "picojoule;picojoules;pJ"),
      ki18nc("amount in units (real)", "%1 picojoules"),
      ki18ncp("amount in units (integer)", "%1 picojoule", "%1 picojoules")
    );
    U(Femtojoule, 1e-15,
      i18nc("energy unit symbol", "fJ"),
      i18nc("unit description in lists", "femtojoules"),
      i18nc("unit synonyms for matching user input", "femtojoule;femtojoules;fJ"),
      ki18nc("amount in units (real)", "%1 femtojoules"),
      ki18ncp("amount in units (integer)", "%1 femtojoule", "%1 femtojoules")
    );
    U(Attojoule, 1e-18,
      i18nc("energy unit symbol", "aJ"),
      i18nc("unit description in lists", "attojoules"),
      i18nc("unit synonyms for matching user input", "attojoule;attojoules;aJ"),
      ki18nc("amount in units (real)", "%1 attojoules"),
      ki18ncp("amount in units (integer)", "%1 attojoule", "%1 attojoules")
    );
    U(Zeptojoule, 1e-21,
      i18nc("energy unit symbol", "zJ"),
      i18nc("unit description in lists", "zeptojoules"),
      i18nc("unit synonyms for matching user input", "zeptojoule;zeptojoules;zJ"),
      ki18nc("amount in units (real)", "%1 zeptojoules"),
      ki18ncp("amount in units (integer)", "%1 zeptojoule", "%1 zeptojoules")
    );
    U(Yoctojoule, 1e-24,
      i18nc("energy unit symbol", "yJ"),
      i18nc("unit description in lists", "yoctojoules"),
      i18nc("unit synonyms for matching user input", "yoctojoule;yoctojoules;yJ"),
      ki18nc("amount in units (real)", "%1 yoctojoules"),
      ki18ncp("amount in units (integer)", "%1 yoctojoule", "%1 yoctojoules")
    );
    U(GuidelineDailyAmount, 8.3736e+06,
      i18nc("energy unit symbol", "GDA"),
      i18nc("unit description in lists", "guideline daily amount"),
      i18nc("unit synonyms for matching user input",
            "guideline daily amount;guideline daily amount;GDA"),
      ki18nc("amount in units (real)", "%1 guideline daily amount"),
      ki18ncp("amount in units (integer)", "%1 guideline daily amount", "%1 guideline daily amount")
    );
    U(Electronvolt, 1.60218e-19,
      i18nc("energy unit symbol", "eV"),
      i18nc("unit description in lists", "electronvolts"),
      i18nc("unit synonyms for matching user input", "electronvolt;electronvolts;eV"),
      ki18nc("amount in units (real)", "%1 electronvolts"),
      ki18ncp("amount in units (integer)", "%1 electronvolt", "%1 electronvolts")
    );
    U(JoulePerMole, 1.66054238581e-24,
      i18nc("energy unit symbol", "J/mol"),
      i18nc("unit description in lists", "joule per mole"),
      i18nc("unit synonyms for matching user input", "joule per mole;joulepermole;joulemol;jmol;j/mol"),
      ki18nc("amount in units (real)", "%1 joules per mole"),
      ki18ncp("amount in units (integer)", "%1 joule per mole", "%1 joules per mole")
    );
    U(KiloJoulePerMole, 1.66054238581e-21,
      i18nc("energy unit symbol", "kJ/mol"),
      i18nc("unit description in lists", "kilojoule per mole"),
      i18nc("unit synonyms for matching user input", "kilojoule per mole;kilojoulepermole;kilojoule per mole;kilojoulemol;kjmol;kj/mol"),
      ki18nc("amount in units (real)", "%1 kilojoules per mole"),
      ki18ncp("amount in units (integer)", "%1 kilojoule per mole", "%1 kilojoules per mole")
    );
    U(Rydberg, 2.17987e-18,
      i18nc("energy unit symbol", "Ry"),
      i18nc("unit description in lists", "rydbergs"),
      i18nc("unit synonyms for matching user input", "rydberg;rydbergs;Ry"),
      ki18nc("amount in units (real)", "%1 rydbergs"),
      ki18ncp("amount in units (integer)", "%1 rydberg", "%1 rydbergs")
    );
    U(Kilocalorie, 4186.8,
      i18nc("energy unit symbol", "kcal"),
      i18nc("unit description in lists", "kilocalories"),
      i18nc("unit synonyms for matching user input", "kilocalorie;kilocalories;kcal"),
      ki18nc("amount in units (real)", "%1 kilocalories"),
      ki18ncp("amount in units (integer)", "%1 kilocalorie", "%1 kilocalories")
    );
    U(PhotonWavelength, new PhotonWavelengthConv(),
      i18nc("energy unit symbol", "nm"),
      i18nc("unit description in lists", "photon wavelength in nanometers"),
      i18nc("unit synonyms for matching user input", "nm;photon wavelength"),
      ki18nc("amount in units (real)", "%1 nanometers"),
      ki18ncp("amount in units (integer)", "%1 nanometer", "%1 nanometers")
    );

    setMostCommonUnits(QList<int>() <<
            Joule << Kilojoule << Kilocalorie << GuidelineDailyAmount);
}
