/*  This file is part of the KDE libraries
    Copyright (C) 2001,2002 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>
    Copyright (C) 2006 Andreas Hartmetz <ahartmetz@gmail.com>

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

#include "kshortcut.h"

#include <QtGui/QActionEvent>
#include <QtGui/QKeySequence>
#include <QtCore/QCharRef>
#include <QtCore/QMutableStringListIterator>

#include "kdebug.h"
#include "kglobal.h"
#include "klocale.h"


KShortcut::KShortcut()
{
}

KShortcut::KShortcut(const QKeySequence &primary)
{
    if (!primary.isEmpty())
        append(primary);
}

KShortcut::KShortcut(const QKeySequence &primary, const QKeySequence &alternate)
{
    if (!primary.isEmpty())
        append(primary);
    if (!alternate.isEmpty())
        append(alternate);
}

KShortcut::KShortcut(int keyQtPri, int keyQtAlt)
{
    if (keyQtPri)
        append(keyQtPri);
    if (keyQtAlt)
        append(keyQtAlt);
}

KShortcut::KShortcut(const QString &s)
{
    if (s == QLatin1String("none"))
        return;

    QStringList sCuts = s.split(';');
    if (sCuts.count() > 2)
        kWarning() << "KShortcut: asked to store more than two key sequences but can only hold two."
        <<endl;

    for( int i=0; i < sCuts.count(); i++)
        if( sCuts[i].startsWith( "default(" ) )
            sCuts[i] = sCuts[i].mid( 8, sCuts[i].length() - 9 );

    for (int i = 0; i < sCuts.count(); ++i)
        append(QKeySequence::fromString(sCuts.at(i)));
}

KShortcut::KShortcut(const QList<QKeySequence> &seqs)
    : QList<QKeySequence>(seqs)
{
}

void KShortcut::setPrimary(const QKeySequence &newPrimary)
{
    while (count() < 1)
        append(QKeySequence());
    operator[](0) = newPrimary;
}

void KShortcut::setAlternate(const QKeySequence &newAlternate)
{
    while (count() < 2)
        append(QKeySequence());
    operator[](1) = newAlternate;
}

QString KShortcut::toString() const
{
    QString result;
    for (int i = 0; i < count(); ++i) {
        if (i > 0)
            result += QLatin1Char(';');
        result += at(i).toString();
    }
    return result;
}

