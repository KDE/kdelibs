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

#ifndef KUNITCONVERSION_UNITCATEGORY_H
#define KUNITCONVERSION_UNITCATEGORY_H

#include "value.h"
#include "unit.h"
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <kurl.h>
#include "kunitconversion_export.h"

namespace KUnitConversion
{

class KUNITCONVERSION_EXPORT UnitCategory
{
public:
    UnitCategory(int id);
    virtual ~UnitCategory();

    /**
     * Returns name for the unit category.
     *
     * @return Translated name for category.
     **/
    QString name() const;

    /**
     * @return unit category description
     **/
    QString description() const;

    /**
     * @return unit category url for description
     **/
    KUrl url() const;

    /**
     * Returns default unit.
     *
     * @return default unit.
     **/
    UnitPtr defaultUnit() const;

    /**
     * Check if unit category has a unit.
     *
     * @return True if unit is found
     **/
    bool hasUnit(const QString &unit) const;

    /**
     * Return unit for string.
     *
     * @return Pointer to unit class.
     **/
    UnitPtr unit(const QString& s) const;

    /**
    * Return unit for unit enum.
    *
    * @return Pointer to unit class.
    **/
    UnitPtr unit(int unitId) const;

    /**
     * Return units in this category.
     *
     * @return list of units.
     **/
    QList<UnitPtr> units() const;

    /**
     * Return all unit names, short names and unit synonyms in this category.
     *
     * @return list of units.
     **/
    QStringList allUnits() const;

    /**
     * Convert value to another unit.
     *
     * @param value value to convert
     * @param toUnit unit to convert to. If empty default unit is used.
     * @return converted value
     **/
    Value convert(const Value& value, const QString& toUnit = QString());
    Value convert(const Value& value, int toUnit);
    virtual Value convert(const Value& value, UnitPtr toUnit);

    /**
     * @return category id.
     **/
    int id() const;

protected:
    void setName(const QString& name);
    void setDefaultUnit(UnitPtr defaultUnit);
    void addUnitName(const QString& name);
    void addUnitMapValues(UnitPtr unit, const QString& names);
    void addIdMapValue(UnitPtr unit, int id);
    void setDescription(const QString& desc);
    void setUrl(const KUrl& url);
    void setSymbolStringFormat(const KLocalizedString& symbolStringFormat);
    KLocalizedString symbolStringFormat() const;

private:
    friend class Unit;
    class Private;
    Private* const d;
};

} // KUnitConversion namespace

#endif
