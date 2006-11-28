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

#include <qevent.h>
#include <qkeysequence.h>
#include <qstring.h>
#include <qstringlist.h>

#include "kdebug.h"
#include "kglobal.h"
#include "klocale.h"
#include "ksimpleconfig.h"


class KShortcutPrivate
{
public:
    inline KShortcutPrivate()
    {
    }

    inline KShortcutPrivate(const QKeySequence &ks1)
     : primary(ks1)
    {
    }

    inline KShortcutPrivate(const QKeySequence &ks1, const QKeySequence &ks2)
     : primary(ks1),
       alternate(ks2)
    {
    }

    QKeySequence primary;
    QKeySequence alternate;
};


KShortcut::KShortcut()
 : d(new KShortcutPrivate())
{
}

KShortcut::KShortcut(const QKeySequence &primary)
 : d(new KShortcutPrivate(primary))
{
}

KShortcut::KShortcut(const QKeySequence &primary, const QKeySequence &alternate)
 : d(new KShortcutPrivate(primary, alternate))
{
}

KShortcut::KShortcut(int keyQtPri, int keyQtAlt)
 : d(new KShortcutPrivate(keyQtPri, keyQtAlt))
{
}

KShortcut::KShortcut(const KShortcut &other)
 : d(new KShortcutPrivate(other.d->primary, other.d->alternate))
{
}

KShortcut::KShortcut(const QString &s)
 : d(new KShortcutPrivate())
{
    if (s == "none")
        return;

    QStringList sCuts = s.split(';');
    if (sCuts.count() > 2)
        kWarning() << "KShortcut: asked to store more than two key sequences but can only hold two."
        <<endl;

    for( int i=0; i < sCuts.count(); i++)
        if( sCuts[i].startsWith( "default(" ) )
            sCuts[i] = sCuts[i].mid( 8, sCuts[i].length() - 9 );

    if (sCuts.count())
        d->primary = QKeySequence::fromString(sCuts[0]);
        if (sCuts.count() > 1)
            d->alternate = QKeySequence::fromString(sCuts[1]);
}

KShortcut::~KShortcut()
{
    delete d;
}

const QKeySequence &KShortcut::primary() const
{
    return d->primary;
}

const QKeySequence &KShortcut::alternate() const
{
    return d->alternate;
}

void KShortcut::setPrimary(const QKeySequence &newPrimary)
{
    d->primary = newPrimary;
}

void KShortcut::setAlternate(const QKeySequence &newAlternate)
{
    d->alternate = newAlternate;
}

void KShortcut::remove(const QKeySequence &other)
{
    if (d->alternate == other)
        d->alternate = QKeySequence();
    if (d->primary == other) {
        if (!d->alternate.isEmpty()) {
            d->primary = d->alternate;
            d->alternate = QKeySequence();
        } else
            d->primary = QKeySequence();
    }
}

void KShortcut::clear()
{
    QKeySequence empty;
    d->primary = empty;
    d->alternate = empty;
}

bool KShortcut::isEmpty() const
{
    return d->primary.isEmpty() && d->alternate.isEmpty();
}

bool KShortcut::contains( const QKeySequence &other ) const
{
    return d->primary == other || d->alternate == other;
}

QString KShortcut::toString() const
{
    if (!d->primary.isEmpty()) {
        if (!d->alternate.isEmpty())
            return d->primary.toString() + ";" + d->alternate.toString();
        else
            return d->primary.toString();
    } else
        return d->alternate.toString();
}

//dummy
//TODO: is this really needed? If yes, it takes an argument like the old version did.
//This is because this function without any argument is equivalent to toString().
//The old version did it that way.
QString KShortcut::toStringInternal() const
{
    return toString();
}

QList<QKeySequence> KShortcut::toList() const
{
    QList<QKeySequence> list;
    if (!d->primary.isEmpty())
        list.append(d->primary);
    if (!d->alternate.isEmpty())
        list.append(d->alternate);
    return list;
}

KShortcut &KShortcut::operator=(const KShortcut &other)
{
    d->primary = other.d->primary;
    d->alternate = other.d->alternate;
    return *this;
}

bool KShortcut::operator==(const KShortcut &other) const
{
    return d->primary == other.d->primary && d->alternate == other.d->alternate;
}
