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

#ifndef KUITFORMATS_P_H
#define KUITFORMATS_P_H

#include <QString>
#include <QHash>

namespace KuitFormats {
    /**
     * Reformat keyboard shortcut. The first of encountered of '+' or '-'
     * is taken for key delimiter, and the supplied delimiter is used instead.
     * A dictionary of replacement key names can also be provided, which can
     * be used for localization of the key names.
     *
     * @param shstr raw shortcut string
     * @param delim replacement delimiter for keys
     * @param keydict replacement dictionary of key names
     * @returns shortcut in proper format
     */
    QString toKeyCombo (const QString &shstr, const QString &delim,
                        const QHash<QString, QString> &keydict);

    /**
     * Reformat GUI element path. Consider the first encountered of
     * '/', '|' or "->" as path delimiter, and replace them with the supplied
     * delimiter.
     *
     * @param inpstr raw interface path string
     * @param delim replacement path delimiter
     * @returns path in proper format
     */
    QString toInterfacePath (const QString &inpstr, const QString &delim);
}

#endif
