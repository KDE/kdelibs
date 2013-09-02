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

#include "frequency.h"
#include "converter.h"
#include <klocalizedstring.h>

using namespace KUnitConversion;

Frequency::Frequency() : UnitCategory(FrequencyCategory)
{
    setName(i18n("Frequency"));
    setSymbolStringFormat(ki18nc("%1 value, %2 unit symbol (frequency", "%1 %2"));

    U(Yottahertz, 1e+24,
      i18nc("frequency unit symbol", "YHz"),
      i18nc("unit description in lists", "yottahertzs"),
      i18nc("unit synonyms for matching user input", "yottahertz;yottahertzs;YHz"),
      ki18nc("amount in units (real)", "%1 yottahertzs"),
      ki18ncp("amount in units (integer)", "%1 yottahertz", "%1 yottahertzs")
    );
    U(Zettahertz, 1e+21,
      i18nc("frequency unit symbol", "ZHz"),
      i18nc("unit description in lists", "zettahertzs"),
      i18nc("unit synonyms for matching user input", "zettahertz;zettahertzs;ZHz"),
      ki18nc("amount in units (real)", "%1 zettahertzs"),
      ki18ncp("amount in units (integer)", "%1 zettahertz", "%1 zettahertzs")
    );
    U(Exahertz, 1e+18,
      i18nc("frequency unit symbol", "EHz"),
      i18nc("unit description in lists", "exahertzs"),
      i18nc("unit synonyms for matching user input", "exahertz;exahertzs;EHz"),
      ki18nc("amount in units (real)", "%1 exahertzs"),
      ki18ncp("amount in units (integer)", "%1 exahertz", "%1 exahertzs")
    );
    U(Petahertz, 1e+15,
      i18nc("frequency unit symbol", "PHz"),
      i18nc("unit description in lists", "petahertzs"),
      i18nc("unit synonyms for matching user input", "petahertz;petahertzs;PHz"),
      ki18nc("amount in units (real)", "%1 petahertzs"),
      ki18ncp("amount in units (integer)", "%1 petahertz", "%1 petahertzs")
    );
    U(Terahertz, 1e+12,
      i18nc("frequency unit symbol", "THz"),
      i18nc("unit description in lists", "terahertzs"),
      i18nc("unit synonyms for matching user input", "terahertz;terahertzs;THz"),
      ki18nc("amount in units (real)", "%1 terahertzs"),
      ki18ncp("amount in units (integer)", "%1 terahertz", "%1 terahertzs")
    );
    U(Gigahertz, 1e+09,
      i18nc("frequency unit symbol", "GHz"),
      i18nc("unit description in lists", "gigahertzs"),
      i18nc("unit synonyms for matching user input", "gigahertz;gigahertzs;GHz"),
      ki18nc("amount in units (real)", "%1 gigahertzs"),
      ki18ncp("amount in units (integer)", "%1 gigahertz", "%1 gigahertzs")
    );
    U(Megahertz, 1e+06,
      i18nc("frequency unit symbol", "MHz"),
      i18nc("unit description in lists", "megahertzs"),
      i18nc("unit synonyms for matching user input", "megahertz;megahertzs;MHz"),
      ki18nc("amount in units (real)", "%1 megahertzs"),
      ki18ncp("amount in units (integer)", "%1 megahertz", "%1 megahertzs")
    );
    U(Kilohertz, 1000,
      i18nc("frequency unit symbol", "kHz"),
      i18nc("unit description in lists", "kilohertzs"),
      i18nc("unit synonyms for matching user input", "kilohertz;kilohertzs;kHz"),
      ki18nc("amount in units (real)", "%1 kilohertzs"),
      ki18ncp("amount in units (integer)", "%1 kilohertz", "%1 kilohertzs")
    );
    U(Hectohertz, 100,
      i18nc("frequency unit symbol", "hHz"),
      i18nc("unit description in lists", "hectohertzs"),
      i18nc("unit synonyms for matching user input", "hectohertz;hectohertzs;hHz"),
      ki18nc("amount in units (real)", "%1 hectohertzs"),
      ki18ncp("amount in units (integer)", "%1 hectohertz", "%1 hectohertzs")
    );
    U(Decahertz, 10,
      i18nc("frequency unit symbol", "daHz"),
      i18nc("unit description in lists", "decahertzs"),
      i18nc("unit synonyms for matching user input", "decahertz;decahertzs;daHz"),
      ki18nc("amount in units (real)", "%1 decahertzs"),
      ki18ncp("amount in units (integer)", "%1 decahertz", "%1 decahertzs")
    );
    setDefaultUnit(UP(Hertz, 1,
      i18nc("frequency unit symbol", "Hz"),
      i18nc("unit description in lists", "hertzs"),
      i18nc("unit synonyms for matching user input", "hertz;hertzs;Hz"),
      ki18nc("amount in units (real)", "%1 hertzs"),
      ki18ncp("amount in units (integer)", "%1 hertz", "%1 hertzs")
    ));
    U(Decihertz, 0.1,
      i18nc("frequency unit symbol", "dHz"),
      i18nc("unit description in lists", "decihertzs"),
      i18nc("unit synonyms for matching user input", "decihertz;decihertzs;dHz"),
      ki18nc("amount in units (real)", "%1 decihertzs"),
      ki18ncp("amount in units (integer)", "%1 decihertz", "%1 decihertzs")
    );
    U(Centihertz, 0.01,
      i18nc("frequency unit symbol", "cHz"),
      i18nc("unit description in lists", "centihertzs"),
      i18nc("unit synonyms for matching user input", "centihertz;centihertzs;cHz"),
      ki18nc("amount in units (real)", "%1 centihertzs"),
      ki18ncp("amount in units (integer)", "%1 centihertz", "%1 centihertzs")
    );
    U(Millihertz, 0.001,
      i18nc("frequency unit symbol", "mHz"),
      i18nc("unit description in lists", "millihertzs"),
      i18nc("unit synonyms for matching user input", "millihertz;millihertzs;mHz"),
      ki18nc("amount in units (real)", "%1 millihertzs"),
      ki18ncp("amount in units (integer)", "%1 millihertz", "%1 millihertzs")
    );
    U(Microhertz, 1e-06,
      i18nc("frequency unit symbol", "µHz"),
      i18nc("unit description in lists", "microhertzs"),
      i18nc("unit synonyms for matching user input", "microhertz;microhertzs;µHz;uHz"),
      ki18nc("amount in units (real)", "%1 microhertzs"),
      ki18ncp("amount in units (integer)", "%1 microhertz", "%1 microhertzs")
    );
    U(Nanohertz, 1e-09,
      i18nc("frequency unit symbol", "nHz"),
      i18nc("unit description in lists", "nanohertzs"),
      i18nc("unit synonyms for matching user input", "nanohertz;nanohertzs;nHz"),
      ki18nc("amount in units (real)", "%1 nanohertzs"),
      ki18ncp("amount in units (integer)", "%1 nanohertz", "%1 nanohertzs")
    );
    U(Picohertz, 1e-12,
      i18nc("frequency unit symbol", "pHz"),
      i18nc("unit description in lists", "picohertzs"),
      i18nc("unit synonyms for matching user input", "picohertz;picohertzs;pHz"),
      ki18nc("amount in units (real)", "%1 picohertzs"),
      ki18ncp("amount in units (integer)", "%1 picohertz", "%1 picohertzs")
    );
    U(Femtohertz, 1e-15,
      i18nc("frequency unit symbol", "fHz"),
      i18nc("unit description in lists", "femtohertzs"),
      i18nc("unit synonyms for matching user input", "femtohertz;femtohertzs;fHz"),
      ki18nc("amount in units (real)", "%1 femtohertzs"),
      ki18ncp("amount in units (integer)", "%1 femtohertz", "%1 femtohertzs")
    );
    U(Attohertz, 1e-18,
      i18nc("frequency unit symbol", "aHz"),
      i18nc("unit description in lists", "attohertzs"),
      i18nc("unit synonyms for matching user input", "attohertz;attohertzs;aHz"),
      ki18nc("amount in units (real)", "%1 attohertzs"),
      ki18ncp("amount in units (integer)", "%1 attohertz", "%1 attohertzs")
    );
    U(Zeptohertz, 1e-21,
      i18nc("frequency unit symbol", "zHz"),
      i18nc("unit description in lists", "zeptohertzs"),
      i18nc("unit synonyms for matching user input", "zeptohertz;zeptohertzs;zHz"),
      ki18nc("amount in units (real)", "%1 zeptohertzs"),
      ki18ncp("amount in units (integer)", "%1 zeptohertz", "%1 zeptohertzs")
    );
    U(Yoctohertz, 1e-24,
      i18nc("frequency unit symbol", "yHz"),
      i18nc("unit description in lists", "yoctohertzs"),
      i18nc("unit synonyms for matching user input", "yoctohertz;yoctohertzs;yHz"),
      ki18nc("amount in units (real)", "%1 yoctohertzs"),
      ki18ncp("amount in units (integer)", "%1 yoctohertz", "%1 yoctohertzs")
    );
    U(RPM, 1.0 / 60.0,
      i18nc("frequency unit symbol", "RPM"),
      i18nc("unit description in lists", "revolutions per minute"),
      i18nc("unit synonyms for matching user input",
            "revolutions per minute;revolution per minute;RPM"),
      ki18nc("amount in units (real)", "%1 revolutions per minute"),
      ki18ncp("amount in units (integer)", "%1 revolution per minute", "%1 revolutions per minute")
    );

    setMostCommonUnits(QList<int>() <<
            Kilohertz << Megahertz << Gigahertz << Hertz << RPM);
}

