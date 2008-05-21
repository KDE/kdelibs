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
    QString trfont;
    if (foundry.isEmpty()) {
        // i18n: Filtering message, so that translators can translate the
        // font names on their own should they want. May be replaced in
        // the future with conventional messages in a PO file.
        trfont = i18nc("@item Font name", "%1", family);
    } else {
        trfont = i18nc("@item Font name [foundry]", "%1 [%2]", family, foundry);
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
    QStringList trnames;
    QHash<QString, QString> trmap;
    foreach (const QString &name, names) {
        QString trname = translateFontName(name);
        trnames.append(trname);
        trmap.insert(trname, name);
    }
    qSort(trnames.begin(), trnames.end(), localeLessThan);

    if (trToRawNames) {
        *trToRawNames = trmap;
    }
    return trnames;
}
