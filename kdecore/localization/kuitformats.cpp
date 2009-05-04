/*  This file is part of the KDE libraries
    Copyright (C) 2007 Chusslove Illich <caslav.ilic@gmx.net>

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

#include <kuitformats_p.h>

#include <config.h>
#include <kglobal.h>
#include <klocale.h>

#include <QStringList>
#include <QRegExp>

#include <kdebug.h>

QString KuitFormats::toKeyCombo (const QString &shstr, const QString &delim,
                                 const QHash<QString, QString> &keydict)
{
    static QRegExp delRx("[+-]");

    int p = delRx.indexIn(shstr); // find delimiter

    QStringList keys;
    if (p < 0) { // single-key shortcut, no delimiter found
        keys.append(shstr);
    }
    else { // multi-key shortcut
        QChar oldDelim = shstr[p];
        keys = shstr.split(oldDelim, QString::SkipEmptyParts);
    }

    for (int i = 0; i < keys.size(); ++i) {
        // Normalize key, trim and all lower-case.
        QString nkey = keys[i].trimmed().toLower();
        bool isFunctionKey = nkey.length() > 1 && nkey[1].isDigit();
        if (!isFunctionKey) {
            keys[i] = keydict.contains(nkey) ? keydict[nkey] : keys[i].trimmed();
        }
        else {
            keys[i] = keydict["f%1"].arg(nkey.mid(1));
        }
    }
    return keys.join(delim);
}

QString KuitFormats::toInterfacePath (const QString &inpstr,
                                      const QString &delim)
{
    static QRegExp delRx("\\||->");

    int p = delRx.indexIn(inpstr); // find delimiter
    if (p < 0) { // single-element path, no delimiter found
        return inpstr;
    }
    else { // multi-element path
        QString oldDelim = delRx.capturedTexts().at(0);
        QStringList guiels = inpstr.split(oldDelim, QString::SkipEmptyParts);
        return guiels.join(delim);
    }
}

