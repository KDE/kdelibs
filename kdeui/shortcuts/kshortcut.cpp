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


class KShortcutPrivate
{
public:
    KShortcutPrivate() {}

    QKeySequence primary;
    QKeySequence alternate;
};


KShortcut::KShortcut()
 : d(new KShortcutPrivate)
{
    qRegisterMetaType<KShortcut>();
}

KShortcut::KShortcut(const QKeySequence &primary)
 : d(new KShortcutPrivate)
{
    qRegisterMetaType<KShortcut>();
    d->primary = primary;
}

KShortcut::KShortcut(const QKeySequence &primary, const QKeySequence &alternate)
 : d(new KShortcutPrivate)
{
    qRegisterMetaType<KShortcut>();
    d->primary = primary;
    d->alternate = alternate;
}

KShortcut::KShortcut(int keyQtPri, int keyQtAlt)
 : d(new KShortcutPrivate)
{
    qRegisterMetaType<KShortcut>();
    d->primary = keyQtPri;
    d->alternate = keyQtAlt;
}

KShortcut::KShortcut(const KShortcut &other)
 : d(new KShortcutPrivate)
{
    d->primary = other.d->primary;
    d->alternate = other.d->alternate;
}

KShortcut::KShortcut(const QList<QKeySequence> &seqs)
 : d(new KShortcutPrivate)
{
    qRegisterMetaType<KShortcut>();
    if (seqs.count() >= 1)
        d->primary = seqs.at(0);
    if (seqs.count() >= 2)
        d->alternate = seqs.at(1);
}

KShortcut::KShortcut(const QString &s)
 : d(new KShortcutPrivate)
{
    qRegisterMetaType<KShortcut>();
    if (s == QLatin1String("none"))
        return;

    QStringList sCuts = s.split("; ");
    if (sCuts.count() > 2)
        kWarning() << "asked to store more than two key sequences but can only hold two.";

    //TODO: what is the "(default)" thingie used for?
    for( int i=0; i < sCuts.count(); i++)
        if( sCuts[i].startsWith( QLatin1String("default(") ) )
            sCuts[i] = sCuts[i].mid( 8, sCuts[i].length() - 9 );

    if (sCuts.count() >= 1) {
        QString k = sCuts.at(0);
        k.replace( "Win+", "Meta+" ); // workaround for KDE3-style shortcuts
        k.replace("Plus", "+"); // workaround for KDE3-style "Alt+Plus"
        k.replace("Minus", "-"); // workaround for KDE3-style "Alt+Plus"
        d->primary = QKeySequence::fromString(k);
        // Complain about a unusable shortcuts sequence only if we have got
        // something.
        if (d->primary.isEmpty() && !k.isEmpty()) {
            kDebug(240) << "unusable primary shortcut sequence " << sCuts[0];
        }
    }

    if (sCuts.count() >= 2) {
        QString k = sCuts.at(1);
        k.replace( "Win+", "Meta+" ); // workaround for KDE3-style shortcuts
        d->alternate = QKeySequence::fromString(k);
        if (d->alternate.isEmpty()) {
            kDebug(240) << "unusable alternate shortcut sequence " << sCuts[1];
        }
    }
}

KShortcut::~KShortcut()
{
    delete d;
}

QKeySequence KShortcut::primary() const
{
    return d->primary;
}

QKeySequence KShortcut::alternate() const
{
    return d->alternate;
}

bool KShortcut::isEmpty() const
{
    return d->primary.isEmpty() && d->alternate.isEmpty();
}

bool KShortcut::contains(const QKeySequence &needle) const
{
    if (needle.isEmpty())
        return false;
    return d->primary == needle || d->alternate == needle;
}

bool KShortcut::conflictsWith(const QKeySequence &needle) const
{
    if (needle.isEmpty())
        return false;

    bool primaryConflicts = false;
    bool alternateConflicts = false;

    if (!d->primary.isEmpty()) {
        primaryConflicts =
            (    d->primary.matches(needle) == QKeySequence::NoMatch
              && needle.matches(d->primary) == QKeySequence::NoMatch )
            ? false
            : true;
    }

    if (!d->alternate.isEmpty()) {
        alternateConflicts=
            (    d->alternate.matches(needle) == QKeySequence::NoMatch
              && needle.matches(d->alternate) == QKeySequence::NoMatch )
            ? false
            : true;
    }

    return primaryConflicts || alternateConflicts;
}


void KShortcut::setPrimary(const QKeySequence &newPrimary)
{
    d->primary = newPrimary;
}

void KShortcut::setAlternate(const QKeySequence &newAlternate)
{
    d->alternate = newAlternate;
}

void KShortcut::remove(const QKeySequence &keySeq, enum EmptyHandling handleEmpty)
{
    if (keySeq.isEmpty())
        return;

    if (d->primary == keySeq) {
        if (handleEmpty == KeepEmpty)
            d->primary = QKeySequence();
        else {
            d->primary = d->alternate;
            d->alternate = QKeySequence();
        }
    }
    if (d->alternate == keySeq)
        d->alternate = QKeySequence();
}

KShortcut &KShortcut::operator=(const KShortcut &other)
{
    d->primary = other.d->primary;
    d->alternate = other.d->alternate;
    return (*this);
}

bool KShortcut::operator==(const KShortcut &other) const
{
    return (d->primary == other.d->primary && d->alternate == other.d->alternate);
}

bool KShortcut::operator!=(const KShortcut &other) const
{
    return !operator==(other);
}

KShortcut::operator QList<QKeySequence>() const
{
    return toList(RemoveEmpty);
}

QList<QKeySequence> KShortcut::toList(enum EmptyHandling handleEmpty) const
{
    QList<QKeySequence> ret;
    if (handleEmpty == RemoveEmpty) {
        if (!d->primary.isEmpty())
            ret.append(d->primary);
        if (!d->alternate.isEmpty())
            ret.append(d->alternate);
    } else {
        ret.append(d->primary);
        ret.append(d->alternate);
    }

    return ret;
}

QString KShortcut::toString() const
{
    return toString(QKeySequence::PortableText);
}

QString KShortcut::toString(QKeySequence::SequenceFormat format) const
{
    QString ret;
    foreach(const QKeySequence &seq, toList()) {
        ret.append(seq.toString(format));
        ret.append("; ");
    }
    ret.chop(2);
    return ret;
}

KShortcut::operator QVariant() const
{
    return qVariantFromValue(*this);
}
