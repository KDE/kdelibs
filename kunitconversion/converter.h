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

#ifndef KUNITCONVERSION_CONVERTER_H
#define KUNITCONVERSION_CONVERTER_H

#include <kunitconversion/value.h>
#include <kunitconversion/kunitconversion_export.h>
#include <QtCore/QVariant>

namespace KUnitConversion
{
enum CategoryId {
    InvalidCategory = -1, LengthCategory, AreaCategory, VolumeCategory, TemperatureCategory,
    VelocityCategory, MassCategory, PressureCategory, EnergyCategory, CurrencyCategory,
    PowerCategory, TimeCategory, FuelEfficiencyCategory, DensityCategory
};
enum UnitId {
    InvalidUnit = -1, NoUnit = 0, Percent = 1,
    // Area
    SquareYottameter = 1000, SquareZettameter, SquareExameter, SquarePetameter, SquareTerameter,
    SquareGigameter, SquareMegameter, SquareKilometer, SquareHectometer, SquareDecameter,
    SquareMeter, SquareDecimeter, SquareCentimeter, SquareMillimeter, SquareMicrometer,
    SquareNanometer, SquarePicometer, SquareFemtometer, SquareAttometer, SquareZeptometer,
    SquareYoctometer, Acre, SquareFoot, SquareInch, SquareMile,

    // Length
    Yottameter = 2000, Zettameter, Exameter, Petameter, Terameter, Gigameter, Megameter,
    Kilometer, Hectometer, Decameter, Meter, Decimeter, Centimeter, Millimeter, Micrometer,
    Nanometer, Picometer, Femtometer, Attometer, Zeptometer, Yoctometer, Inch, Foot, Yard,
    Mile, NauticalMile, LightYear, Parsec, AstronomicalUnit,

    // Volume
    CubicYottameter = 3000, CubicZettameter, CubicExameter, CubicPetameter, CubicTerameter,
    CubicGigameter, CubicMegameter, CubicKilometer, CubicHectometer, CubicDecameter, CubicMeter,
    CubicDecimeter, CubicCentimeter, CubicMillimeter, CubicMicrometer, CubicNanometer,
    CubicPicometer, CubicFemtometer, CubicAttometer, CubicZeptometer, CubicYoctometer,
    Yottaliter, Zettaliter, Exaliter, Petaliter, Teraliter, Gigaliter, Megaliter, Kiloliter,
    Hectoliter, Decaliter, Liter, Deciliter, Centiliter, Milliliter, Microliter, Nanoliter,
    Picoliter, Femtoliter, Attoliter, Zeptoliter, Yoctoliter, CubicFoot, CubicInch, CubicMile,
    FluidOunce, Cup, GallonUS, PintImperial,

    // Mass
    Yottagram = 4000, Zettagram, Exagram, Petagram, Teragram, Gigagram, Megagram, Kilogram,
    Hectogram, Decagram, Gram, Decigram, Centigram, Milligram, Microgram, Nanogram, Picogram,
    Femtogram, Attogram, Zeptogram, Yoctogram, Ton, Carat, Pound, Ounce, TroyOunce, MassNewton,
    Kilonewton,

    // Pressure
    Yottapascal = 5000, Zettapascal, Exapascal, Petapascal, Terapascal, Gigapascal, Megapascal,
    Kilopascal, Hectopascal, Decapascal, Pascal, Decipascal, Centipascal, Millipascal,
    Micropascal, Nanopascal, Picopascal, Femtopascal, Attopascal, Zeptopascal, Yoctopascal,
    Bar, Millibar, Decibar, Torr, TechnicalAtmosphere, Atmosphere, PoundForcePerSquareInch,
    InchesOfMercury,

    // Temperature
    Kelvin = 6000, Celsius, Fahrenheit, Rankine, Delisle, TemperatureNewton, Reaumur, Romer,

    // Energy
    Yottajoule = 7000, Zettajoule, Exajoule, Petajoule, Terajoule, Gigajoule, Megajoule,
    Kilojoule, Hectojoule, Decajoule, Joule, Decijoule, Centijoule, Millijoule, Microjoule,
    Nanojoule, Picojoule, Femtojoule, Attojoule, Zeptojoule, Yoctojoule, GuidelineDailyAmount,
    Electronvolt, Rydberg, Kilocalorie,

