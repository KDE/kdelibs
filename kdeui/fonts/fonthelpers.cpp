/*
    Requires the Qt widget libraries, available at no cost at
    http://www.troll.no

    Copyright (C) 2008 Chusslove Illich <caslav.ilic@gmx.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "fonthelpers_p.h"

#include "klocale.h"

#ifdef NEVERDEFINE // never true
// Font names up for translation, listed for extraction.

// i18n: Generic sans serif font presented in font choosers. When selected,
// the system will choose a real font, mandated by distro settings.
I18N_NOOP2("@item Font name", "Sans Serif")
// i18n: Generic serif font presented in font choosers. When selected,
// the system will choose a real font, mandated by distro settings.
I18N_NOOP2("@item Font name", "Serif")
// i18n: Generic monospace font presented in font choosers. When selected,
// the system will choose a real font, mandated by distro settings.
I18N_NOOP2("@item Font name", "Monospace")

#endif

void splitFontString (const QString &name, QString *family, QString *foundry)
{
    int p1 = name.indexOf('[');
    if (p1 < 0) {
        if (family) {
            *family = name.trimmed();
        }
        if (foundry) {
            foundry->clear();
        }
    } else {
        int p2 = name.indexOf(']', p1);
        p2 = p2 > p1 ? p2 : name.length();
        if (family) {
            *family = name.left(p1).trimmed();
        }
        if (foundry) {
            *foundry = name.mid(p1 + 1, p2 - p1 - 1).trimmed();
        }
    }
}

QString translateFontName (const QString &name)
{
    QString family, foundry;
    splitFontString(name, &family, &foundry);

    // Obtain any regular translations for the family and foundry.
    QString trFamily = i18nc("@item Font name", family.toUtf8());
    QString trFoundry = foundry;
    if (!foundry.isEmpty()) {
        trFoundry = i18nc("@item Font foundry", foundry.toUtf8());
    }

    // Assemble full translation.
    QString trfont;
    if (foundry.isEmpty()) {
        // i18n: Filter by which the translators can translate, or otherwise
        // operate on the font names not put up for regular translation.
        trfont = i18nc("@item Font name", "%1", trFamily);
    } else {
        // i18n: Filter by which the translators can translate, or otherwise
        // operate on the font names not put up for regular translation.
        trfont = i18nc("@item Font name [foundry]", "%1 [%2]",
                       trFamily, trFoundry);
    }
    return trfont;
}

static bool localeLessThan (const QString &a, const QString &b)
{
    return QString::localeAwareCompare(a, b) < 0;
}

QStringList translateFontNameList (const QStringList &names,
                                   QHash<QString, QString> *trToRawNames)
{
    // Generic fonts, in the inverse of desired order.
    QStringList genericNames;
    genericNames.append("Monospace");
    genericNames.append("Serif");
    genericNames.append("Sans Serif");

    // Translate fonts, but do not add generics to the list right away.
    QStringList trNames;
    QHash<QString, QString> trMap;
    foreach (const QString &name, names) {
        QString trName = translateFontName(name);
        if (!genericNames.contains(name)) {
            trNames.append(trName);
        }
        trMap.insert(trName, name);
    }

    // Sort real fonts alphabetically.
    qSort(trNames.begin(), trNames.end(), localeLessThan);

    // Prepend generic fonts, in the predefined order.
    foreach (const QString &genericName, genericNames) {
        QString trGenericName = translateFontName(genericName);
        if (trMap.contains(trGenericName)) {
            trNames.prepend(trGenericName);
        }
    }

    if (trToRawNames) {
        *trToRawNames = trMap;
    }
    return trNames;
}