    // Currency
    Eur = 8000, Ats, Bef, Nlg, Fim, Frf, Dem, Iep, Itl, Luf, Pte, Esp, Grd, Sit, Cyp, Mtl, Skk,
    Usd, Jpy, Bgn, Czk, Dkk, Eek, Gbp, Huf, Ltl, Lvl, Pln, Ron, Sek, Chf, Nok, Hrk, Rub, Try,
    Aud, Brl, Cad, Cny, Hkd, Idr, Inr, Krw, Mxn, Myr, Nzd, Php, Sgd, Thb, Zar,

    // Velocity
    MeterPerSecond = 9000, KilometerPerHour, MilePerHour, FootPerSecond, InchPerSecond, Knot,
    Mach, SpeedOfLight, Beaufort,

    // Power
    Yottawatt = 10000, Zettawatt, Exawatt, Petawatt, Terawatt, Gigawatt, Megawatt, Kilowatt,
    Hectowatt, Decawatt, Watt, Deciwatt, Centiwatt, Milliwatt, Microwatt, Nanowatt, Picowatt,
    Femtowatt, Attowatt, Zeptowatt, Yoctowatt, Horsepower,

    // Time
    Yottasecond = 11000, Zettasecond, Exasecond, Petasecond, Terasecond, Gigasecond, Megasecond,
    Kilosecond, Hectosecond, Decasecond, Second, Decisecond, Centisecond, Millisecond,
    Microsecond, Nanosecond, Picosecond, Femtosecond, Attosecond, Zeptosecond, Yoctosecond,
    Minute, Hour, Day, Week, JulianYear, LeapYear, Year,

    // FuelEfficiency
    LitersPer100Kilometers = 12000, MilePerUsGallon, MilePerImperialGallon, KilometrePerLitre,

    // Density
    YottakilogramsPerCubicMeter = 13000, ZettakilogramPerCubicMeter, ExakilogramPerCubicMeter,
    PetakilogramPerCubicMeter, TerakilogramPerCubicMeter, GigakilogramPerCubicMeter,
    MegakilogramPerCubicMeter, KilokilogramPerCubicMeter, HectokilogramsPerCubicMeter,
    DecakilogramsPerCubicMeter, KilogramsPerCubicMeter, DecikilogramsPerCubicMeter,
    CentikilogramsPerCubicMeter, MillikilogramsPerCubicMeter, MicrokilogramsPerCubicMeter,
    NanokilogramsPerCubicMeter, PicokilogramsPerCubicMeter, FemtokilogramsPerCubicMeter,
    AttokilogramsPerCubicMeter, ZeptokilogramsPerCubicMeter, YoctokilogramsPerCubicMeter,
    KilogramPerLiter, GramPerLiter, GramPerMilliliter,
    OuncePerCubicInch, OuncePerCubicFoot, OuncePerCubicYard,
    PoundPerCubicInch, PoundPerCubicFoot, PoundPerCubicYard
};

class UnitCategory;

class ConverterPrivate;
class KUNITCONVERSION_EXPORT Converter : public QObject
{
    Q_OBJECT
public:
    explicit Converter(QObject* parent = 0);
    ~Converter();

    /**
     * Convert value to another unit.
     *
     * @param value value to convert
     * @param toUnit unit to convert to. If empty default unit is used.
     * @return converted value
     **/
    Value convert(const Value& value, const QString& toUnit = QString()) const;
    Value convert(const Value& value, int toUnit) const;
    Value convert(const Value& value, UnitPtr toUnit) const;

    /**
     * Find unit category for unit.
     *
     * @param unit unit to find category for.
     * @return unit category for unit
     **/
    UnitCategory* categoryForUnit(const QString& unit) const;

    /**
     * Find unit for string unit.
     *
     * @param unitString unit string to find unit for.
     * @return unit for string unit
     **/
    UnitPtr unit(const QString& unitString) const;

    /**
    * Find unit for unit enum.
    *
    * @param unit unit enum to find unit for.
    * @return unit for string unit
    **/
    UnitPtr unit(int unitId) const;

    /**
     * Find unit category.
     *
     * @param category name of the category to find (length, area, mass, etc.).
     * @return unit category named category or invalid category.
     **/
    UnitCategory* category(const QString& category) const;

    /**
     * Find unit category.
     *
     * @param categoryId id of the category to find (LengthCategory, AreaCategory, etc.).
     * @return unit category which id is categoryId or invalid category.
     **/
    UnitCategory* category(int categoryId) const;

    /**
     * Returns a list of all unit categories.
     *
     * @return list of unit categories.
     **/
    QList<UnitCategory*> categories() const;

private:
    ConverterPrivate *d;
};

} // KUnitConversion namespace

#endif
